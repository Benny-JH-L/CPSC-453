#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

out vec2 tc;

uniform mat4 transformationMatrix;
uniform mat4 transformationMatrixTexture; // delete

void main()
{
	tc = texCoord;
	//vec4 transformedTexCoord = transformationMatrixTexture * vec4(texCoord, 0.0, 1.0); // Apply texTransform to texCoord
	//tc = transformedTexCoord.xy; // Pass the transformed texture coordinate to the fragment shader
	gl_Position = transformationMatrix * vec4(pos, 1.0);
	//gl_Position = vec4(pos, 1.0);
}
