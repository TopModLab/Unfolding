#version 330
layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 vn;

uniform mat4 view_matrix;
uniform float scale = 1.0f;

out vec4 vNorm;

void main()
{
	gl_Position = view_matrix * vec4(scale * vp, 1.0f);
	vNorm = view_matrix * vec4(vn, 0.0f);
}