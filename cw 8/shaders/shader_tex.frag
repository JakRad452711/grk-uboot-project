#version 410 core

uniform sampler2D textureSampler;
uniform vec3 lightDir;
uniform vec4 skyColor;
uniform float fogAppearDistance;
uniform float fogFullEffectDistance;

in vec3 interpNormal;
in vec2 interpTexCoord;
in float distance;

float countIntensity(float aDistance) {
	if(aDistance < fogAppearDistance)
		return 0;

	if(aDistance > fogFullEffectDistance)
		return 1;

	return (aDistance - fogAppearDistance) / (fogFullEffectDistance - fogAppearDistance);
}

void main()
{
	vec2 modifiedTexCoord = vec2(interpTexCoord.x, 1.0 - interpTexCoord.y); // Poprawka dla tekstur Ziemi, ktore bez tego wyswietlaja sie 'do gory nogami'
	vec3 color = texture2D(textureSampler, modifiedTexCoord).rgb;
	vec3 normal = normalize(interpNormal);
	float ambient = 0.2;
	float diffuse = max(dot(normal, -lightDir), 0.0);
	gl_FragColor = mix(vec4(color * (ambient + (1-ambient) * diffuse), 1.0), skyColor, countIntensity(distance));
}
