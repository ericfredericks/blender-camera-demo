#version 330 core


layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aColor;
uniform mat4 M,V,P;
out vec3 color;


void main()
{
	gl_Position = P * V * M * vec4(aPos,1.0);
	color = aColor;
}
