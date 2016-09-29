#include "UI/MeshViewer.h"
#include <QFileDialog>
#include "GeomProc/meshunfolder.h"
#include "UI/SelectByRefIdPanel.h"
//MeshViewer* MeshViewer::instance = nullptr;


MeshViewer::MeshViewer(QWidget *parent)
	: QOpenGLWidget(parent)
	, interactionState(ROAM_CAMERA)
	, heMesh(nullptr)
	, renderFlag(0)
	, shadingSate(SHADE_WF_FLAT)
	, dispComp(DispComp::DISP_GRID)
	, hlComp(HighlightComp::HIGHLIGHT_CUTEDGE)
	, grid(4, 6.0f, this)
	, view_cam(QVector3D(4, 2, 4), QVector3D(0, 0, 0),
		QVector3D(0, 1, 0), 54.3, 1.67, 0.1, 100)
	, mesh_changed(false), view_scale(1.0)
	, vRBO(this), fRBO(this), heRBO(this)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	// To enable anti-aliasing, set sample to sampleNumber^2
	//format.setSamples(9);
	format.setVersion(3, 3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);

	//
	fRBO.vbo = heRBO.vbo = vRBO.vbo = make_shared<oglBuffer>(oglBuffer::Type::VertexBuffer);
}

MeshViewer::~MeshViewer()
{
}

void MeshViewer::bindHalfEdgeMesh(HDS_Mesh *mesh)
{
	heMesh = mesh;
	mesh_changed = true;
	
	update();
}


void MeshViewer::setInteractionMode(InteractionState state)
{
	interactionState = state;
}

void MeshViewer::showShading(ShadingState shading)
{
	shadingSate ^= shading;
	update();
}

void MeshViewer::showComp(DispComp comp)
{
	dispComp ^= comp;
	update();
}

void MeshViewer::highlightComp(HighlightComp comp)
{
	hlComp ^= comp;
	update();
}

void MeshViewer::initializeGL()
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
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LEQUAL);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front
//	glEnable(GL_CULL_FACE);

	//////////////////////////////////////////////////////////////////////////
	// Init Vertex VAO
	vRBO.vao.create();
	vRBO.vao.bind();

	vRBO.vbo->create();
	vRBO.vbo->setUsagePattern(oglBuffer::StreamDraw);
	vRBO.vbo->bind();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	vRBO.releaseAll();

	// Init face & edge VAO
	auto createPrimitiveBuffers = [this](RenderBufferObject &RBO)
	{
		// Bind VAO
		RBO.vao.create();
		RBO.vao.bind();

		RBO.vbo->bind();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		RBO.ibo.create();
		RBO.ibo.setUsagePattern(oglBuffer::StaticDraw);
		RBO.ibo.bind();

		RBO.releaseAll();
	};
	createPrimitiveBuffers(fRBO);
	createPrimitiveBuffers(heRBO);
	//////////////////////////////////////////////////////////////////////////
	// TBO
	glGenTextures(1, vRBO.tex);
	glGenBuffers(1, vRBO.tbo);
	glGenTextures(2, heRBO.tex);
	glGenBuffers(2, heRBO.tbo);
	glGenTextures(2, fRBO.tex);
	glGenBuffers(2, fRBO.tbo);
}

void MeshViewer::allocateGL()
{
	// Clear Selection
	while (!selVTX.empty()) selVTX.pop();
	while (!selHE.empty()) selHE.pop();
	while (!selFACE.empty()) selFACE.pop();
	heMesh->exportSelection(&selVTX, &selHE, &selFACE);

	heMesh->exportVertVBO(&vtx_array, &vRBO.flags);
	heMesh->exportEdgeVBO(&heRBO.ibos, &heRBO.ids, &heRBO.flags);
	heMesh->exportFaceVBO(&fRBO.ibos, &fRBO.ids, &fRBO.flags);
	vtx_array.shrink_to_fit();
	vRBO.shrink_to_fit();
	heRBO.shrink_to_fit();
	fRBO.shrink_to_fit();

	// Bind Vertices Buffer
	vRBO.vbo->bind();
	if (!vtx_array.empty())
	{
		vRBO.vbo->allocate(&vtx_array[0], sizeof(GLfloat) * vtx_array.size());
	}
	vRBO.vbo->release();
	vRBO.allocateTBO(1);// Bind only flag tbo
	
	// Bind Face Buffers
	fRBO.allocateIBO();
	fRBO.allocateTBO();
	
	// Bind Edge Buffers
	heRBO.allocateIBO();
	heRBO.allocateTBO();

	mesh_changed = false;
}

