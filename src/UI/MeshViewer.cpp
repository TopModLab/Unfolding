#include "UI/MeshViewer.h"
#include "Utils/utils.h"
#include <QFileDialog>
#include "UI/SelectByRefIdPanel.h"

MeshViewer* MeshViewer::instance = nullptr;

MeshViewer::MeshViewer(QWidget *parent)
	: QOpenGLWidget(parent)
	//, interactionState(ROAM_CAMERA)
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

	// create vbo shared by all components
	for (size_t i = 0; i < RenderBufferObject::VBO_TYPE_COUNT; i++)
	{
		fRBO.vbo[i] = heRBO.vbo[i] = vRBO.vbo[i]
			= make_shared<oglBuffer>(oglBuffer::Type::VertexBuffer);
	}

	vertTrait[RenderBufferObject::POSITION].offset = offsetof(HDS_Vertex, pos);
	vertTrait[RenderBufferObject::POSITION].stride = sizeof(HDS_Vertex);
	vertTrait[RenderBufferObject::NORMAL].offset = 0;
	vertTrait[RenderBufferObject::NORMAL].stride = sizeof(QVector3D);
}

void MeshViewer::bindHalfEdgeMesh(HDS_Mesh *mesh)
{
	heMesh = mesh;
	mesh_changed = true;
	
	update();
}

void MeshViewer::saveScreenShot()
{
	QString filename = QFileDialog::getSaveFileName(
		this, "Save Screenshot file...", "default", tr("PNG(*.png)"));
	if (!filename.isEmpty())
	{
		this->grab().save(filename);
	}
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

	for (size_t i = 0; i < RenderBufferObject::VBO_TYPE_COUNT; i++)
	{
		vRBO.vbo[i]->create();
		vRBO.vbo[i]->setUsagePattern(oglBuffer::StreamDraw);
		vRBO.vbo[i]->bind();
		glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE,
                              vertTrait[i].stride,
                              (void*)vertTrait[i].offset);
		glEnableVertexAttribArray(i);
	}

	vRBO.releaseAll();

	// Init face & edge VAO
	auto createPrimitiveBuffers = [this](RenderBufferObject &RBO)
	{
		// Bind VAO
		RBO.vao.create();
		RBO.vao.bind();

		for (size_t i = 0; i < RenderBufferObject::VBO_TYPE_COUNT; i++)
		{
			RBO.vbo[i]->bind();
			glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE,
                                  vertTrait[i].stride,
                                  (void*)vertTrait[i].offset);
			glEnableVertexAttribArray(i);
		}

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

void MeshViewer::unfoldView(const HDS_Mesh* inMesh)
{
	QVector3D boundMid = inMesh->bound->getMidPoint();
	view_cam.setTarget(boundMid);
	boundMid.setZ(10);

	view_cam.WorldToCamera.setToIdentity();
	view_cam.WorldToCamera.lookAt(
		boundMid, view_cam.getTarget(), QVector3D(0, 1, 0));
	view_cam.CameraToWorld = view_cam.WorldToCamera.inverted();
}

