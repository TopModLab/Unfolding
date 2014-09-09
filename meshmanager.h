#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include "common.h"
#include <QMutex>

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

private:

};

#endif // MESHMANAGER_H
