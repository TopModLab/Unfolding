#version 330

in layout(location = 0) vec3 vp;
uniform int mode = 0;

uniform mat4 view_matrix, proj_matrix;
uniform float scale = 1.f;

void main()
{
	vec4 vCam = view_matrix * vec4(scale * vp, 1.0f);
	if (mode > 0)
	{
		vCam -= vec4(0.03125 * normalize(vCam.xyz), 0.f);
	}
	gl_Position = proj_matrix * vCam;
	
}