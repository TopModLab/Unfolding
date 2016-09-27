#include "MeshOrigami.h"

HDS_Mesh* MeshOrigami::create(
	const mesh_t* ref, const confMap &conf)
{
	return createOrigami(ref, conf);
}

HDS_Mesh* MeshOrigami::createOrigami(
	const mesh_t* ref_mesh, const confMap &conf)
{
	if (!ref_mesh) return nullptr;

	// TODO: conf should be passed in as configuration
	const float patchScale = 0.8;
	const float foldDepth = 0.3;

	HDS_Vertex::resetIndex();
	auto &ref_verts = ref_mesh->verts();
	auto &ref_hes = ref_mesh->halfedges();
	size_t refHeCount = ref_hes.size();
	vector<vert_t> verts(refHeCount);
	vector<he_t> hes(ref_hes);
	vector<face_t> faces(ref_mesh->faces());
	
	size_t heSize = hes.size();
	for (int i = 0; i < heSize ; i++)
	{
		//reconstruct verts, unlink faces
		verts[i].pos = patchScale * ref_verts[ref_hes[i].vid].pos 
			+ (1-patchScale) * ref_mesh->faceCenter(ref_hes[i].fid);
		constructHE(&verts[i], &hes[i]);
		
	}

	for (int i = 0; i < heSize; i++) {
		hdsid_t flipid = ref_hes[i].flip()->index;
		if (flipid > i) {
			cout << flipid << " not visited " << i << endl;
			//if selected edge, generate flaps
			if (hes[i].isPicked) continue;
			//else generate bridge
			else generateBridge(i, flipid, hes, verts, faces);
		}
	}



	unordered_set<hdsid_t> exposedHEs;
	for (auto &he : hes)
	{
		if (!he.flip_offset)
			exposedHEs.insert(he.index);
	}
	fillNullFaces(hes, faces, exposedHEs);

	return new HDS_Mesh(verts, hes, faces);

}

