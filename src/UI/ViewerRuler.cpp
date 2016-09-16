#include "ViewerRuler.h"

const char* ruldervs =
"#version 330\n"
"in layout(location = 0) vec2 vp;"
"uniform mat4 view, proj;"
"uniform float size;"
"void main() {"
"	gl_Position = proj * view * vec4(vp.x * size, 0.0, vp.y * size, 1.0);"
"}";

const char* rulerfs =
"#version 330\n"
"out vec4 frag_color;"
"void main() {"
"	frag_color = vec4(0.5,0.5,0.5, 1.0);"
"}";

ViewerRuler::ViewerRuler(oglFuncs * f)
{
}

ViewerRuler::~ViewerRuler()
{
}

void ViewerRuler::init()
{
	shader.addShaderFromSourceCode(oglShader::Vertex, ruldervs);
	shader.addShaderFromSourceCode(oglShader::Fragment, rulerfs);

	shader.link();
}
