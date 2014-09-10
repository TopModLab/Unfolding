#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "meshmanager.h"

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
  return true;
}

void MainWindow::createActions()
{
  try {
    QAction *newAct = new QAction(QIcon(":/images/document-new.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(slot_newFile()));
    actionsMap["new"] = newAct;

    QAction *camAct = new QAction(QIcon(":/icons/cube.png"), tr("Camera Operation"), this);
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

    QAction *cutAct = new QAction(QIcon(":/icons/cut.png"), tr("Cut"), this);
    cutAct->setStatusTip(tr("Cut mesh"));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(slot_performMeshCut()));
    actionsMap["mesh cut"] = cutAct;
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
    QActionGroup *group = new QActionGroup(ui->mainToolBar);
    group->addAction(actionsMap["camera"]);
    group->addAction(actionsMap["face select"]);
    group->addAction(actionsMap["edge select"]);
    group->addAction(actionsMap["vertex select"]);
    group->setExclusive(true);
    ui->mainToolBar->addAction(actionsMap["camera"]);

    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(actionsMap["face select"]);
    ui->mainToolBar->addAction(actionsMap["edge select"]);
    ui->mainToolBar->addAction(actionsMap["vertex select"]);

    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(actionsMap["mesh cut"]);
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
  QString filename = QFileDialog::getOpenFileName(this, "Select an OBJ file");
  MeshManager::getInstance()->loadOBJFile(filename.toStdString());
  viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getHalfEdgeMesh());
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
  viewer->bindHalfEdgeMesh(MeshManager::getInstance()->getHalfEdgeMesh());
}
