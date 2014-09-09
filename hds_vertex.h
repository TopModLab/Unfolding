#ifndef HDS_VERTEX_H
#define HDS_VERTEX_H

#include <QVector3D>

class HDS_HalfEdge;

class HDS_Vertex
{
public:
    HDS_Vertex();
    HDS_Vertex(const QVector3D &pos):pos(pos){}
    ~HDS_Vertex();

    HDS_Vertex(const HDS_Vertex &other);
    HDS_Vertex operator=(const HDS_Vertex &other);

    qreal x() { return pos.x(); }
    qreal y() { return pos.y(); }
    qreal z() { return pos.z(); }

    QVector3D pos;
    HDS_HalfEdge *he;
};

#endif // HDS_VERTEX_H
