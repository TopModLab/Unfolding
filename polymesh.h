#ifndef POLYMESH_H
#define POLYMESH_H

#include "OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh"

using namespace OpenMesh;
using namespace OpenMesh::Attributes;

struct MyTraits : public OpenMesh::DefaultTraits
{
  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

typedef OpenMesh::PolyMesh_ArrayKernelT<MyTraits>  PolyMesh;

#endif // POLYMESH_H
