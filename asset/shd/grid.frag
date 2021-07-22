#version 330 core


uniform sampler2D color;
uniform sampler2D alpha;
in vec2 texCoord;


out vec4 FragColor;


void main()
{
	FragColor = texture(color,texCoord);
	FragColor *= texture(alpha,texCoord).r;
}
