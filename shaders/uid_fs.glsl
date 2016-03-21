#version 330

out vec4 frag_color;

uniform usamplerBuffer id_tex;

uniform bool depthonly = false;

void main()
{
	if (depthonly)
	{
		frag_color = vec4(0.f, 0.f, 0.f, 0.f);
	}
	else
	{
		uint uid = texelFetch(id_tex, gl_PrimitiveID).r;
		//int uid = gl_PrimitiveID;
		frag_color = vec4(float(uid >> uint(16)) / 255.0,
						float((uid >> uint(8)) & uint(0xFF)) / 255.0,
						float(uid & uint(0xFF)) / 255.0, 1.0);
	}
}