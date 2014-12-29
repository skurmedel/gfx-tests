/*
	blur.c

	Simple bokeh blur demo program.

	Requires:

		pthreads
		unistd.h

	Compile with:

		gcc --std=c11 blur.c

	Known Bugs:

		- Kernel size [0, 1) gives a black image.
*/

/*     
	The zlib/libpng License

	Copyright (c) 2014 Simon Otter

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you
	must not claim that you wrote the original software. If you use
	this software in a product, an acknowledgment in the product
	documentation would be appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and
	must not be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
	distribution. 
*/


#include <stdio.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

#define STBI_NO_HDR
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define LOGF(typ, format, args...)  printf("% 4s: "  format, (typ), args)
#define LOG(typ, str) LOGF(typ, "%s", str)

typedef unsigned char byte;

/*
	Holds the command line arguments for the program. Note that this structure is not valid until cmd_args_init(...) has been called.
*/
struct cmd_args_t
{
	char **args;
	int argc;

	float kernel_size;
	char *input_file;
	char *output_file;
} cmd_args;

/*
	Process the command line arguments and fill in the global cmd_args 
	structure.

	May exit the software.

	NOT THREAD SAFE.
*/
void cmd_args_init(int argc, char **argv);

struct image_descr_t
{
	unsigned int const w;
	unsigned int const h;
	unsigned int const pixel_stride;
	unsigned int const row_stride;
	byte * const data;
};

struct image_descr_t image_descr_rgb8(
	unsigned int w,
	unsigned int h,
	byte * const data);
unsigned int image_descr_pixel_index(
	struct image_descr_t const *img,
	unsigned int x,
	unsigned int y);

struct filter_kernel_t
{
	/*
		Width of the kernel array.
		The kernel is square, this means it contains
		width^2 items.
	*/
	byte width;
	/*
		Offset in discrete units to center of kernel.
	*/
	byte center;
	/*
		A flat array representing a kernel.
	*/
	float *values;
};

int   kernel_init(
	struct filter_kernel_t *, 
	float radius);
float kernel_sample(
	struct filter_kernel_t const * const, 
	byte x,
	byte y);
void  kernel_free(
	struct filter_kernel_t *);

struct blur_task_t
{
	/* 
		The position of the upper left corner
		of this block of pixels.
	*/
	unsigned int x;
	unsigned int y;
	/*
		Pointer to output array and input array.
		This simplifies processing a lot but costs memory.
	*/
	struct image_descr_t *in;
	struct image_descr_t *out;
	/*
		Width and height of block of pixels to process.
	*/
	byte w;
	byte h;
};

struct context_t
{
	struct filter_kernel_t kernel;
	struct blur_task_t *tasks;
	pthread_mutex_t task_mutex;
	unsigned int current_task;
	unsigned int task_count;
} ctx = {{0, 0, NULL}, NULL, PTHREAD_MUTEX_INITIALIZER, 1, 0};

void *blur_worker(void *);

