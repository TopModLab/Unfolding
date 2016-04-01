#version 330
uniform vec3 color = vec3(1.f,0.f,0.f);
out vec4 frag_color; // final colour of surface
flat in uint flag;
void main()
{
	if (flag == uint(2))
	{
		frag_color = vec4(color, 1.f);
	}
	else
	{
		//discard;
		frag_color = vec4(0.f, 0.f, 1.f, 1.f);
	}
}