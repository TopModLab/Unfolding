#include "meshunfolder.h"
#include "hds_mesh.h"

#include "utils.hpp"
#include "mathutils.hpp"

#include <QDebug>

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

  // print the corners of the previous face
  auto corners_prev = face_prev->corners();
  qDebug() << "corners of face_prev";
  for(auto p : corners_prev) {
    qDebug() << p->index << " @ " << p->pos;
  }

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

  /// compute the spanning vectors for the unfolded mesh
  QVector3D xvec = vs->pos - ve->pos;
  xvec.normalize();
  QVector3D yvec = uvec - QVector3D::dotProduct(uvec, xvec) * xvec;
  const qreal eps = 1e-6;
  if( yvec.length() < eps ) {
    yvec = vvec - QVector3D::dotProduct(vvec, xvec) * xvec;
  }
  yvec.normalize();
  QVector3D cface_prev = face_prev->center();
  if( QVector3D::dotProduct(cface_prev - vs->pos, yvec) > 0 ) yvec = -yvec;

  /// shared edge in the reference mesh
  he_t *he_share_ref = ref_mesh->heMap.at(he_share->index);
  face_t *face_cur_ref = ref_mesh->faceMap.at(face_cur->index);
  vert_t *vs_ref = he_share_ref->v;
  vert_t *ve_ref = he_share_ref->flip->v;

  /// compute the spanning vectors for the face in the reference mesh
  QVector3D cface_ref = face_cur_ref->center();
  qDebug() << "cface ref:  " << cface_ref;
  qDebug() << "cface prev: " << cface_prev;
  QVector3D xvec_ref = vs_ref->pos - ve_ref->pos;
  xvec_ref.normalize();
  QVector3D yvec_ref = (cface_ref - vs_ref->pos) - QVector3D::dotProduct(cface_ref - vs_ref->pos, xvec_ref) * xvec_ref;
  yvec_ref.normalize();

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
      qDebug() << "x = " << xcoord << ", " << "y = " << ycoord << "\t"
               << v->index << " @ " << v->pos;
    }
    curHE = curHE->next;
  } while( curHE != he );
}

bool MeshUnfolder::unfoldable(HDS_Mesh *cutted_mesh) {
  /// for each vertex in the cutted_mesh, check the condition
  auto isBadVertex = [](HDS_Vertex* v) -> bool {
    double sum = 0;
    auto he = v->he;
    auto curHE = he->flip->next;
    bool hasCutFace = false;
    do {
      if( !curHE->f->isCutFace ) {
        QVector3D v1 = he->flip->v->pos - he->v->pos;
        QVector3D v2 = curHE->flip->v->pos - curHE->v->pos;
        double nv1pnv2 = v1.length() * v2.length();
        double inv_nv1pnv2 = 1.0 / nv1pnv2;
        double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
        double angle = acos(cosVal);
        sum += angle;
      }
      else hasCutFace = true;

      he = curHE;
      curHE = he->flip->next;
    }while( he != v->he ) ;

    /// the sum must be smaller than PI2.
    return sum > PI2 || (sum < PI2 && !hasCutFace);
  };

  if( any_of(cutted_mesh->vertSet.begin(), cutted_mesh->vertSet.end(), isBadVertex) ) return false;
  else return true;
}

bool MeshUnfolder::unfold(HDS_Mesh *unfolded_mesh, HDS_Mesh *ref_mesh, set<int> fixedFaces)
{
  if( !unfoldable(ref_mesh) ) {
    cout << "Mesh can not be unfolded. Check if the cuts are well defined." << endl;
    return false;
  }

  if( fixedFaces.empty() ) {
    cout << "No face is selected, finding fixed faces..." << endl;
    /// find all fixed faces
    unordered_set<int> visitedFaces;

    for(auto f : ref_mesh->faceSet) {
      if( f->isCutFace ) continue;
      if( visitedFaces.find(f->index) == visitedFaces.end() ) {
        visitedFaces.insert(f->index);
        fixedFaces.insert(f->index);
        set<HDS_Face*> connectedFaces = Utils::filter_set(f->connectedFaces(), [](HDS_Face* f){
          return !(f->isCutFace);
        });
        for(auto cf : connectedFaces) {
          visitedFaces.insert(cf->index);
        }
      }
    }

    cout << "Fixed faces found:" << endl;
    Utils::print(fixedFaces);
  }

  for( auto fid : fixedFaces ) {
    /// start from a face, expand all faces
    queue<HDS_Face*> Q;
    Q.push(unfolded_mesh->faceMap[fid]);
    vector<int> expSeq;     // sequence of expansion
    map<int, int> parentMap;
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
          parentMap.insert(make_pair(f->index, cur->index));
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
      unfoldFace(parentMap.at(expSeq[i]), expSeq[i], unfolded_mesh, ref_mesh, uvec, vvec);
    }
  }

  return true;
}
