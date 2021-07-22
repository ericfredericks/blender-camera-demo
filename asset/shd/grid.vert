#version 330 core


layout (location = 0) in vec3 aPos;
uniform mat4 P,V,M;


out vec2 texCoord;


void main()
{
	texCoord = aPos.xy * 500.0;
	gl_Position = P * V * M * vec4(aPos,1.0);
}
