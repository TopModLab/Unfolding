#ifndef MESHLOADER_H
#define MESHLOADER_H

#include "common.h"
// #include <QtGui/QVector2D>
// #include <QtGui/QVector3D>
#include <QVector2D>
#include <QVector3D>

class MeshLoader
{
public:
    typedef QVector3D vert_t;
    struct face_t {
        face_t() {
            // at least 3 vertices
            v.reserve(8);
            n.reserve(8);
            t.reserve(8);
        }
        vector<int> v, n, t;
        QVector3D normal;
    };
    typedef QVector2D texcoord_t;
    typedef QVector3D norm_t;

    virtual bool load(const string& filename) = 0;

    const vector<vert_t>& getVerts() const { return verts; }
    const vector<face_t>& getFaces() const { return faces; }
    const vector<norm_t>& getNormals() const { return normals; }
    const vector<texcoord_t>& getTexcoords() const { return texcoords; }

protected:

    bool triangulated;
    bool hasVertexTexCoord;
    bool hasVertexNormal;
    vector<vert_t> verts;
    vector<face_t> faces;
    vector<texcoord_t> texcoords;
    vector<norm_t> normals;

protected:
    void clear();
    void triangulate();
    void estimateNormals();
};

class PLYLoader : public MeshLoader
{
public:
    virtual bool load(const string& filename);

protected:
    struct vert_t {
        float x, y, z;
        unsigned char r, g, b;
    };

    struct face_t {
        unsigned char nVerts;
        vector<int> verts;
    };
};

class OBJLoader : public MeshLoader
{
public:
    virtual bool load(const string& filename);
};


#endif // MESHLOADER_H
