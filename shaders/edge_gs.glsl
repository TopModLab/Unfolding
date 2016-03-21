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
	if (bool(flag & + uint(4)) && bool(hl_comp & + uint(1)))// Cut Edge
	{
		p0.z -= 0.00125f;
		p1.z -= 0.00125f;
		color = vec3(0.f, 1.f, 0.2f);
	}
	else if (bool(flag & uint(2)))
	{
		color = vec3(1.f, 0.f, 0.f);
	}
	else// Regular edge
	{
		color = vec3(0.f, 0.f, 1.f);
	}
	
	gl_Position = p0;
	EmitVertex();

	gl_Position = p1;
	EmitVertex();
}


