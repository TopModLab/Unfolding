#include "MeshViewerModern.h"


MeshViewerModern::MeshViewerModern(QWidget *parent)
	: QOpenGLWidget(parent)
	, vtx_vbo(oglBuffer::Type::VertexBuffer)
	, he_ibo(oglBuffer::Type::IndexBuffer)
	, face_ibo(oglBuffer::Type::IndexBuffer)
	, view_cam(QVector3D(4, 2, 4), QVector3D(0.0, 0.0, 0.0), QVector3D(0,1,0)
	, 45, 1.67, 1, 100)
//	, grid_vtx_array(29, 0.0)
	, grid_clr_array(29, 0.2)
//	, grid_idx_array(12, 0)
	, grid_vtx_vbo(oglBuffer::Type::VertexBuffer)
	, grid_clr_vbo(oglBuffer::Type::VertexBuffer)
	, grid_ibo(oglBuffer::Type::IndexBuffer)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
//	format.setSamples(16);
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);
	
	grid_vtx_array = {
		-1, 0, -1,		0, 0, -1,		1, 0, -1,
		-1, 0, 0,		0, 0, 0,		1, 0, 0,
		-1, 0, 1,		0, 0, 1,		1, 0, 1
	};
	/*grid_clr_array = {
		-1, 0, -1, 0, 0, -1, 1, 0, -1,
		-1, 0, 0, 0, 0, 0, 1, 0, 0,
		- 1, 0, 1, 0, 0, 1, 1, 0, 1
	};*/
	grid_idx_array = { 0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8,
		0, 3, 3, 6, 1, 4, 4, 7, 2, 5, 5, 8 };

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
	bindGrid();

	// Enable OpenGL features
	glEnable(GL_MULTISAMPLE);
//	glEnable(GL_LINE_SMOOTH);
//	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LEQUAL);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front
//	glEnable(GL_CULL_FACE);
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

void MeshViewerModern::bindGrid()
{
	grid_vao.create();
	grid_vao.bind();

	grid_vtx_vbo.create();
	grid_vtx_vbo.setUsagePattern(oglBuffer::StaticDraw);
	grid_vtx_vbo.bind();
	grid_vtx_vbo.allocate(&grid_vtx_array[0], sizeof(GLfloat) * grid_vtx_array.size());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	grid_clr_vbo.create();
	grid_clr_vbo.setUsagePattern(oglBuffer::StaticDraw);
	grid_clr_vbo.bind();
	grid_clr_vbo.allocate(&grid_clr_array[0], sizeof(GLfloat) * grid_clr_array.size()); 
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	grid_ibo.create();
	grid_ibo.setUsagePattern(oglBuffer::StaticDraw);
	grid_ibo.bind();
	grid_ibo.allocate(&grid_idx_array[0], sizeof(GLushort) * grid_idx_array.size());
	
	grid_vao.release();
	grid_ibo.release();
	grid_vtx_vbo.release();
	grid_clr_vbo.release();
}

void MeshViewerModern::drawGrid()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1.0);
	grid_vao.bind();
	grid_shader.bind();
	grid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
	grid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);
	glDrawElements(GL_LINES, grid_idx_array.size(), GL_UNSIGNED_SHORT, 0);

	grid_vao.release();
	grid_shader.release();
}

