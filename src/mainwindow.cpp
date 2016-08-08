#define NOMINMAX
#define MAX_PATH 100
#include "meshmanager.h"
#include <windows.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UnfoldingAppException.h"

#include <QMessageBox>
#include <QString>
#include <QFileDialog>
#include <QToolButton>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	//setWindowTitle(tr("Geometry Unfolding"));

	createComponents();
	layoutComponents();
	connectComponents();
	initialization();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::initialization()
{
	isExtended = false;
	
	initMesh(":meshes/cube.obj");
}

void MainWindow::initMesh(const string& filename)
{
	if (!filename.empty())
	{
		auto manager = MeshManager::getInstance();
		auto mstack = manager->getMeshStack();
		mstack->clear();
		//manager->getMeshStack()->setCurrentFlag(OperationStack::Original);
		if (manager->loadOBJFile(filename))
		{
			viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
		}
	}
}

bool MainWindow::createComponents()
{
	try {
		viewer.reset(new viewer_t(this));
		viewer->setFocusPolicy(Qt::StrongFocus);
		color_editor.reset(new ColormapEditor);
		conn_panel.reset(new ConnectorPanel);
		cp_panel.reset(new CriticalPointsPanel);
		cl_panel.reset(new CutLocusPanel);
		grs_panel.reset(new BridgerPanel);
		origami_panel.reset(new OrigamiPanel);
		quad_panel.reset(new QuadEdgePanel);
		ges_panel.reset(new GESPanel);
		rim_panel.reset(new RimFacePanel);
		wv_panel.reset(new WeavePanel);

		createActions();
		createMenus();
		createToolBar();
		createDock();
		createStatusBar();
	}
	catch(UnfoldingAppException &e) {
		QMessageBox::critical(this, tr("Error"), e.what());
	}
	return true;
}

bool MainWindow::layoutComponents()
{
	//setCentralWidget(viewer);
	ui->viewerLayout->addWidget(viewer.data());
	return true;
}

bool MainWindow::connectComponents()
{
	connect(color_editor.data(), &ColormapEditor::colorChanged, this, &MainWindow::slot_updateViewerColormap);
	//kkkkkkkkkkkkkkkkkkkkkkkkkkk
	/*connect(viewer, SIGNAL(updateMeshColorByGeoDistance(int)), this, &MainWindow::slot_updateMeshColorByGeoDistance(int)));
	connect(viewer, SIGNAL(updateMeshColorByGeoDistance(int, int, int, double)), this, &MainWindow::slot_updateMeshColorByGeoDistance(int, int, int, double)));*/
	connect(conn_panel.data(), &ConnectorPanel::sig_saved, this, &MainWindow::exportSVG);

	connect(cp_panel.data(), &CriticalPointsPanel::sig_methodChanged, this, &MainWindow::slot_updateCriticalPointsMethod);
	connect(cp_panel.data(), &CriticalPointsPanel::sig_smoothingTimesChanged, this, &MainWindow::slot_updateCriticalPointsSmoothingTimes);
	connect(cp_panel.data(), &CriticalPointsPanel::sig_smoothingTypeChanged, this, &MainWindow::slot_updateCriticalPointsSmoothingType);

	//kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
	/*connect(cppanel, SIGNAL(closedSignal()), viewer, &viewer_t::disablecpp()));

	connect(clpanel, SIGNAL(sig_methodChanged(int)), this, &MainWindow::slot_updateCutLocusMethod(int)));

	connect(clpanel, SIGNAL(sig_toggleCut()), viewer, &viewer_t::toggleCutLocusCut()));
	connect(clpanel, SIGNAL(sig_toggleCutMode(int)), viewer, &viewer_t::toggleCutLocusCutMode()));

	connect(clpanel, SIGNAL(sig_displayMinMax(int)), viewer, &viewer_t::toggleCutLocusPoints(int)));

	connect(clpanel, SIGNAL(sig_closedSignal()), viewer, &viewer_t::disableclp()));
	*/
	connect(cl_panel.data(), &CutLocusPanel::sig_closedSignal, this, &MainWindow::slot_disableclp);

	connect(grs_panel.data(), &BridgerPanel::sig_saved, this, &MainWindow::slot_setBridger);
	connect(grs_panel.data(), &BridgerPanel::sig_save2extend, this, &MainWindow::slot_GRS);

	connect(origami_panel.data(), &OrigamiPanel::sig_save2origami, this, &MainWindow::slot_origami);

	connect(quad_panel.data(), &QuadEdgePanel::sig_saved, this, &MainWindow::slot_quadEdge);
	connect(quad_panel.data(), &QuadEdgePanel::sig_setBridger, this, &MainWindow::slot_triggerGRS);

	connect(ges_panel.data(), &GESPanel::sig_saved, this, &MainWindow::slot_GES);
	connect(ges_panel.data(), &GESPanel::sig_setBridger, this, &MainWindow::slot_triggerGRS);

	connect(rim_panel.data(), &RimFacePanel::sig_saved, this, &MainWindow::slot_rimmed3DMesh);
	connect(rim_panel.data(), &RimFacePanel::sig_setBridger, this, &MainWindow::slot_triggerGRS);

	connect(wv_panel.data(), &WeavePanel::sig_saved, this, &MainWindow::slot_weaveMesh);
	connect(wv_panel.data(), &WeavePanel::sig_setBridger, this, &MainWindow::slot_triggerGRS);

	connect (ui->panelCBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[=](int id){
		switch (id) {
		case 0: //single panel
			ui->GESBtn->setEnabled(false);
			ui->quadEdgeBtn->setEnabled(false);
			ui->wingedEdgeBtn->setEnabled(false);
			ui->origamiBtn->setEnabled(true);
			break;
		case 1: //mult panel
			ui->GESBtn->setEnabled(true);
			ui->quadEdgeBtn->setEnabled(true);
			ui->wingedEdgeBtn->setEnabled(true);
			ui->origamiBtn->setEnabled(false);
			break;
		case 2: //reduced panel
			ui->GESBtn->setEnabled(false);
			ui->quadEdgeBtn->setEnabled(false);
			ui->wingedEdgeBtn->setEnabled(false);
			ui->origamiBtn->setEnabled(true);
			break;
		}
	});

	return true;
}

