#ifndef HDS_EDGE_H
#define HDS_EDGE_H

#include "hds_face.h"
#include "hds_vertex.h"

class HDS_Face;
class HDS_Vertex;

class HDS_HalfEdge
{
public:
    HDS_HalfEdge();
    ~HDS_HalfEdge();

private:
    HDS_Face *f;
    HDS_Vertex *v;
    HDS_HalfEdge *prev, *next, *flip;
};

#endif // HDS_EDGE_H
