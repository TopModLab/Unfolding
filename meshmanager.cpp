#include "meshmanager.h"
#include "meshloader.h"

#include <vector>

MeshManager* MeshManager::instance = NULL;

bool MeshManager::loadOBJFile(const string& filename) {
    OBJLoader loader;
    if( loader.load(filename) ) {
        cout << "file " << filename << " loaded." << endl;
        return true;
    }
    else return false;
}
