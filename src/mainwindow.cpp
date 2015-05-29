#define NOMINMAX
#include "meshmanager.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "meshviewer.h"

#include <QMessageBox>
#include <QString>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("Object Unfolding"));

    createComponents();
    layoutComponents();
    connectComponents();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::createComponents()
{
    try {
        viewer = new MeshViewer(this);
        ceditor = new ColormapEditor;
        cppanel = new CriticalPointsPanel;
        clpanel = new CutLocusPanel;
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
    connect(ceditor, SIGNAL(colorChanged()), this, SLOT(slot_updateViewerColormap()));
    connect(viewer, SIGNAL(updateMeshColorByGeoDistance(int)), this, SLOT(slot_updateMeshColorByGeoDistance(int)));
    connect(viewer, SIGNAL(updateMeshColorByGeoDistance(int, int, int, double)), this, SLOT(slot_updateMeshColorByGeoDistance(int, int, int, double)));

    connect(cppanel, SIGNAL(sig_methodChanged(int)), this, SLOT(slot_updateCriticalPointsMethod(int)));
    connect(cppanel, SIGNAL(sig_smoothingTimesChanged(int)), this, SLOT(slot_updateCriticalPointsSmoothingTimes(int)));
    connect(cppanel, SIGNAL(sig_smoothingTypeChanged(int)), this, SLOT(slot_updateCriticalPointsSmoothingType(int)));

    connect(clpanel, SIGNAL(sig_methodChanged(int)), this, SLOT(slot_updateCutLocusMethod(int)));
    //connect(clpanel, SIGNAL(sig_displayCut(bool)), this, );  //perform cut based on selected vertices
    connect(clpanel, SIGNAL(sig_displayMinMax(bool)), this, SLOT(slot_toggleMinMaxPoints(bool)));

    return true;
}

void MainWindow::createActions()
{
    try {
        //file menu
        QAction *newAct = new QAction(QIcon(":/icons/open.png"), tr("New"), this);
        newAct->setShortcuts(QKeySequence::New);
        newAct->setStatusTip(tr("Create a new file"));
        connect(newAct, SIGNAL(triggered()), this, SLOT(slot_newFile()));
		actionsMap["new"] = newAct;

		QAction *exportAct = new QAction(QIcon(":/icons/export.png"), tr("Export"), this);//new added
		exportAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
		exportAct->setStatusTip(tr("Export data as"));
		connect(exportAct, SIGNAL(triggered()), this, SLOT(slot_exportFile()));//need to change
		actionsMap["export"] = exportAct;

		QAction *closeAct = new QAction(QIcon(":/icons/close.png"), tr("Close"), this);//later added
		closeAct->setShortcuts(QKeySequence::Quit);
		closeAct->setStatusTip(tr("close a file"));
		connect(closeAct, SIGNAL(triggered()), this, SLOT(slot_closeFile()));
		actionsMap["close"] = closeAct;

        //Edit Menu
        QAction *resetAct = new QAction(tr("Reset"), this);
        resetAct->setStatusTip(tr("Reset"));
        connect(resetAct, SIGNAL(triggered()), this, SLOT(slot_reset()));
        actionsMap["reset"] = resetAct;

        //selection menu

        QAction *selectAllAct = new QAction(tr("Select All"), this);
        selectAllAct->setShortcut(QKeySequence::SelectAll);
        selectAllAct->setStatusTip(tr("Select All"));
        connect(selectAllAct, SIGNAL(triggered()), this, SLOT(slot_selectAll()));
        actionsMap["select all"] = selectAllAct;

        QAction *selectInverseAct = new QAction(tr("Select Inverse"), this);
        selectInverseAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
        selectInverseAct->setStatusTip(tr("Select Inverse"));
        connect(selectInverseAct, SIGNAL(triggered()), this, SLOT(slot_selectInverse()));
        actionsMap["select inverse"] = selectInverseAct;

        QAction *selectMultipleAct = new QAction(tr("Select Multiple"), this);
        selectMultipleAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
        selectMultipleAct->setStatusTip(tr("Select Multiple"));
        connect(selectMultipleAct, SIGNAL(triggered()), this, SLOT(slot_selectMultiple()));
        actionsMap["select multiple"] = selectMultipleAct;
        
        QAction *selectCCAct = new QAction(tr("Select Connected Component"), this);
        selectCCAct->setShortcut(QKeySequence(Qt::Key_Asterisk));
        selectCCAct->setStatusTip(tr("Select Connected Component"));
        connect(selectCCAct, SIGNAL(triggered()), this, SLOT(slot_selectCC()));
        actionsMap["select cc"] = selectCCAct;
        
        QAction *selectGrowAct = new QAction(tr("Grow Selection"), this);
        selectGrowAct->setShortcut(QKeySequence(Qt::Key_Equal));
        selectGrowAct->setStatusTip(tr("Select Connected Component"));
        connect(selectGrowAct, SIGNAL(triggered()), this, SLOT(slot_selectGrow()));
        actionsMap["select grow"] = selectGrowAct;

        QAction *selectShrinkAct = new QAction(tr("Shrink Selection"), this);
        selectShrinkAct->setShortcut(QKeySequence(Qt::Key_Minus));
        selectShrinkAct->setStatusTip(tr("Select Connected Component"));
        connect(selectShrinkAct, SIGNAL(triggered()), this, SLOT(slot_selectShrink()));
        actionsMap["select shrink"] = selectShrinkAct;

        QAction *selectClearAct = new QAction(tr("Clear Selection"), this);
        selectClearAct->setShortcut(QKeySequence(Qt::Key_Backspace));
        selectClearAct->setStatusTip(tr("Clear Selection"));
        connect(selectClearAct, SIGNAL(triggered()), this, SLOT(slot_selectClear()));
        actionsMap["select clear"] = selectClearAct;

        
        //display menu
        //"C: mesh color  E: edges  V : vertices  F : faces  L : lighting  R : critical points  M : change critical point mode "
        QAction *showEdgesAct = new QAction(tr("Show &Edges"), this);
        showEdgesAct->setCheckable(true);
        showEdgesAct->setChecked(true);
        showEdgesAct->setStatusTip(tr("Show &Edges"));
        connect(showEdgesAct, SIGNAL(triggered()), this, SLOT(slot_showEdges()));
        actionsMap["show edges"] = showEdgesAct;

        //main menu bar

        QAction *saveAct = new QAction(QIcon(":/icons/save.png"), tr("Save"), this);
        saveAct->setShortcuts(QKeySequence::Save);
        saveAct->setStatusTip(tr("Save a file"));
        connect(saveAct, SIGNAL(triggered()), this, SLOT(slot_saveFile()));
        actionsMap["save"] = saveAct;

        QAction *camAct = new QAction(QIcon(":/icons/select.png"), tr("Camera Operation"), this);
        camAct->setStatusTip(tr("Camera operation"));
        camAct->setCheckable(true);
        camAct->setChecked(true);
        connect(camAct, SIGNAL(triggered()), this, SLOT(slot_toggleCameraOperation()));
        actionsMap["camera"] = camAct;

        QAction *fsAct = new QAction(QIcon(":/icons/face.png"), tr("Face Select"), this);
        fsAct->setStatusTip(tr("Select faces"));
        fsAct->setCheckable(true);
        connect(fsAct, SIGNAL(triggered()), this, SLOT(slot_toggleFaceSelection()));
        actionsMap["face select"] = fsAct;

        QAction *esAct = new QAction(QIcon(":/icons/edge.png"), tr("Edge Select"), this);
        esAct->setStatusTip(tr("Select edges"));
        esAct->setCheckable(true);
        connect(esAct, SIGNAL(triggered()), this, SLOT(slot_toggleEdgeSelection()));
        actionsMap["edge select"] = esAct;

        QAction *vsAct = new QAction(QIcon(":/icons/vertex.png"), tr("Vertex Select"), this);
        vsAct->setStatusTip(tr("Select vertices"));
        vsAct->setCheckable(true);
        connect(vsAct, SIGNAL(triggered()), this, SLOT(slot_toggleVertexSelection()));
        actionsMap["vertex select"] = vsAct;

        QAction *smoothAct = new QAction(QIcon(":/icons/smooth.png"), tr("Smooth Mesh"), this);
        smoothAct->setStatusTip(tr("Smooth mesh"));
        connect(smoothAct, SIGNAL(triggered()), this, SLOT(slot_smoothMesh()));
        actionsMap["smooth"] = smoothAct;

        QAction *extendAct = new QAction(QIcon(":/icons/extend.png"), tr("Extend Mesh"), this);
        extendAct->setStatusTip(tr("Extend mesh"));
        connect(extendAct, SIGNAL(triggered()), this, SLOT(slot_extendMesh()));
        actionsMap["extend"] = extendAct;

        QAction *cutAct = new QAction(QIcon(":/icons/cut.png"), tr("Cut"), this);
		cutAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_C));
        cutAct->setStatusTip(tr("Cut mesh (ALT+C)"));
        connect(cutAct, SIGNAL(triggered()), this, SLOT(slot_performMeshCut()));
        actionsMap["mesh cut"] = cutAct;

        QAction *unfoldAct = new QAction(QIcon(":/icons/unfold.png"), tr("Unfold"), this);
		unfoldAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_U));
        unfoldAct->setStatusTip(tr("Unfold mesh (ALT+U)"));
        unfoldAct->setCheckable(true);
        unfoldAct->setChecked(false);
        connect(unfoldAct, SIGNAL(triggered()), this, SLOT(slot_unfoldMesh()));
        actionsMap["mesh unfold"] = unfoldAct;

        QAction *foldAct = new QAction(QIcon(":/icons/fold.png"), tr("Fold"), this);
        foldAct->setStatusTip(tr("Fold"
                                 " mesh"));
        foldAct->setCheckable(true);
        foldAct->setChecked(true);
        connect(foldAct, SIGNAL(triggered()), this, SLOT(slot_reset()));
        actionsMap["mesh fold"] = foldAct;

        QAction *colormapAct = new QAction(QIcon(":/icons/colormap.png"), tr("Colormap"), this);
        colormapAct->setStatusTip(tr("Color map"));
        connect(colormapAct, SIGNAL(triggered()), this, SLOT(slot_triggerColormap()));
        actionsMap["colormap"] = colormapAct;

        QAction *cpAct = new QAction(QIcon(":/icons/cp.png"), tr("Critical Points"), this);
        cpAct->setStatusTip(tr("Critical points"));
        connect(cpAct, SIGNAL(triggered()), this, SLOT(slot_triggerCriticalPoints()));
        actionsMap["critical_points"] = cpAct;

        QAction *clAct = new QAction(QIcon(":/icons/cl.png"), tr("Cut Locus"), this);
        clAct->setStatusTip(tr("Cut Locus"));
        connect(clAct, SIGNAL(triggered()), this, SLOT(slot_triggerCutLocusPanel()));
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
        fileMenu->addAction(actionsMap["close"]);//later added;
        //fileMenu->addAction(actionsMap["print"]);//later added

        QMenu *editMenu = ui->menuBar->addMenu(tr("&Edit"));
        editMenu->addAction(actionsMap["reset"]);

        QMenu *selectionMenu = ui->menuBar->addMenu(tr("&Selection"));
        selectionMenu->addAction(actionsMap["select all"]);
        selectionMenu->addAction(actionsMap["select inverse"]);
        selectionMenu->addAction(actionsMap["select multiple"]);
        selectionMenu->addAction(actionsMap["select cc"]);
        selectionMenu->addSeparator();
        selectionMenu->addAction(actionsMap["select grow"]);
        selectionMenu->addAction(actionsMap["select shrink"]);
        selectionMenu->addAction(actionsMap["select clear"]);

        QMenu *displayMenu = ui->menuBar->addMenu(tr("&Display"));
        displayMenu->addAction(actionsMap["show edges"]);

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
        ui->mainToolBar->addSeparator();

        QActionGroup *selectGroup = new QActionGroup(ui->mainToolBar);
        selectGroup->addAction(actionsMap["camera"]);
        selectGroup->addAction(actionsMap["face select"]);
        selectGroup->addAction(actionsMap["edge select"]);
        selectGroup->addAction(actionsMap["vertex select"]);
        selectGroup->setExclusive(true);

        QActionGroup *unfoldGroup = new QActionGroup(ui->mainToolBar);
        unfoldGroup->addAction(actionsMap["mesh unfold"]);
        unfoldGroup->addAction(actionsMap["mesh fold"]);
        unfoldGroup->setExclusive(true);

        ui->mainToolBar->addAction(actionsMap["camera"]);

        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addAction(actionsMap["face select"]);
        ui->mainToolBar->addAction(actionsMap["edge select"]);
        ui->mainToolBar->addAction(actionsMap["vertex select"]);

        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addAction(actionsMap["smooth"]);
        ui->mainToolBar->addAction(actionsMap["extend"]);

        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addAction(actionsMap["mesh cut"]);
        ui->mainToolBar->addAction(actionsMap["mesh unfold"]);
        ui->mainToolBar->addAction(actionsMap["mesh fold"]);

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
    cout << "loading a new obj file." << endl;

    QString filename = QFileDialog::getOpenFileName(this, "Select an OBJ file",  "../meshes", tr("OBJ files(*.obj)")); //later added
    MeshManager::getInstance()->loadOBJFile(string(filename.toUtf8().constData()));
    viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getHalfEdgeMesh());
