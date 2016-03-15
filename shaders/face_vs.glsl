#version 330
in layout(location = 0) vec3 vp;

uniform mat4 view_matrix;

void main()
{
	//gl_Position = proj_matrix * view_matrix * vec4(vp, 1.0);
	gl_Position = view_matrix * vec4(vp, 1.0);
}