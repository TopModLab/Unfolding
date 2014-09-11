#ifndef MESHUNFOLDER_H
#define MESHUNFOLDER_H

#include "common.h"

class HDS_Mesh;

class MeshUnfolder
{
public:
  MeshUnfolder();

  static bool unfold(HDS_Mesh *mesh, HDS_Mesh *ref);

};

#endif // MESHUNFOLDER_H
