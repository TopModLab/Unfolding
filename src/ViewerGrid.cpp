#include "ViewerGrid.h"

const char* gridvs =
"#version 400\n"\
"in layout(location = 0) vec2 vp;"\
"uniform mat4 view, proj;"\
"void main() {"\
"	gl_Position = proj * view * vec4(vp.x, 0.0, vp.y, 1.0);"\
"}";

const char* gridfs =
"#version 400\n"\
"out vec4 frag_color;"\
"void main() {"\
"	frag_color = vec4(0.5,0.5,0.5, 1.0);"\
"}";


ViewerGrid::ViewerGrid(size_t seg, GLfloat scale, oglFuncs* f)
	: funcs(f)
	, vbo(oglBuffer::Type::VertexBuffer)
	, ibo(oglBuffer::Type::IndexBuffer)
{
	resize(seg, scale);
}


ViewerGrid::~ViewerGrid()
{
	destroy();
}

void ViewerGrid::destroy()
{
	vbo.destroy();
	ibo.destroy();
	vao.destroy();
	shader.removeAllShaders();
}

void ViewerGrid::release()
{
	vao.release();
	ibo.release();
	vbo.release();
	shader.release();
}

void ViewerGrid::bind()
{
	vao.create();
	vao.bind();

	vbo.create();
	vbo.setUsagePattern(oglBuffer::StaticDraw);
	vbo.bind();
	vbo.allocate(&verts[0], sizeof(GLfloat) * verts.size());
	funcs->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	funcs->glEnableVertexAttribArray(0);

	ibo.create();
	ibo.setUsagePattern(oglBuffer::StaticDraw);
	ibo.bind();
	ibo.allocate(&indices[0], sizeof(GLushort) * indices.size());

	vao.release();
	ibo.release();
	vbo.release();
}

void ViewerGrid::draw(const QMatrix4x4 &proj, const QMatrix4x4 &view)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1.0);
	vao.bind();
	shader.bind();
	shader.setUniformValue("proj", proj);
	shader.setUniformValue("view", view);
	glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_SHORT, 0);

	vao.release();
	shader.release();
}

void ViewerGrid::resize(size_t seg, GLfloat scale)
{
	segment = seg;
	size = scale;

	size_t twoseg = static_cast<size_t>(segment) << 1;
	size_t foursegsq = twoseg * twoseg;
	// verts number: 4 * (seg + 1)
	size_t vertCount = (segment + 1) << 2;
	// indices number: (2seg + 1) * 2
	size_t indexCount = (segment << 2) + 2;

	verts.clear();
	verts.insert(verts.end(), vertCount << 1, -scale);
	indices.clear();
	indices.reserve(indexCount << 1);

	GLfloat segsize = scale / static_cast<float>(segment);
	size_t vidx = 0;
	for (size_t i = -segment; i < segment; i++, vidx += 2)
	{
		verts[vidx] = -segsize * i;
	}
	size_t offset = segment << 1;

	for (size_t i = 0; i < 3 * offset; i++)
	{
		verts[vidx++] = -verts[vidx - offset + 1];
		verts[vidx++] = verts[vidx - offset - 1];
	}
	////
	vidx = 0;
	for (size_t i = 0; i < twoseg + 1; i++)
	{
		for (size_t j = 0; j < twoseg; j++)
		{
			indices.push_back(vidx++);
			indices.push_back(vidx);
		}
		vidx++;
	}
}

void ViewerGrid::initShader()
{
	shader.addShaderFromSourceCode(oglShader::Vertex, gridvs);
	shader.addShaderFromSourceCode(oglShader::Fragment, gridfs);

	shader.link();
}
