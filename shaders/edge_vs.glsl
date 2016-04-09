#version 330
in layout(location = 0) vec3 vp;

uniform mat4 view_matrix, proj_matrix;
//uniform mat4 view_matrix;

void main()
{
	//gl_Position = proj_matrix * view_matrix * vec4(vp, 1.0f);
	vec4 vCam = view_matrix * vec4(vp, 1.0f);
	vCam -= vec4(0.03125 * normalize(vCam.xyz), 0.f);
	gl_Position = proj_matrix * vCam;
	//gl_Position.z -= 0.005f;
}