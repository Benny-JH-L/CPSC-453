#version 330 core

in vec3 fragPos;
//in vec3 fragColor;
in vec3 n;
in vec2 texCoord;

//uniform vec3 lightPosition;
uniform sampler2D texture1;

out vec4 color;
//out vec4 FragColor;

void main()
{
	//color = vec4(fragColor, 1.0); // original
	color = texture(texture1, texCoord);

	// test
	//vec4 d = texture(texture1, texCoord);
	//FragColor = d;
}
