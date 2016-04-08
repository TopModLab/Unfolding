#define NOMINMAX
#define MAX_PATH 100
#include "meshmanager.h"
#include <windows.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QString>
#include <QFileDialog>
#include <QToolButton>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
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
		manager->getMeshStack()->clear();
		manager->getMeshStack()->setCurrentFlag(OperationStack::Original);
		if (manager->loadOBJFile(filename))
		{
			viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
		}
	}
}

bool MainWindow::createComponents()
{
	try {
		viewer = new viewer_t(this);
		viewer->setFocusPolicy(Qt::StrongFocus);
		ceditor = new ColormapEditor;
		cppanel = new CriticalPointsPanel;
		clpanel = new CutLocusPanel;
		conpanel = new BridgerPanel;
		hmpanel = new HollowMeshPanel;
		bmpanel = new BindingMeshPanel;
		rmpanel = new RimFacePanel;

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
	ui->viewerLayout->addWidget(viewer);
	return true;
}

bool MainWindow::connectComponents()
{
	connect(ceditor, &ColormapEditor::colorChanged, this, &MainWindow::slot_updateViewerColormap);
	//kkkkkkkkkkkkkkkkkkkkkkkkkkk
	/*connect(viewer, SIGNAL(updateMeshColorByGeoDistance(int)), this, &MainWindow::slot_updateMeshColorByGeoDistance(int)));
	connect(viewer, SIGNAL(updateMeshColorByGeoDistance(int, int, int, double)), this, &MainWindow::slot_updateMeshColorByGeoDistance(int, int, int, double)));*/

	connect(cppanel, &CriticalPointsPanel::sig_methodChanged, this, &MainWindow::slot_updateCriticalPointsMethod);
	connect(cppanel, &CriticalPointsPanel::sig_smoothingTimesChanged, this, &MainWindow::slot_updateCriticalPointsSmoothingTimes);
	connect(cppanel, &CriticalPointsPanel::sig_smoothingTypeChanged, this, &MainWindow::slot_updateCriticalPointsSmoothingType);

	//kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
	/*connect(cppanel, SIGNAL(closedSignal()), viewer, &viewer_t::disablecpp()));

	connect(clpanel, SIGNAL(sig_methodChanged(int)), this, &MainWindow::slot_updateCutLocusMethod(int)));

	connect(clpanel, SIGNAL(sig_toggleCut()), viewer, &viewer_t::toggleCutLocusCut()));
	connect(clpanel, SIGNAL(sig_toggleCutMode(int)), viewer, &viewer_t::toggleCutLocusCutMode()));

	connect(clpanel, SIGNAL(sig_displayMinMax(int)), viewer, &viewer_t::toggleCutLocusPoints(int)));

	connect(clpanel, SIGNAL(sig_closedSignal()), viewer, &viewer_t::disableclp()));
	*/
	connect(clpanel, &CutLocusPanel::sig_closedSignal, this, &MainWindow::slot_disableclp);

	connect(conpanel, &BridgerPanel::sig_saved, this, &MainWindow::slot_setBridger);
	connect(conpanel, &BridgerPanel::sig_save2extend, this, &MainWindow::slot_extendMesh);

	connect(hmpanel, &HollowMeshPanel::sig_saved, this, &MainWindow::slot_hollowMesh);
	connect(hmpanel, &HollowMeshPanel::sig_setBridger, this, &MainWindow::slot_triggerExtendMesh);

	connect(bmpanel, &BindingMeshPanel::sig_saved, this, &MainWindow::slot_bindingMesh);
	connect(bmpanel, &BindingMeshPanel::sig_setBridger, this, &MainWindow::slot_triggerExtendMesh);

	connect(rmpanel, &RimFacePanel::sig_saved, this, &MainWindow::slot_rimmed3DMesh);
	connect(rmpanel, &RimFacePanel::sig_setBridger, this, &MainWindow::slot_triggerExtendMesh);
	return true;
}

void MainWindow::createActions()
{
	try {
		ui->actionImport->setStatusTip(ui->actionImport->toolTip());
		connect(ui->actionImport, &QAction::triggered, this, &MainWindow::slot_newFile);
		ui->actionExport->setStatusTip(ui->actionExport->toolTip());
		connect(ui->actionExport, &QAction::triggered, this, &MainWindow::slot_exportFile);//need to change
		ui->actionExit->setStatusTip(ui->actionExit->toolTip());
		connect(ui->actionExit, &QAction::triggered, this, &MainWindow::slot_closeFile);
		ui->actionSave->setStatusTip(ui->actionSave->toolTip());
		connect(ui->actionSave, &QAction::triggered, this, &MainWindow::slot_saveFile);

		/************************************************************************/
		/* Create Menu                                                          */
		/************************************************************************/
		connect(ui->actGenCube, &QAction::triggered,
			[&] { this->initMesh(":meshes/cube.obj"); });
		/*connect(ui->actGenTorus, &QAction::triggered,
			[&] { this->initMesh(":meshes/torus.obj"); });*/
		connect(ui->actGenTetra, &QAction::triggered,
			[&] { this->initMesh(":meshes/tetrahedron.obj"); });
		//////////////////////////////////////////////////////////////////////////
		connect(ui->actionReset, &QAction::triggered, this, &MainWindow::slot_reset);
		connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::slot_undo);
		connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::slot_redo);

		//selection menu
		connect(ui->actionSelAll, &QAction::triggered, viewer, &viewer_t::selectAll);
		connect(ui->actionSelInv, &QAction::triggered, viewer, &viewer_t::selectInverse);
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
		

		// Higlight
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
		
		connect(ui->actionSmooth, &QAction::triggered, this, &MainWindow::slot_smoothMesh);

		ui->actionExtend->setChecked(false);
		connect(ui->actionExtend, &QAction::triggered, this, &MainWindow::slot_triggerExtendMesh);
		
		ui->actionHollow->setChecked(false);
		connect(ui->actionHollow, &QAction::triggered, this, &MainWindow::slot_triggerHollowMesh);

		ui->actionBinding->setChecked(false);
		connect(ui->actionBinding, &QAction::triggered, this, &MainWindow::slot_triggerBindingMesh);
		

		ui->actionRim->setChecked(false);
		connect(ui->actionRim, &QAction::triggered, this, &MainWindow::slot_triggerRimmed3DMesh);

		ui->actionCut->setChecked(false);
		connect(ui->actionCut, &QAction::triggered, this, &MainWindow::slot_performMeshCut);
		
		ui->actionUnfold->setChecked(false);
		connect(ui->actionUnfold, &QAction::toggled, this, &MainWindow::slot_unfoldMesh);
		
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


void MainWindow::slot_newFile()
{
#ifdef _DEBUG
	QString filename = QFileDialog::getOpenFileName(this, "Select an OBJ file",  "meshes/", tr("OBJ files(*.obj)"));
#else // Release
	QString filename = QFileDialog::getOpenFileName(this, "Select an OBJ file",  "/", tr("OBJ files(*.obj)"));
#endif
	
	if (filename != NULL) {
#ifdef _DEBUG
		QTime clock;
		clock.start();
#endif
		MeshManager::getInstance()->getMeshStack()->clear();
		MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Original);
		cout<<"loading obj file: "<<string(filename.toUtf8().constData())<<"..."<<endl;
#ifdef _DEBUG
		qDebug("Clear ObjectStack Takes %d ms In Total.", clock.elapsed());
		clock.restart();
#endif
		if (!MeshManager::getInstance()->loadOBJFile(string(filename.toUtf8().constData())))
			return;
		
		//kkkkkkkk
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

void MainWindow::slot_exportFile()
{
	if (MeshManager::getInstance()->getMeshStack()->getCurrentFlag() != OperationStack::Unfolded)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Unable to export mesh! Unfold it first!"), QMessageBox::Close);
		return;
	}

	MeshManager::getInstance()->exportXMLFile();
}

void MainWindow::slot_closeFile()          //later add this function
{
	QMessageBox::warning(this, tr("Warning"), tr("Do you want to quit?"), QMessageBox::Yes | QMessageBox::No);
	this->close();
}

void MainWindow::slot_saveFile()
{
	QString filename = QFileDialog::getSaveFileName(this, "Input a file name");
	cout << "saving file " << filename.toStdString() << endl;
	MeshManager::getInstance()->saveMeshes();
}

void MainWindow::slot_performMeshCut() {

	if (MeshManager::getInstance()->getMeshStack()->canCut)
	{
		MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Cutted);

		MeshManager::getInstance()->cutMeshWithSelectedEdges();
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

	}

}

