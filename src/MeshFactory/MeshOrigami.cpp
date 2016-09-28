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
	const float foldDepth = 0.1;

	HDS_Vertex::resetIndex();
	auto &ref_verts = ref_mesh->verts();
	auto &ref_hes = ref_mesh->halfedges();
	size_t refHeCount = ref_hes.size();
	size_t refFaceCount = ref_mesh->faces().size();

	vector<vert_t> verts(refHeCount);
	vector<he_t> hes(ref_hes);
	vector<face_t> faces(ref_mesh->faces());

	//calculate and store face normals
	vector<QVector3D> fNorms(refFaceCount);
	for (int i = 0; i < refFaceCount; i++)
	{
		fNorms[i] = ref_mesh->faceNormal(i);
	}

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
			he_t &he1 = hes[i];
			he_t &he2 = hes[flipid];
			cout << flipid << " not visited " << i << endl;
			// calculate the bridge pos
			QVector3D n = (fNorms[he1.fid]
				+ fNorms[he2.fid]).normalized();
			vector<QVector3D> vpos1({
				(verts[he1.vid].pos + verts[he2.next()->vertID()].pos) /2.0
				- foldDepth * n});
			vector<QVector3D> vpos2({ 
				(verts[he1.next()->vertID()].pos + verts[he2.vid].pos) / 2.0
				- foldDepth * n });
			// generate bridge
			generateBridge(i, flipid, hes, verts, faces, vpos1, vpos2);
			//if selected edge, unlink bridge edge pairs to make flaps
			if (hes[i].isPicked) {

			}
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