void MainWindow::createActions()
{
	try {
		ui->actionImport->setStatusTip(ui->actionImport->toolTip());
		connect(ui->actionImport, &QAction::triggered, this, &MainWindow::newFile);
		ui->actionExport->setStatusTip(ui->actionExport->toolTip());
		connect(ui->actionExport, &QAction::triggered, this, &MainWindow::triggerExportSVG);//need to change
		ui->actionExit->setStatusTip(ui->actionExit->toolTip());
		connect(ui->actionExit, &QAction::triggered, this, &MainWindow::closeFile);
		ui->actionSave->setStatusTip(ui->actionSave->toolTip());
		connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);

		/************************************************************************/
		/* Create Menu                                                          */
		/************************************************************************/
		connect(ui->actGenCube, &QAction::triggered,
			[&] { this->initMesh(":meshes/cube.obj"); });
		connect(ui->actGenTorus, &QAction::triggered,
			[&] { this->initMesh(":meshes/torus.obj"); });
		connect(ui->actGenTetra, &QAction::triggered,
			[&] { this->initMesh(":meshes/tetrahedron.obj"); });
		//////////////////////////////////////////////////////////////////////////
		connect(ui->actionReset, &QAction::triggered, this, &MainWindow::slot_reset);
		connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::slot_undo);
		connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::slot_redo);

		//selection menu
		connect(ui->actionSelAll, &QAction::triggered, viewer.data(), &viewer_t::selectAll);
		connect(ui->actionSelInv, &QAction::triggered, viewer.data(), &viewer_t::selectInverse);
		connect(ui->actionSelMulti, &QAction::triggered, this, &MainWindow::slot_selectMultiple);
		//kkkkkkkkkkk
		/*connect(ui->actionSelTwinP, &QAction::triggered, viewer, &viewer_t::selectTwinPair()));
		connect(ui->actionSelNE, &QAction::triggered, viewer, &viewer_t::selectNextEdge()));
		connect(ui->actionSelCPts, &QAction::triggered, viewer, &viewer_t::selectCP()));
		connect(ui->actionSelMST, &QAction::triggered, viewer, &viewer_t::selectMSTEdges()));
		connect(ui->actionSelConnComp, &QAction::triggered, viewer, &viewer_t::selectCC()));
		connect(ui->actionGrowSel, &QAction::triggered, viewer, &viewer_t::selectGrow()));
		connect(ui->actionShrinkSel, &QAction::triggered, viewer, &viewer_t::selectShrink()));
		connect(ui->actionClearSel, &QAction::triggered, viewer, &viewer_t::selectClear()));*/


		//display menu
		connect(ui->actionDispGrid, &QAction::triggered,
			[&] { viewer->showComp(viewer_t::DISP_GRID); });
		// Shading State
		QActionGroup *shadingGroup = new QActionGroup(ui->menuShading);
		shadingGroup->addAction(ui->actWireframe);
		shadingGroup->addAction(ui->actShaded);
		shadingGroup->addAction(ui->actWfShaded);
		shadingGroup->setExclusive(true);
		connect(ui->actShaded, &QAction::triggered,
			[&] { viewer->showShading(viewer_t::SHADE_FLAT); });
		connect(ui->actWireframe, &QAction::triggered,
			[&] { viewer->showShading(viewer_t::SHADE_WF); });
		connect(ui->actWfShaded, &QAction::triggered,
			[&] { viewer->showShading(viewer_t::SHADE_WF_FLAT); });
		connect(ui->actDispVert, &QAction::triggered,
			[&] { viewer->showShading(viewer_t::SHADE_VERT); });
		

		// Highlight
		connect(ui->actHL_CutEdge, &QAction::triggered,
			[&] { viewer->highlightComp(viewer_t::HIGHLIGHT_CUTEDGE); });
		connect(ui->actHL_Bridger, &QAction::triggered,
			[&] { viewer->highlightComp(viewer_t::HIGHLIGHT_BRIDGER); });
		//"E: edges  V : vertices  F : faces  N: Normals L : lighting  C : critical points"
		QAction *showEdgesAct = new QAction(tr("Show Edges"), this);
		showEdgesAct->setShortcut(Qt::Key_E);
		showEdgesAct->setCheckable(true);
		showEdgesAct->setChecked(true);
		showEdgesAct->setStatusTip(tr("Show Edges"));
		connect(showEdgesAct, &QAction::triggered, this, &MainWindow::slot_toggleEdges);
		actionsMap["show edges"] = showEdgesAct;

		//vertices
		QAction *showVerticesAct = new QAction(tr("Show Vertices"), this);
		showVerticesAct->setShortcut(Qt::Key_V);
		showVerticesAct->setCheckable(true);
		showVerticesAct->setChecked(true);
		showVerticesAct->setStatusTip(tr("Show All Vertices"));
		connect(showVerticesAct, &QAction::triggered, this, &MainWindow::slot_toggleVertices);
		actionsMap["show vertices"] = showVerticesAct;

		QAction *showVMinAct = new QAction(tr("Show Mins"), this);
		showVMinAct->setCheckable(true);
		showVMinAct->setChecked(true);
		showVMinAct->setStatusTip(tr("Show Minimums"));
		//kkkkkkkkkkk
		//connect(showVMinAct, &QAction::triggered, viewer, &viewer_t::toggleMins()));
		actionsMap["show mins"] = showVMinAct;

		QAction *showVMaxAct = new QAction(tr("Show Maxs"), this);
		showVMaxAct->setCheckable(true);
		showVMaxAct->setChecked(true);
		showVMaxAct->setStatusTip(tr("Show Maximums"));
		//kkkkkkkkkkk
		//connect(showVMaxAct, &QAction::triggered, viewer, &viewer_t::toggleMaxs()));
		actionsMap["show maxs"] = showVMaxAct;

		QAction *showVSaddlesAct = new QAction(tr("Show Saddles"), this);
		showVSaddlesAct->setCheckable(true);
		showVSaddlesAct->setChecked(true);
		showVSaddlesAct->setStatusTip(tr("Show Saddles"));
		//kkkkkkkkkkk
		//connect(showVSaddlesAct, &QAction::triggered, viewer, &viewer_t::toggleSaddles()));
		actionsMap["show saddles"] = showVSaddlesAct;

		//face
		QAction *showFacesAct = new QAction(tr("Show &Faces"), this);
		showFacesAct->setShortcut(Qt::Key_F);
		showFacesAct->setCheckable(true);
		showFacesAct->setChecked(true);
		showFacesAct->setStatusTip(tr("Show Faces"));
		connect(showFacesAct, &QAction::triggered, this, &MainWindow::slot_toggleFaces);
		actionsMap["show faces"] = showFacesAct;

		QAction *showNormalsAct = new QAction(tr("Show &Normals"), this);
		showNormalsAct->setShortcut(Qt::Key_N);
		showNormalsAct->setCheckable(true);
		showNormalsAct->setChecked(false);
		showNormalsAct->setStatusTip(tr("Show Vertex Normals"));
		connect(showNormalsAct, &QAction::triggered, this, &MainWindow::slot_toggleNormals);
		actionsMap["show normals"] = showNormalsAct;

		QAction *showIndexAct = new QAction(tr("Show Vertex &Index"), this);
		showIndexAct->setShortcut(Qt::Key_I);
		showIndexAct->setCheckable(true);
		showIndexAct->setChecked(false);
		showIndexAct->setStatusTip(tr("Show Vertex Index"));
		//kkkkkkkkkkk
		//connect(showIndexAct, &QAction::triggered, viewer, &viewer_t::toggleText()));
		actionsMap["show index"] = showIndexAct;

		QAction *showLightingSmoothAct = new QAction(tr("Smooth Shading"), this);
		showLightingSmoothAct->setCheckable(true);
		showLightingSmoothAct->setChecked(false);
		showLightingSmoothAct->setStatusTip(tr("Lighting set to smooth shading"));
		//kkkkkkkkkkk
		//connect(showLightingSmoothAct, &QAction::triggered, viewer, &viewer_t::toggleLightingSmooth()));
		actionsMap["lighting smooth"] = showLightingSmoothAct;

		QAction *showLightingFlatAct = new QAction(tr("Flat Shading"), this);
		showLightingFlatAct->setCheckable(true);
		showLightingFlatAct->setChecked(false);
		showLightingFlatAct->setStatusTip(tr("Lighting set to flat shading"));
		//kkkkkkkkkkk
		//connect(showLightingFlatAct, &QAction::triggered, viewer, &viewer_t::toggleLightingFlat()));
		actionsMap["lighting flat"] = showLightingFlatAct;

		QAction *showLightingWireframeAct = new QAction(tr("Wireframe"), this);
		showLightingWireframeAct->setCheckable(true);
		showLightingWireframeAct->setChecked(true);
		showLightingWireframeAct->setStatusTip(tr("Lighting set to wireframe"));
		//kkkkkkkkkkk
		//connect(showLightingWireframeAct, &QAction::triggered, viewer, &viewer_t::toggleLightingWireframe()));
		actionsMap["lighting wireframe"] = showLightingWireframeAct;

		QAction *showCPAct = new QAction(tr("Show &Critical Points"), this);
		showCPAct->setShortcut(Qt::Key_C);
		showCPAct->setCheckable(true);
		showCPAct->setChecked(false);
		showCPAct->setStatusTip(tr("Show Critical Points"));
		//kkkkkkkkkkk
		//connect(showCPAct, &QAction::triggered, viewer, &viewer_t::toggleCriticalPoints()));
		actionsMap["show CP"] = showCPAct;

		//main menu bar

		ui->actionCamOP->setChecked(true);
		connect(ui->actionCamOP, &QAction::triggered, this, &MainWindow::slot_toggleCameraOperation);
		
		ui->actionSelFace->setChecked(false);
		connect(ui->actionSelFace, &QAction::triggered, this, &MainWindow::slot_toggleFaceSelection);
		
		ui->actionSelEdge->setChecked(false);
		connect(ui->actionSelEdge, &QAction::triggered, this, &MainWindow::slot_toggleEdgeSelection);
		
		ui->actionSelVertex->setChecked(false);
		connect(ui->actionSelVertex, &QAction::triggered, this, &MainWindow::slot_toggleVertexSelection);
		
		///connect panel type to meshManager
		MeshManager::setPanelType(ui->panelCBox->currentIndex());
		connect(ui->panelCBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MeshManager::setPanelType);

		connect(ui->actionSmooth, &QAction::triggered, this, &MainWindow::slot_smoothMesh);

		//ui->actionExtend->setChecked(false);
		//connect(ui->actionExtend, &QAction::triggered, this, &MainWindow::slot_triggerGES);
		
		connect(ui->quadEdgeBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerQuadEdge);
		connect(ui->wingedEdgeBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerWingedEdge);

		connect(ui->GRSBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerGRS);
		connect(ui->GESBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerGES);
		connect(ui->origamiBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerOrigami);

		connect(ui->FBWBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerRimmedMesh);

		connect(ui->weaveBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerWeaveMesh);

		connect(ui->halfEdgeBtn, &QToolButton::clicked, this, &MainWindow::slot_performMeshCut);

		connect(ui->unfoldBtn, &QToolButton::clicked, this, &MainWindow::slot_unfoldMesh);

		connect(ui->actionColormap, &QAction::triggered, this, &MainWindow::slot_triggerColormap);
		connect(ui->actionCPts, &QAction::triggered, this, &MainWindow::slot_triggerCriticalPoints);
		connect(ui->actionCutLocus, &QAction::triggered, this, &MainWindow::slot_triggerCutLocusPanel);
	}
	catch(...) {
		throw UnfoldingAppException("Failed to create actions!");
	}
}