void MainWindow::slot_unfoldMesh(bool checked) {

	if (MeshManager::getInstance()->getMeshStack()->canUnfold)
	{
		MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Unfolded);
		MeshManager::getInstance()->unfoldMesh();
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
}

void MainWindow::slot_triggerExtendMesh()
{
	if (MeshManager::getInstance()->getMeshStack()->canExtend)
	{
	conpanel->setSaveMode((sender() == ui->actionExtend/*actionsMap["extend"]*/ )? true:false);
	conpanel->show();
	conpanel->activateWindow();
	}
}

void MainWindow::slot_triggerHollowMesh(bool checked)
{
	if (MeshManager::getInstance()->getMeshStack()->canHollow)
	{
		hmpanel->show();
		hmpanel->activateWindow();

		ui->actionHollow->setChecked(false);
	}
}

void MainWindow::slot_triggerBindingMesh(bool checked)
{
	if (MeshManager::getInstance()->getMeshStack()->canBind)
	{
		bmpanel->show();
		bmpanel->activateWindow();

	}

	ui->actionBinding->setChecked(false);
}

void MainWindow::slot_triggerRimmedMesh(bool checked)
{

	if (MeshManager::getInstance()->getMeshStack()->canRim)
	{
		MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Rimmed);

		MeshManager::getInstance()->rimMesh();
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

	}

	ui->actionRim->setChecked(false);
}

