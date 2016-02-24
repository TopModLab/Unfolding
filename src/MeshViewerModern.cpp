#include "MeshViewerModern.h"
#include <QFileDialog>

MeshViewerModern::MeshViewerModern(QWidget *parent)
	: QOpenGLWidget(parent)
	, view_cam(QVector3D(4, 2, 4), QVector3D(0.0, 0.0, 0.0), QVector3D(0, 1, 0)
	, 54.3, 1.67, 1, 100)
	, vtx_vbo(oglBuffer::Type::VertexBuffer)
	, interactionState(Camera)
	, selectionState(SingleSelect)
	, shadingSate(FLAT)
	, heMesh(nullptr)
	, grid(4, 6.0f, this)
	, fRBO(this), heRBO(this)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
//	format.setSamples(16);
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);
}

MeshViewerModern::~MeshViewerModern()
{
}

void MeshViewerModern::bindHalfEdgeMesh(HDS_Mesh *mesh)
{
	heMesh = mesh;
	mesh_changed = true;
	
	update();
}


void MeshViewerModern::setInteractionMode(InteractionState state)
{
	interactionState = state;
	while (!selectedElementsIdxQueue.empty())
	{
		selectedElementsIdxQueue.pop();
	}
}

void MeshViewerModern::setSelectionMode(SelectionState mode)
{
	selectionState = mode;
}

void MeshViewerModern::initializeGL()
{
	// OpenGL extension initialization
	initializeOpenGLFunctions();

	// Print OpenGL version
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl; 
	cout << "OpenGL version supported " << glGetString(GL_VERSION) << endl;
	initShader();
	grid.bind();

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
		&fRBO.ibos, &fRBO.ids, &fRBO.flags,
		&heRBO.ibos, &heRBO.ids, &heRBO.flags);

	initialVBO();
	bindVertexVBO();
	bindFaceVAO();
	bindFaceTBO();
	bindEdgesVAO();
	bindEdgesTBO();

	mesh_changed = false;
}

void MeshViewerModern::initialVBO()
{
	vtx_vbo.destroy();
	heRBO.destroy();
	fRBO.destroy();
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
	heRBO.vao.create();
	heRBO.vao.bind();

	vtx_vbo.bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	heRBO.ibo.create();
	heRBO.ibo.setUsagePattern(oglBuffer::StaticDraw);
	heRBO.ibo.bind();
	
	heRBO.ibo.allocate(&heRBO.ibos[0], sizeof(GLuint) * heRBO.ibos.size());

	heRBO.vao.release();
	heRBO.ibo.release();
	vtx_vbo.release();
}

void MeshViewerModern::bindEdgesTBO()
{
	glGenTextures(2, heRBO.tex);
	glGenBuffers(2, heRBO.tbo);

	glBindBuffer(GL_TEXTURE_BUFFER, heRBO.tbo[0]);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(uint16_t) * heRBO.flags.size(), &heRBO.flags[0], GL_STATIC_DRAW);

	glBindBuffer(GL_TEXTURE_BUFFER, heRBO.tbo[1]);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(uint32_t) * heRBO.ids.size(), &heRBO.ids[0], GL_STATIC_DRAW);
	cout << "tbo[]:\t" << heRBO.tbo[0] << ", " << heRBO.tbo[1]
		<< "\tflagt: " << heRBO.flag_tbo << ", " << heRBO.id_tbo << endl;
	/*
	glGenTextures(1, &heRBO.flag_tex);
	//glBindTexture(GL_TEXTURE_BUFFER, fRBO.flag_tex);
	glGenBuffers(1, &heRBO.flag_tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, heRBO.flag_tbo);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(uint16_t) * heRBO.flags.size(), &heRBO.flags[0], GL_STATIC_DRAW);

	
	glGenTextures(1, &heRBO.id_tex);
	glGenBuffers(1, &heRBO.id_tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, heRBO.id_tbo);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(uint32_t) * heRBO.ids.size(), &heRBO.ids[0], GL_STATIC_DRAW);
*/
	//glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, heRBO.flag_tbo);
	//glBindTexture(GL_TEXTURE_BUFFER, 0);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void MeshViewerModern::bindFaceVAO()
{
	// Bind VAO
	fRBO.vao.create();
	fRBO.vao.bind();

	vtx_vbo.bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	fRBO.ibo.create();
	fRBO.ibo.setUsagePattern(oglBuffer::StaticDraw);
	fRBO.ibo.bind();
	fRBO.ibo.allocate(&fRBO.ibos[0], sizeof(GLuint) * fRBO.ibos.size());

	fRBO.vao.release();
	fRBO.ibo.release();
	vtx_vbo.release();

}

