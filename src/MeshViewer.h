#pragma once

#ifdef OPENGL_LEGACY
#undef OPENGL_LEGACY
#endif

#include "glutils.hpp"
// Legacy OpenGL
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
#include "ViewerGrid.h"
#include "colormap.h"
#include "Graph.hpp"
#include "morsesmalecomplex.h"

class MeshViewer;
struct RenderBufferObject;

struct MouseState {
	MouseState() : isPressed(false), x(0), y(0){}
	QVector2D m_Pos;
	int x, y;
	bool isPressed;
};
struct RenderBufferObject// : protected oglFuncs
{
	RenderBufferObject(oglFuncs* f)
		: funcs(f)
		, ibo(oglBuffer::Type::IndexBuffer)
		//, flag_tbo(0), flag_tex(0), id_tbo(0), id_tex(0)
		, tbo{}, tex{}
	{
	}
	~RenderBufferObject() { destroy(); }

	void destroy()
	{
		vao.destroy();
		ibo.destroy();
		funcs->glDeleteTextures(2, tex);
		funcs->glDeleteBuffers(2, tbo);
	}

	oglFuncs* funcs;
	oglVAO vao;
	oglBuffer ibo;
	union
	{
		GLuint tbo[2];
		struct { GLuint flag_tbo, id_tbo; };
	};
	union
	{
		GLuint tex[2];
		struct { GLuint flag_tex, id_tex; };
	};

	vector<GLuint> ibos;// he ibo data
	vector<GLuint> ids;// he id, for querying
	ui16s_t flags;// he flag data
};
class MeshViewer
	: public QOpenGLWidget, oglFuncs
{
	Q_OBJECT
public:
	explicit MeshViewer(QWidget *parent = nullptr);
	~MeshViewer();

	void bindHalfEdgeMesh(HDS_Mesh *mesh);
	
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void wheelEvent(QWheelEvent* e) Q_DECL_OVERRIDE;
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
	void bindEdgesTBO();
	void bindFaceVAO();
	void bindFaceTBO();

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
		Camera_Translation = 1,
		Camera_Zoom = 2,
		SelectVertex = 4,
		SelectFace = 5,
		SelectEdge = 6
	};
	enum ShadingState : uint8_t
	{
		SHADE_NONE = 0,
		SHADE_FLAT = 1 << 0,
		SHADE_WF = 1 << 1,
		SHADE_WF_FLAT = SHADE_FLAT | SHADE_WF
	};
	enum DataTypeMark : uint8_t
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
	enum DispComp : uint32_t// Display Compoment
	{
		DISP_NONE = 0,
		DISP_GRID = 1 << 0,
		DISP_CLDistance = 1 << 1, //show cut locus dists
		DISP_CPDistance = 1 << 2, //show critical points dists
		
		DISP_MULT_CUT = 1 << 4,
		DISP_ONE_CUT = 1 << 5,
		DISP_REEB_POINTS = 1 << 6,
		DISP_TEXT = 1 << 7,
		DISP_V_INDEX = 1 << 8
	};
	enum HighlightComp : uint32_t
	{
		HIGHLIGHT_NONE = 0,
		HIGHLIGHT_CUTEDGE = 1 << 0,// related to shader
		HIGHLIGHT_NON_PLANAR_FACE = 1 << 1,
		HIGHLIGHT_BRIDGER = 1 << 2
	};
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

	void showShading(ShadingState shading);
	void showComp(DispComp comp);
	void highlightComp(HighlightComp comp);
private://interaction ie selection
	QScopedPointer<oglFBO> fbo;
	void initializeFBO();
	void drawMeshToFBO();
	
	InteractionState interactionState;
	stack<InteractionState> interactionStateStack;
	SelectionState selectionState;
	queue<int> selVTX, selHE, selFACE;

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

	uint8_t shadingSate;
	uint32_t dispComp;//Display Components Flag
	uint32_t hlComp;// Highlight Components Flag
private://viewer status
	perspCamera view_cam;

private://Mesh Data
	// Ground Grid VBO & VAO
	ViewerGrid grid;

	HDS_Mesh* heMesh;   /// not own
	bool mesh_changed;
	float scale;

	// VBOs and VAOs
	// Vertices data and vao
	oglBuffer vtx_vbo;
	floats_t vtx_array;

	// Face indices and vao
	RenderBufferObject fRBO;
	// Edge indices and vao
	RenderBufferObject heRBO;

	// Shader Programs
	oglShaderP face_solid_shader, edge_solid_shader;
	oglShaderP uid_shader, he_uid_shader, face_uid_shader;
};
