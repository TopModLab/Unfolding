#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "meshviewer.h"
#include "extras/colormap_editor/colormapeditor.h"

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
  void createToolBar();
  void createDock();
  void createStatusBar();

private slots:
  void slot_newFile();
  void slot_saveFile();

  void slot_toggleCameraOperation();
  void slot_toggleFaceSelection();
  void slot_toggleEdgeSelection();
  void slot_toggleVertexSelection();

  void slot_performMeshCut();
  void slot_unfoldMesh();

  void slot_triggerColormap();
  void slot_updateViewerColormap();

private:
  void closeEvent(QCloseEvent *e);

private:
  Ui::MainWindow *ui;
  QMap<QString, QAction*> actionsMap;
  MeshViewer *viewer;
  ColormapEditor *ceditor;
};

#endif // MAINWINDOW_H
