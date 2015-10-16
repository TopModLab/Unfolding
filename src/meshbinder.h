#ifndef MESHBINDER_H
#define MESHBINDER_H

#include "hds_connector.h"
#include "hds_mesh.h"

class MeshBinder
{
public:
	MeshBinder();
	~MeshBinder();
	static void bindingMesh(HDS_Mesh* mesh);

};

#endif // MESHBINDER_H
