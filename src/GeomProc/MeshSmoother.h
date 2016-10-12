#ifndef MESHSMOOTHER_H
#define MESHSMOOTHER_H

#include "Utils/common.h"
#include "HDS/HDS_Mesh.h"

class MeshSmoother
{
public:
	static void smoothMesh_perVertex(HDS_Mesh *mesh);
	static void smoothMesh_wholeMesh(HDS_Mesh *mesh);
	static void smoothMesh(HDS_Mesh *mesh);
	static void smoothMesh_Laplacian(HDS_Mesh *mesh);

private:
	MeshSmoother() = delete;
};

#endif // MESHSMOOTHER_H
