#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

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

    }
    catch(...) {
        throw UnfoldingAppException("Failed to create actions!");
    }
}

void MainWindow::createMenus()
{
    try {

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
