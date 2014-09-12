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

    HDS_HalfEdge(const HDS_HalfEdge& other);
    HDS_HalfEdge operator=(const HDS_HalfEdge& other);

    void setPicked(bool v) { isPicked = v; flip->isPicked = v; }
    void setCutEdge(bool v) { isCutEdge = v; flip->isCutEdge = v; }

    HDS_Face *f;
    HDS_Vertex *v;
    HDS_HalfEdge *prev, *next, *flip;

    int index;
    bool isPicked;
    bool isCutEdge;
};

#endif // HDS_EDGE_H
