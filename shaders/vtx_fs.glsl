#version 330
uniform in vec3 color = vec3(1.f,0.f,0.f);
out vec4 frag_color; // final colour of surface

void main()
{
	frag_color = vec4(color, 1.0f);
}