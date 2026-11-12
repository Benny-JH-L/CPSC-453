#version 330 core

in vec3 fragPos;
in vec3 n;
in vec2 texCoord;

uniform vec3 lightPosition;
uniform vec3 viewPos;				// position of the camera viewer
uniform vec3 lightColour;
uniform float ambientCoefficient;	// Ambient intensity
uniform float diffuseCoefficient;	// Diffuse intensity
uniform float specularCoefficient;	// Specular intensity
uniform float shininessCoefficient;	// Shininess factor (for specular component)

uniform sampler2D texture1;

out vec4 color;

// Uses Phong reflection model (Diffuse, specular, and ambient lighting components)
// and applys them onto each fragment (Phong shading model).
void main()
{
	vec3 textureColour = texture(texture1, texCoord).rgb;
	vec3 norm = normalize(n);

	// Shading
	vec3 lightDirection = normalize(lightPosition - fragPos); 
	vec3 viewDirection = normalize(viewPos - fragPos);

	// Diffuse
	float dotProductDiff = max(dot(norm, lightDirection), 0.0);

	vec3 diffuseComponent = diffuseCoefficient * dotProductDiff * lightColour * textureColour;

	// Specular
	vec3 reflectedLightDirection = reflect(-lightDirection, norm);

	float dotProductSpecToShinePow = pow(max(dot(viewDirection, reflectedLightDirection), 0.0), shininessCoefficient);
	vec3 specularComponent = specularCoefficient * dotProductSpecToShinePow * lightColour * textureColour;

	// Ambient
	vec3 ambientComponent = ambientCoefficient * lightColour * textureColour;

	vec3 lightingResult = diffuseComponent + specularComponent + ambientComponent;
	color = vec4(lightingResult, 1.0);
}
