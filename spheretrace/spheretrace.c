#include "spheretrace.h"

/*
	MATHS.
*/
quadratic_zeros find_quadratic_zeros(float p, float q)
{
	float phalf = p / 2.0f;
	/* Midpoint for equation. */
	float mid = -(phalf);
	float discriminant = (phalf * phalf) - q;

	quadratic_zeros s;
	s.real_count = 0;
	if (discriminant > 0.000001f)
		s.real_count = 2;
	else if (discriminant >= 0.0f)
		s.real_count = 1;

	s.s1 = NAN;
	s.s2 = NAN;	
	if (s.real_count > 0)
	{
		float discr_sqrt = sqrt(discriminant);

		s.s1 = mid + discr_sqrt;
		if (s.real_count > 1)
			s.s2 = mid - discr_sqrt;
		else
			s.s2 = s.s1;	
	}	

	return s;
}


/*
	VECTOR METHODS.
*/
vec3 mkvec3(float a, float b, float c)
{
	vec3 v = {a, b, c};
	return v;
}

vec3 vec3_add(vec3 a, vec3 b)
{
	vec3 v;
	v.x = a.x + b.x;
	v.y = a.y + b.y;
	v.z = a.z + b.z;
	return v;
}

vec3 vec3_sub(vec3 a, vec3 b)
{
	vec3 v;
	v.x = a.x - b.x;
	v.y = a.y - b.y;
	v.z = a.z - b.z;
	return v;
}