void MainWindow::slot_triggerRimmed3DMesh()
{
	HDS_Bridger::setSamples(16);
	QObject* obj = sender();
	MeshManager::getInstance()->getMeshStack()->reset();

    if (MeshManager::getInstance()->getMeshStack()->canRim)
	{
		rmpanel->show();
        rmpanel->activateWindow();
	}

	ui->actionRim->setChecked(false);
}

void MainWindow::slot_rimmed3DMesh()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Rimmed);

	MeshManager::getInstance()->set3DRimMesh(rmpanel->getConfig(), rmpanel->getW(), rmpanel->getH());
	viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
}

void MainWindow::slot_hollowMesh()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Hollowed);

	HDS_Bridger::setScale(hmpanel->getBridgerSize());
	MeshManager::getInstance()->setHollowMesh(hmpanel->getFlapSize(), hmpanel->getType(), hmpanel->getShift());
	viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

}

void MainWindow::slot_bindingMesh()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Binded);

	HDS_Bridger::setScale(bmpanel->getBridgerSize());
	MeshManager::getInstance()->setBindMesh();
	viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
}

void MainWindow::slot_setBridger()
{
	HDS_Bridger::setBridger(conpanel->getConfigValues());

}

void MainWindow::slot_extendMesh()
{

	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Extended);

	MeshManager::getInstance()->extendMesh(conpanel->getConfigValues());
	viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

}

void MainWindow::slot_rimMesh()
{

}

void MainWindow::slot_smoothMesh() {
	MeshManager::getInstance()->smoothMesh();
	viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getSmoothedMesh());
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
	ceditor->show();
	ceditor->activateWindow();
}

void MainWindow::slot_triggerCriticalPoints() {
	actionsMap["show CP"]->setChecked(true);
	//kkkkkkkkkkk
	//viewer->setCriticalPointsMethod(viewer->getCmode());
	//viewer->showCriticalPoints();
	viewer->update();
	cppanel->show();
}

void MainWindow::slot_triggerCutLocusPanel() {
	actionsMap["show CP"]->setChecked(clpanel->isMinMaxChecked());

	//kkkkkkkkkkk
	//viewer->setCutLocusMethod(viewer->getLmode());
	if (clpanel->isMinMaxChecked()) {
		//kkkkkkkkkkk
		//viewer->showCutLocusPoints();
	}
	//viewer->showCutLocusCut();
	viewer->update();
	clpanel->show();
}

void MainWindow::slot_disableclp()
{
	actionsMap["show CP"]->setChecked(clpanel->isMinMaxChecked());
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
	ceditor->close();
	cppanel->close();
	clpanel->close();
}


void MainWindow::updateCurrentMesh()
{
	curMesh = meshStack.top();
#if _DEBUG
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