void MainWindow::createMenus()
{
	try {
		//QMenu *displayMenu = ui->menuBar->addMenu(tr("&Display"));
		//		QMenu *displayVertexMenu = displayMenu->addMenu(tr("Show Vertices"));
		//		displayVertexMenu->setDefaultAction(actionsMap["show vertices"]);
		//		displayVertexMenu->addAction(actionsMap["show vertices"]);
		//		displayVertexMenu->addAction(actionsMap["show mins"]);
		//		displayVertexMenu->addAction(actionsMap["show maxs"]);
		//		displayVertexMenu->addAction(actionsMap["show saddles"]);
		/*
		displayMenu->addAction(actionsMap["show vertices"]);

		displayMenu->addAction(actionsMap["show edges"]);
		displayMenu->addAction(actionsMap["show faces"]);
		displayMenu->addAction(actionsMap["show normals"]);
		displayMenu->addAction(actionsMap["show index"]);
		displayMenu->addSeparator()->setText(tr("Lighting"));

		QActionGroup *lightGroup = new QActionGroup(displayMenu);

		lightGroup->addAction(actionsMap["lighting smooth"]);
		lightGroup->addAction(actionsMap["lighting flat"]);
		lightGroup->addAction(actionsMap["lighting wireframe"]);
		lightGroup->setExclusive(true);

		displayMenu->addAction(actionsMap["lighting smooth"]);
		displayMenu->addAction(actionsMap["lighting flat"]);
		displayMenu->addAction(actionsMap["lighting wireframe"]);

		displayMenu->addSeparator();
		displayMenu->addAction(actionsMap["show CP"]);

*/

		//editMenu->addAction(actionsMap["erase"]);//later added
	}
	catch(...) {
		throw UnfoldingAppException("Failed to create menus!");
	}
}

