#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>

#include <tchar.h>
#ifdef OPENGL_LEGACY
#include "meshviewer.h"
using viewer_t = MeshViewer;
#else
#include "MeshViewerModern.h"
using viewer_t = MeshViewerModern;
#endif
#include "../extras/colormap_editor/colormapeditor.h"
#include "criticalpointspanel.h"
#include "cutlocuspanel.h"
#include "bridgerpanel.h"
#include "hollowmeshpanel.h"
#include "bindingmeshpanel.h"
#include "rimfacepanel.h"

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

	void slot_undo();
	void slot_redo();
	void slot_reset();

	void slot_toggleCameraOperation();
	void slot_toggleFaceSelection();
	void slot_toggleEdgeSelection();
	void slot_toggleVertexSelection();

	void slot_performMeshCut();
	void slot_unfoldMesh(bool);

	void slot_triggerColormap();
	void slot_updateViewerColormap();

	void slot_smoothMesh();
	void slot_triggerExtendMesh();
	void slot_triggerHollowMesh(bool);
	void slot_triggerRimmedMesh(bool);
	void slot_triggerRimmed3DMesh();
	void slot_triggerBindingMesh(bool);

	void slot_setBridger();
	void slot_extendMesh();
	void slot_hollowMesh();
	void slot_bindingMesh();
	void slot_rimmed3DMesh();

	void slot_rimMesh();

	void slot_updateMeshColorByGeoDistance(int vidx);
	void slot_updateMeshColorByGeoDistance(int vidx, int lev0, int lev1, double ratio);

	void slot_triggerCriticalPoints();
	void slot_updateCriticalPointsSmoothingTimes(int);
	void slot_updateCriticalPointsSmoothingType(int);
	void slot_updateCriticalPointsMethod(int);

	void slot_triggerCutLocusPanel();
	void slot_updateCutLocusMethod(int);
	void slot_disableclp();


private:
	void closeEvent(QCloseEvent *e);

	enum CurrentMesh {
		Original = 0,
		Extended = 1,
		Cutted = 2,
		Unfolded = 3,
		Rimmed = 4
	};

	CurrentMesh curMesh;
	stack<CurrentMesh> meshStack;
	bool isExtended;

	void updateCurrentMesh();
	

private:
	Ui::MainWindow *ui;
	QMap<QString, QAction*> actionsMap;

	viewer_t *viewer;
	ColormapEditor *ceditor;
	CriticalPointsPanel *cppanel;
	CutLocusPanel *clpanel;
	BridgerPanel *conpanel;
	HollowMeshPanel *hmpanel;
	BindingMeshPanel *bmpanel;
	RimFacePanel *rmpanel;

private:


};

#endif // MAINWINDOW_H
