#ifndef MESHVIEWER_H
#define MESHVIEWER_H

#include "common.h"

#include <QGLWidget>
#include <QGLFunctions>
#include <QGLFormat>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>

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

signals:

protected:
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
            translation.setZ(-5.0);
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

        struct {
            int x, y, w, h;
        } viewport;
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
        QVector2D prev_pos;
    };

    MouseState mouseState;

    enum InteractionState {
        Camera = 0,
        SelectVertex,
        SelectFace,
        SelectEdge
    };
    InteractionState interactionState;
};

#endif // MESHVIEWER_H