#if USE_REEB_GRAPH
    viewer->bindReebGraph(MeshManager::getInstance()->getReebGraph());
#endif
}

void MainWindow::slot_exportFile()
{
	cout << "Export as..." << endl;
	QString filename = QFileDialog::getSaveFileName(this, "Export file as", "../untitled.svg", tr("XML files (*.svg *.xml)"));
	MeshManager::getInstance()->exportXMLFile(filename.toUtf8().constData());
	//Needed to be implemented
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

void MainWindow::slot_selectAll()
{

}

void MainWindow::slot_selectInverse()
{

}

void MainWindow::slot_selectMultiple()
{

}

void MainWindow::slot_selectCC()
{

}

void MainWindow::slot_selectGrow()
{

}

void MainWindow::slot_selectShrink()
{

}

void MainWindow::slot_selectClear()
{

}

void MainWindow::slot_toggleCameraOperation()
{
    viewer->setInteractionMode(MeshViewer::Camera);
}

void MainWindow::slot_toggleFaceSelection()
{
    viewer->setInteractionMode(MeshViewer::SelectFace);
}

void MainWindow::slot_toggleEdgeSelection()
{
    viewer->setInteractionMode(MeshViewer::SelectEdge);
}

void MainWindow::slot_toggleVertexSelection()
{
    viewer->setInteractionMode(MeshViewer::SelectVertex);
}

void MainWindow::slot_performMeshCut() {
    MeshManager::getInstance()->cutMeshWithSelectedEdges();
    viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getCuttedMesh());
}

