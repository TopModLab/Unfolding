#pragma once

// for sparse graph computing
#include "../extras/SVG_LC_code/SVG_precompute/LocalGeodesics/svg_precompute.h"

// for sparse graph
#include "../extras/SVG_LC_code/SVG_LC/wxn_dijstra.h"

#include "common.h"
#include "meshloader.h"

class GeodesicComputer
{
public:
	GeodesicComputer();
	GeodesicComputer(const string &filename,
		const doubles_t* verts = nullptr,
		const ui32s_t* faces = nullptr);
	
	doubles_t distanceTo(int vIdx) const;

private:
	unique_ptr<SparseGraph<float>> s_graph;
};