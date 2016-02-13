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
	setWindowTitle(tr("Geometry Unfolding"));

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

	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Original);
	QString curPath = QDir::currentPath();
	QString filename = curPath + "/meshes/cube.obj";
	if (MeshManager::getInstance()->loadOBJFile(string(filename.toUtf8().constData()))) {
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());
		//meshStack.push((CurrentMesh)Original);
		//updateCurrentMesh();
	}
}

bool MainWindow::createComponents()
{
	try {
		viewer = new viewer_t(this);
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
	setCentralWidget(viewer);

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
		//file menu
		QAction *newAct = new QAction(QIcon(":/icons/open.png"), tr("Import"), this);
		newAct->setShortcuts(QKeySequence::New);
		newAct->setStatusTip(tr("Import a new obj file"));
		connect(newAct, &QAction::triggered, this, &MainWindow::slot_newFile);
		actionsMap["new"] = newAct;

		QAction *exportAct = new QAction(QIcon(":/icons/export.png"), tr("Export"), this);//new added
		exportAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
		exportAct->setStatusTip(tr("Export data as"));
		connect(exportAct, &QAction::triggered, this, &MainWindow::slot_exportFile);//need to change
		actionsMap["export"] = exportAct;

		QAction *closeAct = new QAction(QIcon(":/icons/close.png"), tr("Close"), this);
		closeAct->setShortcuts(QKeySequence::Quit);
		closeAct->setStatusTip(tr("close a file"));
		connect(closeAct, &QAction::triggered, this, &MainWindow::slot_closeFile);
		actionsMap["close"] = closeAct;

		QAction *saveAct = new QAction(QIcon(":/icons/save.png"), tr("Save"), this);
		saveAct->setShortcuts(QKeySequence::Save);
		saveAct->setStatusTip(tr("Save a file"));
		connect(saveAct, &QAction::triggered, this, &MainWindow::slot_saveFile);
		actionsMap["save"] = saveAct;



		//Edit Menu
		QAction *resetAct = new QAction(QIcon(":/icons/reset.png"), tr("Reset mesh"), this);
		resetAct->setStatusTip(tr("Reset"));
		connect(resetAct, &QAction::triggered, this, &MainWindow::slot_reset);
		actionsMap["reset"] = resetAct;

		QAction *undoAct = new QAction(QIcon(":/icons/undo.png"), tr("Undo last operation"), this);
		undoAct->setStatusTip(tr("Undo last mesh operation"));
		undoAct->setCheckable(false);
		connect(undoAct, &QAction::triggered, this, &MainWindow::slot_undo);
		actionsMap["mesh undo"] = undoAct;

		QAction *redoAct = new QAction(QIcon(":/icons/redo.png"), tr("Redo last operation"), this);
		redoAct->setStatusTip(tr("Redo last mesh operation"));
		redoAct->setCheckable(false);
		connect(redoAct, &QAction::triggered, this, &MainWindow::slot_redo);
		actionsMap["mesh redo"] = redoAct;

		//selection menu

		QAction *selectAllAct = new QAction(tr("Select All"), this);
		selectAllAct->setShortcut(QKeySequence::SelectAll);
		selectAllAct->setStatusTip(tr("Select All"));
		connect(selectAllAct, &QAction::triggered, viewer, &viewer_t::selectAll);
		actionsMap["select all"] = selectAllAct;

		QAction *selectInverseAct = new QAction(tr("Select Inverse"), this);
		selectInverseAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
		selectInverseAct->setStatusTip(tr("Select Inverse"));
		connect(selectInverseAct, &QAction::triggered, viewer, &viewer_t::selectInverse);
		actionsMap["select inverse"] = selectInverseAct;

		QAction *selectMultipleAct = new QAction(tr("Select Multiple"), this);
		selectMultipleAct->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_M));
		selectMultipleAct->setStatusTip(tr("Select Multiple"));
		connect(selectMultipleAct, &QAction::triggered, this, &MainWindow::slot_selectMultiple);
		actionsMap["select multiple"] = selectMultipleAct;

		QAction *selectCutEdgePairAct = new QAction(tr("Select Twin Pair"), this);
		selectCutEdgePairAct->setStatusTip(tr("Select the counterpart for current selected elements"));
		//kkkkkkkkkkk
		//connect(selectCutEdgePairAct, &QAction::triggered, viewer, &viewer_t::selectTwinPair()));
		actionsMap["select cut edge pair"] = selectCutEdgePairAct;

		QAction *selectnextEdgeAct = new QAction(tr("Select Next Edge"), this);
		selectnextEdgeAct->setStatusTip(tr("Select next half edge"));
		//kkkkkkkkkkk
		//connect(selectnextEdgeAct, &QAction::triggered, viewer, &viewer_t::selectNextEdge()));
		actionsMap["select next edge"] = selectnextEdgeAct;

		QAction *selectCPAct = new QAction(tr("Select Critical Points"), this);
		selectCPAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
		selectCPAct->setStatusTip(tr("Select All Critical Points (turn into minimum points)"));
		//kkkkkkkkkkk
		//connect(selectCPAct, &QAction::triggered, viewer, &viewer_t::selectCP()));
		actionsMap["select cp"] = selectCPAct;

		QAction *selectMSTEdgesAct = new QAction(tr("Select MST Edges"), this);
		selectMSTEdgesAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
		selectMSTEdgesAct->setStatusTip(tr("Select Minimal Spanning Tree based cuttable edges"));
		//kkkkkkkkkkk
		//connect(selectMSTEdgesAct, &QAction::triggered, viewer, &viewer_t::selectMSTEdges()));
		actionsMap["select mst"] = selectMSTEdgesAct;


		QAction *selectCCAct = new QAction(tr("Select Connected Component"), this);
		selectCCAct->setShortcut(QKeySequence(Qt::Key_Asterisk));
		selectCCAct->setStatusTip(tr("Select Connected Component"));
		//kkkkkkkkkkk
		//connect(selectCCAct, &QAction::triggered, viewer, &viewer_t::selectCC()));
		actionsMap["select cc"] = selectCCAct;

		QAction *selectGrowAct = new QAction(tr("Grow Selection"), this);
		selectGrowAct->setShortcut(QKeySequence(Qt::Key_Equal));
		selectGrowAct->setStatusTip(tr("Grow Selection"));
		//kkkkkkkkkkk
		//connect(selectGrowAct, &QAction::triggered, viewer, &viewer_t::selectGrow()));
		actionsMap["select grow"] = selectGrowAct;

		QAction *selectShrinkAct = new QAction(tr("Shrink Selection"), this);
		selectShrinkAct->setShortcut(QKeySequence(Qt::Key_Minus));
		selectShrinkAct->setStatusTip(tr("Shrink Selection"));
		//kkkkkkkkkkk
		//connect(selectShrinkAct, &QAction::triggered, viewer, &viewer_t::selectShrink()));
		actionsMap["select shrink"] = selectShrinkAct;

		QAction *selectClearAct = new QAction(tr("Clear Selection"), this);
		selectClearAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Backspace));
		selectClearAct->setStatusTip(tr("Clear Selection"));
		//kkkkkkkkkkk
		//connect(selectClearAct, &QAction::triggered, viewer, &viewer_t::selectClear()));
		actionsMap["select clear"] = selectClearAct;


		//display menu
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

		QAction *camAct = new QAction(QIcon(":/icons/select.png"), tr("Camera Operation"), this);
		camAct->setStatusTip(tr("Camera operation"));
		camAct->setCheckable(true);
		camAct->setChecked(true);
		connect(camAct, &QAction::triggered, this, &MainWindow::slot_toggleCameraOperation);
		actionsMap["camera"] = camAct;

		QAction *fsAct = new QAction(QIcon(":/icons/face.png"), tr("Face Select"), this);
		fsAct->setStatusTip(tr("Select faces"));
		fsAct->setCheckable(true);
		fsAct->setChecked(false);
		connect(fsAct, &QAction::triggered, this, &MainWindow::slot_toggleFaceSelection);
		actionsMap["face select"] = fsAct;

		QAction *esAct = new QAction(QIcon(":/icons/edge.png"), tr("Edge Select"), this);
		esAct->setStatusTip(tr("Select edges"));
		esAct->setCheckable(true);
		esAct->setChecked(false);
		connect(esAct, &QAction::triggered, this, &MainWindow::slot_toggleEdgeSelection);
		actionsMap["edge select"] = esAct;

		QAction *vsAct = new QAction(QIcon(":/icons/vertex.png"), tr("Vertex Select"), this);
		vsAct->setStatusTip(tr("Select vertices"));
		vsAct->setCheckable(true);
		vsAct->setChecked(false);
		connect(vsAct, &QAction::triggered, this, &MainWindow::slot_toggleVertexSelection);
		actionsMap["vertex select"] = vsAct;

		QAction *smoothAct = new QAction(QIcon(":/icons/smooth.png"), tr("Smooth Mesh"), this);
		smoothAct->setStatusTip(tr("Smooth mesh"));
		connect(smoothAct, &QAction::triggered, this, &MainWindow::slot_smoothMesh);
		actionsMap["smooth"] = smoothAct;

		QAction *extendAct = new QAction(QIcon(":/icons/extend.png"), tr("Extend Mesh"), this);
		extendAct->setStatusTip(tr("Extend mesh"));
		extendAct->setCheckable(false);
		extendAct->setChecked(false);
		connect(extendAct, &QAction::triggered, this, &MainWindow::slot_triggerExtendMesh);
		actionsMap["extend"] = extendAct;

		QAction *hollowAct = new QAction(QIcon(":/icons/HollowSphere.png"), tr("Generate Hollow Mesh"), this);
		hollowAct->setStatusTip(tr("Generate Hollow Mesh"));
		hollowAct->setCheckable(false);
		hollowAct->setChecked(false);
		connect(hollowAct, &QAction::triggered, this, &MainWindow::slot_triggerHollowMesh);
		actionsMap["hollow"] = hollowAct;

		QAction *bindingAct = new QAction(QIcon(":/icons/bind.png"), tr("Generate Binding Composition Mesh"), this);
		bindingAct->setStatusTip(tr("Generate Bind Mesh"));
		bindingAct->setCheckable(false);
		bindingAct->setChecked(false);
		connect(bindingAct, &QAction::triggered, this, &MainWindow::slot_triggerBindingMesh);
		actionsMap["bind"] = bindingAct;
		
		QAction *rimfaceAct = new QAction(QIcon(":/icons/rimface.png"), tr("Rim faces 2D"), this);
		rimfaceAct->setStatusTip(tr("Rim Faces"));
		rimfaceAct->setCheckable(false);
		rimfaceAct->setChecked(false);
		connect(rimfaceAct, &QAction::triggered, this, &MainWindow::slot_triggerRimmedMesh);
		actionsMap["rimface"] = rimfaceAct;

		QAction *rimface3DAct = new QAction(QIcon(":/icons/rimface_3d.png"), tr("Rim faces 3D"), this);
		rimface3DAct->setStatusTip(tr("Rim Faces"));
		rimface3DAct->setCheckable(false);
		rimface3DAct->setChecked(false);
		connect(rimface3DAct, &QAction::triggered, this, &MainWindow::slot_triggerRimmed3DMesh);
		actionsMap["rimface3d"] = rimface3DAct;

		QAction *cutAct = new QAction(QIcon(":/icons/cut.png"), tr("Cut"), this);
		cutAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_C));
		cutAct->setStatusTip(tr("Cut mesh (ALT+C)"));
		connect(cutAct, &QAction::triggered, this, &MainWindow::slot_performMeshCut);
		actionsMap["mesh cut"] = cutAct;


		QAction *unfoldAct = new QAction(QIcon(":/icons/unfold.png"), tr("Unfold"), this);
		unfoldAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_U));
		unfoldAct->setStatusTip(tr("Unfold mesh (ALT+U)"));
		unfoldAct->setCheckable(true);
		unfoldAct->setChecked(false);
		connect(unfoldAct, &QAction::toggled, this, &MainWindow::slot_unfoldMesh);
		actionsMap["mesh unfold"] = unfoldAct;


		QAction *colormapAct = new QAction(QIcon(":/icons/colormap.png"), tr("Colormap"), this);
		colormapAct->setStatusTip(tr("Color map"));
		connect(colormapAct, &QAction::triggered, this, &MainWindow::slot_triggerColormap);
		actionsMap["colormap"] = colormapAct;

		QAction *cpAct = new QAction(QIcon(":/icons/cp.png"), tr("Critical Points"), this);
		cpAct->setStatusTip(tr("Critical points"));
		connect(cpAct, &QAction::triggered, this, &MainWindow::slot_triggerCriticalPoints);
		actionsMap["critical_points"] = cpAct;

		QAction *clAct = new QAction(QIcon(":/icons/cl.png"), tr("Cut Locus"), this);
		clAct->setStatusTip(tr("Cut Locus"));
		connect(clAct, &QAction::triggered, this, &MainWindow::slot_triggerCutLocusPanel);
		actionsMap["cut_locus"] = clAct;

	}
	catch(...) {
		throw UnfoldingAppException("Failed to create actions!");
	}
}

