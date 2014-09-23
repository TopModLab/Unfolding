#include "meshmanager.h"
#include "meshcutter.h"
#include "meshunfolder.h"

MeshManager* MeshManager::instance = NULL;

bool MeshManager::loadOBJFile(const string& filename) {
  OBJLoader loader;
  if( loader.load(filename) ) {
    cout << "file " << filename << " loaded." << endl;

    /// build a half edge mesh here
    buildHalfEdgeMesh(loader.getFaces(), loader.getVerts());
    return true;
  }
  else return false;
}

void MeshManager::buildHalfEdgeMesh(const vector<MeshLoader::face_t> &inFaces,
                                    const vector<MeshLoader::vert_t> &inVerts) {
  hds_mesh.reset(new mesh_t);

  cout << "building the half edge mesh ..." << endl;

  size_t vertsCount = inVerts.size();
  size_t facesCount = inFaces.size();
  size_t curFaceCount = facesCount;
  size_t heCount = 0;

  vector<vert_t*> verts;
  vector<face_t*> faces;
  vector<he_t*> hes;

  verts.resize(vertsCount);
  faces.resize(facesCount);

  for(size_t i=0;i<inFaces.size();i++)
    heCount += inFaces[i].v.size();

  hes.resize(heCount);

  for(size_t i=0;i<vertsCount;i++)
  {
    verts[i] = new vert_t(inVerts[i]);
  }

  for(size_t i=0;i<facesCount;i++)
  {
    faces[i] = new face_t;
  }

  map<pair<int, int>, he_t*> heMap;
  heMap.clear();

  for(size_t i=0, heIdx = 0;i<facesCount;i++)
  {
    auto& Fi = inFaces[i];
    face_t* curFace = faces[i];

    for(size_t j=0;j<Fi.v.size();j++)
    {
      he_t* curHe = new he_t;;
      vert_t* curVert = verts[Fi.v[j]];
      curHe->v = curVert;
      curHe->f = curFace;

      if( curVert->he == nullptr )
        curVert->he = curHe;

      hes[heIdx + j] = curHe;
    }

    // link the half edge of the face
    for(int j=0;j<Fi.v.size();j++)
    {
      int jp = j-1;
      if( jp < 0 ) jp += Fi.v.size();
      int jn = j+1;
      if( jn >= Fi.v.size() ) jn -= Fi.v.size();

      hes[heIdx+j]->prev = hes[heIdx+jp];
      hes[heIdx+j]->next = hes[heIdx+jn];

      int vj = Fi.v[j];
      int vjn = Fi.v[jn];

      pair<int, int> vPair = make_pair(vj, vjn);

      if( heMap.find(vPair) == heMap.end() )
      {
        heMap[vPair] = hes[heIdx+j];
      }
    }

    curFace->he = hes[heIdx];
    curFace->computeNormal();

    heIdx += Fi.v.size();
  }

  set<pair<int, int> > pairedHESet;

  // for each half edge, find its flip
  for(auto heit = heMap.begin();heit!=heMap.end();heit++)
  {
    int from, to;

    pair<int, int> hePair = (*heit).first;

    if( pairedHESet.find(hePair) == pairedHESet.end() )
    {

      from = hePair.first;
      to = hePair.second;
      pair<int, int> invPair = make_pair(to, from);

      auto invItem = heMap.find(invPair);

      if( invItem != heMap.end() )
      {
        he_t* he = (*heit).second;
        he_t* hef = (*invItem).second;

        he->flip = hef;
        hef->flip = he;
      }

      pairedHESet.insert( hePair );
      pairedHESet.insert( invPair );
    }
  }

  for( auto &v : verts ) {
    v->computeCurvature();
  }

  hds_mesh->setMesh(faces, verts, hes);
  cout << "finished building halfedge structure." << endl;
  cout << "halfedge count = " << hds_mesh->halfedges().size() << endl;
  hds_mesh->printMesh("original");
}

void MeshManager::cutMeshWithSelectedEdges()
{
  /// cut the mesh using the selected edges
  set<he_t*> selectedEdges;
  for(auto he : hds_mesh->halfedges()) {
    if( he->isPicked ) {
      /// use picked edges as cut edges
      he->setPicked(false);
      he->setCutEdge(true);

      if( selectedEdges.find(he) == selectedEdges.end() &&
          selectedEdges.find(he->flip) == selectedEdges.end() ) {
        selectedEdges.insert(he);
      }
    }
  }
  cout << "Number of selected edges = " << selectedEdges.size() << endl;

  bool isUnfoldable = false;
  QScopedPointer<HDS_Mesh> ref_mesh;
  ref_mesh.reset(new HDS_Mesh(*hds_mesh));
  while( !isUnfoldable ) {
    /// make a copy of the mesh with selected edges
    cutted_mesh.reset(new HDS_Mesh(*ref_mesh));

    bool cutSucceeded = MeshCutter::cutMeshUsingEdges(cutted_mesh.data(), selectedEdges);
    if( cutSucceeded ) {
      /// cutting performed successfully
      cutted_mesh->printInfo("cutted mesh:");
      //cutted_mesh->printMesh("cutted mesh:");
    }
    else {
      /// can not cut it
    }

    isUnfoldable = true;// MeshUnfolder::unfoldable(cutted_mesh.data());
    /// replace the ref mesh with the cutted mesh
    ref_mesh.reset(new HDS_Mesh(*cutted_mesh));
    /// discard the selected edges now
    selectedEdges.clear();
  }
}

void MeshManager::unfoldMesh()
{
  unfolded_mesh.reset(new HDS_Mesh(*cutted_mesh));

  /// cut the mesh using the selected edges
  set<int> selectedFaces;
  for(auto f : cutted_mesh->faces()) {
    if( f->isPicked ) {
      /// use picked edges as cut edges
      f->setPicked(false);

      if( selectedFaces.find(f->index) == selectedFaces.end() ) {
        selectedFaces.insert(f->index);
      }
    }
  }

  if( MeshUnfolder::unfold(unfolded_mesh.data(), cutted_mesh.data(), selectedFaces) ) {
    /// unfolded successfully
    unfolded_mesh->printInfo("unfolded mesh:");
    //unfolded_mesh->printMesh("unfolded mesh:");
  }
  else {
    /// failed to unfold
    cout << "Failed to unfold." << endl;
  }

}

bool MeshManager::saveMeshes() {
  /// save only the cutted and unfolded
  return true;
}
