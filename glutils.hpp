#ifndef GLUTILS_HPP
#define GLUTILS_HPP

#include <QGLFunctions>
#include <QVector3D>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

namespace GLUtils {

inline void setColor(const QColor& color) {
  GLfloat colorv[] = {(GLfloat)color.redF(), (GLfloat)color.greenF(), (GLfloat)color.blueF(), (GLfloat)color.alphaF()};
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colorv);
  glColor4fv(colorv);
}

inline void setColor(const QColor& color, float alpha) {
  GLfloat colorv[] = { (GLfloat)color.redF(), (GLfloat)color.greenF(), (GLfloat)color.blueF(), alpha };
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colorv);
  glColor4fv(colorv);
}

inline void useVertex(const QVector3D& p) {
  glVertex3f(p.x(), p.y(), p.z());
}

inline void useNormal(const QVector3D& p) {
  glNormal3f(p.x(), p.y(), p.z());
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

static void drawQuad(const QVector3D p[4],
const QColor& color = Qt::black) {
  setColor(color);
  glBegin(GL_LINE_LOOP);
  useVertex(p[0]);
  useVertex(p[1]);
  useVertex(p[2]);
  useVertex(p[3]);
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

static void fillQuad(const QVector3D p[4],
const QColor& color = Qt::black) {
  setColor(color);
  glBegin(GL_QUADS);
  useVertex(p[0]);
  useVertex(p[1]);
  useVertex(p[2]);
  useVertex(p[3]);
  glEnd();
}

static void fillQuad(const QVector3D& p0,
                     const QVector3D& p1,
                     const QVector3D& p2,
                     const QVector3D& p3,
                     const QColor& color = Qt::black) {
  setColor(color);
  glBegin(GL_QUADS);
  useVertex(p0);
  useVertex(p1);
  useVertex(p2);
  useVertex(p3);
  glEnd();
}

}

#endif // GLUTILS_HPP
