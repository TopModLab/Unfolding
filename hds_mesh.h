#ifndef HDS_MESH_H
#define HDS_MESH_H

#include <QVector3D>

#include "common.h"
#include "hds_halfedge.h"
#include "hds_vertex.h"
#include "hds_face.h"

#include "colormap.h"

class HDS_Mesh
{
public:
    typedef QVector3D point_t;
    typedef HDS_Face face_t;
    typedef HDS_Vertex vert_t;
    typedef HDS_HalfEdge he_t;

    enum ElementType {
      Face = 0,
      Edge,
      Vertex
    };

    HDS_Mesh();
    HDS_Mesh(const HDS_Mesh& other);
    ~HDS_Mesh();

    HDS_Mesh operator=(const HDS_Mesh& rhs);

    void printInfo(const string &msg = "");
    void printMesh(const string &msg = "");
    void releaseMesh();

    void setMesh(const vector<face_t*> &faces,
                 const vector<vert_t*> &verts,
                 const vector<he_t*> &hes);

    unordered_set<vert_t*> getReebPoints();

    void colorVertices(const vector<double> &val);

    void draw(ColorMap cmap);
    void drawFaceIndices();
    void drawVertexIndices();
    void drawEdgeIndices();
    void flipShowEdges();
    void flipShowFaces();
    void flipShowVertices();

    const unordered_set<he_t*>& halfedges() const { return heSet; }
    unordered_set<he_t*>& halfedges() { return heSet; }
    const unordered_set<face_t*>& faces() const { return faceSet; }
    unordered_set<face_t*>& faces() { return faceSet; }
    const unordered_set<vert_t*>& verts() const { return vertSet; }
    unordered_set<vert_t*>& verts() { return vertSet; }

    vector<face_t *> incidentFaces(vert_t *v);
    vector<he_t *> incidentEdges(vert_t *v);
    vector<face_t *> incidentFaces(face_t *f);

    template <typename T>
    void flipSelectionState(int idx, unordered_map<int, T> &m);
    void selectFace(int idx);
    void selectEdge(int idx);
    void selectVertex(int idx);

    void validate();
private:
    bool validateVertex(vert_t *v);
    bool validateFace(face_t *f);
    bool validateEdge(he_t *e);

protected:
    friend class MeshCutter;
    friend class MeshUnfolder;
    friend class MeshSmoother;
    friend class MeshExtender;

private:
    unordered_set<he_t*> heSet;
    unordered_set<face_t*> faceSet;
    unordered_set<vert_t*> vertSet;

    unordered_map<int, he_t*> heMap;
    unordered_map<int, face_t*> faceMap;
    unordered_map<int, vert_t*> vertMap;

private:
    bool showFace, showEdge, showVert;
};

inline ostream& operator<<(ostream &os, const HDS_Vertex& v) {
  os << v.index
     << ": (" << v.pos.x() << ", " << v.pos.y() << ", " << v.pos.z() << ")"
     << "\t"
     << v.he;
  return os;
}

inline ostream& operator<<(ostream &os, const HDS_HalfEdge& e) {
  os << e.index << "::"
     << " prev: " << e.prev->index
     << " next: " << e.next->index
     << " flip: " << e.flip->index
     << " v: " << e.v->index
     << " f:" << e.f->index;
  return os;
}

inline ostream& operator<<(ostream &os, const HDS_Face& f) {
  os << "face #" << f.index << " " << f.n << endl;
  HDS_HalfEdge *he = f.he;
  HDS_HalfEdge *curHE = he;
  do {
    os << "(" << curHE->index << ", " << curHE->v->index << ") ";
    curHE = curHE->next;
  } while( curHE != he );
  return os;
}

#endif // HDS_MESH_H
