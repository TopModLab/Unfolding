#pragma once

#ifdef OPENGL_LEGACY
#undef OPENGL_LEGACY
#endif

#include "glutils.hpp"
// Legacy OpenGL
#include <QGLFormat>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>
#include <QEvent>
#include <QMouseEvent>
// Modern OpenGL
#include <QOpenGLWidget>
#include "common.h"
#include "hds_mesh.h"
#include "Camera.h"
#include "colormap.h"
#include "Graph.hpp"
#include "morsesmalecomplex.h"

/*
using oglShader		= QOpenGLShader;
using oglShaderP	= QOpenGLShaderProgram;
using oglVAO		= QOpenGLVertexArrayObject;
using oglBuffer		= QOpenGLBuffer;
using oglFBO		= QOpenGLFramebufferObject;
*/

struct MouseState {
	MouseState() : isPressed(false), x(0), y(0){}
	QVector2D m_Pos;
	int x, y;
	bool isPressed;
};
class MeshViewerModern
	: public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
public:
	explicit MeshViewerModern(QWidget *parent = nullptr);
	~MeshViewerModern();

	void bindHalfEdgeMesh(HDS_Mesh *mesh);
	
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	//void wheelEvent(QWheelEvent* e) Q_DECL_OVERRIDE;
public:// slots functions
	void disableclp();
	void disablecpp();

	void resetEdges();
	void resetFaces();
	void resetVertices();

	void selectAll();
	void selectInverse();

	void selectTwinPair();
	void selectNextEdge();

	void selectCC();
	void selectCP();
	void selectMSTEdges();

	void selectGrow();
	void selectShrink();
	void selectClear();

	void toggleCriticalPoints();
	void toggleCutLocusCut();
	void toggleCutLocusCutMode();
	void toggleCutLocusPoints(int);
	void toggleLightingFlat();
	void toggleLightingSmooth();
	void toggleLightingWireframe();
	void toggleText();

private: // paint function
	void resetCamera();
	void bind();
	void initialVBO();
	void bindVertexVBO();
	void bindEdgesVAO();
	void bindFaceVAO();

	void bindGrid();
	void drawGrid();
	void initShader();
public:
	enum SelectionState
	{
		SingleSelect = 0,
		MultiSelect
	};
	enum InteractionState : size_t
	{
		Camera = 0,
		Camera_Translation,
		Camera_Zoom,
		SelectVertex,
		SelectFace,
		SelectEdge
	};
	enum ShadingState
	{
		SMOOTH = 0,
		FLAT = 1,
		WIREFRAME = 2
	};
	enum DataTypeMark : size_t
	{
		NULL_MARK = 0,
		VERTEX_MARK = 1,
		EDGE_MARK = 2,
		FACE_MARK = 3
	};
	enum CriticalPointMode
	{
		Geodesics = 0,
		Z = 1,           //this line and below later added "= number";
		PointNormal = 2,
		Curvature = 3,
		Random = 5,
		Quadratic = 4,
		NCModes
	} cmode;
	enum CutLocusMode
	{
		GraphDist = 0,
		GeodesicsDist = 1
	}lmode;

public:
	void selectCutLocusEdges();
	void setCriticalPointsMethod(int midx);
	void setCriticalPointsSmoothingTimes(int times);
	void setCriticalPointsSmoothingType(int t);
	void setCurvatureColormap(ColorMap cmap);
	void setCutLocusMethod(int midx);

	void setInteractionMode(InteractionState state);
	void setSelectionMode(SelectionState mode);

	void showCriticalPoints();
	void showCutLocusCut();
	void showCutLocusPoints();

private://interaction ie selection
	QScopedPointer<oglFBO> fbo;
	void initializeFBO();
	void drawMeshToFBO();
	
	InteractionState interactionState;
	stack<InteractionState> interactionStateStack;
	SelectionState selectionState;
	queue<int> selectedElementsIdxQueue;
	ShadingState shadingSate;
	MouseState mouseState;

	union SelectionID{
		size_t vertexID;
		size_t faceID;
		size_t edgeID;
		size_t selID;
	} selectionID;

private:
	bool isCriticalPointModeSet = false;
	bool isCutLocusModeset = false;
	bool isSelecting;
	bool showCLDistance; //show cut locus dists
	bool showCPDistance; //show critical points dists
	bool showCut;
	bool showMultCut;
	bool showOneCut;
	bool showReebPoints;
	bool showText;
	bool showVIndex; // show vertex index
private://viewer status
	perspCamera view_cam;

private://Mesh Data
	// Ground Grid VBO & VAO
	oglBuffer grid_vtx_vbo;
	oglBuffer grid_clr_vbo;
	oglBuffer grid_ibo;
	oglVAO grid_vao;
	vector<GLfloat> grid_vtx_array;
	vector<GLfloat> grid_clr_array;
	vector<GLushort> grid_idx_array;

	HDS_Mesh* heMesh;   /// not own
	bool mesh_changed;
	float scale;

	// VBOs and VAOs
	// Vertices data and vao
	oglBuffer vtx_vbo;
	vector<GLfloat> vtx_array;

	// Face indices and vao
	oglVAO face_vao;
	oglBuffer face_ibo;
	vector<GLuint> fib_array;
	vector<GLuint> fid_array;
	vector<GLuint> fflag_array;

	// Edge indices and vao
	oglVAO he_vao;
	oglBuffer he_ibo;
	vector<GLuint> heib_array;// he ibo data
	vector<GLuint> heid_array;// he id, for querying
	vector<GLuint> heflag_array;// he flag data
	
	// Shader Programs
	oglShaderP grid_shader;
	oglShaderP face_solid_shader, edge_solid_shader;
	oglShaderP uid_shader, he_uid_shader, face_uid_shader;
};
