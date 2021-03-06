#define NOMINMAX
#define MAX_PATH 100
#include "GeomProc/meshmanager.h"
#include <windows.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Utils/UnfoldingAppException.h"

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
	//delete ui;
}

void MainWindow::initialization()
{
	isExtended = false;
#ifdef _DEBUG
	initMesh("meshes/2-cube.obj");
#else
	initMesh(":meshes/2-cube.obj");
#endif // _DEBUG
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
			MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
		}
	}
}

bool MainWindow::createComponents()
{
	try {
		MeshViewer::getInstance()->setFocusPolicy(Qt::StrongFocus);
		color_editor.reset(new ColormapEditor);
		conn_panel.reset(new ConnectorPanel);
		cp_panel.reset(new CriticalPointsPanel);
		cl_panel.reset(new CutLocusPanel);
		grs_panel.reset(new BridgerPanel);
		quad_panel.reset(new QuadEdgePanel);
		ges_panel.reset(new GESPanel);
		rim_panel.reset(new RimFacePanel);
		wv_panel.reset(new WeavePanel);
		neowv_panel.reset(new NeoWeavePanel);
		origami_panel.reset(new OrigamiPanel);
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
	ui->viewerLayout->addWidget(MeshViewer::getInstance());
	return true;
}

bool MainWindow::connectComponents()
{
	connect(color_editor.data(), &ColormapEditor::colorChanged, this, &MainWindow::slot_updateViewerColormap);
	// TODO: add support for updateMeshColorByGeoDistance()
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

	connect(quad_panel.data(), &QuadEdgePanel::sig_saved, this, &MainWindow::slot_quadEdge);
	connect(quad_panel.data(), &QuadEdgePanel::sig_setBridger, this, &MainWindow::slot_triggerGRS);

	connect(ges_panel.data(), &GESPanel::sig_saved, this, &MainWindow::slot_GES);
	connect(ges_panel.data(), &GESPanel::sig_setBridger, this, &MainWindow::slot_triggerGRS);

	connect(rim_panel.data(), &RimFacePanel::sig_saved, this, &MainWindow::slot_rimmed3DMesh);
	connect(rim_panel.data(), &RimFacePanel::sig_setBridger, this, &MainWindow::slot_triggerGRS);

	connect(wv_panel.data(), &WeavePanel::sig_saved, this, &MainWindow::slot_weaveMesh);
	connect(wv_panel.data(), &WeavePanel::sig_setBridger, this, &MainWindow::slot_triggerGRS);
	
	connect(neowv_panel.data(), &NeoWeavePanel::sig_saved, this, &MainWindow::slot_neoWeaveMesh);
	connect(origami_panel.data(), &OrigamiPanel::sig_saved, this, &MainWindow::slot_origamiMesh);

	connect (ui->panelCBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[=](int id){
		switch (id) {
		case 0: //single panel
			ui->GESBtn->setEnabled(false);
			ui->quadEdgeBtn->setEnabled(false);
			ui->wingedEdgeBtn->setEnabled(false);
			break;
		case 1: //mult panel
			ui->GESBtn->setEnabled(true);
			ui->quadEdgeBtn->setEnabled(true);
			ui->wingedEdgeBtn->setEnabled(true);
			break;
		case 2: //reduced panel
			ui->GESBtn->setEnabled(false);
			ui->quadEdgeBtn->setEnabled(false);
			ui->wingedEdgeBtn->setEnabled(false);
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
			[&] { this->initMesh(":meshes/2-cube.obj"); });
/*
		connect(ui->actGenTorus, &QAction::triggered,
			[&] { this->initMesh(":meshes/torus.obj"); });
*/
		connect(ui->actGenTetra, &QAction::triggered,
			[&] { this->initMesh(":meshes/1-tetrahedron.obj"); });
		//////////////////////////////////////////////////////////////////////////
		connect(ui->actionReset, &QAction::triggered, this, &MainWindow::slot_reset);
		connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::slot_undo);
		connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::slot_redo);

		//selection menu
		connect(ui->actionSelAll, &QAction::triggered, MeshViewer::getInstance(), &viewer_t::selectAll);
		connect(ui->actionSelInv, &QAction::triggered, MeshViewer::getInstance(), &viewer_t::selectInverse);
		connect(ui->actSelRefID, &QAction::triggered, MeshViewer::getInstance(), &viewer_t::selectByRefID);
		// TODO: Support all section options
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
			[] { MeshViewer::getInstance()->showComp(viewer_t::DISP_GRID); });
		// Shading State
		QActionGroup *shadingGroup = new QActionGroup(ui->menuShading);
		shadingGroup->addAction(ui->actWireframe);
		shadingGroup->addAction(ui->actShaded);
		shadingGroup->addAction(ui->actWfShaded);
		shadingGroup->setExclusive(true);
		connect(ui->actShaded, &QAction::triggered,
			[] { MeshViewer::getInstance()->showShading(viewer_t::SHADE_FLAT); });
		connect(ui->actWireframe, &QAction::triggered,
			[] { MeshViewer::getInstance()->showShading(viewer_t::SHADE_WF); });
		connect(ui->actWfShaded, &QAction::triggered,
			[] { MeshViewer::getInstance()->showShading(viewer_t::SHADE_WF_FLAT); });
		connect(ui->actDispVert, &QAction::triggered,
			[] { MeshViewer::getInstance()->showShading(viewer_t::SHADE_VERT); });
		

		// Highlight
		connect(ui->actHL_CutEdge, &QAction::triggered,
			[] { MeshViewer::getInstance()->highlightComp(viewer_t::HIGHLIGHT_CUTEDGE); });
		connect(ui->actHL_Bridger, &QAction::triggered,
			[] { MeshViewer::getInstance()->highlightComp(viewer_t::HIGHLIGHT_BRIDGER); });
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

		connect(ui->FBWBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerRimmedMesh);

		connect(ui->weaveBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerWeaveMesh);
		connect(ui->neoWeaveBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerNeoWeaveMesh);

		connect(ui->halfEdgeBtn, &QToolButton::clicked, this, &MainWindow::slot_performMeshCut);

		connect(ui->origamiBtn, &QToolButton::clicked, this, &MainWindow::slot_triggerOrigamiMesh);

		// unfold action button signal is connected in mainwindow.ui file
		connect(ui->actionUnfold, &QAction::triggered, this, &MainWindow::slot_unfoldMesh);

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
			auto viewer = MeshViewer::getInstance();
			viewer->view_scale = val; viewer->update(); });
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
    QString filename = QFileDialog::getOpenFileName(this, "Select a Geometry File",
#ifdef _DEBUG
                                                    "meshes/",
#else // Release
                                                    "",
#endif
                                                    tr("Geometry Files(*.obj *.hds)"));
	if (!filename.isEmpty())
    {
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
		

        if (filename.endsWith("obj"))
        {
            if (!MeshManager::getInstance()->loadOBJFile(string(filename.toUtf8().constData())))
                return;
        }
        else if (filename.endsWith("hds"))
        {
            cout << "Open HDS file\n";
            MeshManager::getInstance()->loadHDSFile(string(filename.toUtf8().constData()));
        }
        else
        {
            return;
        }
		
#ifdef _DEBUG
		qDebug("Loading Object Takes %d ms In Total.", clock.elapsed());
		clock.restart();
#endif
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
		
#if USE_REEB_GRAPH
		MeshViewer::getInstance()->bindReebGraph(MeshManager::getInstance()->getReebGraph());
#endif

		MeshViewer::getInstance()->update();
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
#if _DEBUG
	cout << "saving file " << filename.toStdString() << endl;
#endif
	MeshManager::getInstance()->saveMeshes(filename.toStdString());
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
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

	}

}

