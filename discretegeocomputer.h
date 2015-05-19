#ifndef DISCRETEGEOCOMPUTER_H
#define DISCRETEGEOCOMPUTER_H

#include "hds_mesh.h"


class DiscreteGeoComputer
{
public:
    DiscreteGeoComputer();
    DiscreteGeoComputer(HDS_Mesh* hds_mesh);
    ~DiscreteGeoComputer();

    vector<double> discreteDistanceTo(HDS_Vertex* init) const;
private:

    QScopedPointer<HDS_Mesh> hds_mesh;
};

#endif // DISCRETEGEOCOMPUTER_H