void MeshViewer::allocateGL()
{
	// Clear Selection
	selVTX.clear(); selHE.clear(); selFACE.clear();
	/*while (!selVTX.empty()) selVTX.pop();
	while (!selHE.empty()) selHE.pop();
	while (!selFACE.empty()) selFACE.pop();*/
	heMesh->exportSelection(&selVTX, &selHE, &selFACE);

	heMesh->exportVertVBO(vertTrait, vertTrait + 1, &vRBO.flags);
	heMesh->exportEdgeVBO(&heRBO.ibos, &heRBO.ids, &heRBO.flags);
	heMesh->exportFaceVBO(&fRBO.ibos, &fRBO.ids, &fRBO.flags);
	vRBO.shrink_to_fit();
	heRBO.shrink_to_fit();
	fRBO.shrink_to_fit();

	// Bind Vertices Buffer
	for (size_t i = 0; i < RenderBufferObject::VBO_TYPE_COUNT; i++)
	{
		vRBO.vbo[i]->bind();
		if (vertTrait[i].count > 0 && vertTrait[i].data)
		{
			vRBO.vbo[i]->allocate(vertTrait[i].data, vertTrait[i].size);
		}

		vRBO.vbo[i]->release();
	}
	
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

	if (interactionState.sel_vert)
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
		glDrawArrays(GL_POINTS, 0, vertTrait[0].count);
		vRBO.vao.release();//vtx_vbo.release();
		//draw vertices
	}
	else if (interactionState.sel_face)
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
	}
	else if (interactionState.sel_edge)
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
	}

	fbo->release();
	uid_shader.release();

	auto fboRes = fbo->toImage();
	//fboRes.save("fbo.png");
	QRgb pixel = fboRes.pixel(mouseState.x, mouseState.y);
	bool selected = pixel >> 24;
	uint32_t renderID = pixel & 0xFFFFFF;
	
	if (interactionState.sel_vert)
	{
		glBindBuffer(GL_TEXTURE_BUFFER, vRBO.flag_tbo);
		auto dataptr = (uint16_t*)glMapBuffer(GL_TEXTURE_BUFFER, GL_READ_WRITE);
		if (selected)
		{
			if (heMesh->vertSet[renderID].isPicked)
			{
				selVTX.erase(renderID);
				heMesh->vertSet[renderID].isPicked = false;
			}
			else
			{
				selVTX.insert(renderID);
				heMesh->vertSet[renderID].isPicked = true;
			}
			*(dataptr + renderID) ^= 2;
		} 
		else if (interactionState.unselect)
		{
			for (auto v : selVTX)
			{
				heMesh->vertSet[v].isPicked = false;
				*(dataptr + v) &= 0xFFFD;
			}
			selVTX.clear();
		}
		glUnmapBuffer(GL_TEXTURE_BUFFER);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}
	else if (interactionState.sel_face)
	{
		if (selected)
		{
			if (heMesh->faceSet[renderID].isPicked
				&& interactionState.unselect)
			{
				selFACE.erase(renderID);
				heMesh->faceSet[renderID].isPicked = false;
			}
			else
			{
				selFACE.insert(renderID);
				heMesh->faceSet[renderID].isPicked = true;
			}
		}
		else if (interactionState.unselect)
		{
			for (auto fid : selFACE)
			{
				heMesh->faceSet[fid].isPicked = false;
			}
			selFACE.clear();
		}
		heMesh->exportFaceVBO(nullptr, nullptr, &fRBO.flags);
		fRBO.allocateTBO();
	}
	else if (interactionState.sel_edge)
	{
		if (selected)
		{
			auto he = &heMesh->heSet[renderID];
			auto hef = he->flip();
			if (he->isPicked && interactionState.unselect)
			{
				selHE.erase(renderID);
				selHE.erase(hef->index);
				he->isPicked = hef->isPicked = false;
			}
			else
			{
				selHE.insert(renderID);
				selHE.insert(hef->index);
				he->isPicked = hef->isPicked = true;
			}
		}
		else if (interactionState.unselect)
		{
			for (auto heid : selHE)
			{
				heMesh->heSet[heid].isPicked = false;
			}
			selHE.clear();
		}
		heMesh->exportEdgeVBO(nullptr, nullptr, &heRBO.flags);
		heRBO.allocateTBO();
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

	auto checkDispMode = [](uint32_t disp, DispComp mode)->bool {
		return disp & static_cast<uint32_t>(mode);
	};

	if (checkDispMode(dispComp, DispComp::DISP_GRID))
	{
		grid.draw(view_cam.CameraToScreen, view_cam.WorldToCamera);
	}
	
	if (heMesh != nullptr)
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
		if (shadingSate & SHADE_VERT || interactionState.sel_vert)
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
			glDrawArrays(GL_POINTS, 0, vertTrait[0].count);

			vRBO.vao.release();//vtx_vbo.release();
			vtx_solid_shader.release();
		}


		// Draw Faces
		if (shadingSate & SHADE_FLAT || interactionState.sel_face)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//glEnable(GL_CULL_FACE);
			//glCullFace(GL_FRONT); // cull back face
			fRBO.vao.bind();
			// use shader
			face_solid_shader.bind();
			face_solid_shader.setUniformValue("proj_matrix", view_cam.CameraToScreen);
			face_solid_shader.setUniformValue("view_matrix", view_cam.WorldToCamera);
			glUniform1ui(oglUniLoc(face_solid_shader, "hl_comp"), hlComp);
			face_solid_shader.setUniformValue("scale", view_scale);
			face_solid_shader.setUniformValue("thickness", heMesh->getThickness());
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
		if (shadingSate & SHADE_WF || interactionState.sel_edge)
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
	/*case Qt::Key_S:
	{
		if (e->modifiers() == Qt::AltModifier)
		{
			saveScreenShot();
		}
		break;
	}*/
	case Qt::Key_Z:
	{
		interactionState.unselect = true;
		break;
	}
	}
}