void MainWindow::createDock()
{
	try {

	}
	catch(...) {
		throw UnfoldingAppException("Failed to create dock!");
	}
}

void MainWindow::createToolBar()
{
	try {
		QActionGroup *selectGroup = new QActionGroup(ui->mainToolBar);
		selectGroup->addAction(ui->actionCamOP);
		selectGroup->addAction(ui->actionSelFace);
		selectGroup->addAction(ui->actionSelEdge);
		selectGroup->addAction(ui->actionSelVertex);
		selectGroup->setExclusive(true);

		//QActionGroup *unfoldGroup = new QActionGroup(ui->mainToolBar);
		//unfoldGroup->addAction(actionsMap["mesh unfold"]);
		//unfoldGroup->addAction(actionsMap["mesh fold"]);

		QDoubleSpinBox* scaleValBox = new QDoubleSpinBox;
		scaleValBox->setPrefix("Scale: ");
		scaleValBox->setValue(1.);

		connect(scaleValBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			[&](double val) {
			viewer->scale = val; viewer->update(); });
		ui->displayToolBar->addWidget(scaleValBox);
		ui->displayToolBar->addSeparator();
	}
	catch(...) {
		throw UnfoldingAppException("Failed to create status bar!");
	}
}

void MainWindow::createStatusBar()
{
	try {

	}
	catch(...) {
		throw UnfoldingAppException("Failed to create status bar!");
	}
}


