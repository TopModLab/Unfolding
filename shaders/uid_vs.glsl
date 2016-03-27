#version 330

in layout(location = 0) vec3 vp;

uniform mat4 view_matrix, proj_matrix;

void main()
{
	gl_Position = proj_matrix * view_matrix * vec4(vp, 1.f);
}