float vec3_dot(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_len2(vec3 a)
{
	return vec3_dot(a, a);
}

float vec3_len(vec3 a)
{
	return sqrt(vec3_len2(a));
}

vec3 vec3_scale(vec3 a, float s)
{
	vec3 v;
	v.x = a.x * s;
	v.y = a.y * s;
	v.z = a.z * s;
	return v;
}

vec3 vec3_norm(vec3 a)
{
	float l = vec3_len(a);
	float fraction = 1.0f / l;
	return vec3_scale(a, fraction);
}

vec3 vec3_reflect(vec3 const *n, vec3 const *i)
{
	//return vec3_sub(vec3_scale(*n, 2.0 * vec3_dot(*i, *n)), *i);

	return vec3_sub(vec3_scale(*n, 2.0f * (i->x * n->x + i->y * n->y + i->z * n->z)), *i);
}

vec3 vec3_faceforward(vec3 const *n, vec3 const *i)
{
	if (vec3_dot(*n, *i) < 0.000001f)
	{
		return *n;
	}

	return vec3_scale(*n, -1);
}

rgb rgb_mult(rgb a, rgb b)
{
	rgb v;
	v.x = a.x * b.x;
	v.y = a.y * b.y;
	v.z = a.z * b.z;
	return v;
}

rgb rgb_gamma(rgb a, float g)
{
	rgb ga;
	ga.x = powf(a.x, g);
	ga.y = powf(a.y, g);
	ga.z = powf(a.z, g);
	return ga;
}

rgb rgb_clamp(rgb a)
{
	rgb b;
	b.x = a.x > 1.0f? 1.0f : a.x;
	b.y = a.y > 1.0f? 1.0f : a.y;
	b.z = a.z > 1.0f? 1.0f : a.z;
	return b;
}

/*
	RAYS.
*/
ray mkray(vec3 origin, vec3 dir)
{
	ray r;
	r.origin = origin;
	r.dir = vec3_norm(dir);
	return r;
}

/*
	Return a point p along the ray, where p=origin if t=0.
*/
vec3 ray_point(ray r, float t)
{
	vec3 d = vec3_scale(r.dir, t);
	return vec3_add(r.origin, d);
}

int ray_intersects_sphere(ray r, sphere_primitive s, ray_sphere_test *rst)
{
	vec3 l = vec3_norm(r.dir); /* l . l = ||l||^2 = 1*1 = 1 now */
	vec3 o_min_c = vec3_sub(r.origin, s.center);
	float o_min_c_sqr = vec3_dot(o_min_c, o_min_c);

	float p = 2.0f * vec3_dot(l, o_min_c);
	float q = o_min_c_sqr - (s.radius * s.radius);

	quadratic_zeros zeros = find_quadratic_zeros(p, q);

	rst->hits = 0;
	if (zeros.real_count > 0)
	{
		float t0 = zeros.s1;
		float t1 = zeros.s2;

		if (t0 > t1)
		{
			float tmp = t0;
			t0 = t1;
			t1 = tmp;
		}

		if (t1 < 0.00001f)
		{
			return rst->hits;
		}

		if (t0 < 0.00001f)
		{
			rst->hits = 1;
			rst->p1 = ray_point(r, t1);
			rst->n1 = vec3_norm(vec3_sub(rst->p1, s.center));
		}
		else
		{
			rst->hits = 2;
			rst->p1 = ray_point(r, t0);
			rst->n1 = vec3_norm(vec3_sub(rst->p1, s.center));
		}
	}

	return rst->hits;
}

/*
	TGA METHODS.
*/
uint64_t tga_len(uint16_t w, uint16_t h, uint8_t bitdepth)
{
	uint64_t bytespp = bitdepth == 24? 3 : 4;
	uint64_t len = ((uint32_t) w) * ((uint32_t) h) * bytespp;
	return len;
}

// bithdepth 24 or 32
tga_data *tga_create(uint32_t w, uint32_t h, uint8_t bitdepth)
{
	if (bitdepth != 24 && bitdepth != 32)
		return NULL;

	tga_data *tga = malloc(sizeof(tga_data));
	if (tga != NULL)
	{
		tga->width = w;
		tga->height = h;
		tga->bitdepth = bitdepth;

		uint64_t len = tga_len(w, h, bitdepth);
		tga->data = malloc(len);

		if (tga->data == NULL)
		{
			free(tga);
			tga = NULL;
		}
	}
	return tga;
}

void tga_free(tga_data *tga)
{
	if (tga != NULL)
		free(tga->data);
	free(tga);
}

void tga_write(tga_data *data, FILE *f)
{
	/* Write the header. */

	/* Identification field. */
	/* We don't support the image identification field. */
	putc(0, f);

	/* Color map type. */
	/* Always zero because we don't support color mapped images. */
	putc(0, f);

	/* Image type code. */
	/* Always 2 since we only support uncompressed RGB. */
	putc(2, f);

	/*  Color map specification, not used. */
	for (int i = 0; i < 5; i++)
		putc(0, f);

	/* X-origin. lo-hi 2 byte integer. */
	putc(0, f); putc(0, f);

	/* Y-origin. lo-hi 2 byte integer. */
	putc(0, f); putc(0, f);

	/* Image width. lo-hi 2 byte integer. */
	putc(data->width & 0x00FF, f);
	putc((data->width & 0xFF00) >> 8, f);

	/* Image height. lo-hi 2 byte integer. */
	putc(data->height & 0x00FF, f);
	putc((data->height & 0xFF00) >> 8, f);

	/* Image Pixel Size (amount of bits per pixel.) */
	putc(data->bitdepth, f);

	/* Image Descriptor Byte */
	putc(0x20 | (data->bitdepth == 32? 0x08 : 0x00), f);

	/* Write the image data. */
	uint64_t len = tga_len(data->width, data->height, data->bitdepth);

	for (uint64_t i = 0; i < len; ++i)
	{
		putc(data->data[i], f);
	}
}

/*
	TRACING.
*/

rgb red_lambert(vec3 *p, vec3 *n, vec3 *i, shading_globals *sg)
{
	float lambert = fmaxf(vec3_dot(vec3_norm(sg->light_pos), *n), 0);

	lambert = (lambert + 0.5f) * 0.5f;
	return vec3_scale(mkvec3(1.0, 0.0, 0.0f), lambert);
}

rgb mirror(vec3 *p, vec3 *n, vec3 *i, shading_globals *sg)
{
	float lambert = fmaxf(vec3_dot(vec3_norm(sg->light_pos), *n), 0);
	lambert = (lambert + 0.1f) + 0.1f;

	if (sg->depth > 0)
	{
		vec3 i_inv = vec3_scale(*i, -1.0f);
		vec3 nff = vec3_faceforward(n, &i_inv);
		vec3 refl = vec3_norm(vec3_reflect(n, &i_inv));
		char hit = 0;
		sg->depth--;
		ray r = mkray(*p, refl);
		rgb col = trace_ray(&r, sg, &hit);
		sg->depth++;


		vec3 h = vec3_add( vec3_sub(sg->light_pos, *p), *i);
		h = vec3_norm(h);
		float blinn = powf(fmaxf(vec3_dot(h, *n), 0.0f), 10.0f);

		return vec3_add( vec3_add(vec3_scale(mkvec3(0.8f, 0.8f, 0.5f), blinn), col), vec3_scale(mkvec3(0.3f, 0.3f, 0.5f), lambert));
	}

	return mkvec3(0.0, 0.0, 0.0f);
}

/*
	This is the scene description, really...
*/
object objects[] = {
	{
		/* Shader. */
		red_lambert,
		/* Sphere primitive */ 
		{ 		
			{0.0f, -.25f, 0.0f},
			0.35f
		}
	},
	{
		/* Shader. */
		mirror,
		/* Sphere primitive */ 
		{ 		
			{0, 1.0, -1.5f},
			0.85f
		}	
	},
	{
		/* Shader. */
		mirror,
		/* Sphere primitive */ 
		{ 		
			{-1.0, -1.0, -1.5f},
			0.85f
		}	
	},
	{
		/* Shader. */
		mirror,
		/* Sphere primitive */ 
		{ 		
			{1.0, -1.0, -1.5f},
			0.85f
		}	
	}
};

rgb trace_ray(ray *r, shading_globals *sg, char *hit)
{
	for (int i = 0; i < ( sizeof(objects) / sizeof(object) ); ++i)
	{
		ray_sphere_test tst;
		if (ray_intersects_sphere(*r, objects[i].s, &tst))
		{
			*hit = 1;

			return objects[i].shader(&tst.p1, &tst.n1, &r->dir, sg);
		}
	}
	*hit = 0;
	return mkvec3(0.000f, 0.537f, 0.973f);
}

int main(int argc, char *argv[])
{
	printf("Hello my dear! You supplied %d arguments.\n", argc);

	sphere_primitive s;
	s.radius = 1.5f;
	s.center = mkvec3(0, 0, 0);

	vec3 camera_origin = mkvec3(0, 0, 5.0f);
	float imgplane_w = 4.0;
	int res = 512;

	vec3 topleft = vec3_add(mkvec3(-imgplane_w / 2.0f, imgplane_w / 2.0f, 0), camera_origin);
	vec3 pshiftx = mkvec3((imgplane_w / (float)res), 0, 0);
	vec3 pshifty = mkvec3(0, -(imgplane_w / (float)res), 0);

	printf("Image Res: %dx%d. Image Plane Width: %f. Pixel Shift X: %f %f %f.\n", res, res, imgplane_w, pshiftx.x, pshiftx.y, pshiftx.z);

	tga_data *tga = tga_create(res, res, 24);
	if (tga == NULL)
	{
		puts("Could not create TGA Buffer... I'm outta here.");
		return 1;
	}

	shading_globals sg = {0};
	sg.light_pos =  mkvec3(.707f, .707f, .707f);
	sg.depth = 9;

	int ray_count = 0, ray_hits = 0;
	for (int y = 0; y < res; ++y)
	{
		for (int x = 0; x < res; ++x)
		{
			int supersample = 2.0;
			rgb col = {0, 0, 0};
			vec3 ray_origin = vec3_add(vec3_add(topleft, vec3_scale(pshiftx, x)), vec3_scale(pshifty, y));
			for (int j = 0; j < supersample; j++)
			{
				ray_origin = vec3_add(ray_origin, vec3_scale(pshifty, 1.0f / supersample));
				for (int i = 0; i < supersample; i++)
				{
					ray_origin = vec3_add(ray_origin, vec3_scale(pshiftx, 1.0f / supersample));
					ray r = mkray(ray_origin, mkvec3(0.0f, 0.0f, -1.0f));
					sg.eye = ray_origin;

					char hit = 0;
					col = vec3_add(trace_ray(&r, &sg, &hit), col);

					if (hit)
					{
						ray_hits++;
					}
				}
			}

			col = vec3_scale(col, 1.0f / (supersample * supersample));
			col = rgb_gamma(col, 1.0f / 2.2f); 
			col = rgb_clamp(col);

			/*printf("Shooting ray from %f %f %f.\n", r.origin.x, r.origin.y, r.origin.z);*/

			int stride = x + (y * tga->height);

			tga->data[stride * 3 + 0] = (int)(col.z * 255.9);
			tga->data[stride * 3 + 1] = (int)(col.y * 255.9);
			tga->data[stride * 3 + 2] = (int)(col.x * 255.9);

			ray_count++;
		}
	}

	FILE *f = fopen("render.512x512x24b.tga", "wb+");
	if (f == NULL)
	{
		puts("Couldn't open output file. I can't work with this. Quitting.");
		tga_free(tga);
		return 2;
	}

	tga_write(tga, f);

	tga_free(tga);

	printf("\n%07d rays shot, %07d hits.\n", ray_count, ray_hits);

	return 0;
}