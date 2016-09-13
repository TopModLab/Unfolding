#version 330
layout(location = 0) in vec3 vp;
uniform int mode = 0;

uniform mat4 view_matrix, proj_matrix;
uniform float scale = 1.0f;

void main()
{
	vec4 vCam = view_matrix * vec4(scale * vp, 1.0f);
	if (mode > 0)
	{
		vCam -= vec4(0.03125f * normalize(vCam.xyz), 0.0f);
	}
	gl_Position = proj_matrix * vCam;
	
}