int main(int argc, char *argv[])
{
	cmd_args_init(argc, argv);

	ctx.tasks = NULL;

	int w, h, layers;
	byte *pixels = stbi_load(cmd_args.input_file, &w, &h, &layers, 3);
	if (pixels == NULL)
	{
		LOG("ARG", "Image load error:");
		puts(stbi_failure_reason());
		return 1;
	}
	if (w < 64 || h < 64)
	{
		LOG("ARG", "Sorry, image size must be at least 64x64.");
		return 2;
	}
	if (w > 16320 || h > 16320)
	{
		LOG("ARG", "Sorry, image size too large.");
		return 2;
	}

	/*
		Allocate out data array.
	*/
	byte *outpixels = malloc(w * h * 3 * sizeof(byte));
	if (outpixels == NULL)
	{
		LOG("MEM", "Sorry, couldn't allocate memory for result pixels.\n");
	}

	unsigned short tasks_x = w / 64;
	unsigned short tasks_y = h / 64;
	unsigned short last_task_x = w % 64;
	unsigned short last_task_y = h % 64;

	if (last_task_x != 0)
		tasks_x += 1;
	if (last_task_y != 0)
		tasks_y += 1;

	ctx.tasks = malloc(tasks_x * tasks_y * sizeof(struct blur_task_t));
	if (ctx.tasks == NULL)
	{
		LOG("MEM", "Couldn't allocate memory. Good bye.");
		stbi_image_free(pixels);
		return 3;
	}

	LOGF("INFO", "%u tasks allocated!\n", tasks_x * tasks_y);
	LOGF("MEM", "%u x %u x %u = %u bytes used by tasks\n", tasks_x, tasks_y, sizeof(struct blur_task_t), tasks_x * tasks_y * sizeof(struct blur_task_t));

	struct image_descr_t in  = image_descr_rgb8(w, h, pixels);
	struct image_descr_t out = image_descr_rgb8(w, h, outpixels);

	/*
		Create our task list by dividing up our image into buckets.
		Each bucket is a task which will be processed by the worker
		threads.
	*/
	unsigned int index = 0;
	unsigned int start_y = 0;
	for (int y = 0; y < tasks_y; y++)
	{
		unsigned int start_x = 0;
		byte task_h = 64;
		if (y == tasks_y - 1 && last_task_y > 0)
			task_h = last_task_y; 

		for (int x = 0; x < tasks_x; x++)
		{
			byte task_w = 64;
			if (x == tasks_x - 1 && last_task_x > 0)
				task_w = last_task_x;
			unsigned int task_stride = (y * tasks_x) + x; 
			ctx.tasks[task_stride].x = start_x;
			ctx.tasks[task_stride].y = start_y;
			ctx.tasks[task_stride].w = task_w;
			ctx.tasks[task_stride].h = task_h;
			ctx.tasks[task_stride].in  = &in;
			ctx.tasks[task_stride].out = &out;
			start_x += (x == tasks_x - 1 && last_task_x != 0)? last_task_x : task_w;
		}
		start_y += (y == tasks_y - 1 && last_task_y != 0)? last_task_y : task_h;
	}

	ctx.current_task = 0;
	ctx.task_count = tasks_y * tasks_x;

	if (!kernel_init(&ctx.kernel, cmd_args.kernel_size))
	{
		LOG("ERR", "Could not init kernel.\n");
		return 4;
	}


	LOG("TASK", "Launching task workers.\n");
	pthread_t t1, t2, t3, t4;
	/*
		Set up worker threads.
	*/
	pthread_create(&t1, NULL, blur_worker, ctx.tasks);
	pthread_create(&t2, NULL, blur_worker, ctx.tasks);
	pthread_create(&t3, NULL, blur_worker, ctx.tasks);
	pthread_create(&t4, NULL, blur_worker, ctx.tasks);

	/* 
		Wait for workers to end processing.
	*/
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	pthread_join(t4, NULL);

	/*
		Cleanup mutex, pixels and then tasks.
		Probably best in that order.
	*/
	pthread_mutex_destroy(&ctx.task_mutex);

	/*
		Todo: Check errors.
	*/
	stbi_write_png(cmd_args.output_file, w, h, 3, outpixels, 0);

	LOG("MEM", "Freeing pixels.\n");
	stbi_image_free(pixels);
	free(outpixels);
	LOG("MEM", "Freeing tasks.\n");
	free(ctx.tasks);
	LOG("MEM", "Freeing kernel.\n");
	kernel_free(&ctx.kernel);

	puts("Good bye.");
}

void *blur_worker(void *arg_raw)
{
	struct blur_task_t *task_list = (struct blur_task_t *) arg_raw;
	struct blur_task_t task;
	pthread_t me = pthread_self();
	/*
		Worker loop.
	*/
	while (1)
	{
		/*
			Acquire new task, which requires mutex acquisition.
		*/
		pthread_mutex_lock(  &ctx.task_mutex);
		if (ctx.current_task < ctx.task_count)
		{
			task = task_list[ctx.current_task];
			ctx.current_task += 1;
		}
		else
		{
			/*
				No more tasks to process, so we can exit and die.
			*/
			pthread_mutex_unlock(&ctx.task_mutex);
			return NULL;			
		}
		pthread_mutex_unlock(&ctx.task_mutex);

		/*
			Do work on task.
		*/
		int pprocessed = 0;
		for (int y = 0; y < task.h; ++y)
		{
			for (int x = 0; x < task.w; ++x)
			{				
				float r = 0.0f, g = 0.0f, b = 0.0f;
				for (int j = 0; j < ctx.kernel.width; ++j)
				{
					for (int i = 0; i < ctx.kernel.width; ++i)
					{
						unsigned int offset = 
							image_descr_pixel_index(task.in, task.x + x - ctx.kernel.center + i, task.y + y - ctx.kernel.center + j);
						byte const * const p = task.in->data + offset;

						r += ((float) p[0]) * ctx.kernel.values[j * ctx.kernel.width + i];
						g += ((float) p[1]) * ctx.kernel.values[j * ctx.kernel.width + i];
						b += ((float) p[2]) * ctx.kernel.values[j * ctx.kernel.width + i];

					}
				}
				unsigned int offset = 
					image_descr_pixel_index(task.in, task.x + x, task.y + y);

				*(task.out->data + offset    ) = ((byte) (r > 254.5? 255.0 : r));
				*(task.out->data + offset + 1) = ((byte) (g > 254.5? 255.0 : g));
				*(task.out->data + offset + 2) = ((byte) (b > 254.5? 255.0 : b));

				pprocessed++;
				// printf("%x%x%x  ", p[0], p[1], p[2]);
			}
		}
		// LOGF("WORK", "(%d): %d x %d @ (%u, %u).\n", me, task.w, task.h, task.x, task.y);
		// LOGF("WORK", "(%d): %d pixels read.\n", me, pprocessed);
	}
	return NULL;
}

