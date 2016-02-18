#version 400
uniform usamplerBuffer flag_tex;

out vec4 frag_color; // final colour of surface

void main()
{
	uint flag = texelFetch(flag_tex, gl_PrimitiveID).r;
	// final colour
	if (bool(flag & 4))
	{
		frag_color = vec4(1.0, 0.0, 1.0, 1.0);
	}
	else
	{
		frag_color = vec4(0.0, 0.0, 1.0, 1.0);
	}
}