void MeshViewerModern::bindFaceTBO()
{
	glGenTextures(2, fRBO.tex);
	glGenBuffers(2, fRBO.tbo);

	glBindBuffer(GL_TEXTURE_BUFFER, fRBO.tbo[0]);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(uint16_t) * fRBO.flags.size(), &fRBO.flags[0], GL_STATIC_DRAW);

	glBindBuffer(GL_TEXTURE_BUFFER, fRBO.tbo[1]);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(uint32_t) * fRBO.ids.size(), &fRBO.ids[0], GL_STATIC_DRAW);
	/*
	glGenTextures(1, &fRBO.flag_tex);

	glGenBuffers(1, &fRBO.flag_tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, fRBO.flag_tbo);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(uint16_t) * fRBO.flags.size(), &fRBO.flags[0], GL_STATIC_DRAW);
*/
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void MeshViewerModern::initShader()
{
	//////////////////////////////////////////////////////////////////////////
	// Grid Shader
	grid.initShader();
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
	edge_solid_shader.link();
	//////////////////////////////////////////////////////////////////////////
	uid_shader.addShaderFromSourceFile(oglShader::Vertex, "shaders/uid_vs.glsl");
	uid_shader.addShaderFromSourceFile(oglShader::Fragment, "shaders/uid_fs.glsl");
	uid_shader.link();

}

void MeshViewerModern::initializeFBO()
{
	fbo.reset(new oglFBO(width(), height(), oglFBO::CombinedDepthStencil, GL_TEXTURE_2D));
//	selectionBuffer.resize(width()*height() * 4);
}

void MeshViewerModern::drawMeshToFBO()
{
	makeCurrent();
	fbo->bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind shader
	uid_shader.bind();
	uid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
	uid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);

	switch (interactionState)
	{
	case InteractionState::SelectVertex:
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		fRBO.vao.bind();
		uid_shader.setUniformValue("depthonly", true);
		glDrawElements(GL_TRIANGLES, fRBO.ibos.size(), GL_UNSIGNED_INT, 0);

		vtx_vbo.bind();
		glPointSize(15.0);
		uid_shader.setUniformValue("depthonly", false);
		glDrawArrays(GL_POINTS, 0, vtx_array.size());
		vtx_vbo.release();
		//draw vertices
		break;
	}
	case InteractionState::SelectFace:
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		fRBO.vao.bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, fRBO.id_tex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, fRBO.id_tbo);
		uid_shader.setUniformValue("id_tex", 0);
		uid_shader.setUniformValue("depthonly", false);

		glDrawElements(GL_TRIANGLES, fRBO.ibos.size(), GL_UNSIGNED_INT, 0);
		
		fRBO.vao.release();
		glBindTexture(GL_TEXTURE_BUFFER, 0);
		//draw faces
		break;
	}
	case InteractionState::SelectEdge:
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		fRBO.vao.bind();
		uid_shader.setUniformValue("depthonly", true);
		glDrawElements(GL_TRIANGLES, fRBO.ibos.size(), GL_UNSIGNED_INT, 0);

		glLineWidth(10.0);
		heRBO.vao.bind();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		uid_shader.setUniformValue("depthonly", false);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, heRBO.id_tex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, heRBO.id_tbo);
		uid_shader.setUniformValue("id_tex", 0);


		glDrawElements(GL_LINES, heRBO.ibos.size(), GL_UNSIGNED_INT, 0);
		
		heRBO.vao.release();
		glBindTexture(GL_TEXTURE_BUFFER, 0);
		//draw edges
		break;
	}
	default: break;
	}

	fbo->release();
	uid_shader.release();

	auto fboRes = fbo->toImage();
	//fboRes.save("fbo.png");
	QRgb pixel = fboRes.pixel(mouseState.x, mouseState.y);
	if ((pixel >> 24) == 0)
	{
		selectionID.selID = 0;
		cout << "no object selected" << endl;
		return;
	}
	size_t renderID = pixel & 0xFFFFFF;
	unordered_set<int> settest;
	switch (interactionState)
	{
	case InteractionState::SelectVertex:
		selectionID.vertexID = renderID << 2
			| static_cast<size_t>(DataTypeMark::VERTEX_MARK);
		break;
	case InteractionState::SelectFace:
		selectionID.faceID = renderID << 2
			| static_cast<size_t>(DataTypeMark::FACE_MARK);
		break;
	case InteractionState::SelectEdge:
		selectionID.edgeID = renderID << 2
			| static_cast<size_t>(DataTypeMark::EDGE_MARK);
		break;
	}
	cout << "draw primitive id:" << (selectionID.selID >> 2) << endl;
}

