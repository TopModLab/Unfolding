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

// Modern OpenGL
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
typedef QOpenGLVertexArrayObject oglVAO;
typedef QOpenGLBuffer oglBuffer;

#include "hds_mesh.h"
#include "colormap.h"
#include "Graph.hpp"
#include "ViewerState.h"
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
	void slot_toggleLightingSmooth();
	void slot_toggleLightingFlat();
	void slot_toggleLightingWireframe();

	void slot_toggleCriticalPoints();
	void slot_toggleText();

	void slot_toggleCutLocusPoints(int);
	void slot_toggleCutLocusCut();
	void slot_toggleCutLocusCutMode();

private:
	

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
	void setInteractionMode(InteractionState state) { interactionState = state; while(!selectedElementsIdxQueue.empty()) selectedElementsIdxQueue.pop();}

	enum SelectionMode {
		single = 0,
		multiple

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

	void slot_selectTwinPair();
	void slot_selectNextEdge();

	void slot_selectCP();
	void slot_selectCC();
	void slot_selectMSTEdges();

	void slot_selectGrow();
	void slot_selectShrink();
	void slot_selectClear();

private:
	InteractionState interactionState;
	SelectionMode selectionMode;
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

class MeshViewerModern
	: public QOpenGLWidget, protected QOpenGLFunctions
{
public:
	MeshViewerModern(QWidget *parent = nullptr);
	~MeshViewerModern();

	void bindHalfEdgeMesh(HDS_Mesh *mesh);
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private: // paint function
	void initialVBO();
	void bindVertexVBO();
	void bindEdgesVAO();
	void bindFaceVAO();
private:
	const HDS_Mesh *heMesh;   /// not own
	float scale;

	// VBOs and VAOs
	oglBuffer vtx_vbo;
	vector<GLfloat> vtx_array;

	oglVAO face_vao;
	oglBuffer face_ibo;
	vector<GLuint> fib_array;
	vector<GLuint> fid_array;
	vector<GLuint> fflag_array;

	oglVAO he_vao;
	oglBuffer he_ibo;
	vector<GLuint> heib_array;
	vector<GLuint> heid_array;
	vector<GLuint> heflag_array;
};
#endif // MESHVIEWER_H
