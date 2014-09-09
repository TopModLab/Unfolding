#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include "common.h"
#include <QMutex>
#include <QScopedPointer>

#include "meshloader.h"

#include "hds_mesh.h"
#include "hds_face.h"
#include "hds_halfedge.h"
#include "hds_vertex.h"

class MeshManager
{
public:
    static MeshManager* getInstance()
    {
        static QMutex mutex;
        if (!instance)
        {
            mutex.lock();

            if (!instance)
                instance = new MeshManager;

            mutex.unlock();
        }

        return instance;
    }

    HDS_Mesh* getHalfEdgeMesh() {
        return hds_mesh.data();
    }

protected:
    /// should not be externally accessible
    static void drop()
    {
        static QMutex mutex;
        mutex.lock();
        delete instance;
        instance = NULL;
        mutex.unlock();
    }

private:
    MeshManager() {}
    MeshManager(const MeshManager &) = delete;
    MeshManager& operator=(const MeshManager &) = delete;

    static MeshManager* instance;

public:
    bool loadOBJFile(const string& filename);
    void buildHalfEdgeMesh(const vector<MeshLoader::face_t> &faces, const vector<MeshLoader::vert_t> &verts);

private:
    typedef HDS_Mesh mesh_t;
    typedef HDS_HalfEdge he_t;
    typedef HDS_Face face_t;
    typedef HDS_Vertex vert_t;

    QScopedPointer<HDS_Mesh> hds_mesh;
};

#endif // MESHMANAGER_H
