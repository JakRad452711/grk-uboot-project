#version 410 core

uniform vec3 objectColor;
uniform vec3 lightDir;
uniform float fogAppearDistance;
uniform float fogFullEffectDistance;

in vec3 interpNormal;
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
	vec3 normal = normalize(interpNormal);
	float ambient = 0.2;
	float diffuse = max(dot(normal, -lightDir), 0.0);
	//gl_FragColor = vec4(objectColor * (ambient + (1-ambient) * diffuse), 1.0);

	gl_FragColor = mix(vec4(objectColor * (ambient + (1-ambient) * diffuse), 1.0), vec4(0, 0, 0, 1), countIntensity(distance));
	gl_FragColor = vec4(0, 0, 0, 0);
}