void MeshViewerModern::paintGL()
{
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0.6, 0.6, 0.6, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	grid.draw(view_cam.CameraToScreen, view_cam.WorldToCamera);
	
	if (heMesh != nullptr)
	{
		// Selection mode
		/*switch (interactionState)
		{
		case MeshViewerModern::Camera:
			break;
		case MeshViewerModern::Camera_Translation:
			break;
		case MeshViewerModern::Camera_Zoom:
			break;
		case MeshViewerModern::SelectVertex:
			break;
		case MeshViewerModern::SelectFace:
			break;
		case MeshViewerModern::SelectEdge:
			break;
		default:
			drawMeshToFBO();
			break;
		}*/
		// Draw Mesh
		if (true)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			if (mesh_changed)
			{
				bind();
			}
			fRBO.vao.bind();
			// use shader
			face_solid_shader.bind();
			face_solid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
			face_solid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);
			// Bind Texture Buffer
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_BUFFER, fRBO.flag_tex);
			glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, fRBO.flag_tbo);
			face_solid_shader.setUniformValue("flag_tex", 0);
			glDrawElements(GL_TRIANGLES, fRBO.ibos.size(), GL_UNSIGNED_INT, 0);
			fRBO.vao.release();
			
			// Draw edge
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(1.0);
			heRBO.vao.bind();
			edge_solid_shader.bind();
			edge_solid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
			edge_solid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);
			// Bind Texture Buffer
			glBindTexture(GL_TEXTURE_BUFFER, heRBO.flag_tex);
			glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, heRBO.flag_tbo);
			edge_solid_shader.setUniformValue("flag_tex", 0);
			glDrawElements(GL_LINES, heRBO.ibos.size(), GL_UNSIGNED_INT, 0);
			heRBO.vao.release();

			edge_solid_shader.release();
			glBindTexture(GL_TEXTURE_BUFFER, 0);
		}
	}
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
		view_cam.translation.setY(view_cam.translation.y() - numSteps);
		view_cam.updateModelView();*/
		break;
	}
	case  Qt::Key_Up:
	{
		/*double numSteps = .20f;
		view_cam.translation.setY(view_cam.translation.y() + numSteps);
		view_cam.updateModelView();*/
		break;
	}
	case Qt::Key_Left:
	{
		/*double numSteps = .20f;
		view_cam.translation.setX(view_cam.translation.x() - numSteps);
		view_cam.updateModelView();*/
		break;
	}
	case Qt::Key_Right:
	{
		/*double numSteps = .20f;
		view_cam.translation.setX(view_cam.translation.x() + numSteps);
		view_cam.updateModelView();*/
		break;
	}
	case Qt::Key_P:
	{
		QString filename = QFileDialog::getSaveFileName(
				this, "Save Screenshot file...", "default", tr("PNG(*.png)"));
		this->grab().save(filename);
		
		break;
	}
	}
}

void MeshViewerModern::keyReleaseEvent(QKeyEvent* e)
{
}

void MeshViewerModern::mousePressEvent(QMouseEvent* e)
{
	mouseState.x = e->x();
	mouseState.y = e->y();

	if (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::NoModifier
		&& interactionState != Camera)
	{
		drawMeshToFBO();
		update();
	}
}

