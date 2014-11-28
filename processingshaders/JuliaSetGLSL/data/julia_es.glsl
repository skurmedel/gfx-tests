#version 120
#ifdef GL_ES
precision highp float; 
precision highp vec4;
precision highp vec2;
#endif

#define PROCESSING_TEXTURE_COLOR

#define ITER_COUNT 500
#define ITER_BOUND float(ITER_COUNT)
#define FLIGHT_OF_TIME(x) float((x - distlog) / ITER_BOUND)

uniform vec2 mousePoint;
uniform float zoom;
uniform vec2 off;
uniform sampler2D gradient;

varying vec4 vertColor;
varying vec4 vertTexCoord;
uniform float aspect;

int julia_sample(inout vec2 z, in vec2 c)
{
	int i = 0;
	while (length(z) < 2.001 && i < ITER_COUNT)
	{
		vec2 old = z;
		z.x = old.x * old.x - old.y * old.y;
		z.y = old.x * old.y + old.y * old.x;
		z.x += c.x;
		z.y += c.y;
		i++;
	}
	return i;
}

float random(vec2 n) 
{
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

void main() 
{
	float x = (vertTexCoord.x - 0.5) * aspect * zoom;
	float y = (1.0 - vertTexCoord.y - 0.5) * zoom;

	float dx = ((vertTexCoord.x + off.s * 0.65f) - 0.5) * aspect * zoom - x;
	float dy = (1.0 - (vertTexCoord.y + off.t * 0.65f) - 0.5) * zoom - y;
	float rv = (random(vec2(x, y) * 0.9f) / 255.0f) * 0.001f;
	dx += rv;
	dy += rv;

	x += (mousePoint.x - 0.5) * aspect;
	y += (1.0 - mousePoint.y - 0.5);

	vec2 z = vec2(x, y);
	//vec2 c = vec2(0.285, 0.01);
	vec2 c = vec2(-0.8 , 0.159);	
	int i = julia_sample(z, c);

	float distlog = log2(log2(clamp(length(z), 0.0000001f, 1000000f)));
	float fot  = FLIGHT_OF_TIME(i);

	vec3 light = normalize(vec3(1,0.25,1));
	vec3 view =  normalize(vec3(-1.5,1,-1.5));
	vec3 ltov =  normalize(light + view);

	vec4 tint = texture2D(gradient, sqrt(vec2(fot, fot))); 

	vec3 normal  = cross(
		normalize(vec3(dx, clamp(dFdx(pow(fot, 0.8)), 0, 1.0), 0)), 
		normalize(vec3(0,  clamp(dFdy(pow(fot, 0.8)), 0, 1.0), dy)));
	vec4 diffuse = tint * max(dot(light, normal), 0);
	vec4 specular= pow(max(dot(ltov, normal), 0) + 0.0, 1500.0) * vec4(0.5);

	vec4 color = clamp(tint * 0.7 + diffuse + specular + vec4( random(vec2(x, y) * 0.1f) / 255.0 ), vec4(0), vec4(1));

	gl_FragColor = color;
}