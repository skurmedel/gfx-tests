#version 120
#define PROCESSING_COLOR_TEXTURE
#extension GL_EXT_gpu_shader4 : enable
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D texture;
uniform vec2 texOffset;

varying vec4 vertTexCoord;

/*
	Samples 7x7 pixels from an area of the texture with the center at p.

	offsets is a vector that contains the X and Y sampling offset to the 
	pixel next and above it. By scaling this larger than the actual offsets 
	the	sampling rectangle could be made larger, if needed, to simulate a
	blur-radius etc. 

	bias is the same as the bias parameter accepted by texture2D().
*/
void sampleKernel7x7(const vec2 p, const vec2 offsets, const float bias, out vec4[49] target)
{
	vec2 corner = vec2(p.x - offsets.x * 3, p.y - offsets.y * 3);
#ifndef MANUAL_UNROLLS
	for (int y = 0; y < 7; ++y)
	{
		float yCoord = corner.y + offsets.y * y;
		for (int x = 0; x < 7; ++x)
		{
			int i = (y * 7 + x);
			target[i] = texture2D(texture, vec2(corner.x + offsets.x * x, yCoord));
		}
	}
#else
#error I haven't written this portion yet.
#endif
}

void dilate5x5(in float[9] structElement, in vec4[49] pixels, out vec4[49] target)
{
#ifndef MANUAL_UNROLLS
	for (int y = 1; y < 6; ++y)
	{
		for (int x = 1; x < 6; ++x)
		{
			int iover = ((y + 1) * 6 + x);
			int i (y * 6 + x);
			int iunder = ((y - 1) * 6 + x);

			

		}
	}
#else
#error I haven't written this portion either yet.
#endif
}

void main() 
{
	vec2 offset = texOffset.xy;

	vec4 samples[49];
	sampleKernel7x7(vertTexCoord.xy, offset, 1.0, samples);

	vec3 rgb = vec3(0);
	for (int i = 0; i < 49; i++)
	{
		rgb += samples[i].xyz;
	}
	gl_FragColor = vec4(rgb * 0.0204081, 1.0);
}