void MainWindow::newFile()
{
#ifdef _DEBUG
	QString filename = QFileDialog::getOpenFileName(this, "Select an OBJ file",  "meshes/", tr("OBJ files(*.obj)"));
#else // Release
	QString filename = QFileDialog::getOpenFileName(this, "Select an OBJ file",  "", tr("OBJ files(*.obj)"));
#endif
	
	if (filename != NULL) {
#ifdef _DEBUG
		QTime clock;
		clock.start();
#endif
		MeshManager::getInstance()->getMeshStack()->clear();
		//MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Original);
		cout<<"loading obj file: "<<string(filename.toUtf8().constData())<<"..."<<endl;
#ifdef _DEBUG
		qDebug("Clear ObjectStack Takes %d ms In Total.", clock.elapsed());
		clock.restart();
#endif
		if (!MeshManager::getInstance()->loadOBJFile(string(filename.toUtf8().constData())))
			return;
		
#ifdef _DEBUG
		qDebug("Loading Object Takes %d ms In Total.", clock.elapsed());
		clock.restart();
#endif
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
		
#if USE_REEB_GRAPH
		viewer->bindReebGraph(MeshManager::getInstance()->getReebGraph());
#endif

		viewer->update();
#ifdef _DEBUG
		qDebug("Render New Object Takes %d ms In Total.", clock.elapsed());
#endif
	}
}

