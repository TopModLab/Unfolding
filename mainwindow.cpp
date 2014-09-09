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
}
