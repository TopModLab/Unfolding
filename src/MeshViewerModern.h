#pragma once

//#include "GL/glew.h"
#include "glutils.hpp"
// Legacy OpenGL
#include <QGLFormat>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>

// Modern OpenGL
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
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

class MeshViewerModern
	: public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
public:
	MeshViewerModern(QWidget *parent = nullptr);
	~MeshViewerModern();

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

private: // paint function
	void bind();
	void initialVBO();
	void bindVertexVBO();
	void bindEdgesVAO();
	void bindFaceVAO();

	void initShader();
private:
	perspCamera cam;
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

	oglShaderP m_shader;
};
