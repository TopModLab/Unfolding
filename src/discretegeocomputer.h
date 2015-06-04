#ifndef DISCRETEGEOCOMPUTER_H
#define DISCRETEGEOCOMPUTER_H

#include "hds_mesh.h"
#include <limits>
#include <algorithm>


class DiscreteGeoComputer
{
public:
	DiscreteGeoComputer();
	DiscreteGeoComputer(HDS_Mesh* hds_mesh);
	~DiscreteGeoComputer();

	vector<double> computeDistanceTo(HDS_Vertex* init) const;
	vector<double> discreteDistanceTo(unordered_set<HDS_Vertex*> initSet) const;

private:

	QScopedPointer<HDS_Mesh> hds_mesh;
};

#endif // DISCRETEGEOCOMPUTER_H
