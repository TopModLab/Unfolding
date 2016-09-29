#pragma once

#ifdef OPENGL_LEGACY
#undef OPENGL_LEGACY
#endif

// Legacy OpenGL
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>
#include <QEvent>
#include <QMouseEvent>
// Modern OpenGL
#include <QOpenGLWidget>
#include "UI/glutils.h"
#include "Utils/common.h"
#include "HDS/hds_mesh.h"
#include "UI/Camera.h"
#include "UI/ViewerGrid.h"
#include "UI/colormap.h"
#include "GeomUtils/Graph.h"
#include "GeomUtils/morsesmalecomplex.h"

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
		, vao(nullptr)
		, ibo(oglBuffer::Type::IndexBuffer)
		, tbo{}, tex{}
	{
	}
	~RenderBufferObject() { destroy(); }

	void destroy()
	{
		vao.destroy();
		ibo.destroy();
		destroyTextures();
	}
	void destroyTextures()
	{
		funcs->glDeleteTextures(2, tex);
		funcs->glDeleteBuffers(2, tbo);
	}
	void releaseAll()
	{
		vao.release();
		ibo.release();
		vbo->release();
	}
	void allocateIBO()
	{
		ibo.bind();
		ibo.allocate(&ibos[0], sizeof(GLuint) * ibos.size());
		ibo.release();
	}
	void allocateTBO(int nTBO = 2)
	{
		funcs->glBindBuffer(GL_TEXTURE_BUFFER, tbo[0]);
		funcs->glBufferData(GL_TEXTURE_BUFFER, sizeof(uint16_t) * flags.size(), &flags[0], GL_STATIC_DRAW);
		if (nTBO > 1)
		{
			funcs->glBindBuffer(GL_TEXTURE_BUFFER, tbo[1]);
			funcs->glBufferData(GL_TEXTURE_BUFFER, sizeof(uint32_t) * ids.size(), &ids[0], GL_STATIC_DRAW);
		}
		funcs->glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}
	void shrink_to_fit()
	{
		ibos.shrink_to_fit();
		ids.shrink_to_fit();
		flags.shrink_to_fit();
	}

	oglFuncs* funcs;
	shared_ptr<oglBuffer> vbo;
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
	ui32s_t ids;// he id, for querying
	ui16s_t flags;// he flag data
};
class MeshViewer
	: public QOpenGLWidget, oglFuncs
{
	Q_OBJECT
private:
	explicit MeshViewer(QWidget *parent = nullptr);
	MeshViewer(const MeshViewer &) = delete;
	MeshViewer& operator = (const MeshViewer &) = delete;

	~MeshViewer();
public:
	static MeshViewer* getInstance()
	{
		if (!instance)
		{
			instance = new MeshViewer;
		}
		return instance;
	}

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

	void selectByRefID();
	
	void toggleCriticalPoints();
	void toggleCutLocusCut();
	void toggleCutLocusCutMode();
	void toggleCutLocusPoints(int);
	void toggleLightingFlat();
	void toggleLightingSmooth();
	void toggleLightingWireframe();
	void toggleText();

	void unfoldView(const HDS_Mesh* inMesh);
private: // paint function
	void allocateGL();

	void initShader();
public:
	enum ShadingState : uint8_t
	{
		SHADE_NONE = 0,
		SHADE_FLAT = 1 << 0,
		SHADE_WF = 1 << 1,
		SHADE_WF_FLAT = SHADE_FLAT | SHADE_WF,
		SHADE_VERT = 1 << 2
	};
	/*enum SelectionState
	{
	SingleSelect = 0,
	MultiSelect
	};*/
	enum InteractionState : uint8_t
	{
		ROAM_CAMERA = 0,
		SEL_MULTI = 1 << 1,
		SEL_VERT = 1 << 2,
		SEL_FACE = 1 << 3,
		SEL_EDGE = 1 << 4
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
	//void setSelectionMode(SelectionState mode);

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
	//SelectionState selectionState;
	queue<uint32_t> selVTX, selHE, selFACE;

	MouseState mouseState;

private:
	union
	{
		uint16_t renderFlag;
		struct
		{
			bool isCriticalPointModeSet : 1;
			bool isCutLocusModeset : 1;
			bool isSelecting : 1;
			bool showCLDistance : 1; //show cut locus dists
			bool showCPDistance : 1; //show critical points dists
			bool showCut : 1;
			bool showMultCut : 1;
			bool showOneCut : 1;
			bool showReebPoints : 1;
			bool showText : 1;
			bool showVIndex : 1; // show vertex index
		};
	};
	
	uint8_t shadingSate;
	uint32_t dispComp;//Display Components Flag
	uint32_t hlComp;// Highlight Components Flag
private:
	static MeshViewer* instance;
	//viewer status
	perspCamera view_cam;

	//Mesh Data
	// Ground Grid
	ViewerGrid grid;

	HDS_Mesh* heMesh;   // not own
	bool mesh_changed;
	double view_scale;
	//QMatrix4x4 model_matrix;

	// VBOs and VAOs
	// Vertices data and vao
	floats_t vtx_array;

	RenderBufferObject vRBO;
	RenderBufferObject fRBO;
	RenderBufferObject heRBO;

	// Shader Programs
	oglShaderP face_solid_shader, edge_solid_shader;
	oglShaderP vtx_solid_shader;
	oglShaderP uid_shader, he_uid_shader, face_uid_shader;

	friend class MainWindow;
};
