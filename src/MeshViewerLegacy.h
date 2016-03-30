#ifndef MESHVIEWER_H
#define MESHVIEWER_H

#ifndef OPENGL_LEGACY
#define OPENGL_LEGACY
#endif
// Legacy OpenGL
#include <QGLWidget>
#include <QGLFunctions>
#include <QGLFormat>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>

#include <QGLFramebufferObject>

#include "glutils.hpp"
#include "common.h"
#include "hds_mesh.h"
#include "colormap.h"
#include "Graph.hpp"
#include "CameraLegacy.h"
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
	//, public QOpenGLWidget, protected QOpenGLFunctions
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
	void toggleLightingSmooth();
	void toggleLightingFlat();
	void toggleLightingWireframe();

	void toggleCriticalPoints();
	void toggleText();

	void toggleCutLocusPoints(int);
	void toggleCutLocusCut();
	void toggleCutLocusCutMode();

private:
	

	CameraLegacy view_cam;

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
	void setInteractionMode(InteractionState state) { interactionState = state; while(!selectedElementsIdxQueue.empty()) selectedElementsIdxQueue.pop();}

	enum SelectionState {
		SingleSelect = 0,
		MultiSelect

	};
	void setSelectionMode(SelectionState mode) {selectionMode = mode; }

	struct SelectionBox
	{
	GLdouble corner_win[4];
	GLdouble corner_global[12];
	QVector3D gcorners[4];
	} sbox;
	bool isSelecting;
	vector<unsigned char> selectionBuffer;
	int lastSelectedIndex;


	//void computeGlobalSelectionBox();
	bool QtUnProject(const QVector3D &pos_screen, QVector3D &pos_world);
	int getSelectedElementIndex(const QPoint& p);
public slots:
	void selectAll();
	void selectInverse();

	void selectTwinPair();
	void selectNextEdge();

	void selectCP();
	void selectCC();
	void selectMSTEdges();

	void selectGrow();
	void selectShrink();
	void selectClear();

private:
	InteractionState interactionState;
	SelectionState selectionMode;
	stack<InteractionState> interactionStateStack;

	queue<int> selectedElementsIdxQueue;
private:
	HDS_Mesh *heMesh;   /// not own
	float scale;
private:
	void drawSelectionBox();

private:
	ColorMap colormap;    /// color for negative curvature and positive curvature

	enum LightingState {
		Smooth,
		Flat,
		Wireframe
	};
	LightingState lightingState;

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
	void disablecpp();
	void disableclp();

	void resetVertices();
	void resetEdges();
	void resetFaces();
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

private:
	struct Face{
		Face(){}
		Face(HDS_Face* f, double w):f(f), weight(w){}
		HDS_Face* f;
		HDS_Face* parent = nullptr;
		double weight = std::numeric_limits<double>::infinity();

		void setParent(HDS_Face* e) {parent = e;}
		bool operator>(const Face& e) const {
			return weight > e.weight;
		}

	};

	typedef std::priority_queue<Face, std::vector<Face>, std::greater<Face>> PQ_MST_Face;


};


#endif // MESHVIEWER_H
