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
		m->verts()[i].setRefId(ref_hes[i].vid);
		constructHE(&m->verts()[i], &m->halfedges()[i]);
		
	}
	vector<QVector3D> vns = ref_mesh->allVertNormal();
	for (int i = 0; i < heSize; i++) {
		hdsid_t flipid = ref_hes[i].flip()->index;
		if (flipid > i) {
			he_t &he1 = m->halfedges()[i];
			he_t &he2 = m->halfedges()[flipid];
			// calculate the bridge pos
			// project vertex normal to edge normal's plane to ensure planar bridge
			QVector3D vn1 = vns[(m->verts()[he1.vid]).refid >> 2];
			QVector3D vn2 = vns[(m->verts()[he2.vid]).refid >> 2];
			QVector3D en = (fNorms[he1.faceID()] + fNorms[he2.faceID()]).normalized();
			QVector3D e = (ref_verts[(m->verts()[he2.vid]).refid >> 2].pos
				- ref_verts[(m->verts()[he1.vid]).refid >> 2].pos).normalized();
			QVector3D pn = QVector3D::normal(e, en);
			QVector3D n1 = (vn1 - (QVector3D::dotProduct(vn1, pn)) * pn).normalized();
			QVector3D n2 = (vn2 - (QVector3D::dotProduct(vn2, pn)) * pn).normalized();
			QVector3D v11 = m->verts()[he1.vid].pos;
			QVector3D v12 = m->verts()[he1.next()->vertID()].pos;
			QVector3D v21 = m->verts()[he2.next()->vertID()].pos;
			QVector3D v22 = m->verts()[he2.vid].pos;
			
			vector<QVector3D> vpos1({
				v11 - foldDepth * n1, v21 - foldDepth * n1 });
			vector<QVector3D> vpos2({ 
				v12 - foldDepth * n2, v22 - foldDepth * n2 });

			size_t heOriSize = m->halfedges().size();
			// generate bridge
			generateBridge(i, flipid, m, vpos1, vpos2);

			//if selected edge, unlink bridge edge pairs to make flaps
			if (m->halfedges()[i].isPicked) {
				//for (int index = 0; index < vpos1.size(); index++)
				//{
					m->splitHeFromFlip(heOriSize + 2);
				//}
					m->halfedges()[i].isPicked = false;
					m->halfedges()[flipid].isPicked = false;
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

