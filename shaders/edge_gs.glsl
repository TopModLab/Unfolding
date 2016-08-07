#version 330

layout(lines) in;
layout(line_strip, max_vertices = 2) out;

uniform uint hl_comp;// Highlight Components
uniform usamplerBuffer flag_tex;

flat out vec3 color;

void main()
{
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	
	uint flag = texelFetch(flag_tex, gl_PrimitiveIDIn).r;
	if (bool(flag & 4u) && bool(hl_comp & 1u))// Cut Edge
	{
		p0.z -= 0.00125f;
		p1.z -= 0.00125f;
		color = vec3(0.0f, 1.0f, 0.2f);
	}
	else// Regular edge
	{
		color = vec3(0.0f, 0.0f, 1.0f);
	}

	// Selected edge will override edge color
	if (bool(flag & 2u))
	{
		color = vec3(1.f, 0.25f, 0.0f);
	}

	gl_Position = p0;
	EmitVertex();

	gl_Position = p1;
	EmitVertex();
}