void MeshViewer::initShader()
{
	//////////////////////////////////////////////////////////////////////////
	// Grid Shader
	grid.initShader();
#ifdef _DEBUG
	QString rcDir = "";
#else
	QString rcDir = ":";
#endif
	//////////////////////////////////////////////////////////////////////
	face_solid_shader.addShaderFromSourceFile(oglShader::Vertex, rcDir + "shaders/face_vs.glsl");
	face_solid_shader.addShaderFromSourceFile(oglShader::Fragment, rcDir + "shaders/face_fs.glsl");
	face_solid_shader.addShaderFromSourceFile(oglShader::Geometry, rcDir + "shaders/face_gs.glsl");
	face_solid_shader.link();

	//////////////////////////////////////////////////////////////////////////
	edge_solid_shader.addShaderFromSourceFile(oglShader::Vertex, rcDir + "shaders/edge_vs.glsl");
	edge_solid_shader.addShaderFromSourceFile(oglShader::Fragment, rcDir + "shaders/edge_fs.glsl");
	edge_solid_shader.addShaderFromSourceFile(oglShader::Geometry, rcDir + "shaders/edge_gs.glsl");
	edge_solid_shader.link();

	//////////////////////////////////////////////////////////////////////////
	vtx_solid_shader.addShaderFromSourceFile(oglShader::Vertex, rcDir + "shaders/vtx_vs.glsl");
	vtx_solid_shader.addShaderFromSourceFile(oglShader::Fragment, rcDir + "shaders/vtx_fs.glsl");
	vtx_solid_shader.link();
	//////////////////////////////////////////////////////////////////////////
	uid_shader.addShaderFromSourceFile(oglShader::Vertex, rcDir + "shaders/uid_vs.glsl");
	uid_shader.addShaderFromSourceFile(oglShader::Fragment, rcDir + "shaders/uid_fs.glsl");
	uid_shader.link();

}
void MeshViewer::initializeFBO()
{
	fbo.reset(new oglFBO(width(), height(), oglFBO::CombinedDepthStencil, GL_TEXTURE_2D));
	//selectionBuffer.resize(width()*height() * 4);
}

void MeshViewer::drawMeshToFBO()
{
	makeCurrent();
	fbo->bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind shader
	uid_shader.bind();
	uid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
	uid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);
	uid_shader.setUniformValue("scale", static_cast<Float>(view_scale));

	switch (interactionState)
	{
	case InteractionState::SEL_VERT:
	{
		if (shadingSate & ShadingState::SHADE_FLAT)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			fRBO.vao.bind();
			uid_shader.setUniformValue("mode", 0);
			glDrawElements(GL_TRIANGLES, fRBO.ibos.size(), GL_UNSIGNED_INT, 0);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		vRBO.vao.bind();
		glPointSize(15.0);
		uid_shader.setUniformValue("mode", 1);
		glDrawArrays(GL_POINTS, 0, vtx_array.size() / 3);
		vRBO.vao.release();//vtx_vbo.release();
		//draw vertices
		break;
	}
	case InteractionState::SEL_FACE:
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		fRBO.vao.bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, fRBO.id_tex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, fRBO.id_tbo);
		uid_shader.setUniformValue("id_tex", 0);
		uid_shader.setUniformValue("mode", 2);

		glDrawElements(GL_TRIANGLES, fRBO.ibos.size(), GL_UNSIGNED_INT, 0);
		
		fRBO.vao.release();
		glBindTexture(GL_TEXTURE_BUFFER, 0);
		//draw faces
		break;
	}
	case InteractionState::SEL_EDGE:
	{
		if (shadingSate & ShadingState::SHADE_FLAT)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			fRBO.vao.bind();
			uid_shader.setUniformValue("mode", 0);
			glDrawElements(GL_TRIANGLES, fRBO.ibos.size(), GL_UNSIGNED_INT, 0);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(10);
		heRBO.vao.bind();


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, heRBO.id_tex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, heRBO.id_tbo);
		uid_shader.setUniformValue("id_tex", 0);
		uid_shader.setUniformValue("mode", 2);


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
	bool selected = pixel >> 24;
	uint32_t renderID = pixel & 0xFFFFFF;
	
	switch (interactionState)
	{
	case InteractionState::SEL_VERT:
	{
		if (selected)
		{
			selVTX.push(renderID);
			heMesh->vertSet[renderID].isPicked = true;
		} 
		else
		{
			while (!selVTX.empty())
			{
				heMesh->vertSet[selVTX.front()].isPicked = false;
				selVTX.pop();
			}
		}
		heMesh->exportVertVBO(nullptr, &vRBO.flags);
		vRBO.allocateTBO(1);
		break;
	}
	case InteractionState::SEL_FACE:
	{
		if (selected)
		{
			selFACE.push(renderID);
			heMesh->faceSet[renderID].isPicked = true;
		}
		else
		{
			while (!selFACE.empty())
			{
				heMesh->faceSet[selFACE.front()].isPicked = false;
				selFACE.pop();
			}
		}
		heMesh->exportFaceVBO(nullptr, nullptr, &fRBO.flags);
		fRBO.allocateTBO();
		break;
	}
	case InteractionState::SEL_EDGE:
	{
		if (selected)
		{
			auto he = &heMesh->heSet[renderID];
			auto hef = he->flip();
			selHE.push(renderID);
			selHE.push(hef->index);
			he->isPicked = hef->isPicked = true;
		}
		else
		{
			while (!selHE.empty())
			{
				heMesh->heSet[selHE.front()].isPicked = false;
				selHE.pop();
			}
		}
		heMesh->exportEdgeVBO(nullptr, nullptr, &heRBO.flags);
		heRBO.allocateTBO();
		break;
	}
	}
