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
	~GeodesicComputer();

	doubles_t distanceTo(int vIdx) const;

private:
	unique_ptr<SparseGraph<float>> s_graph;
};

void svg_precompute(const string &input_obj_name, int fixed_k, string &svg_file_name, const doubles_t* verts, const vector<PolyIndex*>* faces);
