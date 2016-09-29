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
	const float patchScale = conf.at("patchScale");
	const float foldDepth = conf.at("foldDepth");

	HDS_Vertex::resetIndex();
	auto &ref_verts = ref_mesh->verts();
	auto &ref_hes = ref_mesh->halfedges();
	size_t refHeCount = ref_hes.size();
	size_t refFaceCount = ref_mesh->faces().size();

	vector<vert_t> verts(refHeCount);
	vector<he_t> hes(ref_hes);
	vector<face_t> faces(ref_mesh->faces());
	mesh_t* m = new HDS_Mesh(verts, hes, faces);

	//calculate and store face normals
	vector<QVector3D> fNorms(refFaceCount);
	for (int i = 0; i < refFaceCount; i++)
	{
		fNorms[i] = ref_mesh->faceNormal(i);
	}

	size_t heSize = m->halfedges().size();
	for (int i = 0; i < heSize ; i++)
	{
		//reconstruct verts, unlink faces
		m->verts()[i].pos = patchScale * ref_verts[ref_hes[i].vid].pos 
			+ (1-patchScale) * ref_mesh->faceCenter(ref_hes[i].fid);
		constructHE(&m->verts()[i], &m->halfedges()[i]);
		
	}

	for (int i = 0; i < heSize; i++) {
		hdsid_t flipid = ref_hes[i].flip()->index;
		if (flipid > i) {
			he_t &he1 = m->halfedges()[i];
			he_t &he2 = m->halfedges()[flipid];
			// calculate the bridge pos
			QVector3D n = (fNorms[he1.fid]
				+ fNorms[he2.fid]).normalized();
			vector<QVector3D> vpos1({
				(m->verts()[he1.vid].pos + m->verts()[he2.next()->vertID()].pos) /2.0
				- foldDepth * n});
			vector<QVector3D> vpos2({ 
				(m->verts()[he1.next()->vertID()].pos + m->verts()[he2.vid].pos) / 2.0
				- foldDepth * n });
			size_t heOriSize = m->halfedges().size();
			// generate bridge
			generateBridge(i, flipid, m, vpos1, vpos2);

			//if selected edge, unlink bridge edge pairs to make flaps
			if (m->halfedges()[i].isPicked) {
				for (int index = 0; index < vpos1.size(); index++)
				{
					m->splitHeFromFlip(heOriSize + 4 * (index + 1) - 2);
				}
			}
		}
	}



	unordered_set<hdsid_t> exposedHEs;
	for (auto &he : m->halfedges())
	{
		if (!he.flip_offset)
			exposedHEs.insert(he.index);
	}
	fillNullFaces(m->halfedges(), m->faces(), exposedHEs);

	return m;

}