void MainWindow::slot_unfoldMesh() {
    MeshManager::getInstance()->unfoldMesh();
    viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getUnfoldedMesh());
}

void MainWindow::slot_triggerColormap() {
    ceditor->show();
    ceditor->activateWindow();
}

void MainWindow::slot_triggerCriticalPoints() {
    viewer->showCriticalPoints();
    viewer->setCriticalPointsMethod(viewer->getCmode());
    viewer->update();
    cppanel->show();
}

void MainWindow::slot_triggerCutLocusPanel() {
    viewer->showCriticalPoints();
    viewer->setCutLocusMethod(viewer->getLmode());
    viewer->update();
    clpanel->show();
}

void MainWindow::slot_updateViewerColormap()
{
    viewer->setCurvatureColormap(ceditor->getColormap());
}

void MainWindow::slot_updateMeshColorByGeoDistance(int vidx) {
    MeshManager::getInstance()->colorMeshByGeoDistance(vidx);
    viewer->update();
}

void MainWindow::slot_updateMeshColorByGeoDistance(int vidx, int lev0, int lev1, double ratio) {
    MeshManager::getInstance()->colorMeshByGeoDistance(vidx, lev0, lev1, ratio);
    viewer->update();
}

void MainWindow::slot_smoothMesh() {
    MeshManager::getInstance()->smoothMesh();
    viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getSmoothedMesh());
}

void MainWindow::closeEvent(QCloseEvent *e) {
    ceditor->close();
    cppanel->close();
    clpanel->close();

}

void MainWindow::slot_extendMesh()
{
    MeshManager::getInstance()->extendMesh();
    viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getExtendedMesh());
}

void MainWindow::slot_reset()
{
    viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getHalfEdgeMesh());
}


void MainWindow::slot_updateCriticalPointsMethod(int midx) {
    viewer->setCriticalPointsMethod(midx);
    viewer->update();
}

void MainWindow::slot_updateCriticalPointsSmoothingTimes(int times) {
    viewer->setCriticalPointsSmoothingTimes(times);
    viewer->update();
}

void MainWindow::slot_updateCriticalPointsSmoothingType(int t) {
    cout << "Smoothing type = " << t << endl;
    viewer->setCriticalPointsSmoothingType(t);

    viewer->update();
}

void MainWindow::slot_updateCutLocusMethod(int midx)
{
    viewer->setCutLocusMethod(midx);
    viewer->update();

}

void MainWindow::slot_toggleMinMaxPoints(bool checked)
{
    //show min max points
    viewer->toggleCriticalPoints();
    viewer->update();
}

