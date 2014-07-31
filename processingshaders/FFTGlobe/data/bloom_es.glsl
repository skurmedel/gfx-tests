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
uniform float mixAmount;

/*
	Samples 7x7 pixels from an area of the texture with the center at p.

	offsets is a vector that contains the X and Y sampling offset to the 
	pixel next and above it. By scaling this larger than the actual offsets 
	the	sampling rectangle could be made larger, if needed, to simulate a
	blur-radius etc. 

	bias is the same as the bias parameter accepted by texture2D().
*/
void sampleKernel7x7(in vec2 p, in vec2 offsets, in float bias, out vec4[49] target)
{
	vec2 corner = vec2(p.x + offsets.x * 3.5, p.y + offsets.y * 3.5);
#ifndef MANUAL_UNROLLS
	for (int y = 0; y < 7; ++y)
	{
		float yCoord = corner.y - offsets.y * y;
		for (int x = 0; x < 7; ++x)
		{
			int i = (y * 7 + x);
			target[i] = texture2D(texture, vec2(corner.x - offsets.x * x, yCoord), bias);
		}
	}
#else
#error I haven't written this portion yet.
#endif
}

/*
	Dilates a 7x7 matrix with a 3x3 structuring element.
	Outputs a 5x5 matrix. Since the border data is not present
	to do an accurate dilation of the 7x7 matrix, we dilate 
	the inner 5x5 sub-matrix and return that.

	structElement is assumed to only hold 1 and 0, anything
	outside this range will yield incorrect results.

	pixels is an array representing a 7x7 image.

	target is an array representing a 5x5 image.
*/
void dilate5x5(
	in int[9] structElement, 
	in vec4[49] pixels, 
	out vec4[25] target)
{
#ifndef MANUAL_UNROLLS
	for (int y = 1; y < 6; ++y)
	{
		for (int x = 1; x < 6; ++x)
		{
			vec4 maxv = vec4(0, 0, 0, 0.0);
			for (int i = 0; i < 9; ++i)
			{
				/*
					Compute the sub 3x3 rectangle coords.
					Take 1 to get us in a [-1, 1] range.
				*/
				int sy = (i / 3) - 1;
				int sx = (i % 3) - 1;

				vec4 p = pixels[(y + sy) * 7 + x + sx];
				/*
					Using max in this way avoids using proper
					branching.
				*/
				maxv.r = max(p.r + structElement[i], maxv.r);
				maxv.g = max(p.g + structElement[i], maxv.g);
				maxv.b = max(p.b + structElement[i], maxv.b);
				maxv.a = max(p.a + structElement[i], maxv.a);
			}
			/*
				Since we assume (description) that the structuring
				element is only integers [0, 1] we can safely 
				normalize back our pixeldata through a subtraction. 
				Without this assumption we would probably have to
				use an if or other costly trickery, for little gain.
			*/
			maxv.r -= 1;
			maxv.g -= 1;
			maxv.b -= 1;
			maxv.a -= 1;
			/*
				Put the data into the target array, since we operate
				from 1 to 5, we'll have to subtract 1 from the indices
				to get into a [0, 4] range.
			*/
			target[(y - 1) * 5 + (x - 1)] = maxv;
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
	sampleKernel7x7(vertTexCoord.xy, offset, 1, samples);
	
	int structElement[9] = int[9](0, 1, 0, 1, 1, 1, 0, 1, 0);
	vec4 dilated[25] = vec4[25](0);
	dilate5x5(structElement, samples, dilated);
	
	vec4 rgb = vec4(0);
	/*
		Box Blur.
	*/
	for (int i = 0; i < 25; ++i)
	{
		rgb += dilated[i] * 0.04;
	}
	vec4 original = texture2D(texture, vertTexCoord.st);
	
	vec4 final = rgb * 1.0 + original;

	gl_FragColor = final;
}
