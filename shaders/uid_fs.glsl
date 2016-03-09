#version 400

out vec4 frag_color;

uniform usamplerBuffer id_tex;

uniform bool depthonly = false;

void main()
{
	if (depthonly)
	{
		frag_color = vec4(0, 0, 0, 0);
	}
	else
	{
		uint uid = texelFetch(id_tex, gl_PrimitiveID).r;
		//int uid = gl_PrimitiveID;
		frag_color = vec4(float(uid >> 16) / 255.0, float((uid >> 8) & 0xFF) / 255.0,
			float(uid & 0xFF) / 255.0, 1.0);
	}
}