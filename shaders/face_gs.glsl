#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 proj_matrix;
uniform uint hl_comp;// Highlight Components
uniform usamplerBuffer flag_tex;

flat out vec3 Kd;
out vec3 normal;
out vec3 pos;

void main()
{
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;

	vec4 v1 = normalize(p1 - p0);
	vec4 v2 = normalize(p2 - p0);

	normal = normalize(cross(v1.xyz, v2.xyz));
	
	uint flag = texelFetch(flag_tex, gl_PrimitiveIDIn).r;
	if (bool(flag & 16u) && bool(hl_comp & 4u))// Bridger
	{
		Kd = vec3(0.0f, 0.8f, 1.f);
	}
	else if (bool(flag & 2u))
	{
		Kd = vec3(1.f, 0.f, 0.f);
	}
	else// Regular color
	{
		Kd = vec3(0.6, 0.8, 1.f);
	}
	
	// Send the triangle along with the edge distances
	gl_Position = proj_matrix * p0;
	pos = p0.xyz;
	EmitVertex();

	gl_Position = proj_matrix * p1;
	pos = p1.xyz;
	EmitVertex();

	gl_Position = proj_matrix * p2;
	pos = p2.xyz;
	EmitVertex();
}


