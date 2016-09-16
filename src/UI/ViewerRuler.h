#pragma once
#include "glutils.hpp"
#include "common.h"
#include <QMatrix4x4>

class ViewerGrid
{
public:
	ViewerGrid(size_t seg, GLfloat scale, oglFuncs* f);
	~ViewerGrid();

	void destroy();
	void release();
	void bind();
	void draw(const QMatrix4x4 &proj, const QMatrix4x4 &view);
	void resize(size_t, GLfloat scale);
	void initShader();
private:
	oglFuncs* funcs;
	size_t segment;
	GLfloat size;

	//oglBuffer clr_vbo;
	oglBuffer vbo;
	oglBuffer ibo;
	oglVAO vao;
	vector<GLfloat> verts;
	//vector<GLfloat> grid_clr_array;
	vector<GLushort> indices;
	oglShaderP shader;
};


class ViewerRuler
{
public:
	ViewerRuler(oglFuncs* f);
	~ViewerRuler();

	void init();
private:
	oglFuncs* funcs;

	//oglBuffer clr_vbo;
	oglBuffer vbo;
	oglBuffer ibo;
	oglVAO vao;
	vector<GLfloat> verts;
	//vector<GLfloat> grid_clr_array;
	vector<GLushort> indices;
	oglShaderP shader;
};

