#ifndef MESHSMOOTHER_H
#define MESHSMOOTHER_H

#include "common.h"

class HDS_Mesh;
class HDS_Face;
class HDS_HalfEdge;
class HDS_Vertex;

class MeshSmoother
{
public:
    MeshSmoother();

    static void smoothMesh(HDS_Mesh *mesh);

private:
};

#endif // MESHSMOOTHER_H
