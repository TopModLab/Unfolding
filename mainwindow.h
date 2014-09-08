#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "meshviewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool createComponents();
    bool layoutComponents();
    bool connectComponents();

protected:
    void createActions();
    void createMenus();
    void createDock();
    void createStatusBar();

private:
    Ui::MainWindow *ui;
    MeshViewer *viewer;
};

#endif // MAINWINDOW_H
