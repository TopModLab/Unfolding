#version 400

uniform uint hl_comp;// Highlight Components

uniform usamplerBuffer flag_tex;

out vec4 frag_color; // final colour of surface

void main()
{
	uint flag = texelFetch(flag_tex, gl_PrimitiveID).r;
	// final colour
	if (bool(flag & 4) && bool(hl_comp & 1))// Cut Edge
	//if (hl_comp_flag > 0)
	{
		gl_FragDepth = gl_FragCoord.z - 0.00125f;
		frag_color = vec4(0.f, 1.f, 0.2f, 1.f);
	}
	else// Regular edge
	{
		gl_FragDepth = gl_FragCoord.z;
		frag_color = vec4(0.f, 0.f, 1.f, 1.f);
	}
}