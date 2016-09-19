#include "MeshFactory.h"

const HDS_Mesh* MeshFactory::refMesh = nullptr;

void MeshFactory::init()
{
	// TODO
}

void MeshFactory::setRefMesh(const mesh_t* ref)
{
	refMesh = ref;
}

void MeshFactory::constructEdge(vert_t* v1, vert_t* v2, he_t* he1, he_t* he2)
{
	// Link edge between vertices
}

void MeshFactory::constructFace(he_t* unlinkedHE, size_t edgeCount, face_t* f)
{
	// Link edge loops of a face 
	// Restriction: unlinkedHE has to be stored in order
	// TODO
}

void MeshFactory::fillNullFaces(
	vector<he_t> &hes, vector<face_t> &faces,
	unordered_set<hdsid_t> &exposedHEs)
{
	// Functionality
	// Input buffers: half-edges, faces,
	//                hash set of indices of exposed edges(flip == null)
	// Add null face and edges directly into original buffer
	// to make mesh validate.

	// record initial edge number
	size_t initSize = hes.size();
	// allocate unassigned edges
	hes.resize(initSize + exposedHEs.size());
	while (!exposedHEs.empty())
	{
		he_t* he = &hes[*exposedHEs.begin()];

		// Skip checked edges, won't skip in first check
		if (he->flip_offset) continue;

		faces.emplace_back();
		face_t* nullface = &faces.back();

		vector<hdsid_t> null_hes, null_hefs;
		size_t heIdOffset = initSize;
		// record null edges on the same null face
		auto curHE = he;
		do
		{
			null_hes.push_back(curHE->index);
			exposedHEs.erase(curHE->index);
			null_hefs.push_back(heIdOffset++);

			// if curHE->next->flip == null (offset != 0),
			//     found the next exposed edge
			///                       ___curHE___
			///                      /
			///     exposed edge--> / curHE->next
			///                    /
			// else, move to curHE->next->flip->next
			///                    \    <--exposed edge
			///         curHE->nex  \ 
			///         ->flip->next \  ___curHE___ 
			///                      / / 
			///         curHE->next / /curHE->next
			///         ->flip     / / 
			curHE = curHE->next();
			// Loop adjacent edges to find the exposed edge
			while (curHE->flip_offset)
			{
				curHE = curHE->flip()->next();
			}
		} while (curHE != he);
		// get edge number of current null face
		size_t nNullEdges = null_hes.size();

		// construct null face
		for (size_t i = 0; i < nNullEdges; i++)
		{
			curHE = &hes[null_hes[i]];
			he_t* curHEF = &hes[initSize + i];
			null_hefs[i] = initSize + i;

			curHE->isCutEdge = curHEF->isCutEdge = true;
			curHE->flip_offset = curHEF - curHE;
			curHEF->flip_offset = -curHE->flip_offset;
			curHEF->vid = curHE->next()->vid;
			curHEF->fid = nullface->index;

			/// Buffer: ...(existing edges)..., 0, 1, 2, ..., n-1
			/// Structure:   e(n-1)-> ... -> e1 -> e0 -> e(n-1)
			// prev edge is the next one in buffer,
			// except the last one, previous edge is the first one in buffer
			curHEF->prev_offset = (i == nNullEdges - 1) > 0 ? 1 : i;
			// next edge is the previous one in buffer,
			// except the first one, next edge is the last one in buffer
			curHEF->next_offset = (i > 0) ? -1 : nNullEdges - 1;
		}
		// Update Null Face Component and Flag
		nullface->isCutFace = true;
		//nullface->heid = he->index;
		nullface->heid = null_hefs[0];

		initSize += nNullEdges;
	}
}

void MeshFactory::generateBridger(
	he_t* he1, he_t *he2, const BridgerConfig& config,
	vector<vert_t> &verts, vector<he_t> &hes, vector<face_t> &fs)
{
	// TODO
}
