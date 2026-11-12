#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 aTexCoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 fragPos;
out vec3 n;
out vec2 texCoord;

void main()
{
	fragPos = pos;
	n = mat3(transpose(inverse(M))) * normal;	// apply objecet transforms to the previously calculated normal 
	texCoord = aTexCoord;
	gl_Position = P * V * M * vec4(pos, 1.0);
}
