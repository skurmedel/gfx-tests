#version 120
#define PROCESSING_COLOR_SHADER
#extension GL_EXT_gpu_shader4 : enable
#ifdef GL_ES
precision highp float;
precision mediump int;
#endif

uniform sampler2D texture;
uniform float beat;
uniform float time;

varying vec4 vertPos;
varying vec4 vertColor;
varying vec3 vertNormal;
varying vec4 vertTexCoord;

const vec4 color1 = vec4(0.624, 0.769, 0.980, 1.0); 
const vec4 color2 = vec4(0.608, 0.761, 0.522, 1.0);

float calculateCos(vec2 v, vec2 axis)
{
	return (dot(v, axis) + 1) * 0.5;
}

float normalizeCos(float cosVal)
{
	return (cosVal + 1) * 0.5;
}

void main() 
{	
	vec3 rotNormal = normalize(vertNormal);

	float yCos = calculateCos(rotNormal.zx, vec2(0, 1));
	float zCos = calculateCos(rotNormal.xy, vec2(0, 1));
	
	vec4 col = mix(
		color1,
		color2, 
		(yCos + zCos) * 0.15 * beat);
	
	float period = 227;
	float peak = 0.21;
	float yoffset = (1.0 - peak);
	float offset = -0.125;

	col.a = clamp((cos((yCos + offset) * period) - yoffset) / peak, 0.0, 1.0) 
	      * clamp((cos((zCos + offset) * period) - yoffset) / peak, 0.0, 1.0);
	
	float adev = fwidth(col.a);
	if (col.a < 0.01)
	{
		col.a = mix(0.0, col.a + adev * 0.5, adev * 0.5);
	}

	col.a *= 0.80;

	col = mix(col, color1, clamp(beat * (yCos + zCos - 2.0) * 0.5, 0.0, 1.0) * 0.05);

	gl_FragColor = col;
}