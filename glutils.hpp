#ifndef GLUTILS_HPP
#define GLUTILS_HPP

#include <QGLFunctions>
#include <QVector3D>

namespace GLUtils {

inline void setColor(const QColor& color) {
    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

inline void useVertex(const QVector3D& p) {
    glVertex3f(p.x(), p.y(), p.z());
}

static void drawLine(const QVector3D& p0,
                     const QVector3D& p1,
                     const QColor& color = Qt::black) {
    setColor(color);
    glBegin(GL_LINES);
    useVertex(p0);
    useVertex(p1);
    glEnd();
}

static void drawQuad(const QVector3D& p0,
                     const QVector3D& p1,
                     const QVector3D& p2,
                     const QVector3D& p3,
                     const QColor& color = Qt::black) {
    setColor(color);
    glBegin(GL_LINE_LOOP);
    useVertex(p0);
    useVertex(p1);
    useVertex(p2);
    useVertex(p3);
    glEnd();
}

}

#endif // GLUTILS_HPP