int   kernel_init(
	struct filter_kernel_t *k, 
	float radius)
{
	if (radius < 1.0001f || radius > 126.9999f)
	{
		LOG("ARG", "A radius bigger than 127 or less than 1 is not supported.\n");
		return 0;
	}

	k->width = (byte) radius * 2;
	float fwidth = k->width;
	k->values = malloc(k->width * k->width * sizeof(float));
	if (k->values == NULL)
	{
		LOG("MEM", "Could not allocate kernel.\n");
		return 0;
	}

	/*
		We create our kernel by evaluating the distance function to the edges
		of a sphere of radius 1. To get some smoothness we need to pad the 
		range of x-values we evaluate somewhat. A diameter of 3.0 gives
		us a pretty smooth transition, and it doesn't truncate the edges.
	*/
	float const step_x  = 3.0f / ((float) k->width - 1);

	/*
		Totals for the kernel, we use it to normalise the kernel later.
	*/
	float total = 0.0;
	for (int y = 0; y < k->width; ++y)
	{
		for (int x = 0; x < k->width; ++x)
		{
			/*
				Map our x and y values into the [-1.5, 1.5] range.
			*/
			float evx = step_x * x - 1.5f;
			float evy = step_x * y - 1.5f;
			/*
				Calculate the distance from a point to a circle of radius one,
				centered at the origin.
			*/
			float v = -fabs(evx * evx + evy * evy - 1.0f) + 1.0f;
			v = v < 0.0f? 0.0f : v;
			/*
				Kill negative values, or we'll get a difference kernel.
			*/
			k->values[y * k->width + x] = v;
			total += v;
		}
	}

	/*
		Normalise kernel.
	*/
	for (int i = 0; i < (k->width * k->width); ++i)
	{
		k->values[i] = k->values[i] / total;
	}
	
	k->center = k->width / 2; /* Truncation may leave this zero...  */
	if (k->center == 0)       /* which calls for a conditional ceil */
	{			
		k->center = 1;
	}

	return 1;
}

float kernel_sample(
	struct filter_kernel_t const * const k, 
	byte x,
	byte y)
{
	if (x > k->width || y > k->width || x < 0 || y < 0)
	{
		LOG("ERR", "Tried to sample out of bounds.");
		return NAN;
	}

	return k->values[k->width * y + x];
}

void  kernel_free(
	struct filter_kernel_t *k)
{
	free(k->values);
	k->values = NULL;
}

struct image_descr_t image_descr_rgb8(
	unsigned int w,
	unsigned int h,
	byte * const data)
{
	assert( !(w < 1 || h < 1) );
	struct image_descr_t descr =
	{
		/* width */	 		w,
		/* height */	 	h,
		/* pixel_stride */	3 * sizeof(byte),
		/* row_stride */	w * 3 * sizeof(byte),
		/* data */	 		data
	};
	return descr;
};

unsigned int image_descr_pixel_index(
	struct image_descr_t const *img,
	unsigned int x,
	unsigned int y)
{
	/*
		FIXME: Handle overflows.
	*/
	unsigned int ax = (x < 0 || x > img->w - 1? (img->w - x) % img->w : x);
	unsigned int ay = (y < 0 || y > img->h - 1? (img->h - y) % img->h : y);

	return img->row_stride   * (ay)
	     + img->pixel_stride * (ax);
}

void cmd_args_init(int argc, char **argv)
{
	if (argc < 2)
	{
		puts("blur [kernel size] [filename in] [filename out]"
		     "\n\n\tBlurs the image with a kernel of specified size."
		     "\n\tThe kernel size determines the amount of blur."
		     "\n\tKernel size may be decimal, e.g 1.5 pixels.");
		goto die;
	}
	else if (argc < 3)
	{
		LOG("ARG", "Missing input filename.\n");
		goto die;
	}
	else if (argc < 4)
	{
		LOG("ARG", "Missing output filename.\n");
		goto die;
	}

	double kernel_size_d = atof(argv[1]);
	if (kernel_size_d <= 1.0)
	{
		LOG("ARG", "Negative kernel size or kernel size < 1.0 makes no sense.\n");
		goto die;
	}

	cmd_args.kernel_size = (float) kernel_size_d;
	cmd_args.input_file = argv[2];
	cmd_args.output_file = argv[3];

	LOGF("ARG", "Processing %s -> %s, K-size: %f.\n", cmd_args.input_file, cmd_args.output_file, cmd_args.kernel_size);

	return;
die:
	exit(100);
}
