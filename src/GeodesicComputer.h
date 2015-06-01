#pragma once

// for sparse graph computing
#include "../extras/SVG_LC_code/SVG_precompute/LocalGeodesics/svg_precompute.h"

// for sparse graph
#include "../extras/SVG_LC_code/SVG_LC/wxn_dijstra.h"

#include "common.h"

class GeodesicComputer
{
public:
	GeodesicComputer();
	GeodesicComputer(const string& filename);
	~GeodesicComputer();

	vector<double> distanceTo(int vIdx) const;

private:
	unique_ptr<SparseGraph<float>> s_graph;
};