void MeshViewerModern::mouseMoveEvent(QMouseEvent* e)
{
	
	int dx = e->x() - mouseState.x;
	int dy = e->y() - mouseState.y;

	if (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::AltModifier)
	{
		view_cam.rotate(dy * 0.25, dx * 0.25, 0.0);
		update();
	}
	else if (e->buttons() == Qt::RightButton && e->modifiers() == Qt::AltModifier)
	{
		if (dx != e->x() && dy != e->y())
		{
			view_cam.zoom(0.0, 0.0, -dx * 0.05);
			update();
		}
	}
	else if (e->buttons() == Qt::MidButton && e->modifiers() == Qt::AltModifier)
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

void MeshViewerModern::mouseReleaseEvent(QMouseEvent* e)
{
	/*
	switch (interactionState)
	{
	case Camera:
	case Camera_Translation:
	case Camera_Zoom:

		mouseState.prev_pos = QVector2D(e->pos());
		break;

		//selection box
	case SelectFace:
	case SelectEdge:
	case SelectVertex: {
		sbox.corner_win[2] = e->x();
		sbox.corner_win[3] = view_cam.viewport.h - e->y();

		//kkkkkkkkkkkkkkk
		//computeGlobalSelectionBox();
		isSelecting = false;
		mouseState.isPressed = false;   //later added
		cout << "releasing mousestate" << mouseState.isPressed << endl;
		int selectedElementIdx = getSelectedElementIndex(e->pos());//mouse positon;
		cout << "selected element " << selectedElementIdx << endl;
		if (selectedElementIdx >= 0) {
			//for testing
			if (interactionState == SelectEdge){
				cout << "select edge's face index: " << heMesh->heMap[selectedElementIdx]->f->index << endl;
				cout << "select edge flip's face index: " << heMesh->heMap[selectedElementIdx]->flip->f->index << endl;
			}
			if (interactionState == SelectFace) {
				cout << "select face is cut face? " << heMesh->faceMap[selectedElementIdx]->isCutFace << endl;
				cout << "select face is bridge? " << heMesh->faceMap[selectedElementIdx]->isBridger << endl;

			}
			selectedElementsIdxQueue.push(selectedElementIdx);

			switch (selectionMode) {
			case SingleSelect:
				if (selectedElementsIdxQueue.size() > 1) {
					if (selectedElementsIdxQueue.front() != selectedElementsIdxQueue.back()) {
						if (interactionState == SelectEdge) {
							heMesh->heMap[selectedElementsIdxQueue.front()]->setPicked(false);//deselect
						}
						else if (interactionState == SelectFace) {
							heMesh->faceMap[selectedElementsIdxQueue.front()]->setPicked(false);//deselect
						}
						else if (interactionState == SelectVertex){
							heMesh->vertMap[selectedElementsIdxQueue.front()]->setPicked(false);//deselect
						}
					}
					selectedElementsIdxQueue.pop();

				}

			case multiple:
				if (interactionState == SelectEdge) {
					heMesh->selectEdge(selectedElementIdx);
				}
				else if (interactionState == SelectFace) {
					heMesh->selectFace(selectedElementIdx);
				}
				else if (interactionState == SelectVertex){
					heMesh->selectVertex(selectedElementIdx);
					if (isCriticalPointModeSet)
						findReebPoints();
					if (isCutLocusModeset)
						findCutLocusPoints();
				}
				break;

			}
		}
		else
			return;


		break;
	}
	}
	mouseState.isPressed = false;
	//cout<<"releasing mousestate"<<mouseState.isPressed<<endl; //later added;
	/// reset interaction mode if in camera mode triggered by holding alt
	if (e->modifiers() & Qt::AltModifier) {
		interactionState = interactionStateStack.top();
		interactionStateStack.pop();
	}

	update();
	*/
}

void MeshViewerModern::wheelEvent(QWheelEvent* e)
{
	view_cam.zoom(0, 0, -e->delta() * 0.01);
	update();
}

void MeshViewerModern::selectAll()
{
	switch (interactionState) {

	case SelectFace:
		for (auto f : heMesh->faces())
			f->setPicked(true);
		break;
	case SelectEdge:
		for (auto e : heMesh->halfedges())
			e->setPicked(true);
		break;
	case SelectVertex:
		for (auto v : heMesh->verts())
			v->setPicked(true);
		break;
	default:
		break;
	}
	update();
}

void MeshViewerModern::selectInverse()
{
	switch (interactionState)
	{
	case SelectFace:
	{
		for (auto f : heMesh->faces())
			heMesh->selectFace(f->index);
		break;
	}
	case SelectEdge:
	{
		unordered_set<HDS_HalfEdge*> selected = heMesh->getSelectedHalfEdges();

		for (auto e : heMesh->halfedges())
		{
			if (selected.find(e) != selected.end())
				e->setPicked(false);
			else
				e->setPicked(true);
		}
		break;
	}
	case SelectVertex:
	{
		for (auto v : heMesh->verts())
			heMesh->selectVertex(v->index);
		break;
	}
	default:
		break;
	}
	update();
}

void MeshViewerModern::resetCamera()
{
	view_cam = perspCamera(QVector3D(4, 2, 4), QVector3D(0.0, 0.0, 0.0));
}
