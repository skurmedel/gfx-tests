#include "spheretrace.h"
#include "stdio.h"

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

vec3 vec3_dot(vec3 a, vec3 b)
{
	vec3 v;
	v.x = a.x * b.x;
	v.y = a.y * b.y;
	v.z = a.z * b.z;
	return v;
}

vec3 vec3_scale(vec3 a, float s)
{
	vec3 v;
	v.x = a.x * s;
	v.y = a.y * s;
	v.z = a.z * s;
	return v;
}

int main(int argc, char *argv[])
{
	printf("Hello my dear! You supplied %d arguments.\n", argc);

	return 0;
}