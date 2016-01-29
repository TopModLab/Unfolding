#pragma once

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
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>
#include "common.h"
#include "hds_mesh.h"
#include "Camera.h"
#include "colormap.h"
#include "Graph.hpp"
#include "morsesmalecomplex.h"

typedef QOpenGLShader				oglShader;
typedef QOpenGLShaderProgram		oglShaderP;
typedef QOpenGLVertexArrayObject	oglVAO;
typedef QOpenGLBuffer				oglBuffer;
typedef QOpenGLFramebufferObject	oglFBO;

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
	virtual ~MeshViewerModern();

	void bindHalfEdgeMesh(HDS_Mesh *mesh);
	//kkkkkkkkkk
	/*
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
public slots:
	void slot_toggleLightingSmooth();
	void slot_toggleLightingFlat();
	void slot_toggleLightingWireframe();

	void slot_toggleCriticalPoints();
	void slot_toggleText();

	void slot_toggleCutLocusPoints(int);
	void slot_toggleCutLocusCut();
	void slot_toggleCutLocusCutMode();
	//kkkkkkkkkkkkkkkkkkkkkkkkkk
	//*/
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *e);
	//void keyReleaseEvent(QKeyEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	//void mouseReleaseEvent(QMouseEvent *e);
	//void wheelEvent(QWheelEvent* e);
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

private://interaction ie selection
	QScopedPointer<oglFBO> fbo;
	void initializeFBO();
	void drawMeshToFBO();
	enum InteractionState {
		Camera = 0,
		Camera_Translation,
		Camera_Zoom,
		SelectVertex,
		SelectFace,
		SelectEdge
	};
	InteractionState m_interactionState;

	enum DataTypeMark
	{
		NULL_MARK	= 0,
		VERTEX_MARK	= 1,
		EDGE_MARK	= 2,
		FACE_MARK	= 3
	};
	union SelectionID{
		size_t vertexID;
		size_t faceID;
		size_t edgeID;
	} selectionID;

private://viewer status
	perspCamera view_cam;
	MouseState mouseState;
public:

private://Mesh Data
	oglBuffer grid_vtx_vbo;
	oglBuffer grid_clr_vbo;
	oglBuffer grid_ibo;
	oglVAO grid_vao;
	vector<GLfloat> grid_vtx_array;
	vector<GLfloat> grid_clr_array;
	vector<GLushort> grid_idx_array;
//	vector<GLfloat>

	const HDS_Mesh* heMesh;   /// not own
	bool mesh_changed;
	float scale;

	// VBOs and VAOs
	oglBuffer vtx_vbo;
	//GLuint vtx_vbo;
	vector<GLfloat> vtx_array;

	oglVAO face_vao;
	oglBuffer face_ibo;
	/*GLuint face_vao;
	GLuint face_ibo;*/
	vector<GLuint> fib_array;
	vector<GLuint> fid_array;
	vector<GLuint> fflag_array;

	oglVAO he_vao;
	oglBuffer he_ibo;// ibo handle
	/*GLuint he_vao;
	GLuint he_ibo;*/
	vector<GLuint> heib_array;// he ibo data
	vector<GLuint> heid_array;// he id, for querying
	vector<GLuint> heflag_array;// he flag data

	oglShaderP grid_shader;
	oglShaderP face_solid_shader, edge_solid_shader;
};
