#version 120
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

// Remove this line to turn off luma-based blurring.
#define BLUR_BY_LUMINISCENCE
#ifdef BLUR_BY_LUMINISCENCE
// Controls the luma blur size.
const float lumaBlurFactor = 0.85;
#endif

#define PROCESSING_TEXTURE_SHADER

uniform sampler2D texture;
uniform sampler2D zTex;
uniform vec2 focusPoint;
uniform float ratio;

varying vec4 vertColor;
varying vec4 vertTexCoord;

// Used by ZSAMPLE_ROW below.
#define ZBLURSAMPLE(x, y, a) (samples += (texture2D(texture, vertTexCoord.st + vec2(x, y)) * a));
// Samples one row of the 7x7 kernel. n is the row number, 0-6. 
// o is a 2 dimensional vector that contains the maximum offsets 
// in x and y.
#define ZBLURSAMPLE_ROW(n, o) {\
	float yoff = ((n - 3.0) / 3.0); \
 	ZBLURSAMPLE( 1.0  * o.x, yoff * o.y, RadialKernel7x7[n * 7 + 0]); \
	ZBLURSAMPLE( 0.66 * o.x, yoff * o.y, RadialKernel7x7[n * 7 + 1]); \
	ZBLURSAMPLE( 0.33 * o.x, yoff * o.y, RadialKernel7x7[n * 7 + 2]); \
	ZBLURSAMPLE( 0.0  * o.x, yoff * o.y, RadialKernel7x7[n * 7 + 3]); \
	ZBLURSAMPLE(-0.33 * o.x, yoff * o.y, RadialKernel7x7[n * 7 + 4]); \
	ZBLURSAMPLE(-0.66 * o.x, yoff * o.y, RadialKernel7x7[n * 7 + 5]); \
	ZBLURSAMPLE(-1.0  * o.x, yoff * o.y, RadialKernel7x7[n * 7 + 6]); \
	}

// A normalized 7x7 kernel representing an edge weighted disc.
const float RadialKernel7x7[49] = float[]
(
	0.0000, 0.0000, 0.0227, 0.0341, 0.0227, 0.0000, 0.0000,
	0.0000, 0.0341, 0.0341, 0.0341, 0.0341, 0.0341, 0.0000,
	0.0227, 0.0341, 0.0227, 0.0114, 0.0227, 0.0341, 0.0227,
	0.0341, 0.0341, 0.0114, 0.0000, 0.0114, 0.0341, 0.0341,
	0.0227, 0.0341, 0.0227, 0.0114, 0.0227, 0.0341, 0.0227,
	0.0000, 0.0341, 0.0341, 0.0341, 0.0341, 0.0341, 0.0000,
	0.0000, 0.0000, 0.0227, 0.0341, 0.0227, 0.0000, 0.0000
);

void main() 
{
	vec4 z = texture2D(zTex, vertTexCoord.st);
	vec4 focusPointZ = texture2D(zTex, focusPoint.st);
	z = abs((z - focusPointZ.r));

	// Max sample offsets.
	vec2 o = vec2(1.0, 1.0) * (ratio * ratio) * 0.05 * z.r;

#ifdef BLUR_BY_LUMINISCENCE
	// Rec.709 luma.
	vec4 lumaSrc = texture2D(texture, vertTexCoord.st) * vec4(0.2126, 0.7152, 0.0722, 0.0);
	float luma = lumaSrc.r + lumaSrc.g + lumaSrc.b;
		    o *= exp(luma * lumaBlurFactor) * lumaBlurFactor;
#endif

	vec4 samples  = vec4(0);
	
	ZBLURSAMPLE_ROW(6, o);
	ZBLURSAMPLE_ROW(5, o);
	ZBLURSAMPLE_ROW(4, o);
	ZBLURSAMPLE_ROW(3, o);
	ZBLURSAMPLE_ROW(2, o);
	ZBLURSAMPLE_ROW(1, o);
	ZBLURSAMPLE_ROW(0, o);

	gl_FragColor = samples;
}