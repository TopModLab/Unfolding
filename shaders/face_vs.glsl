#version 330
layout(location = 0) in vec3 vp;

uniform mat4 view_matrix;
uniform float scale = 1.0f;

void main()
{
	gl_Position = view_matrix * vec4(scale * vp, 1.0f);
}