void MainWindow::slot_unfoldMesh() {

	//if (MeshManager::getInstance()->getMeshStack()->canUnfold)
	{
		MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Unfolded);
		MeshManager::getInstance()->unfoldMesh();
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
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

void MainWindow::slot_triggerNeoWeaveMesh()
{
	MeshManager::getInstance()->getMeshStack()->reset();

	if (MeshManager::getInstance()->getMeshStack()->canRim)
	{
		Float shortestEdge = (Float)INT_MAX;
		HDS_Mesh* ori_mesh = MeshManager::getInstance()->getMeshStack()->getOriMesh();
		for (auto &he : ori_mesh->halfedges())
		{
			if (he.flip_offset>0) continue;
			shortestEdge = min(shortestEdge, ori_mesh->edgeVector(he).length());
		}

		//size corresponding to inch
		neowv_panel->setSize(shortestEdge/10.0f);
		neowv_panel->show();
		neowv_panel->activateWindow();
	}
}

void MainWindow::slot_triggerDForms()
{

}

void MainWindow::slot_triggerOrigamiMesh()
{
	MeshManager::getInstance()->getMeshStack()->reset();

	if (MeshManager::getInstance()->getMeshStack()->canRim)
	{
		origami_panel->show();
		origami_panel->activateWindow();
	}
	
}

void MainWindow::slot_quadEdge()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::QuadEdge);

	HDS_Bridger::setScale(quad_panel->getBridgerSize());
	if (MeshManager::getInstance()->setQuadEdge(
			quad_panel->getFlapSize(), quad_panel->getType(), quad_panel->getShift()))
	{
		statusBar()->showMessage("Succeeded to generate Quad-Edge Mesh!", 5000);
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
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
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
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
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to generate Rim Mesh...", 5000);
	}
}

