#version 330 core
in vec3 pos;
in vec3 normal;

flat in uint flag;
flat in vec3 Kd;
//uniform vec3 Kd = vec3(0.6, 0.8, 1);
uniform vec3 La = vec3(0, 0, 0);
uniform vec3 light_pos = vec3(0.0f, 1.0f, 0.0f);

out vec4 frag_color; // final colour of surface

void main()
{
	if (bool(flag & 2u)
	 && bool(int(gl_FragCoord.x) % 2) && bool(int(gl_FragCoord.y) % 2))
	{
			frag_color = vec4(1.f, 0.5f, 0.f, 1.f);
	}
	else
	{
		vec3 dist2eye = light_pos - pos;
		vec3 dir = normalize(dist2eye);
		float dot_prod = dot(dir, normal);
		dot_prod = (dot_prod + 1.0f) / 2.0f;
		vec3 Id = mix(La, Kd, dot_prod); // final diffuse intensity
		// final color	
		frag_color = vec4(mix(Id, vec3(0.3,0.5,0.5), 1-dot_prod), 1.0f);
	}

}