void MainWindow::exportSVG()
{
	if (MeshManager::getInstance()->exportSVGFile(
		conn_panel->getFilename(), conn_panel->getConfig()))
	{
		statusBar()->showMessage("Succeeded to save SVG file as "
			+ conn_panel->getFilename() + "!", 5000);
	}
	else
	{
		statusBar()->showMessage("Failed to save SVG file...", 5000);
	}
}

void MainWindow::closeFile()          //later add this function
{
	QMessageBox::warning(this, tr("Warning"), tr("Do you want to quit?"), QMessageBox::Yes | QMessageBox::No);
	this->close();
}

void MainWindow::saveFile()
{
	QString filename = QFileDialog::getSaveFileName(this, "Input a file name","",tr("OBJ files(*.obj)"));
	cout << "saving file " << filename.toStdString() << endl;
	MeshManager::getInstance()->saveMeshes();
}

void MainWindow::triggerExportSVG()
{
	auto curStack = MeshManager::getInstance()->getMeshStack();
	if (curStack->getCurrentFlag() != OperationStack::Unfolded)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Unable to export mesh! Unfold it first!"), QMessageBox::Close);
		return;
	}
	conn_panel->resetParas(curStack->getCurrentMesh()->processType);
	conn_panel->show();
}

void MainWindow::slot_performMeshCut() {

	if (MeshManager::getInstance()->getMeshStack()->canCut)
	{
		MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Cutted);

		MeshManager::getInstance()->cutMeshWithSelectedEdges();
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

	}

}

void MainWindow::slot_unfoldMesh() {

	if (MeshManager::getInstance()->getMeshStack()->canUnfold)
	{
		MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Unfolded);
		MeshManager::getInstance()->unfoldMesh();
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
}

void MainWindow::slot_triggerGRS()
{
	if (MeshManager::getInstance()->getMeshStack()->canGRS)
	{
		grs_panel->setSaveMode((sender() == ui->GRSBtn/*actionsMap["extend"]*/ )? true:false);
		grs_panel->show();
		grs_panel->activateWindow();
	}
}

void MainWindow::slot_triggerOrigami()
{
	//Origami
	if (MeshManager::getInstance()->getMeshStack()->canOrigami)
	{
		origami_panel->show();
		origami_panel->activateWindow();
	}
}

void MainWindow::slot_triggerQuadEdge()
{
	MeshManager::getInstance()->getMeshStack()->reset();

	if (MeshManager::getInstance()->getMeshStack()->canQuad)
	{
		quad_panel->setPanelType(0);
		quad_panel->show();
		quad_panel->activateWindow();

		ui->actionHollow->setChecked(false);
	}
}

void MainWindow::slot_triggerWingedEdge()
{
	MeshManager::getInstance()->getMeshStack()->reset();

	if (MeshManager::getInstance()->getMeshStack()->canQuad)
	{
		quad_panel->setPanelType(1);
		quad_panel->show();
		quad_panel->activateWindow();

		ui->actionHollow->setChecked(false);
	}
}

