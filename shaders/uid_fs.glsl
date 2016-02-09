#version 430

out vec4 frag_color;

void main()
{
	int uid = gl_PrimitiveID;
	frag_color = vec4((uid >> 16) / 255.0, ((uid >> 8) & 0xFF) / 255.0,
			(uid & 0xFF) / 255.0, 1.0);
}