#include "meshunfolder.h"
#include "hds_mesh.h"

#include "utils.hpp"

MeshUnfolder::MeshUnfolder()
{
}

void MeshUnfolder::unfoldFace(int fprev, int fcur, HDS_Mesh *unfolded_mesh, HDS_Mesh *ref_mesh,
                              const QVector3D &uvec, const QVector3D &vvec) {
  typedef HDS_Face face_t;
  typedef HDS_Vertex vert_t;
  typedef HDS_HalfEdge he_t;

  // use the previous face as reference, expand current face
  face_t *face_prev = unfolded_mesh->faceMap.at(fprev);
  face_t *face_cur = unfolded_mesh->faceMap.at(fcur);

  /// find the shared half edge
  he_t *he_share = face_prev->he;
  do {
    if( he_share->flip->f == face_cur ) break;
    he_share = he_share->next;
  } while( he_share != face_prev->he );

  /// change the position of the vertices in the current face, except for the end points
  /// of he_share
  vert_t *vs = he_share->v;
  vert_t *ve = he_share->flip->v;

  /// shared edge in the reference mesh
  he_t *he_share_ref = ref_mesh->heMap.at(he_share->index);
  face_t *face_cur_ref = ref_mesh->faceMap.at(face_cur->index);
  vert_t *vs_ref = he_share_ref->v;
  vert_t *ve_ref = he_share_ref->flip->v;

  /// compute the spanning vectors for the face in the reference mesh
  QVector3D cface_ref = face_cur_ref->center();
  QVector3D xvec_ref = he_share_ref->v->pos - he_share_ref->flip->v->pos;
  xvec_ref.normalize();
  QVector3D yvec_ref = cface_ref - QVector3D::dotProduct(cface_ref, xvec_ref) * xvec_ref;
  yvec_ref.normalize();

  /// compute a new pair of spanning vectors for the unfolded mesh
  QVector3D cface = face_cur->center();
  QVector3D xvec = vs->pos - ve->pos;
  xvec.normalize();
  QVector3D yvec = cface - QVector3D::dotProduct(cface, xvec) * xvec;
  yvec.normalize();

  he_t *he = he_share->flip;
  he_t *curHE = he;
  do {
    if( curHE->v == vs || curHE->v == ve ) {
      /// nothing to do, these two should not be modified
    }
    else {
      /// compute the new position of the vertex
      vert_t *v = curHE->v;
      vert_t *v_ref = ref_mesh->vertMap.at(curHE->v->index);

      /// compute the coordinates of this vertex using the reference mesh
      QVector3D dvec_ref = v_ref->pos - vs_ref->pos;
      qreal xcoord = QVector3D::dotProduct(dvec_ref, xvec_ref);
      qreal ycoord = QVector3D::dotProduct(dvec_ref, yvec_ref);

      v->pos = vs->pos + xcoord * xvec + ycoord * yvec;
      cout << v->pos.x() << ", " << v->pos.y() << ", " << v->pos.z() << endl;
    }
    curHE = curHE->next;
  } while( curHE != he );
}

bool MeshUnfolder::unfold(HDS_Mesh *unfolded_mesh, HDS_Mesh *ref_mesh)
{
  /// start from a face, expand all faces
  queue<HDS_Face*> Q;
  Q.push(unfolded_mesh->faceMap[0]);
  vector<int> expSeq;     // sequence of expansion
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

    for( auto f : nonCutNeighborFaces ) {
      if( visited.find(f->index) == visited.end() ) {
        Q.push(f);
      }
    }
  }

  /// print out the sequence of performing unfolding
  Utils::print(expSeq);

  /// compute the spanning vectors for the first face
  QVector3D uvec, vvec;
  HDS_Face *face0 = unfolded_mesh->faceMap.at(expSeq.front());
  QVector3D cface0 = face0->center();
  uvec = face0->he->v->pos - cface0;
  vvec = face0->he->flip->v->pos - cface0;

  /// unfold the mesh using the sequence
  /// update the vertex positions of the unfolded mesh base on the geometry of the reference mesh
  for(int i=1;i<expSeq.size();++i) {
    unfoldFace(expSeq[i-1], expSeq[i], unfolded_mesh, ref_mesh, uvec, vvec);
  }

  return true;
}
