#ifndef HDS_MESH_H
#define HDS_MESH_H

#include <QVector3D>

#include "common.h"
#include "hds_halfedge.h"
#include "hds_vertex.h"
#include "hds_face.h"


class HDS_Mesh
{
public:
    typedef QVector3D point_t;
    typedef HDS_Face face_t;
    typedef HDS_Vertex vert_t;
    typedef HDS_HalfEdge he_t;

    HDS_Mesh();
    ~HDS_Mesh();

    void releaseMesh();

    void setMesh(const vector<face_t*> &faces,
                 const vector<vert_t*> &verts,
                 const vector<he_t*> &hes);

    void draw();
    void drawFaceIndices();

    const unordered_set<he_t*>& halfedges() const { return heSet; }
    unordered_set<he_t*>& halfedges() { return heSet; }
    const unordered_set<face_t*>& faces() const { return faceSet; }
    unordered_set<face_t*>& faces() { return faceSet; }
    const unordered_set<vert_t*>& verts() const { return vertSet; }
    unordered_set<vert_t*>& verts() { return vertSet; }

private:
    unordered_set<he_t*> heSet;
    unordered_set<face_t*> faceSet;
    unordered_set<vert_t*> vertSet;

private:
    bool showFace, showEdge, showVert;
};

#endif // HDS_MESH_H
