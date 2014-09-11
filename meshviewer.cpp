#include "meshviewer.h"
#include "glutils.hpp"
#include "mathutils.hpp"

#include <QMouseEvent>

MeshViewer::MeshViewer(QWidget *parent) :
  QGLWidget(qglformat_3d, parent)
{
  interactionState = Camera;
  viewerState.updateModelView();
  heMesh = nullptr;
}

MeshViewer::~MeshViewer()
{

}

void MeshViewer::bindHalfEdgeMesh(HDS_Mesh *mesh) {
  heMesh = mesh;
  updateGL();
}

bool MeshViewer::QtUnProject(const QVector3D& pos_screen, QVector3D& pos_world)
{
  bool isInvertible;
  QMatrix4x4 proj_modelview_inv = viewerState.projectionModelView().inverted(&isInvertible);
  if(isInvertible)
  {
    QVector3D pos_camera;
    pos_camera.setX((pos_screen.x()-(float)viewerState.viewport.x)/(float)viewerState.viewport.w*2.0-1.0);
    pos_camera.setY((pos_screen.y()-(float)viewerState.viewport.y)/(float)viewerState.viewport.h*2.0-1.0);
    pos_camera.setZ(2.0*pos_camera.z()-1.0);
    pos_world = (proj_modelview_inv*QVector4D(pos_camera, 1.0f)).toVector3DAffine();
  }

  return isInvertible;
}

int MeshViewer::getSelectedElementIndex(const QPoint &p)
{
  int winX = p.x(), winY = height() - p.y();

  auto max = [](int a, int b) { return a>b?a:b; };
  auto min = [](int a, int b) { return a<b?a:b; };

  // search for pixels within a small window
  const int radius = 5;
  map<int, int> counter;
  int maxIdx = -1, maxCount = 0;
  for(int y=max(winY - radius, 0); y<min(winY + radius, height()); ++y) {
    int dy = y - winY;
    for(int x=max(winX - radius, 0); x<min(winX + radius, width()); ++x) {
      int dx = x - winX;
      if( dx*dx + dy*dy <= radius*radius ) {
        int offset = (y * width() + x)*4;
        unsigned char r, g, b, a;
        r = selectionBuffer[offset+0];
        g = selectionBuffer[offset+1];
        b = selectionBuffer[offset+2];
        a = selectionBuffer[offset+3];

        if( a == 0 ) continue;
        else {
          int idx = decodeIndex(r, g, b, 1.0);
          auto it = counter.find(idx);
          if( it == counter.end() ) {
            counter.insert(make_pair(idx, 1));
            if( maxCount == 0 ) {
              maxCount = 1;
              maxIdx = idx;
            }
          }
          else {
            ++it->second;
            if( it->second > maxCount ) {
              maxCount = it->second;
              maxIdx = idx;
            }
          }
        }
      }
    }
  }

  return maxIdx;
}

void MeshViewer::computeGlobalSelectionBox()
{
  /// get GL state
  GLint m_GLviewport[4];
  GLdouble m_GLmodelview[16];
  GLdouble m_GLprojection[16];
  glGetIntegerv(GL_VIEWPORT, m_GLviewport);           // Retrieves The Viewport Values (X, Y, Width, Height)
  glGetDoublev(GL_MODELVIEW_MATRIX, m_GLmodelview);       // Retrieve The Modelview Matrix
  glGetDoublev(GL_PROJECTION_MATRIX, m_GLprojection);     // Retrieve The Projection Matrix

  //Not know why, but it solves the problem, maybe some issue with QT
  if(width()<height())
    m_GLviewport[1] = -m_GLviewport[1];

  GLdouble winX = sbox.corner_win[0];
  GLdouble winY = sbox.corner_win[1];
  QtUnProject(QVector3D(winX,winY,0.001), sbox.gcorners[0]);
  //qDebug()<<sbox.gcorners[0];
  gluUnProject( winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global, sbox.corner_global+1, sbox.corner_global+2);//The new position of the mouse
  //qDebug()<<sbox.corner_global[0]<<sbox.corner_global[1]<<sbox.corner_global[2];

  winX = sbox.corner_win[0];
  winY = sbox.corner_win[3];
  QtUnProject(QVector3D(winX,winY,0.001), sbox.gcorners[1]);
  //qDebug()<<sbox.gcorners[1];
  gluUnProject( winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global+3, sbox.corner_global+4, sbox.corner_global+5);//The new position of the mouse
  //qDebug()<<sbox.corner_global[3]<<sbox.corner_global[4]<<sbox.corner_global[5];

  winX = sbox.corner_win[2];
  winY = sbox.corner_win[3];
  QtUnProject(QVector3D(winX,winY,0.001), sbox.gcorners[2]);
  //qDebug()<<sbox.gcorners[2];
  gluUnProject( winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global+6, sbox.corner_global+7, sbox.corner_global+8);//The new position of the mouse
  //qDebug() << sbox.corner_global[6] << sbox.corner_global[7]<< sbox.corner_global[8];

  winX = sbox.corner_win[2];
  winY = sbox.corner_win[1];
  QtUnProject(QVector3D(winX,winY,0.001), sbox.gcorners[3]);
  //qDebug()<<sbox.gcorners[3];
  gluUnProject( winX, winY, 0.0001, m_GLmodelview, m_GLprojection, m_GLviewport, sbox.corner_global+9, sbox.corner_global+10, sbox.corner_global+11);//The new position of the mouse
  //qDebug() << sbox.corner_global[9] << sbox.corner_global[10]<< sbox.corner_global[11];
}

