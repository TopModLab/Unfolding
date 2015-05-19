#include "discretegeocomputer.h"

DiscreteGeoComputer::DiscreteGeoComputer()
{

}

DiscreteGeoComputer::DiscreteGeoComputer(HDS_Mesh* hds_mesh)
{
    this->hds_mesh.reset(hds_mesh);
}

DiscreteGeoComputer::~DiscreteGeoComputer()
{

}

vector<double> DiscreteGeoComputer::discreteDistanceTo(HDS_Vertex* init) const
{
    auto dists = vector<double>();
    dists = vector<double>(hds_mesh->verts().size());
    //use BFS to traverse graph from init vertex


    queue<HDS_Vertex*> frontier;
    //runtime log parameter
    int iter = 1;

    unordered_set<HDS_Vertex*> visited;
    visited.insert(init);
    frontier.push(init);
    dists[init->index] = 0;

    cout<<"Starting BFS for cut locus.........."<<endl;
    while (!frontier.empty()) {
        HDS_Vertex* testNode = frontier.front();
        //Node* testNode = frontier.top();
        frontier.pop();

        cout<<"iteration="<<iter<<", frontier="<<frontier.size()
           <<", popped="<<testNode->index<<", dist="<<dists[testNode->index]<< endl;


        if (visited.size() == hds_mesh->verts().size()) {

            cout << "=========="<<endl;
            cout << "total iteration  = " <<iter<< endl;
            cout << "vertices visited = "  << visited.size() << "/" <<hds_mesh->verts().size() << endl;
            return dists;
        }

        vector<HDS_Vertex*> children;
        children = testNode->neighbors();
        for (int i = 0; i < children.size(); i++) {

            if (visited.find(children.at(i)) == visited.end()) {
                //set depth
                dists[children.at(i)->index] = dists[testNode->index] + 1;

                frontier.push(children.at(i));
                visited.insert(children.at(i));

            }
        }
        iter++;
    }

    //if fail BFS dists assignment, assign 0 to all vertices
    for (auto &x : dists) {
     x = 0;
    }
    return dists;

}
