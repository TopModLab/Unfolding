#include "MeshFactory.h"

void MeshFactory::init()
{
	// TODO
}

// Functionality:
//	Link edge with vertex, 
//	v->he = he, he->v = v
///	o --------->
///	v		he
// Input:
//	vertex pointer, half edge pointer
void MeshFactory::constructHE(vert_t* v, he_t* he)
{
	if(v->heid == sInvalidHDS) v->heid = he->index;
	he->vid = v->index;
}



// Functionality: 
//	Link edge loops of a face, 
//	hes from unlinkedHE to unlinkedHE+edgeCount will be assigned to face fid
// Restriction: 
//	unlinkedHE has to be stored in order
// Input:
//	start edge's address, num of edges to be assigned,
//	face index of assigned face
void MeshFactory::constructFace(
	he_t* unlinkedHE, size_t edgeCount, 
	face_t* face)
{
	for (int i = 0; i < edgeCount; i++) {
		(unlinkedHE + i)->next_offset = (i < edgeCount - 1) ? 1 : 1 - edgeCount;
		(unlinkedHE + i)->prev_offset = (i > 0) ? -1 : edgeCount - 1;
		(unlinkedHE + i)->fid = face->index;
	}
	face->heid = unlinkedHE->index;
}

// Functionality: 
//	Link edge loops of a face, 
//	hes of indices will be assigned to face fid.
// Input: 
//	half-edges buffer, vector of indices of half edges to be assigned,
//	face index of assigned face
void MeshFactory::constructFace(
	vector<he_t> hes, const vector<int> indices, 
	face_t* face)
{
	int edgeCount = indices.size();
	for (int i = 0; i < edgeCount; i++) {
		(hes[indices[i]]).next_offset = (i < edgeCount - 1) ? 1 : 1 - edgeCount;
		(hes[indices[i]]).prev_offset = (i > 0) ? -1 : edgeCount - 1;
		(hes[indices[i]]).fid = face->index;
	}
	face->heid = indices[0];
}

// Functionality: 
//	Add null face and edges directly into original buffer to make mesh validate.
// Input buffers: 
//	half-edges, faces,
//	hash set of indices of exposed edges(flip == null)
// 
void MeshFactory::fillNullFaces(
	vector<he_t> &hes, vector<face_t> &faces,
	unordered_set<hdsid_t> &exposedHEs)
{
	

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
				curHE = curHE->rotCW();
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
			curHEF->prev_offset = (i < nNullEdges - 1) ? 1 : 1 - nNullEdges;
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

// Functionality:
//	generate a new bridger(all quads) connecting he1 and he2
//	newly generated faces->he is in same direction as he1.flip
///				--------> he1
///				<-------- he1.flip
///		vpos1[0] *		* vpos2[0]
///				 Bridger
///		vpos1[1] *		* vpos2[1]
///				--------> he2.flip 
///				<-------- he2
// Restrictions:
//	vpos1.size = vpos2.size
// Input:
//	half edge pointers,
//	positions for all the intra-vertices	
//	buffers for mesh
void MeshFactory::generateBridge(
	hdsid_t he1, hdsid_t he2, 
	mesh_t* mesh,
	vector<QVector3D> &vpos1, vector<QVector3D> &vpos2)
{
	vector<he_t> &hes = mesh->halfedges();
	vector<vert_t> &verts = mesh->verts();
	vector<face_t> &fs = mesh->faces();
	//resize vectors
	size_t size = vpos1.size();
	size_t vOriSize = verts.size();
	size_t heOriSize = hes.size();
	size_t fOriSize = fs.size();
	verts.resize(vOriSize + 2 * size);
	hes.resize(heOriSize + (size + 1) * 4);
	fs.resize(fOriSize + size + 1);

	//assign new v's pos
	for (int i = 0; i < size ; i++)
	{
		verts[vOriSize + 2 * i].pos = vpos1[i];
		verts[vOriSize + 2 * i + 1].pos = vpos2[i];
	}

	//link v and he
	hdsid_t vid11 = hes[he1].vertID();
	hdsid_t vid12 = hes[he1].next()->vertID();
	hdsid_t vid21 = hes[he2].next()->vertID();
	hdsid_t vid22 = hes[he2].vertID();

	//link edge loop
	for (int face = 0; face < size + 1; face++)
	{
		for (int heOffset = 0; heOffset < 4 ; heOffset++)
		{
			hdsid_t vid = 0;
			if (heOffset == 0) {
				if (face == 0) vid = vid12;
				else vid = vOriSize + 2 * (face-1) + 1;
			}
			else if (heOffset == 1) {
				if (face == 0) vid = vid11;
				else vid = vOriSize + 2 * (face-1);
			}
			else if (heOffset == 2) {
				if (face == size) vid = vid21;
				else vid = vOriSize + 2 * face;
			}
			else {
				if (face == size) vid = vid22;
				else vid = vOriSize + 2 * face + 1;
			}

			constructHE(&verts[vid], &hes[heOriSize + 4 * face + heOffset]);
		}
		constructFace(&hes[heOriSize + 4 * face], 4, &fs[fOriSize + face]);
	}

	//set flip
	hes[he1].setFlip(&hes[heOriSize]);
	hes[he2].setFlip(&hes[hes.size() - 2]);
	for (int i = 0; i < size ; i++)
	{
		hes[heOriSize + 4 * ( i + 1 ) - 2].setFlip(
			&hes[heOriSize + 4 * (i + 1)]);
	}
}