void MeshViewerModern::initShader()
{
	//////////////////////////////////////////////////////////////////////////
	// Grid Shader
	grid_shader.addShaderFromSourceFile(oglShader::Vertex, "shaders/grid_vs.glsl");
	grid_shader.addShaderFromSourceFile(oglShader::Fragment, "shaders/grid_fs.glsl");

	//////////////////////////////////////////////////////////////////////
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

void MeshViewerModern::initializeFBO()
{
	fbo.reset(new oglFBO(width(), height(), oglFBO::CombinedDepthStencil, GL_TEXTURE_2D));
//	selectionBuffer.resize(width()*height() * 4);
}

void MeshViewerModern::drawMeshToFBO()
{
	fbo->bind();
	switch (m_interactionState)
	{
	case SelectVertex:
		//draw vertices
		break;
	case SelectFace:
		//draw faces
		break;
	case SelectEdge:
		//draw edges
		break;
	default: break;
	}
	fbo->release();

	QRgb pixel = fbo->toImage().pixel(mouseState.x, mouseState.y);
	size_t renderID = pixel;//convert rgba to 
	switch (m_interactionState)
	{
	case SelectVertex:
		selectionID.vertexID = renderID << 2 | VERTEX_MARK;
		break;
	case SelectFace:
		selectionID.faceID = renderID << 2 | FACE_MARK;
		break;
	case SelectEdge:
		selectionID.edgeID = renderID << 2 | EDGE_MARK;
		break;
	default:
		selectionID.vertexID = NULL_MARK;
		break;
	}
}

void MeshViewerModern::paintGL()
{
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0.6, 0.6, 0.6, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawGrid();
	
	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // cull back face*/
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//	glDepthFunc(GL_LEQUAL);
//	glDisable(GL_MULTISAMPLE);
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

	glLineWidth(2.0);
//	glDisable(GL_MULTISAMPLE);
//	glDisable(GL_DEPTH_TEST);
//	glDisable(GL_CULL_FACE);
//	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	he_vao.bind();
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

	initializeFBO();
}

void MeshViewerModern::paintEvent(QPaintEvent* e)
{
	paintGL();
}

void MeshViewerModern::keyPressEvent(QKeyEvent* e)
{
	switch (e->key()) {
	case Qt::Key_C:
	{
		/*int lev0 = cp_smoothing_times / 10;
		int lev1 = lev0 + 1;
		const int maxLev = 10;
		if (lev1 > 10) return;

		double ratio = (cp_smoothing_times - lev0) / (double)(lev1 - lev0);
		//emit updateMeshColorByGeoDistance(lastSelectedIndex, lev0, lev1, ratio);
		emit updateMeshColorByGeoDistance(lastSelectedIndex); //changed to non-smoothing method due to bug in smoothed method*/

		break;
	}
	//    case Qt::Key_E:
	//    {
	//        if (heMesh) {
	//            heMesh->flipShowEdges();
	//        }
	//        break;
	//    }
	//    case Qt::Key_V:
	//    {
	//        if (heMesh) {
	//            heMesh->flipShowVertices();
	//        }
	//        break;
	//    }
	//    case Qt::Key_F:
	//    {
	//        if (heMesh) {
	//            heMesh->flipShowFaces();
	//        }
	//        break;
	//    }
	//    case Qt::Key_L:
	//    {
	//        enableLighting = !enableLighting;
	//        break;
	//    }
	//    case Qt::Key_R:
	//    {
	//        toggleCriticalPoints();
	//        break;
	//    }
	case Qt::Key_F:
	{
		resetCamera();
		break;
	}
	case Qt::Key_M:
	{/*
		//loop through all critical point modes
		cmode = CriticalPointMode((cmode + 1) % NCModes);
		findReebPoints();*/
		break;
	}
	case Qt::Key_Down:
	{
		/*double numSteps = .20f;
		viewerState.translation.setY(viewerState.translation.y() - numSteps);
		viewerState.updateModelView();*/
		break;
	}
	case  Qt::Key_Up:
	{
		/*double numSteps = .20f;
		viewerState.translation.setY(viewerState.translation.y() + numSteps);
		viewerState.updateModelView();*/
		break;
	}
	case Qt::Key_Left:
	{
		/*double numSteps = .20f;
		viewerState.translation.setX(viewerState.translation.x() - numSteps);
		viewerState.updateModelView();*/
		break;
	}
	case Qt::Key_Right:
	{
		/*double numSteps = .20f;
		viewerState.translation.setX(viewerState.translation.x() + numSteps);
		viewerState.updateModelView();*/
		break;
	}

	}
	update();
}

void MeshViewerModern::mousePressEvent(QMouseEvent* e)
{
	mouseState.x = e->x();
	mouseState.y = e->y();
}

void MeshViewerModern::mouseMoveEvent(QMouseEvent* e)
{
	
	int dx = e->x() - mouseState.x;
	int dy = e->y() - mouseState.y;

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

	mouseState.x += dx;
	mouseState.y += dy;
}

void MeshViewerModern::resetCamera()
{
	view_cam = perspCamera(QVector3D(4, 2, 4), QVector3D(0.0, 0.0, 0.0));
}
