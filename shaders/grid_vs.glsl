#version 430
in layout(location = 0) vec3 vp;
in layout(location = 1) vec3 vc;

uniform mat4 view_matrix, proj_matrix;

out vec3 color;
void main()
{
	gl_Position = proj_matrix * view_matrix * vec4(vp * 3, 1.0);
	color = vc;
	//gl_Position = vec4(vp, 1.0);
}