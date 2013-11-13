#ifndef SPHERETRACE_H
#define SPHERETRACE_H

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
vec3 vec3_dot(vec3 a, vec3 b);
vec3 vec3_scale(vec3 a, float s);

typedef vec3 rgb;

#endif