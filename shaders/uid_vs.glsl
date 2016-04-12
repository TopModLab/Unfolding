#version 330

in layout(location = 0) vec3 vp;
uniform int mode = 0;

uniform mat4 view_matrix, proj_matrix;
uniform float scale;

void main()
{
	gl_Position = proj_matrix * view_matrix * vec4(scale * vp, 1.f);
	if (mode > 0)
	{
		gl_Position.z -= 0.0075f;
	}
}