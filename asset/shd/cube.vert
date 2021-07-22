#version 330 core


layout (location = 0) in vec3 aPos;
uniform mat4 P,V,M;


void main()
{
	gl_Position = P * V * M * vec4(aPos,1.0);
}