void MainWindow::slot_weaveMesh()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Woven);

	if (MeshManager::getInstance()->setWeaveMesh(wv_panel->getConfig()))
	{
		statusBar()->showMessage("Succeeded to generate Woven Mesh!", 5000);
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to generate Woven Mesh...", 5000);
	}
}

void MainWindow::slot_neoWeaveMesh()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Woven);

	if (MeshManager::getInstance()->setNeoWeaveMesh(neowv_panel->getConfig()))
	{
		statusBar()->showMessage("Succeeded to generate Woven Mesh!", 5000);
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

	}
	else
	{
		statusBar()->showMessage("Failed to generate Woven Mesh...", 5000);
	}
}

void MainWindow::slot_origamiMesh()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Origami);
	confMap conf;
	if (MeshManager::getInstance()->setOrigamiMesh(origami_panel->getConfig()))
	{
		statusBar()->showMessage("Succeeded to generate Origami Mesh!", 5000);
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

	}
	else
	{
		statusBar()->showMessage("Failed to generate Origami Mesh...", 5000);
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
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to generate GRS Mesh...", 5000);
	}
}


void MainWindow::slot_smoothMesh()
{
	if (MeshManager::getInstance()->smoothMesh())
	{
		statusBar()->showMessage("Succeeded to smooth mesh!", 5000);
		MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
	}
	else
	{
		statusBar()->showMessage("Failed to smooth mesh...", 5000);
	}
}


void MainWindow::slot_undo()
{

	MeshManager::getInstance()->getMeshStack()->undo();
	MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());


}

void MainWindow::slot_redo()
{
	MeshManager::getInstance()->getMeshStack()->redo();
	MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

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
	MeshViewer::getInstance()->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

}

//========================================//

void MainWindow::slot_toggleEdges()
{
	// TODO: Check if toggleVertices/Edges/Faces/Normals work
	/*HDS_Mesh* curMesh = MeshManager::getInstance()->getMeshStack()->getCurrentMesh();
	if (curMesh)
	{
		curMesh->flipShowEdges();
	}
	MeshViewer::getInstance()->update();*/

}

void MainWindow::slot_toggleVertices()
{
	/*HDS_Mesh* curMesh = MeshManager::getInstance()->getMeshStack()->getCurrentMesh();
	if (curMesh)
	{
		curMesh->flipShowVertices();
	}

	MeshViewer::getInstance()->update();*/
}
void MainWindow::slot_toggleFaces()
{
	/*HDS_Mesh* curMesh = MeshManager::getInstance()->getMeshStack()->getCurrentMesh();
	if (curMesh)
	{
		curMesh->flipShowFaces();
	}
	MeshViewer::getInstance()->update();*/

}

