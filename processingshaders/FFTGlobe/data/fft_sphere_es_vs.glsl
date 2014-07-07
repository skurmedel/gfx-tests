uniform mat4 transform;
uniform mat3 normalMatrix;

attribute vec4 vertex;
attribute vec4 color;
attribute vec3 normal;

varying vec4 vertPos;
varying vec4 vertColor;
varying vec3 vertNormal;

void main() 
{
  gl_Position = transform * vertex;    
  vertPos = gl_Position;
  vertColor = color;
  vertNormal = normalize(normalMatrix * normal);
}