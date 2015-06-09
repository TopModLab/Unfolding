#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


#include "meshviewer.h"
#include "../extras/colormap_editor/colormapeditor.h"
#include "criticalpointspanel.h"
#include "cutlocuspanel.h"

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

	void initialization();
protected:
	void createActions();
	void createMenus();
	void createToolBar();
	void createDock();
	void createStatusBar();

private slots:
	void slot_newFile();
	void slot_closeFile();
	void slot_saveFile();

    void slot_exportFile();

	void slot_selectMultiple();

	void slot_toggleEdges();
	void slot_toggleFaces();
	void slot_toggleVertices();
	void slot_toggleNormals();

	void slot_reset();

	void slot_toggleCameraOperation();
	void slot_toggleFaceSelection();
	void slot_toggleEdgeSelection();
	void slot_toggleVertexSelection();

	void slot_performMeshCut();
	void slot_unfoldMesh();

	void slot_triggerColormap();
	void slot_updateViewerColormap();

	void slot_smoothMesh();
	void slot_extendMesh();

	void slot_updateMeshColorByGeoDistance(int vidx);
	void slot_updateMeshColorByGeoDistance(int vidx, int lev0, int lev1, double ratio);

	void slot_triggerCriticalPoints();
	void slot_updateCriticalPointsSmoothingTimes(int);
	void slot_updateCriticalPointsSmoothingType(int);
	void slot_updateCriticalPointsMethod(int);

	void slot_triggerCutLocusPanel();
	void slot_updateCutLocusMethod(int);



private:
	void closeEvent(QCloseEvent *e);

private:
	Ui::MainWindow *ui;
	QMap<QString, QAction*> actionsMap;
	MeshViewer *viewer;
	ColormapEditor *ceditor;
	CriticalPointsPanel *cppanel;
	CutLocusPanel *clpanel;
};

#endif // MAINWINDOW_H