void MainWindow::createMenus()
{
	try {
		QMenu *fileMenu = ui->menuBar->addMenu(tr("&File"));



		fileMenu->addAction(actionsMap["new"]);
		fileMenu->addAction(actionsMap["export"]);
		fileMenu->addAction(actionsMap["close"]);

		QMenu *editMenu = ui->menuBar->addMenu(tr("&Edit"));
		editMenu->addAction(actionsMap["mesh undo"]);
		editMenu->addAction(actionsMap["mesh redo"]);
		editMenu->addAction(actionsMap["reset"]);

		QMenu *selectionMenu = ui->menuBar->addMenu(tr("&Selection"));
		selectionMenu->addAction(actionsMap["select all"]);
		selectionMenu->addAction(actionsMap["select inverse"]);
		selectionMenu->addAction(actionsMap["select multiple"]);
		selectionMenu->addAction(actionsMap["select cc"]);

		selectionMenu->addSeparator();
		selectionMenu->addAction(actionsMap["select cut edge pair"]);
		selectionMenu->addAction(actionsMap["select next edge"]);
		selectionMenu->addSeparator();
		selectionMenu->addAction(actionsMap["select mst"]);
		selectionMenu->addAction(actionsMap["select cp"]);
		selectionMenu->addSeparator();
		selectionMenu->addAction(actionsMap["select grow"]);
		selectionMenu->addAction(actionsMap["select shrink"]);
		selectionMenu->addAction(actionsMap["select clear"]);

		QMenu *displayMenu = ui->menuBar->addMenu(tr("&Display"));
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
		ui->mainToolBar->addAction(actionsMap["new"]);
		ui->mainToolBar->addAction(actionsMap["save"]);
		ui->mainToolBar->addAction(actionsMap["export"]);
		ui->mainToolBar->addSeparator();

		ui->mainToolBar->addAction(actionsMap["reset"]);
		ui->mainToolBar->addAction(actionsMap["mesh undo"]);
		ui->mainToolBar->addAction(actionsMap["mesh redo"]);

		ui->mainToolBar->addSeparator();

		QActionGroup *selectGroup = new QActionGroup(ui->mainToolBar);
		selectGroup->addAction(actionsMap["camera"]);
		selectGroup->addAction(actionsMap["face select"]);
		selectGroup->addAction(actionsMap["edge select"]);
		selectGroup->addAction(actionsMap["vertex select"]);
		selectGroup->setExclusive(true);

		//QActionGroup *unfoldGroup = new QActionGroup(ui->mainToolBar);
		//unfoldGroup->addAction(actionsMap["mesh unfold"]);
		//unfoldGroup->addAction(actionsMap["mesh fold"]);


		ui->mainToolBar->addAction(actionsMap["camera"]);

		ui->mainToolBar->addSeparator();
		ui->mainToolBar->addAction(actionsMap["face select"]);
		ui->mainToolBar->addAction(actionsMap["edge select"]);
		ui->mainToolBar->addAction(actionsMap["vertex select"]);

		ui->mainToolBar->addSeparator();
		ui->mainToolBar->addAction(actionsMap["smooth"]);
		ui->mainToolBar->addAction(actionsMap["extend"]);
		ui->mainToolBar->addAction(actionsMap["hollow"]);
		ui->mainToolBar->addAction(actionsMap["bind"]);
		ui->mainToolBar->addAction(actionsMap["rimface"]);
		ui->mainToolBar->addAction(actionsMap["rimface3d"]);

		ui->mainToolBar->addSeparator();
		ui->mainToolBar->addAction(actionsMap["mesh cut"]);

		//QToolButton *cutButton = dynamic_cast<QToolButton*>(ui->mainToolBar->widgetForAction(actionsMap["mesh cut"]));
		//cutButton->setPopupMode(QToolButton::InstantPopup);
		//cutButton->addAction(new QAction("Expanded Cut", this));

		ui->mainToolBar->addAction(actionsMap["mesh unfold"]);

		ui->mainToolBar->addSeparator();
		ui->mainToolBar->addAction(actionsMap["colormap"]);
		ui->mainToolBar->addAction(actionsMap["critical_points"]);
		ui->mainToolBar->addAction(actionsMap["cut_locus"]);
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

	QString filename = QFileDialog::getOpenFileName(this, "Select an OBJ file",  "meshes/", tr("OBJ files(*.obj)")); //later added
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
		MeshManager::getInstance()->loadOBJFile(string(filename.toUtf8().constData()));
		
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
	conpanel->setSaveMode((sender() == actionsMap["extend"] )? true:false);
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

	actionsMap["hollow"]->setChecked(false);
	}
}