void MainWindow::slot_toggleNormals()
{
	/*HDS_Mesh* curMesh = MeshManager::getInstance()->getMeshStack()->getCurrentMesh();
	if (curMesh)
	{
		curMesh->flipShowNormals();
	}
	MeshViewer::getInstance()->update();*/
}

void MainWindow::slot_toggleCameraOperation()
{
	MeshViewer::getInstance()->interactionState.state = 0;
}

void MainWindow::slot_toggleVertexSelection()
{
    MeshViewer::getInstance()->interactionState.state = 2;
}

void MainWindow::slot_toggleEdgeSelection()
{
    MeshViewer::getInstance()->interactionState.state = 4;
}

void MainWindow::slot_toggleFaceSelection()
{
    MeshViewer::getInstance()->interactionState.state = 8;
}

void MainWindow::slot_triggerColormap() {
	color_editor->show();
	color_editor->activateWindow();
}

void MainWindow::slot_triggerCriticalPoints() {
	actionsMap["show CP"]->setChecked(true);
	//kkkkkkkkkkk
	//MeshViewer::getInstance()->setCriticalPointsMethod(MeshViewer::getInstance()->getCmode());
	//MeshViewer::getInstance()->showCriticalPoints();
	MeshViewer::getInstance()->update();
	cp_panel->show();
}

void MainWindow::slot_triggerCutLocusPanel() {
	actionsMap["show CP"]->setChecked(cl_panel->isMinMaxChecked());

	//kkkkkkkkkkk
	//MeshViewer::getInstance()->setCutLocusMethod(MeshViewer::getInstance()->getLmode());
	if (cl_panel->isMinMaxChecked()) {
		//kkkkkkkkkkk
		//MeshViewer::getInstance()->showCutLocusPoints();
	}
	//MeshViewer::getInstance()->showCutLocusCut();
	MeshViewer::getInstance()->update();
	cl_panel->show();
}

void MainWindow::slot_disableclp()
{
	actionsMap["show CP"]->setChecked(cl_panel->isMinMaxChecked());
}

void MainWindow::slot_updateViewerColormap()
{
	// TODO: Implement Update Colormap with New Viewer
	// or Remove it
	//MeshViewer::getInstance()->setCurvatureColormap(ceditor->getColormap());
}

void MainWindow::slot_updateMeshColorByGeoDistance(int vidx) {
	MeshManager::getInstance()->colorMeshByGeoDistance(vidx);
	MeshViewer::getInstance()->update();
}

void MainWindow::slot_updateMeshColorByGeoDistance(int vidx, int lev0, int lev1, double ratio) {
	MeshManager::getInstance()->colorMeshByGeoDistance(vidx, lev0, lev1, ratio);
	MeshViewer::getInstance()->update();
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
	// TODO: Implement Critical Point Method with New Viewer
	//MeshViewer::getInstance()->setCriticalPointsMethod(midx);
	MeshViewer::getInstance()->update();
}

void MainWindow::slot_updateCriticalPointsSmoothingTimes(int times) {

	// TODO: Implement Critical Point Smooth Time Method with New Viewer
	//MeshViewer::getInstance()->setCriticalPointsSmoothingTimes(times);
	MeshViewer::getInstance()->update();
}

void MainWindow::slot_updateCriticalPointsSmoothingType(int t) {
	cout << "Smoothing type = " << t << endl;
	// TODO: Implement Critical Point Smoothing Type Method with New Viewer
	//MeshViewer::getInstance()->setCriticalPointsSmoothingType(t);

	MeshViewer::getInstance()->update();
}

void MainWindow::slot_updateCutLocusMethod(int midx)
{
	// TODO: Implement Cut Locus Method with New Viewer
	//MeshViewer::getInstance()->setCutLocusMethod(midx);
	MeshViewer::getInstance()->update();

}