void MeshViewer::mousePressEvent(QMouseEvent *e)
{
  mouseState.isPressed = true;

  switch( interactionState ) {
  case Camera:
    mouseState.prev_pos = QVector2D(e->pos());
    break;
  case SelectFace:
  case SelectEdge:
  case SelectVertex: {
    sbox.corner_win[0] = e->x();
    sbox.corner_win[1] = viewerState.viewport.h - e->y();
    break;
  }
  }
}

void MeshViewer::mouseMoveEvent(QMouseEvent *e)
{
  switch( interactionState ) {
  case Camera: {
    if( e->buttons() & Qt::LeftButton ) {
      QVector2D diff = QVector2D(e->pos()) - mouseState.prev_pos;

      if((e->modifiers() & Qt::AltModifier) ) {
        viewerState.translation += QVector3D(diff.x()/100.0, -diff.y()/100.0, 0.0);
      }
      else if(e->modifiers() & Qt::ControlModifier)
      {
        viewerState.translation += QVector3D(0.0, 0.0, diff.x()/100.0-diff.y()/100.0);
      }
      else{
        // Rotation axis is perpendicular to the mouse position difference
        // vector
        QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
        // Accelerate angular speed relative to the length of the mouse sweep
        qreal acc = diff.length() / 4.0;
        // Calculate new rotation axis as weighted sum
        viewerState.rotationAxis = (viewerState.rotationAxis * viewerState.angularChange + n * acc).normalized();
        // Change rotation angle
        viewerState.angularChange = acc;

        viewerState.rotation = QQuaternion::fromAxisAndAngle(viewerState.rotationAxis, viewerState.angularChange) * viewerState.rotation;
      }
      viewerState.updateModelView();
      mouseState.prev_pos = QVector2D(e->pos());
    }
    break;
  }
  case SelectFace:
  case SelectEdge:
  case SelectVertex: {
    if( mouseState.isPressed ) {
      isSelecting = true;
      sbox.corner_win[2] = e->x();
      sbox.corner_win[3] = viewerState.viewport.h - e->y();
      computeGlobalSelectionBox();
    }
    else {
      isSelecting = false;
    }
    break;
  }
  }

  updateGL();
}

void MeshViewer::mouseReleaseEvent(QMouseEvent *e)
{
  switch( interactionState ) {
  case Camera:
    mouseState.prev_pos = QVector2D(e->pos());
    break;
  case SelectFace:
  case SelectEdge:
  case SelectVertex: {
    sbox.corner_win[2] = e->x();
    sbox.corner_win[3] = viewerState.viewport.h - e->y();
    computeGlobalSelectionBox();
    isSelecting = false;

    int selectedElementIdx = getSelectedElementIndex(e->pos());
    cout << "selected element " << selectedElementIdx << endl;
    if( selectedElementIdx >= 0 ) {
      if( interactionState == SelectEdge ) {
        heMesh->selectEdge(selectedElementIdx);
      }
      else if( interactionState == SelectFace ) {
        heMesh->selectFace(selectedElementIdx);
      }
      else {
        heMesh->selectVertex(selectedElementIdx);
      }
    }
    break;
  }
  }
  mouseState.isPressed = false;
  updateGL();
}

void MeshViewer::wheelEvent(QWheelEvent *e)
{
  switch(interactionState) {
  case Camera:{
    double numSteps = e->delta()/100.0f;
    viewerState.translation.setZ(viewerState.translation.z() + numSteps);
    viewerState.updateModelView();
    break;
  }
  default:
    break;
  }
  updateGL();
}

void MeshViewer::enterEvent(QEvent *e)
{
  QGLWidget::enterEvent(e);
  grabKeyboard();
  setFocus();
}

