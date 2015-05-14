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

  void toggleCriticalPoints();
  void setCriticalPointsMethod(int midx);
  void setCriticalPointsSmoothingTimes(int times);
  void setCriticalPointsSmoothingType(int t);

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
    SelectVertex,
    SelectFace,
    SelectEdge
  };
  void setInteractionMode(InteractionState state) { interactionState = state; }

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

private:
  InteractionState interactionState;
  stack<InteractionState> interactionStateStack;

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

private:
  QScopedPointer<QGLFramebufferObject> fbo;
  void initializeFBO();
  void drawMeshToFBO();

private:
  bool showReebPoints;
  enum CriticalPointMode {
    Geodesics = 0,
    Z=1,              //this line and below later added "= number";
    PointNormal=2,
    Curvature=3,
    Random=5,
    Quadratic=4,
    CutLocus=5,
    NCModes
  } cmode;
  int cp_smoothing_times;
  int cp_smoothing_type;
  void findReebPoints();
  void drawReebPoints();
  void drawReebGraph();

  unordered_set<HDS_Vertex*> reebPoints;
  SimpleGraph * rbgraph;
  MorseSmaleComplex msc;
};

#endif // MESHVIEWER_H
