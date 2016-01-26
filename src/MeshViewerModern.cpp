#include "MeshViewerModern.h"


MeshViewerModern::MeshViewerModern(QWidget *parent)
	: QOpenGLWidget(parent)
	, vtx_vbo(oglBuffer::Type::VertexBuffer)
	, he_ibo(oglBuffer::Type::IndexBuffer)
	, face_ibo(oglBuffer::Type::IndexBuffer)
	, view_cam(QVector3D(4, 2, 4), QVector3D(0.0, 0.0, 0.0))
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
	// OpenGL extension initialization
	initializeOpenGLFunctions();

	// Print OpenGL version
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl; 
	cout << "OpenGL version supported " << glGetString(GL_VERSION) << endl;
	initShader();
	
	// Enable OpenGL features
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LEQUAL);
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
	face_solid_shader.addShaderFromSourceFile(oglShader::Vertex, "shaders/face_vs.glsl");
	face_solid_shader.addShaderFromSourceFile(oglShader::Fragment, "shaders/face_fs.glsl");
	face_solid_shader.addShaderFromSourceFile(oglShader::Geometry, "shaders/face_gs.glsl");
	face_solid_shader.link();
	//m_shader.bind();
	//cout << "Shader log:\n" << m_shader.log().constData();

	//////////////////////////////////////////////////////////////////////////
	edge_solid_shader.addShaderFromSourceFile(oglShader::Vertex, "shaders/edge_vs.glsl");
	edge_solid_shader.addShaderFromSourceFile(oglShader::Fragment, "shaders/edge_fs.glsl");
}

void MeshViewerModern::paintGL()
{
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0.6, 0.6, 0.6, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glCullFace(GL_BACK); // cull back face
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (mesh_changed)
	{
		bind();
	}
	face_vao.bind();
	// use shader
	face_solid_shader.bind();
	
	face_solid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
	face_solid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);
	glDrawElements(GL_TRIANGLES, fib_array.size(), GL_UNSIGNED_INT, 0);


	//glLineWidth(2.0);
	he_vao.bind();
	// use shader
	edge_solid_shader.bind();
	edge_solid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
	edge_solid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);
	glDrawElements(GL_LINES, heib_array.size(), GL_UNSIGNED_INT, 0);

	he_vao.release();
	edge_solid_shader.release();
}

void MeshViewerModern::resizeGL(int w, int h)
{
	view_cam.resizeViewport(w / static_cast<double>(h));
}

void MeshViewerModern::paintEvent(QPaintEvent *e)
{
	paintGL();
}

void MeshViewerModern::mousePressEvent(QMouseEvent* e)
{
	m_lastMousePos[0] = e->x();
	m_lastMousePos[1] = e->y();
}

void MeshViewerModern::mouseMoveEvent(QMouseEvent* e)
{
	
	int dx = e->x() - m_lastMousePos[0];
	int dy = e->y() - m_lastMousePos[1];

	if ((e->buttons() == Qt::LeftButton) && (e->modifiers() == Qt::AltModifier))
	{
		view_cam.rotate(dy * 0.25, dx * 0.25, 0.0);
		update();
	}
	else if ((e->buttons() == Qt::RightButton) && (e->modifiers() == Qt::AltModifier))
	{
		if (dx != e->x() && dy != e->y())
		{
			view_cam.zoom(0.0, 0.0, -dx * 0.05);
			update();
		}
	}
	else if ((e->buttons() == Qt::MidButton) && (e->modifiers() == Qt::AltModifier))
	{
		if (dx != e->x() && dy != e->y())
		{
			view_cam.zoom(dx * 0.05, dy * 0.05, 0.0);
			update();
		}
	}
	else
	{
		QOpenGLWidget::mouseMoveEvent(e);
	}

	m_lastMousePos[0] = e->x();
	m_lastMousePos[1] = e->y();
}
