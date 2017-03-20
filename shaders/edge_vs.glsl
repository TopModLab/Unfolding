#version 330
layout(location = 0) in vec3 vp;

uniform mat4 view_matrix, proj_matrix;
uniform float scale = 1.0f;

void main()
{
	vec4 vCam = view_matrix * vec4(scale * vp, 1.0f);
	vCam -= vec4(0.0078125 * normalize(vCam.xyz), 0.0f);
	gl_Position = proj_matrix * vCam;
}