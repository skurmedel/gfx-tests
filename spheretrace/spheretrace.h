#ifndef SPHERETRACE_H
#define SPHERETRACE_H

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

/*
	MATHS.
*/
typedef struct quadratic_zeros_t
{
	float s1;
	float s2;
	/*
		Number of real solutions.
	*/
	int real_count;
} quadratic_zeros;
/*
	Solve a quadratic equation x^2 + px + q = 0.

	If your equation is of form ax + bx + c, just divide
	p and q with a: find_quadratic_zeros(p / a, q / a),
	as long as a isn't zero of course.
*/
quadratic_zeros find_quadratic_zeros(float p, float q);

/*
	VECTORS.
*/
typedef struct vect3_t
{
	float x, y, z;
} vec3;

vec3 mkvec3(float a, float b, float c);
vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
float vec3_dot(vec3 a, vec3 b);
float vec3_len2(vec3 a); 
float vec3_len(vec3 a);
#define vec3_mag(x) vec3_len(x)
vec3 vec3_scale(vec3 a, float s);
vec3 vec3_norm(vec3 a);
vec3 vec3_reflect(vec3 const *n, vec3 const *i);
vec3 vec3_faceforward(vec3 const *n, vec3 const *i);

typedef vec3 rgb;

rgb rgb_mult(rgb a, rgb b);
rgb rgb_gamma(rgb a, float gamma);
rgb rgb_clamp(rgb a);

/*
	RAYS.
*/
typedef struct ray_t
{
	vec3 origin;
	/*
		The direction of the ray.
	*/
	vec3 dir;
} ray;

ray mkray(vec3 origin, vec3 dir);
/*
	Return a point p along the ray, where p=origin if t=0.
*/
vec3 ray_point(ray r, float t);

/*
	Describes an implicit sphere:
	
	(p - c) . (p - c) - r^2 = 0

	where p is a point to test, c is the center of the sphere,
	r is the radius of the sphere.  "." is the dot product.
*/
typedef struct sphere_primitive_s
{
	vec3 center;
	float radius;
} sphere_primitive;

typedef struct ray_sphere_test_t
{
	vec3 p1;
	vec3 n1;

	short hits;
} ray_sphere_test;

/*
	Finds intersections for a ray on a sphere.

	Returns rst->hits, which is positive if there was an intersection.
*/
int ray_intersects_sphere(ray r, sphere_primitive s, ray_sphere_test *rst);

/*
	TGA STUFF.
*/
typedef struct tga_data_s
{
	/* 
		The image data is arranged from top left to bottom right,
		BGRA (32 bit) or BGR (24 bit).

		Thus data[0] is the blue byte of the topmost left corner.
	*/
	uint8_t  *data;
	uint16_t width;
	uint16_t height;
	/* 24 or 32 (32 is with alpha mask.) */
	uint8_t  bitdepth;
} tga_data;

uint64_t tga_len(uint16_t w, uint16_t h, uint8_t bitdepth);
tga_data *tga_create(uint32_t w, uint32_t h, uint8_t bitdepth);
void tga_free(tga_data *tga);
void tga_write(tga_data *data, FILE *f);

/*
	SHADERS.
*/
typedef struct shading_globals_t
{
	/*
		How many recursions or bounces we've done.
	*/
	int depth;
	vec3 light_pos;
	vec3 eye;
} shading_globals;

typedef rgb (*shader_f)(vec3 *p, vec3 *n, vec3 *i, shading_globals *sg);

typedef struct object_t
{
	shader_f shader;
	sphere_primitive s;
} object;

rgb trace_ray(ray *r, shading_globals *sg, char *hit);
char trace_shadow_ray(ray *r, shading_globals *sg);

#endif