void MainWindow::slot_triggerBindingMesh(bool checked)
{
	if (MeshManager::getInstance()->getMeshStack()->canBind)
	{
		bmpanel->show();
		bmpanel->activateWindow();

	}

	actionsMap["bind"]->setChecked(false);
}

void MainWindow::slot_triggerRimmedMesh(bool checked)
{

	if (MeshManager::getInstance()->getMeshStack()->canRim)
	{
		MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Rimmed);

		MeshManager::getInstance()->rimMesh();
		viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

	}

	actionsMap["rimface"]->setChecked(false);
}

void MainWindow::slot_triggerRimmed3DMesh()
{
    if (MeshManager::getInstance()->getMeshStack()->canRim)
	{
		rmpanel->show();
		rmpanel->activateWindow();

	}

	actionsMap["rimface3d"]->setChecked(false);
}

void MainWindow::slot_rimmed3DMesh()
{
	MeshManager::getInstance()->getMeshStack()->setCurrentFlag(OperationStack::Rimmed);

	MeshManager::getInstance()->set3DRimMesh(rmpanel->getW(), rmpanel->getH());
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
	actionsMap["mesh unfold"]->setChecked(false);
	actionsMap["extend"]->setChecked(false);
	actionsMap["hollow"]->setChecked(false);

	//reset mesh
	//MeshManager::getInstance()->resetMesh();

	MeshManager::getInstance()->getMeshStack()->reset();
	viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getMeshStack()->getCurrentMesh());

}

//========================================//

void MainWindow::slot_selectMultiple()
{
	//kkkkkkkkkkk
	viewer->setSelectionMode(viewer_t::MultiSelect);
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
	viewer->setInteractionMode(viewer_t::Camera);
}

void MainWindow::slot_toggleFaceSelection()
{
	viewer->setInteractionMode(viewer_t::SelectFace);
}

void MainWindow::slot_toggleEdgeSelection()
{
	viewer->setInteractionMode(viewer_t::SelectEdge);
}

void MainWindow::slot_toggleVertexSelection()
{
	viewer->setInteractionMode(viewer_t::SelectVertex);
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

void MainWindow::slot_disableclp() {
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


