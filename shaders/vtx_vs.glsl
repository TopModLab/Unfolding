#version 330
in layout(location = 0) vec3 vp;// vertex position
//in layout(location = 1) uint vf;// vertex flag
uniform mat4 view_matrix, proj_matrix;
uniform float scale = 1.f;

uniform usamplerBuffer flag_tex;
flat out uint flag;

void main()
{
	vec4 vCam = view_matrix * vec4(scale * vp, 1.0f);
	vCam -= vec4(0.03125 * normalize(vCam.xyz), 0.f);
	gl_Position = proj_matrix * vCam;

	flag = texelFetch(flag_tex, gl_VertexID).r;
	/*if (vf == uint(2))
	{
		gl_PointSize *= 2;
	}*/
}