#ifdef _DEBUG
	cout << "draw primitive id:" << renderID << endl;
#endif // DEBUG
}

void MeshViewer::paintGL()
{
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0.6f, 0.6f, 0.6f, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto checkDispMode = [](uint32_t disp, DispComp mode)->bool{
		return disp & static_cast<uint32_t>(mode);
	};

	if (checkDispMode(dispComp, DispComp::DISP_GRID))
	{
		grid.draw(view_cam.CameraToScreen, view_cam.WorldToCamera);
	}
	
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
		//if (true)
		{
			auto oglUniLoc = [&](const oglShaderP &p, const char* str)
			{
				return glGetUniformLocation(p.programId(), str);
			};
			if (mesh_changed)
			{
				allocateGL();
			}
			// Draw Vertices
			if (shadingSate & SHADE_VERT || interactionState == SEL_VERT)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				glPointSize(10);
				vRBO.vao.bind();
				vtx_solid_shader.bind();
				vtx_solid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
				vtx_solid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera); 
				vtx_solid_shader.setUniformValue("scale", view_scale);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_BUFFER, vRBO.flag_tex);
				glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, vRBO.flag_tbo);
				vtx_solid_shader.setUniformValue("flag_tex", 0);
				glDrawArrays(GL_POINTS, 0, vtx_array.size() / 3);

				vRBO.vao.release();//vtx_vbo.release();
				vtx_solid_shader.release();
			}
			

			// Draw Faces
			if (shadingSate & SHADE_FLAT || interactionState & SEL_FACE)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				fRBO.vao.bind();
				// use shader
				face_solid_shader.bind();
				face_solid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
				face_solid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);
				glUniform1ui(oglUniLoc(face_solid_shader, "hl_comp"), hlComp);
				face_solid_shader.setUniformValue("scale", view_scale);
				// Bind Texture Buffer
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_BUFFER, fRBO.flag_tex);
				glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, fRBO.flag_tbo);
				face_solid_shader.setUniformValue("flag_tex", 0);
				glDrawElements(GL_TRIANGLES, fRBO.ibos.size(), GL_UNSIGNED_INT, 0);

				fRBO.vao.release();
				face_solid_shader.release();
			}
			
			// Draw Edges
			if (shadingSate & SHADE_WF || interactionState == SEL_EDGE)
			{
				// Draw edge
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glLineWidth(1.0);
				heRBO.vao.bind();
				edge_solid_shader.bind();
				edge_solid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
				edge_solid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);
				//edge_solid_shader.setUniformValue("hl_comp", (GLuint)hlComp);
				glUniform1ui(oglUniLoc(edge_solid_shader, "hl_comp"), hlComp);
				edge_solid_shader.setUniformValue("scale", view_scale);
				// Bind Texture Buffer
				glBindTexture(GL_TEXTURE_BUFFER, heRBO.flag_tex);
				glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, heRBO.flag_tbo);
				edge_solid_shader.setUniformValue("flag_tex", 0);
				glDrawElements(GL_LINES, heRBO.ibos.size(), GL_UNSIGNED_INT, 0);

				heRBO.vao.release();
				edge_solid_shader.release();
			}
			glBindTexture(GL_TEXTURE_BUFFER, 0);
		}
	}
}

void MeshViewer::resizeGL(int w, int h)
{
	view_cam.resizeViewport(w / static_cast<Float>(h));

	initializeFBO();
}

