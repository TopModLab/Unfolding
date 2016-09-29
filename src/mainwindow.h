#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>

#include <tchar.h>
#include "UI/MeshViewer.h"
using viewer_t = MeshViewer;
#include "colormap_editor/colormapeditor.h"
#include "UI/ConnectorPanel.h"
#include "UI/criticalpointspanel.h"
#include "UI/cutlocuspanel.h"
#include "UI/BridgerPanel.h"
#include "UI/QuadEdgePanel.h"
#include "UI/GESPanel.h"
#include "UI/rimfacepanel.h"
#include "UI/WeavePanel.h"
#include "UI/NeoWeavePanel.h"
#include "UI/OrigamiPanel.h"
#include <QScopedPointer>

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
	void initMesh(const string &filename);
protected:
	void createActions();
	void createMenus();
	void createToolBar();
	void createDock();
	void createStatusBar();

private:
	void newFile();
	void closeFile();
	void saveFile();

	void triggerExportSVG();
	void exportSVG();

	void selectMultiple();

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
	void slot_unfoldMesh();

	void slot_triggerColormap();
	void slot_updateViewerColormap();

	void slot_smoothMesh();
	void slot_triggerGRS();
	void slot_triggerQuadEdge();
	void slot_triggerWingedEdge();
	void slot_triggerRimmedMesh();
	void slot_triggerGES();
	void slot_triggerWeaveMesh();
	void slot_triggerNeoWeaveMesh();
	void slot_triggerDForms();
	void slot_triggerOrigamiMesh();

	void slot_setBridger();
	void slot_GRS();
	void slot_quadEdge();
	void slot_GES();
	void slot_rimmed3DMesh();
	void slot_weaveMesh();
	void slot_neoWeaveMesh();
	void slot_origamiMesh();

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

	QScopedPointer<ColormapEditor> color_editor;
	QScopedPointer<ConnectorPanel> conn_panel;
	QScopedPointer<CriticalPointsPanel> cp_panel;
	QScopedPointer<CutLocusPanel> cl_panel;
	QScopedPointer<BridgerPanel> grs_panel;
	QScopedPointer<QuadEdgePanel> quad_panel;
	QScopedPointer<GESPanel> ges_panel;
	QScopedPointer<RimFacePanel> rim_panel;
	QScopedPointer<WeavePanel> wv_panel;
	QScopedPointer<NeoWeavePanel> neowv_panel;
	QScopedPointer<OrigamiPanel> origami_panel;


private:


};

#endif // MAINWINDOW_H
