#include "GeomUtils/DiscreteGeoComputer.h"

DiscreteGeoComputer::DiscreteGeoComputer()
{

}

DiscreteGeoComputer::DiscreteGeoComputer(HDS_Mesh* hds_mesh)
{
	this->hds_mesh = hds_mesh;

}

DiscreteGeoComputer::~DiscreteGeoComputer()
{

}

doubles_t DiscreteGeoComputer::discreteDistanceTo(unordered_set<HDS_Vertex*> initSet) const
{
	auto dists = doubles_t();
	dists = doubles_t(hds_mesh->verts().size());
	for (int i = 0; i < dists.size();i++) {
		dists[i] = numeric_limits<double>::infinity();
	}
	for (auto i: initSet) {
		doubles_t curDists = computeDistanceTo(i);
		for (int index = 0; index < curDists.size();index++) {
			dists[index] = min(curDists[index], dists[index]);
		}
	}
	return dists;
}

doubles_t DiscreteGeoComputer::computeDistanceTo(HDS_Vertex* init) const
{
	auto dists = doubles_t();
	cout<<hds_mesh->verts().size()<<endl;
	dists = doubles_t(hds_mesh->verts().size());
	//use BFS to traverse graph from init vertex
	cout<<"dists initialized"<<endl;

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
		dists[x] = 0;
	}
	return dists;

}
