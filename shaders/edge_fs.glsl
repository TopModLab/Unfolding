#version 400
flat in vec3 color;
out vec4 frag_color; // final colour of surface

void main()
{
	frag_color = vec4(color, 1.0f);
}