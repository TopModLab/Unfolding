#version 330
/*
* Face Geometry Shader
*/
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 proj_matrix;
uniform uint hl_comp;// Highlight Components
uniform usamplerBuffer flag_tex;
uniform float thickness = 0.0f;

in vec4 vNorm[];

flat out vec3 Kd;
out vec3 normal;
out vec3 pos;
flat out uint flag;

void EmitSide(vec4 p0, vec4 p1, vec4 n0, vec4 n1)
{
	vec4 p2 = p0 - n0 * thickness;
	vec4 p3 = p1 - n1 * thickness;
	normal = normalize(cross((p3 - p0).xyz, (p1 - p2).xyz));

	pos = p0.xyz;
	gl_Position = proj_matrix * p0;
	EmitVertex();

	pos = p2.xyz;
	gl_Position = proj_matrix * p2;
	EmitVertex();

	pos = p1.xyz;
	gl_Position = proj_matrix * p1;
	EmitVertex();
	
	pos = p3.xyz;
	gl_Position = proj_matrix * p3;
	EmitVertex();

	EndPrimitive();
}

void EmitBottom(vec4 p0, vec4 p1, vec4 p2)
{
	// If prefer backside as normal color, remove the comment here.
	//normal = -normal;
	pos = p0.xyz - vNorm[0].xyz * thickness;
	gl_Position = proj_matrix * vec4(pos, 1.0f);
	EmitVertex();

	pos = p1.xyz - vNorm[1].xyz * thickness;
	gl_Position = proj_matrix * vec4(pos, 1.0f);
	EmitVertex();

	pos = p2.xyz - vNorm[2].xyz * thickness;
	gl_Position = proj_matrix * vec4(pos, 1.0f);
	EmitVertex();

	EndPrimitive();
}

void main()
{
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;

	vec4 v1 = normalize(p1 - p0);
	vec4 v2 = normalize(p2 - p0);

	normal = normalize(cross(v1.xyz, v2.xyz));
	
	flag = texelFetch(flag_tex, gl_PrimitiveIDIn).r;
	if (bool(flag & 8u) && bool(hl_comp & 4u))// Bridger Face
	{
		Kd = vec3(0.0f, 0.8f, 1.f);
	}
	/*else if (bool(flag & 2u))// Selected Face
	{
		Kd = vec3(0.88f, 0.667f, 0.471f);
	}*/
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
	EndPrimitive();
	
	// Create thickness
	if (thickness > 0.0f)
	{
		EmitBottom(p0, p1, p2);

		EmitSide(p0, p1, vNorm[0], vNorm[1]);
		EmitSide(p1, p2, vNorm[1], vNorm[2]);
		EmitSide(p2, p0, vNorm[2], vNorm[0]);
	}
}


