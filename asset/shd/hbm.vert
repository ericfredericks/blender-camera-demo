#version 330 core


#define MAX_BONES 100


layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aBoneWeights;


uniform mat4 PVM;
uniform mat2x4 boneQuats[MAX_BONES];


out vec3 normal;


#define xR (weightedRotQuat[0])
#define yR (weightedRotQuat[1])
#define zR (weightedRotQuat[2])
#define wR (weightedRotQuat[3])
#define xT (weightedTransQuat[0])
#define yT (weightedTransQuat[1])
#define zT (weightedTransQuat[2])
#define wT (weightedTransQuat[3])


void main()
{
	mat2x4 blendDQ = boneQuats[aBoneIDs.x] * aBoneWeights.x;
	blendDQ += boneQuats[aBoneIDs.y] * aBoneWeights.y;
	blendDQ += boneQuats[aBoneIDs.z] * aBoneWeights.z;
	blendDQ += boneQuats[aBoneIDs.w] * aBoneWeights.w;


	float len = length(blendDQ[0]);
	blendDQ /= len;


	vec3 pos = aPos + (2.0 * cross(blendDQ[0].xyz,cross(blendDQ[0].xyz,aPos) + blendDQ[0].w*aPos));
	vec3 trans = 2.0 * (blendDQ[0].w*blendDQ[1].xyz - blendDQ[1].w*blendDQ[0].xyz + cross(blendDQ[0].xyz,blendDQ[1].xyz));
	pos += trans;


	vec3 norm = aNormal + (2.0 * cross(blendDQ[0].xyz,cross(blendDQ[0].xyz,aNormal) + blendDQ[0].w*aNormal));


	gl_Position = PVM * vec4(pos,1.0);
	normal = norm;
}



#undef xR
#undef yR
#undef zR
#undef wR
#undef tX
#undef tY
#undef tZ
#undef tW
#undef MAX_BONES
