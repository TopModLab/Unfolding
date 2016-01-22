#version 430
in vec3 pos;
in vec3 normal;

uniform vec3 Kd = vec3(0.6, 0.8, 1);
uniform vec3 La = vec3(0, 0, 0);
uniform vec3 light_pos = vec3(0.0, 1.0, 0);


out vec4 frag_color; // final colour of surface

void main()
{
	vec3 dist2eye = light_pos - pos;
	vec3 dir = normalize(dist2eye);
	float dot_prod = dot(dir, normal);
	dot_prod = (dot_prod + 1.0) / 2.0;
	vec3 Id = mix(La, Kd, dot_prod); // final diffuse intensity

	// final colour
	frag_color = vec4(Id, 1.0);
}