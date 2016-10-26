#include "MeshOrigami.h"
#include "Utils/utils.h"
#include "MeshFactory/MeshUnfolder.h"

HDS_Mesh* MeshOrigami::create(
	const mesh_t* ref, const confMap &conf)
{
	return createOrigami(ref, conf);
}
HDS_Mesh* MeshOrigami::createOrigami(
	const mesh_t* ref_mesh, const confMap &conf) {
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
	mesh_t* ret = new HDS_Mesh(verts, hes, faces);

	size_t heSize = ret->halfedges().size();
	for (int i = 0; i < heSize; i++)
	{
		//reconstruct verts, unlink faces
		ret->verts()[i].pos = patchScale * ref_verts[ref_hes[i].vid].pos
			+ (1 - patchScale) * ref_mesh->faceCenter(ref_hes[i].fid);
		ret->verts()[i].setRefId(ref_hes[i].vid);
		constructHE(&ret->verts()[i], &ret->halfedges()[i]);

	}

	//give a random layout scheme for now
	for (int i = 0; i < ret->faces().size(); i++) {
			QVector3D vpos = QVector3D(2 * i, 0, 0);
			QVector3D ev = QVector3D(0, 1, 0);
		//unfold
		MeshUnfolder::unfoldSeparateFace(vpos, ev, i, ret);
	}
	//add bridges based on ref_mesh flip twins
	for (int i = 0; i < heSize; i++) {
		hdsid_t flipid = ref_hes[i].flip()->index;
		if ( flipid > i) {
			he_t* he1 = &ret->halfedges()[i];
			he_t* he2 = &ret->halfedges()[flipid];

			vector<QVector3D> vpos1 = {
				(ret->vertFromHe(i)->pos 
				+ ret->vertFromHe(he2->next()->index)->pos) / 2.0 };
			vector<QVector3D> vpos2 = {
				(ret->vertFromHe(flipid)->pos 
				+ ret->vertFromHe(he1->next()->index)->pos) / 2.0 };
			size_t heOriSize = ret->halfedges().size();
			// generate bridge
			generateBridge(i, flipid, ret, vpos1, vpos2);

			//if selected edge, unlink bridge edge pairs to make flaps
			if (ref_hes[i].isPicked) {
				//for (int index = 0; index < vpos1.size(); index++)
				//{
				ret->splitHeFromFlip(heOriSize + 2);
				//}
				
			}
		}
	}

	unordered_set<hdsid_t> exposedHEs;
	for (auto &he : ret->halfedges())
	{
		if (!he.flip_offset)
			exposedHEs.insert(he.index);
	}
	fillNullFaces(ret->halfedges(), ret->faces(), exposedHEs);

	return ret;
}

/*
HDS_Mesh* MeshOrigami::createOrigami(
	const mesh_t* ref_mesh, const confMap &conf)
{
	if (!ref_mesh) return nullptr;

	const float patchScale = conf.at("patchScale");
	const float foldDepth = conf.at("foldDepth");
	const float foldDepthOffset = 0.5;
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
			//extrude along the direction of edge normal

			QVector3D v11_in, v12_in, v21_in, v22_in;
			Utils::LineLineIntersect(v11 - foldDepth *en, v12 - foldDepth*en, v11, v11 - n1, &v11_in);
			Utils::LineLineIntersect(v11 - foldDepth *en, v12 - foldDepth*en, v12, v12 - n2, &v12_in);
			Utils::LineLineIntersect(v21 - foldDepth *en, v22 - foldDepth*en, v21, v21 - n1, &v21_in);
			Utils::LineLineIntersect(v21 - foldDepth *en, v22 - foldDepth*en, v22, v22 - n2, &v22_in);
			vector<QVector3D> vpos1({ v11_in, v21_in });
			vector<QVector3D> vpos2({ v12_in, v22_in });
			if (patchScale == 1) {
				vpos1.pop_back();
				vpos2.pop_back();
			}

			//uneven bridge edges
			

			size_t heOriSize = m->halfedges().size();
			// generate bridge
			generateBridge(i, flipid, m, vpos1, vpos2);
			hdsid_t flipBridgeHeId = m->halfedges()[heOriSize + 2].flip()->index;

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
*/

