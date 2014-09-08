#ifndef HDS_VERTEX_H
#define HDS_VERTEX_H

#include <QVector3D>

class HDS_HalfEdge;

class HDS_Vertex
{
public:
    HDS_Vertex();
    ~HDS_Vertex();

    HDS_Vertex(const HDS_Vertex &other);
    HDS_Vertex operator=(const HDS_Vertex &other);

private:
    QVector3D pos;
    HDS_HalfEdge *he;
};

#endif // HDS_VERTEX_H
