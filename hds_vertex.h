#ifndef HDS_VERTEX_H
#define HDS_VERTEX_H

#include "common.h"

#include <QVector3D>

class HDS_HalfEdge;

class HDS_Vertex
{
private:
  static size_t uid;

public:
  static void resetIndex() { uid = 0; }
  static size_t assignIndex() { return uid++; }

  HDS_Vertex();
  HDS_Vertex(const QVector3D &pos);
  ~HDS_Vertex();

  HDS_Vertex(const HDS_Vertex &other);
  HDS_Vertex operator=(const HDS_Vertex &other);

  void setPicked(bool v) { isPicked = v; }
  void computeCurvature();

  qreal x() { return pos.x(); }
  qreal y() { return pos.y(); }
  qreal z() { return pos.z(); }

  QVector3D pos;
  HDS_HalfEdge *he;

  int index;
  double curvature;
  bool isPicked;
};

#endif // HDS_VERTEX_H