void MeshViewer::keyPressEvent(QKeyEvent* e)
{
	switch (e->key()) {
	case Qt::Key_C:
	{
		/*int lev0 = cp_smoothing_times / 10;
		int lev1 = lev0 + 1;
		const int maxLev = 10;
		if (lev1 > 10) return;

		double ratio = (cp_smoothing_times - lev0) / static_cast<Float>(lev1 - lev0);
		//emit updateMeshColorByGeoDistance(lastSelectedIndex, lev0, lev1, ratio);
		emit updateMeshColorByGeoDistance(lastSelectedIndex); //changed to non-smoothing method due to bug in smoothed method*/

		break;
	}
	//    case Qt::Key_R:
	//    {
	//        toggleCriticalPoints();
	//        break;
	//    }
	case Qt::Key_F:
	{
		view_cam.WorldToCamera.setToIdentity();
		view_cam.WorldToCamera.lookAt(QVector3D(4, 2, 4), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
		view_cam.CameraToWorld = view_cam.WorldToCamera.inverted();
		update();
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
	case Qt::Key_S:
	{
		if (e->modifiers() == Qt::AltModifier)
		{
			QString filename = QFileDialog::getSaveFileName(
				this, "Save Screenshot file...", "default", tr("PNG(*.png)"));
			if (!filename.isEmpty())
			{
				this->grab().save(filename);
			}
		}
		break;
	}
	}
}

void MeshViewer::keyReleaseEvent(QKeyEvent* e)
{
}

void MeshViewer::mousePressEvent(QMouseEvent* e)
{
	mouseState.x = e->x();
	mouseState.y = e->y();

	if (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::NoModifier
		&& interactionState != ROAM_CAMERA)
	{
		drawMeshToFBO();
		update();
	}
}

void MeshViewer::mouseMoveEvent(QMouseEvent* e)
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
			view_cam.zoom(0.0, 0.0, -dx * 0.01);
			update();
		}
	}
	else if (e->buttons() == Qt::MidButton && e->modifiers() == Qt::AltModifier)
	{
		if (dx != e->x() && dy != e->y())
		{
			view_cam.zoom(dx * 0.01, dy * 0.01, 0.0);
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

void MeshViewer::mouseReleaseEvent(QMouseEvent* e)
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
				cout << "select edge flip's face index: " << heMesh->heMap[selectedElementIdx]->flip()->f->index << endl;
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
	// reset interaction mode if in camera mode triggered by holding alt
	if (e->modifiers() & Qt::AltModifier) {
		interactionState = interactionStateStack.top();
		interactionStateStack.pop();
	}

	update();
	*/
}

void MeshViewer::wheelEvent(QWheelEvent* e)
{
	view_cam.zoom(0, 0, -e->delta() * 0.01);
	update();
}

void MeshViewer::selectAll()
{
	switch (interactionState) {

	case SEL_FACE:
		for (auto &f : heMesh->faces())
			f.setPicked(true);
		heMesh->exportFaceVBO(nullptr, nullptr, &fRBO.flags);
		fRBO.allocateTBO();
		break;
	case SEL_EDGE:
		for (auto &e : heMesh->halfedges())
			e.setPicked(true);
		heMesh->exportEdgeVBO(nullptr, nullptr, &heRBO.flags);
		heRBO.allocateTBO();
		break;
	case SEL_VERT:
		for (auto &v : heMesh->verts())
			v.setPicked(true);
		heMesh->exportVertVBO(nullptr, &vRBO.flags);
		vRBO.allocateTBO(1);// Bind only flag tbo
		break;
	default:
		break;
	}
	update();
}

void MeshViewer::selectInverse()
{
	//TODO: buggy
	switch (interactionState)
	{
	case SEL_FACE:
	{
		for (auto &f : heMesh->faces())
		{
			heMesh->selectFace(f.index);
		}
		break;
	}
	case SEL_EDGE:
	{
		for (auto &e : heMesh->halfedges())
		{
			e.setPicked(!e.isPicked);
		}
		break;
	}
	case SEL_VERT:
	{
		for (auto &v : heMesh->verts())
		{
			heMesh->selectVertex(v.index);
		}
		break;
	}
	default:
		break;
	}
	update();
}

void MeshViewer::selectByRefID()
{
	SelectByRefIdPanel optDialog;
	if (optDialog.exec())
	{
		hdsid_t refid = (optDialog.id() << 2) + optDialog.type();

		for_each(heMesh->vertSet.begin(), heMesh->vertSet.end(),
			[&](HDS_Vertex &v) { v.setPicked(v.refid == refid); });
		heMesh->exportVertVBO(nullptr, &vRBO.flags);
		vRBO.allocateTBO(1);

		for_each(heMesh->heSet.begin(), heMesh->heSet.end(),
			[&](HDS_HalfEdge &he) { he.setPicked(he.refid == refid); });
		heMesh->exportEdgeVBO(nullptr, nullptr, &heRBO.flags);
		heRBO.allocateTBO();

		for_each(heMesh->faceSet.begin(), heMesh->faceSet.end(),
			[&](HDS_Face &f) { f.setPicked(f.refid == refid); });
		heMesh->exportFaceVBO(nullptr, nullptr, &fRBO.flags);
		fRBO.allocateTBO();

		update();
	}
}
