#include "MeshViewerModern.h"

MeshViewerModern::MeshViewerModern(QWidget *parent)
	: QOpenGLWidget(parent)
	, vtx_vbo(oglBuffer::Type::VertexBuffer)
	, he_ibo(oglBuffer::Type::IndexBuffer)
	, face_ibo(oglBuffer::Type::IndexBuffer)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setSamples(16);
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);

}

MeshViewerModern::~MeshViewerModern()
{
}

void MeshViewerModern::bindHalfEdgeMesh(HDS_Mesh *mesh)
{
	//makeCurrent();
	heMesh = mesh;
	mesh_changed = true;
	

	update();
}


void MeshViewerModern::initializeGL()
{
	// OpenGL extention initialization
	initializeOpenGLFunctions();

	// Print OpenGL vertion
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl; 
	cout << "OpenGL version supported " << glGetString(GL_VERSION) << endl;
	initShader();

	// Enable OpenGL features
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front
}


void MeshViewerModern::bind()
{
	heMesh->exportVBO(&vtx_array,
		&fib_array, &fid_array, &fflag_array,
		&heib_array, &heid_array, &heflag_array);

	initialVBO();
	bindVertexVBO();
	bindFaceVAO();
	bindEdgesVAO();

	mesh_changed = false;
}

void MeshViewerModern::initialVBO()
{
	vtx_vbo.destroy();
	face_ibo.destroy();
	he_ibo.destroy();

	// OpenGL 4.3 Core Profile
	he_vao.destroy();
	face_vao.destroy();
	/*glDeleteVertexArrays(1, &he_vao);
	glDeleteVertexArrays(1, &face_vao);*/
}

void MeshViewerModern::bindVertexVBO()
{
	vtx_vbo.create();
	vtx_vbo.setUsagePattern(oglBuffer::StaticDraw);
	vtx_vbo.bind();
	vtx_vbo.allocate(&vtx_array[0], sizeof(GLfloat) * vtx_array.size());
	vtx_vbo.release();
}

void MeshViewerModern::bindEdgesVAO()
{
	// Bind VAO
	he_vao.create();
	he_vao.bind();

	vtx_vbo.bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	he_ibo.create();
	he_ibo.setUsagePattern(oglBuffer::StaticDraw);
	he_ibo.bind();
	he_ibo.allocate(&heib_array[0], sizeof(GLuint) * heib_array.size());

	he_vao.release();
	he_ibo.release();
	vtx_vbo.release();
}

void MeshViewerModern::bindFaceVAO()
{
	// Bind VAO
	face_vao.create();
	face_vao.bind();

	vtx_vbo.bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	face_ibo.create();
	face_ibo.setUsagePattern(oglBuffer::StaticDraw);
	face_ibo.bind();
	face_ibo.allocate(&fib_array[0], sizeof(GLuint) * fib_array.size());

	face_vao.release();
	face_ibo.release();
	vtx_vbo.release();
}
void MeshViewerModern::initShader()
{
	m_shader.addShaderFromSourceFile(oglShader::Vertex, "shaders/vert.glsl");
	m_shader.addShaderFromSourceFile(oglShader::Fragment, "shaders/frag.glsl");
	m_shader.addShaderFromSourceFile(oglShader::Geometry, "shaders/geom.glsl");
	m_shader.link();
	//m_shader.bind();
	cout << "Shader log:\n" << m_shader.log().constData();
}

void MeshViewerModern::paintGL()
{
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glCullFace(GL_BACK); // cull back face
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (mesh_changed)
	{
		bind();
	}
	face_vao.bind();
	// use shader
	m_shader.bind();
	glDrawElements(GL_TRIANGLES, fib_array.size(), GL_UNSIGNED_INT, 0);

	/*he_vao.bind();
	// use shader
	glDrawElements(GL_TRIANGLES, heib_array.size(), GL_UNSIGNED_INT, 0);*/

	he_vao.release();
}

void MeshViewerModern::resizeGL(int w, int h)
{

}

void MeshViewerModern::paintEvent(QPaintEvent *e)
{
	paintGL();
}