void MeshViewer::leaveEvent(QEvent *e)
{
  QGLWidget::leaveEvent(e);
  releaseKeyboard();
}

void MeshViewer::initializeGL()
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

  glShadeModel(GL_SMOOTH);

  setMouseTracking(true);

  initializeFBO();
}

void MeshViewer::initializeFBO() {
  fbo.reset(new QGLFramebufferObject(width(), height(), QGLFramebufferObject::Depth));
  selectionBuffer.resize(width()*height()*4);
}

void MeshViewer::resizeGL(int w, int h)
{
  initializeFBO();

  viewerState.updateViewport(w, h);
  viewerState.updateProjection();

  glViewport(viewerState.viewport.x, viewerState.viewport.y, viewerState.viewport.w, viewerState.viewport.h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMultMatrixf(viewerState.projection.constData());
}

void MeshViewer::paintGL()
{
  glClearColor(1, 1, 1, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // the model view matrix is updated somewhere else
  glMultMatrixf(viewerState.modelview.constData());

  glEnable(GL_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if( heMesh == nullptr ) {
    glLineWidth(2.0);
    GLUtils::drawQuad(QVector3D(-1, -1, 0),
                      QVector3D( 1, -1, 0),
                      QVector3D( 1,  1, 0),
                      QVector3D(-1,  1, 0));
  }
  else {
    heMesh->draw();
    switch( interactionState ) {
    case Camera:
      break;
    default:
      drawSelectionBox();
      drawMeshToFBO();
    }
  }
}

static QImage toQImage(const unsigned char* data, int w, int h) {
  QImage qimg(w, h, QImage::Format_ARGB32);
  for(int i=0, idx=0;i<h;i++) {
    for(int j=0;j<w;j++, idx+=4)
    {
      unsigned char r = data[idx+2];
      unsigned char g = data[idx+1];
      unsigned char b = data[idx];
      unsigned char a = 255;
      QRgb qp = qRgba(r, g, b, a);
      qimg.setPixel(j, i, qp);
    }
  }
  return qimg;
}

void MeshViewer::drawMeshToFBO() {
  fbo->bind();

#if 0
  cout << (fbo->isBound()?"bounded.":"not bounded.") << endl;
  cout << (fbo->isValid()?"valid.":"invalid.") << endl;
#endif

  glPushMatrix();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // the model view matrix is updated somewhere else
  glMultMatrixf(viewerState.modelview.constData());

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glDepthMask(GL_TRUE);

  /// must set alpha to zero
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glShadeModel(GL_FLAT);
  glDisable(GL_BLEND);

  switch( interactionState ) {
  case SelectFace:
    heMesh->drawFaceIndices();
    break;
  case SelectEdge:
    heMesh->drawEdgeIndices();
    break;
  case SelectVertex:
    heMesh->drawVertexIndices();
    break;
  }

  glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, &(selectionBuffer[0]));
#if 0
  GLenum errcode = glGetError();
  if (errcode != GL_NO_ERROR) {
    const GLubyte *errString = gluErrorString(errcode);
    fprintf (stderr, "OpenGL Error: %s\n", errString);
  }
#endif

  glPopMatrix();

  fbo->release();

  QImage img = toQImage(&(selectionBuffer[0]), width(), height());
  img.save("fbo.png");
}

void MeshViewer::drawSelectionBox() {
  if( !isSelecting ) return;

#if 1
  //draw selection box
  glColor4f(0.0, 1.0, 19.0/255, 0.2);
  glBegin(GL_QUADS);
  glVertex3dv(sbox.corner_global);
  glVertex3dv(sbox.corner_global+3);
  glVertex3dv(sbox.corner_global+6);
  glVertex3dv(sbox.corner_global+9);
  glEnd();

  //draw selection box
  glLineWidth(3.0);
  glColor4f(0.0, 1.0, 19.0/255, 0.5);
  glBegin(GL_LINE_LOOP);
  glVertex3dv(sbox.corner_global);
  glVertex3dv(sbox.corner_global+3);
  glVertex3dv(sbox.corner_global+6);
  glVertex3dv(sbox.corner_global+9);
  glEnd();
#else
  glPushMatrix();
  glTranslatef(0, 0, 0.5);
  //draw selection box
  GLUtils::fillQuad(sbox.gcorners, QColor(0.0, 1.0, 19.0/255.0, 0.2));

  //draw selection box
  glLineWidth(3.0);
  GLUtils::drawQuad(sbox.gcorners, QColor(0.0, 1.0, 19.0/255.0, 0.5));
  glPopMatrix();
#endif
}