#version 430
in vec3 color;
out vec4 frag_color; // final colour of surface

void main()
{
	// final colour
	frag_color = vec4(color, 1.0);
	//frag_color = vec4(1,1,1, 1.0);
}