void MainWindow::slot_triggerGES()
{
	MeshManager::getInstance()->getMeshStack()->reset();

	if (MeshManager::getInstance()->getMeshStack()->canGES)
	{
		ges_panel->setPanelType(ui->panelCBox->currentIndex());
		ges_panel->show();
		ges_panel->activateWindow();

	}

	ui->actionBinding->setChecked(false);
}


void MainWindow::slot_triggerRimmedMesh()
{
	HDS_Bridger::setSamples(16);
	MeshManager::getInstance()->getMeshStack()->reset();

    if (MeshManager::getInstance()->getMeshStack()->canRim)
	{
		rim_panel->show();
        rim_panel->activateWindow();
	}

	ui->actionRim->setChecked(false);
}

void MainWindow::slot_triggerWeaveMesh()
{
	MeshManager::getInstance()->getMeshStack()->reset();

	if (MeshManager::getInstance()->getMeshStack()->canRim)
	{
		wv_panel->show();
		wv_panel->activateWindow();
	}
}

void MainWindow::slot_triggerDForms()
{

}

void MainWindow::slot_quadEdge()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::QuadEdge);

	HDS_Bridger::setScale(quad_panel->getBridgerSize());
	if (MeshManager::getInstance()->setQuadEdge(
			quad_panel->getFlapSize(), quad_panel->getType(), quad_panel->getShift()))
	{
		statusBar()->showMessage("Succeeded to generate Quad-Edge Mesh!", 5000);
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to generate Quad-Edge Mesh...", 5000);
	}
}

void MainWindow::slot_GES()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::GES);

	HDS_Bridger::setScale(ges_panel->getBridgerSize());
	if (MeshManager::getInstance()->setGES())
	{
		statusBar()->showMessage("Succeeded to generate GES Mesh!", 5000);
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to generate GES Mesh...", 5000);
	}
}

void MainWindow::slot_rimmed3DMesh()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Rimmed);
	if (MeshManager::getInstance()->set3DRimMesh(rim_panel->getConfig()))
	{
		statusBar()->showMessage("Succeeded to generate Rim Mesh!", 5000);
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to generate Rim Mesh...", 5000);
	}
}

void MainWindow::slot_weaveMesh()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Rimmed);

	if (MeshManager::getInstance()->setWeaveMesh(wv_panel->getConfig()))
	{
		statusBar()->showMessage("Succeeded to generate Woven Mesh!", 5000);
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to generate Woven Mesh...", 5000);
	}
}

void MainWindow::slot_setBridger()
{
	HDS_Bridger::setBridger(grs_panel->getConfigValues());

}

void MainWindow::slot_GRS()
{
	//graph rotation
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::GRS);

	if (MeshManager::getInstance()->setGRS(grs_panel->getConfigValues()))
	{
		statusBar()->showMessage("Succeeded to generate GRS Mesh!", 5000);
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to generate GRS Mesh...", 5000);
	}
}

void MainWindow::slot_origami()
{
	//origami
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Origami);

	if (MeshManager::getInstance()->setOrigami(origami_panel->getConfigValues()))
	{
		statusBar()->showMessage("Succeeded to generate Origami Mesh!", 5000);
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to generate Origami Mesh...", 5000);
	}
}

void MainWindow::slot_smoothMesh()
{
	if (MeshManager::getInstance()->smoothMesh())
	{
		statusBar()->showMessage("Succeeded to smooth mesh!", 5000);
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to smooth mesh...", 5000);
	}
}


void MainWindow::slot_undo()
{

	MeshManager::getInstance()->getMeshStack()->undo();
	viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());


}

void MainWindow::slot_redo()
{
	MeshManager::getInstance()->getMeshStack()->redo();
	viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

}

void MainWindow::slot_reset()
{

	//reset actions
	ui->actionUnfold->setChecked(false);
	ui->actionExtend->setChecked(false);
	ui->actionHollow->setChecked(false);
	//ui->actionOrigami->setChecked(false);

	//reset mesh
	//MeshManager::getInstance()->resetMesh();

	MeshManager::getInstance()->getMeshStack()->reset();
	viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

}

//========================================//

void MainWindow::slot_selectMultiple()
{
	//kkkkkkkkkkkkkkkkkkkkkk
	//viewer->setSelectionMode(viewer_t::MultiSelect);
}

