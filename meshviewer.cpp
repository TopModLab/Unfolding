#include "meshviewer.h"
#include "glutils.hpp"

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

void MeshViewer::mousePressEvent(QMouseEvent *e)
{
    switch( interactionState ) {
    case Camera:
        mouseState.prev_pos = QVector2D(e->pos());
        break;
    case SelectFace:
        break;
    case SelectEdge:
        break;
    case SelectVertex:
        break;
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
        break;
    case SelectEdge:
        break;
    case SelectVertex:
        break;
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
        break;
    case SelectEdge:
        break;
    case SelectVertex:
        break;
    }
}

void MeshViewer::wheelEvent(QWheelEvent *e)
{
    double numSteps = e->delta()/100.0f;
    viewerState.translation.setZ(viewerState.translation.z() + numSteps);
    viewerState.updateModelView();
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
}

void MeshViewer::resizeGL(int w, int h)
{
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

    if( heMesh == nullptr ) {
        GLUtils::drawQuad(QVector3D(-1, -1, 0),
                          QVector3D( 1, -1, 0),
                          QVector3D( 1,  1, 0),
                          QVector3D(-1,  1, 0));
    }
    else {
        heMesh->draw();
    }
}
