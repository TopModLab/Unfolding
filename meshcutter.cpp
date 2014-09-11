#include "meshcutter.h"
#include "hds_mesh.h"
#include "utils.hpp"

MeshCutter::MeshCutter()
{
}

bool MeshCutter::cutMeshUsingEdges(HDS_Mesh *mesh, set<HDS_HalfEdge *> &edges)
{
  typedef HDS_HalfEdge he_t;
  typedef HDS_Vertex vert_t;
  typedef HDS_Face face_t;

  /// vertices connected to cut edges
  map<vert_t*, int> cutVerts;

  /// split each cut edge into 2 edges
  for(auto he : edges) {
    auto hef = he->flip;
    auto vs = he->v;
    auto ve = hef->v;

    /// record the cut vertices
    auto cit = cutVerts.find(vs);
    if( cit == cutVerts.end() ) {
      cutVerts.insert(make_pair(vs, 1));
    }
    else {
      ++cutVerts[vs];
    }
    cit = cutVerts.find(ve);
    if( cit == cutVerts.end() ) {
      cutVerts.insert(make_pair(ve, 1));
    }
    else {
      ++cutVerts[ve];
    }

    /// create a new face
    face_t *nf = new face_t;

    /// duplicate half edges
    he_t *he_new_flip = new he_t;
    he_t *hef_new_flip = new he_t;

    /// the flip of he
    he_new_flip->flip = he;
    he_new_flip->f = nf;
    he_new_flip->v = ve;
    he_new_flip->next = hef_new_flip;
    he_new_flip->prev = hef_new_flip;

    he->flip = he_new_flip;

    he_new_flip->index = mesh->heSet.size();
    mesh->heSet.insert(he_new_flip);

    /// the flip of hef
    hef_new_flip->flip = hef;
    hef_new_flip->f = nf;
    hef_new_flip->v = vs;
    hef_new_flip->next = he_new_flip;
    hef_new_flip->prev = he_new_flip;

    hef->flip = hef_new_flip;

    hef_new_flip->index = mesh->heSet.size();
    mesh->heSet.insert(hef_new_flip);

    /// fix the new face
    nf->he = hef;
    nf->index = mesh->faceSet.size();
    nf->isCutFace = true;
    mesh->faceSet.insert(nf);
    mesh->faceMap.insert(make_pair(nf->index, nf));
  }

  mesh->printInfo();

  /// check each cut vertices, merge faces if necessary
  for(auto cv : cutVerts) {
    if( cv.second > 1 ) {
      /// merge all incident faces

      /// get all incident faces
      set<face_t*> incidentFaces = mesh->incidentFaces(cv.first);
      set<face_t*> cutFaces = filter(incidentFaces, [](face_t* f) {
        return f->isCutFace;
      });

      /// merge them
      /// since they are discovered in a given order (CW or CCW), just glue them together
      /// each cut vertex is splitted into n vertices, depending on the number of cut faces
      /// it is connected to. non-cut vertices are not splitted.
      /// as shown below
      ///
      ///            ncv
      ///            / \
      ///           /cf \
      ///           cv----(cv)           <--- cut vertex is splitted into 2 vertices
      ///            \ cf /
      ///             \  /
      ///              \/
      ///             ncv


    }
  }

  return true;
}
