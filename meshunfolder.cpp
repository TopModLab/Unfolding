#include "meshunfolder.h"
#include "hds_mesh.h"

#include "utils.hpp"

MeshUnfolder::MeshUnfolder()
{
}

bool MeshUnfolder::unfold(HDS_Mesh *unfolded_mesh, HDS_Mesh *ref_mesh)
{
  /// start from a face, expand all faces
  queue<HDS_Face*> Q;
  Q.push(unfolded_mesh->faceMap[0]);
  vector<int> expSeq;
  set<int> visited;
  while( !Q.empty() ) {
    auto cur = Q.front();
    Q.pop();
    visited.insert(cur->index);
    expSeq.push_back(cur->index);
    /// get all neighbor faces
    vector<HDS_Face*> neighborFaces = unfolded_mesh->incidentFaces(cur);
    /// get all non-cut neighbor faces
    vector<HDS_Face*> nonCutNeighborFaces = Utils::filter(neighborFaces, [](HDS_Face* f) {
      return !(f->isCutFace);
    });

    /// TODO!!!
    /// there may be duplicates in the vector, remove those

    for( auto f : nonCutNeighborFaces ) {
      if( visited.find(f->index) == visited.end() ) {
        Q.push(f);
      }
    }
  }

  Utils::print(expSeq);

}
