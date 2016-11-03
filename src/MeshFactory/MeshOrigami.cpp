#include "MeshOrigami.h"
#include "Utils/utils.h"
#include "MeshFactory/MeshUnfolder.h"

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
	mesh_t* ret = new HDS_Mesh(verts, hes, faces);

	hdsid_t nulledge = ret->halfedges().size();
	for (auto &he : ret->halfedges())
	{
		if (he.isCutEdge)
		{
			nulledge = he.index;
			break;
		}
		he.flip_offset = 0;
		auto v = &ret->verts()[he.index];
		v->pos = ref_verts[he.vid].pos;
		v->heid = he.vid = he.index;
	}
	ret->halfedges().resize(nulledge);

	return ret;
}

HDS_Mesh* MeshOrigami::processOrigami(
	const mesh_t* ref_mesh, const mesh_t* ori_mesh, int anim) {
	mesh_t* ret = new HDS_Mesh(*ori_mesh);
	auto &ref_hes = ref_mesh->halfedges();
	size_t heSize = ref_hes.size();
	ret->matchIndexToSize();
	int faceSize = ret->faces().size();

	//give a random layout scheme for now
	for (int i = 0; i < faceSize; i++)
	{
		//unfold
		
		MeshUnfolder::unfoldSeparateFace(
			QVector3D(3 * (i - faceSize / 2), abs(i - faceSize / 2), 0),
			QVector3D(
				sin(2* M_PI * i / anim),
				1,
				0).normalized(),
			i, ret
		);
	}

	//add bridges based on ref_mesh flip twins

	for (int i = 0; i < heSize; i++)
	{
		hdsid_t flipid = ref_hes[i].flip()->index;
		if (flipid > i)
		{
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