void MeshViewer::keyReleaseEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_S && e->modifiers() == Qt::AltModifier)
	{
		QString filename = QFileDialog::getSaveFileName(
			this,
			"Save Screenshot file...",
			"default",
			tr("PNG(*.png)")
		);
		if (!filename.isEmpty()) this->grab().save(filename);
	}
	if (e->key() == Qt::Key_Z)
	{
		interactionState.unselect = false;
	}
}

void MeshViewer::mousePressEvent(QMouseEvent* e)
{
	mouseState.x = e->x();
	mouseState.y = e->y();

	if (e->buttons() == Qt::LeftButton &&
		e->modifiers() == Qt::NoModifier &&
		interactionState.state > 1)
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
		view_cam.rotate(dy * 0.25f, -dx * 0.25f, 0.0);
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

void MeshViewer::mouseReleaseEvent(QMouseEvent* /*e*/)
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
	if (interactionState.sel_vert)
	{
		selVTX.clear();
		auto &verts = heMesh->verts();
		glBindBuffer(GL_TEXTURE_BUFFER, vRBO.flag_tbo);
		auto dataptr = (uint16_t*)glMapBuffer(GL_TEXTURE_BUFFER, GL_READ_WRITE);
		for (int i = 0; i < verts.size(); i++)
		{
			verts[i].setPicked(true);
			*(dataptr + i) ^= 2;
			selVTX.insert(i);
		}
		glUnmapBuffer(GL_TEXTURE_BUFFER);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}
	else if (interactionState.sel_face)
	{
		selFACE.clear();
		for (auto &f : heMesh->faces())
			f.setPicked(true);
		heMesh->exportFaceVBO(nullptr, nullptr, &fRBO.flags);
		fRBO.allocateTBO();
	}
	else if (interactionState.sel_edge)
	{
		selHE.clear();
		auto &hes = heMesh->halfedges();
		glBindBuffer(GL_TEXTURE_BUFFER, heRBO.flag_tbo);
		auto dataptr = (uint16_t*)glMapBuffer(GL_TEXTURE_BUFFER, GL_READ_WRITE);
		size_t halfsize = hes.size() / 2;
		for (size_t i = 0; i < halfsize; i++)
		{
			hes[i].setPicked(true);
			hes[i + halfsize].setPicked(true);
			*(dataptr + i) ^= 2;
			selHE.insert(i);
			selHE.insert(i + halfsize);
		}
		glUnmapBuffer(GL_TEXTURE_BUFFER);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}
	update();
}

void MeshViewer::selectInverse()
{
	//TODO: buggy
	if (interactionState.sel_face)
	{
		for (auto &f : heMesh->faces())
		{
			heMesh->selectFace(f.index);
		}
	}
	else if (interactionState.sel_edge)
	{
		for (auto &e : heMesh->halfedges())
		{
			e.setPicked(!e.isPicked);
		}
	}
	else if (interactionState.sel_vert)
	{
		for (auto &v : heMesh->verts())
		{
			heMesh->selectVertex(v.index);
		}
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
		heMesh->exportVertVBO(nullptr, nullptr, &vRBO.flags);
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
