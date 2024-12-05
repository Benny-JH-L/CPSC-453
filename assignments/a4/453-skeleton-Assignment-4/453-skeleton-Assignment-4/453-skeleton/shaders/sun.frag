#version 330 core

in vec2 texCoord;

uniform sampler2D texture1;
uniform vec3 lightColour;
uniform float ambientCoefficient;	// Ambient intensity

out vec4 color;

void main()
{
	vec3 textureColour = texture(texture1, texCoord).rgb;
	// Ambient
	vec3 ambientComponent = ambientCoefficient * lightColour * textureColour;	// using only ambent to light the sun

	color = vec4(ambientComponent, 1.0);

	// backup
	//color = texture(texture1, texCoord);

	// test
	//vec4 d = texture(texture1, texCoord);
	//FragColor = d;
}
