#ifndef MESHVIEWER_H
#define MESHVIEWER_H

#include "common.h"



#include <QGLWidget>
#include <QGLFunctions>
#include <QGLFormat>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>
#include <QGLFramebufferObject>


#include "hds_mesh.h"
#include "colormap.h"
#include "Graph.hpp"
#include "morsesmalecomplex.h"

const QGLFormat qglformat_3d(
	QGL::DoubleBuffer       |
	QGL::DepthBuffer        |
	QGL::AccumBuffer        |
	//QGL::StencilBuffer      |
	//QGL::StereoBuffers      |
	QGL::SampleBuffers      |
	QGL::Rgba               |
	QGL::AlphaChannel       |
	QGL::DirectRendering    |
	QGL::HasOverlay
	);

class MeshViewer : public QGLWidget, protected QGLFunctions
{
	Q_OBJECT
public:
	explicit MeshViewer(QWidget *parent = 0);
	virtual ~MeshViewer();

	void bindHalfEdgeMesh(HDS_Mesh *mesh);
	void bindReebGraph(SimpleGraph *g);
	void setCurvatureColormap(ColorMap cmap);


	void showCriticalPoints();
	void hideCriticalPoints();

	void setCriticalPointsMethod(int midx);
	void setCriticalPointsSmoothingTimes(int times);
	void setCriticalPointsSmoothingType(int t);

	void setCutLocusMethod(int midx);

	void showCutLocusPoints();
	void showCutLocusCut();
signals:
	void updateMeshColorByGeoDistance(int vidx);
	void updateMeshColorByGeoDistance(int vidx, int lev0, int lev1, double alpha);

protected:
	void keyPressEvent(QKeyEvent *e);
	void keyReleaseEvent(QKeyEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent* e);

	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();


public slots:
	void slot_toggleLighting();
	void slot_toggleCriticalPoints();
	void slot_toggleText();

	void slot_toggleCutLocusPoints(int);
	void slot_toggleCutLocusCut();
	void slot_toggleCutLocusCutMode();

private:
	struct ViewerState {
	ViewerState():zNear(1.0), zFar(7000.0), fov(45.0){
		translation = QVector3D(0, 0, -5);
		angularChange = 0;
	}


	void updateViewport(int w, int h) {
		viewport.x = 0;
		viewport.y = 0;
		viewport.w = w;
		viewport.h = h;

		aspect = (qreal)w / (qreal)h;
	}

	void updateProjection() {
		projection.setToIdentity();
		projection.perspective(fov, aspect, zNear, zFar);
	}

	void updateModelView() {
		QMatrix4x4 matrix;
		matrix.translate(translation);
		matrix.rotate(rotation);
		modelview = matrix;
	}

	QMatrix4x4 projectionModelView() const {
		return projection * modelview;
	}

	struct {
		int x, y, w, h;
	} viewport;

	void print() {
		qDebug() << "viewstate:";
		qDebug() << viewport.x << ", " << viewport.y << ", " << viewport.w << ", " << viewport.h;
/*     qDebug() << modelview;
		qDebug() << projection;
		qDebug() << rotationAxis;
		qDebug() << angularChange;
		qDebug() << rotation;
		qDebug() << translation;
		qDebug() << zNear << ", " << zFar << ", " << fov;
		qDebug() << aspect;
		*/
	}

	QMatrix4x4 modelview;
	QMatrix4x4 projection;

	QVector3D rotationAxis;
	qreal angularChange;
	QQuaternion rotation;
	QVector3D translation;

	qreal zNear, zFar, fov;
	qreal aspect;
	};

	ViewerState viewerState;

	struct MouseState {
	MouseState():isPressed(false){}
	QVector2D prev_pos;
	bool isPressed;
	};

	MouseState mouseState;

public:

	int  nn, nm, mm;

	enum InteractionState {
	Camera = 0,
	Camera_Translation,
	Camera_Zoom,
	SelectVertex,
	SelectFace,
	SelectEdge
	};
	void setInteractionMode(InteractionState state) { interactionState = state; while(!selectedElementsIdx.empty()) selectedElementsIdx.pop();}

	enum SelectionMode {
		single = 0,
		multiple,

	};
	void setSelectionMode(SelectionMode mode) {selectionMode = mode; }

	struct SelectionBox
	{
	GLdouble corner_win[4];
	GLdouble corner_global[12];
	QVector3D gcorners[4];
	} sbox;
	bool isSelecting;
	vector<unsigned char> selectionBuffer;
	int lastSelectedIndex;


	void computeGlobalSelectionBox();
	bool QtUnProject(const QVector3D &pos_screen, QVector3D &pos_world);
	int getSelectedElementIndex(const QPoint& p);
public slots:
	void slot_selectAll();
	void slot_selectInverse();

	void slot_selectCutEdgePair();
	void slot_selectCP();
	void slot_selectCC();

	void slot_selectGrow();
	void slot_selectShrink();
	void slot_selectClear();

private:
	InteractionState interactionState;
	SelectionMode selectionMode;
	stack<InteractionState> interactionStateStack;

	queue<int> selectedElementsIdx;
private:
	HDS_Mesh *heMesh;   /// not own
	float scale;
private:
	void drawSelectionBox();

private:
	ColorMap colormap;    /// color for negative curvature and positive curvature

	bool enableLighting;
	void enableLights();
	void disableLights();

	bool showText;
	bool showVIndex; // show vertex index
	bool showCPDistance; //show critical points dists
	bool showCLDistance; //show cut locus dists

private:
	QScopedPointer<QGLFramebufferObject> fbo;
	void initializeFBO();
	void drawMeshToFBO();

public:
	int getCmode(){if (isCriticalPointModeSet) return cmode; else return 0;}//get current cpp mode
	int getLmode(){if (isCutLocusModeset) return lmode; else return 0;}//get current cut locus mode
public slots:
	void slot_disablecpp();
	void slot_disableclp();

	void slot_resetVertices();
	void slot_resetEdges();
	void slot_resetFaces();
private:
	bool showReebPoints;
	vector<double> CPdistances;
	bool isCriticalPointModeSet = false;
	enum CriticalPointMode {
		Geodesics = 0,
		Z=1,              //this line and below later added "= number";
		PointNormal=2,
		Curvature=3,
		Random=5,
		Quadratic=4,
		NCModes
	} cmode;

	int cp_smoothing_times;
	int cp_smoothing_type;
	void findReebPoints();
	void drawReebPoints();
	void selectCutLocusEdges();

private:
	bool showCut;
	bool showOneCut;
	bool showMultCut;
	vector<double> CLdistances;

	bool isCutLocusModeset = false;
	enum CutLocusMode {
		GraphDist = 0,
		GeodesicsDist = 1
	}lmode;

	void findCutLocusPoints();

	unordered_set<HDS_Vertex*> reebPoints;
	SimpleGraph * rbgraph;
	MorseSmaleComplex msc;


};

#endif // MESHVIEWER_H
