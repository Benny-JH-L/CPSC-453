#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

out vec2 tc;

uniform mat4 transformationMatrix;

void main()
{
	tc = texCoord;
	gl_Position = transformationMatrix * vec4(pos, 1.0);
}
