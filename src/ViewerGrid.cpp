#include "ViewerGrid.h"

const char* gridvs =
"#version 330\n"\
"in layout(location = 0) vec2 vp;"\
"uniform mat4 view, proj;"\
"uniform float size;"\
"void main() {"\
"	gl_Position = proj * view * vec4(vp.x * size, 0.0, vp.y * size, 1.0);"\
"}";

const char* gridfs =
"#version 330\n"\
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
	shader.setUniformValue("size", size);
	glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_SHORT, 0);

	vao.release();
	shader.release();
}

void ViewerGrid::resize(size_t seg, GLfloat scale)
{
	segment = seg;
	size = scale;

	// verts number: 8 * seg
	verts.clear();
	verts.insert(verts.end(), segment << 4, -1.0);
	// indices number: 4 * seg + 2
	indices.clear();
	indices.reserve((segment << 3) + 4);

	GLfloat segsize = 1.0 / static_cast<float>(segment);
	size_t edgeVertsNum = segment << 1;
	//size_t idxOff = segment << 2;// Verts index offset
	size_t offset[] = { segment << 2, segment << 3, (segment << 3) + (segment << 2)};
	size_t vidx = 0;
	for (size_t i = 0; i < edgeVertsNum; i++)
	{
		// Bottom
		verts[vidx] = segsize * i - 1.0f;

		// Left
		verts[offset[0]++] = -verts[vidx + 1];
		verts[offset[0]++] = verts[vidx];

		// Top
		verts[offset[1]++] = -verts[vidx];
		verts[offset[1]++] = -verts[vidx + 1];

		// Right
		verts[offset[2]++] = verts[vidx + 1];
		verts[offset[2]++] = -verts[vidx];
		
		vidx += 2;
	}
	//////////////////////////////////////////////////////////////////////////
	//offset[0] = edgeVertsNum;
	offset[1] = edgeVertsNum << 1;
	offset[2] = (edgeVertsNum << 1) + edgeVertsNum;
	for (size_t i = 0; i < edgeVertsNum + 1; i++)
	{
		// Vertical Line
		indices.push_back(i);
		indices.push_back(offset[2] - i);
		// Horizontal Line
		indices.push_back(offset[1] - i);
		indices.push_back(offset[2] + i);
	}
	// Fix index over range
	indices.back() = 0;
}

void ViewerGrid::initShader()
{
	shader.addShaderFromSourceCode(oglShader::Vertex, gridvs);
	shader.addShaderFromSourceCode(oglShader::Fragment, gridfs);

	shader.link();
}
