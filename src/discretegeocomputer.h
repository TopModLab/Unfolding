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

	doubles_t computeDistanceTo(HDS_Vertex* init) const;
	doubles_t discreteDistanceTo(unordered_set<HDS_Vertex*> initSet) const;

private:

	HDS_Mesh* hds_mesh;
};

#endif // DISCRETEGEOCOMPUTER_H