void MainWindow::slot_toggleEdges()
{
	HDS_Mesh* curMesh = MeshManager::getInstance()->getMeshStack()->getCurrentMesh();
	if (curMesh)
	{
		curMesh->flipShowEdges();
	}
	viewer->update();

}

void MainWindow::slot_toggleVertices()
{
	HDS_Mesh* curMesh = MeshManager::getInstance()->getMeshStack()->getCurrentMesh();
	if (curMesh)
	{
		curMesh->flipShowVertices();
	}

	viewer->update();

}
void MainWindow::slot_toggleFaces()
{
	HDS_Mesh* curMesh = MeshManager::getInstance()->getMeshStack()->getCurrentMesh();
	if (curMesh)
	{
		curMesh->flipShowFaces();
	}
	viewer->update();

}

void MainWindow::slot_toggleNormals()
{
	HDS_Mesh* curMesh = MeshManager::getInstance()->getMeshStack()->getCurrentMesh();
	if (curMesh)
	{
		curMesh->flipShowNormals();
	}
	viewer->update();

}

void MainWindow::slot_toggleCameraOperation()
{
	viewer->setInteractionMode(viewer_t::ROAM_CAMERA);
}

void MainWindow::slot_toggleFaceSelection()
{
	viewer->setInteractionMode(viewer_t::SEL_FACE);
}

void MainWindow::slot_toggleEdgeSelection()
{
	viewer->setInteractionMode(viewer_t::SEL_EDGE);
}

void MainWindow::slot_toggleVertexSelection()
{
	viewer->setInteractionMode(viewer_t::SEL_VERT);
}



void MainWindow::slot_triggerColormap() {
	color_editor->show();
	color_editor->activateWindow();
}

void MainWindow::slot_triggerCriticalPoints() {
	actionsMap["show CP"]->setChecked(true);
	//kkkkkkkkkkk
	//viewer->setCriticalPointsMethod(viewer->getCmode());
	//viewer->showCriticalPoints();
	viewer->update();
	cp_panel->show();
}

void MainWindow::slot_triggerCutLocusPanel() {
	actionsMap["show CP"]->setChecked(cl_panel->isMinMaxChecked());

	//kkkkkkkkkkk
	//viewer->setCutLocusMethod(viewer->getLmode());
	if (cl_panel->isMinMaxChecked()) {
		//kkkkkkkkkkk
		//viewer->showCutLocusPoints();
	}
	//viewer->showCutLocusCut();
	viewer->update();
	cl_panel->show();
}

void MainWindow::slot_disableclp()
{
	actionsMap["show CP"]->setChecked(cl_panel->isMinMaxChecked());
}

void MainWindow::slot_updateViewerColormap()
{
	//kkkkkkkkkkk
	//viewer->setCurvatureColormap(ceditor->getColormap());
}

void MainWindow::slot_updateMeshColorByGeoDistance(int vidx) {
	MeshManager::getInstance()->colorMeshByGeoDistance(vidx);
	viewer->update();
}

void MainWindow::slot_updateMeshColorByGeoDistance(int vidx, int lev0, int lev1, double ratio) {
	MeshManager::getInstance()->colorMeshByGeoDistance(vidx, lev0, lev1, ratio);
	viewer->update();
}


void MainWindow::closeEvent(QCloseEvent *e) {
	color_editor->close();
	cp_panel->close();
	cl_panel->close();
}


void MainWindow::updateCurrentMesh()
{
	curMesh = meshStack.top();
#ifdef _DEBUG
	cout << "current mesh mode:::" << curMesh << endl;
#endif
}

void MainWindow::slot_updateCriticalPointsMethod(int midx) {
	//kkkkkkkkkkk
	//viewer->setCriticalPointsMethod(midx);
	viewer->update();
}

void MainWindow::slot_updateCriticalPointsSmoothingTimes(int times) {
	//kkkkkkkkkkk
	//viewer->setCriticalPointsSmoothingTimes(times);
	viewer->update();
}

void MainWindow::slot_updateCriticalPointsSmoothingType(int t) {
	cout << "Smoothing type = " << t << endl;
	//kkkkkkkkkkk
	//viewer->setCriticalPointsSmoothingType(t);

	viewer->update();
}

void MainWindow::slot_updateCutLocusMethod(int midx)
{
	//kkkkkkkkkkk
	//viewer->setCutLocusMethod(midx);
	viewer->update();

}


