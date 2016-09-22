#include "MeshNeoWeaver.h"

HDS_Mesh* MeshNeoWeaver::create(
	const mesh_t* ref, const confMap &conf)
{
	return createWeaving(ref, conf);
}

HDS_Mesh* MeshNeoWeaver::createWeaving(
	const mesh_t* ref_mesh, const confMap &conf)
{
	if (!ref_mesh) return nullptr;

	// scaling should be passed in as configuration
	const float patchScale =  static_cast<float>(conf.at("patchScale"));
	const bool patchUniform = static_cast<float>(conf.at("patchUniform")) == 1.0f;
	auto &ref_verts = ref_mesh->verts();
	auto &ref_hes = ref_mesh->halfedges();
	auto &ref_faces = ref_mesh->faces();
	size_t refHeCount = ref_hes.size();
	size_t refEdgeCount = refHeCount >> 1;
	size_t refFaceCount = ref_faces.size();

	vector<QVector3D> fNorms(refFaceCount);
	vector<QVector3D> heMid(refEdgeCount);
	vector<QVector3D> heDirs(refEdgeCount);
	vector<float> heDirLens(refEdgeCount);
	vector<QVector3D> heNorms(refEdgeCount);
	vector<QVector3D> heTans(refEdgeCount);
	unordered_map<hdsid_t, hdsid_t> heCompMap;

	for (int i = 0; i < refFaceCount; i++)
	{
		fNorms[i] = ref_mesh->faceNormal(i);
	}

	for (hdsid_t i = 0, it = 0; i < refHeCount; i++)
	{
		auto he = &ref_hes[i];
		if (he->flip_offset < 0) continue;
		auto hef = he->flip();
		// TODO: to be finished
		// calc dir
		heMid[it] = (ref_verts[hef->vid].pos
						+ ref_verts[he->vid].pos) * 0.5f;
		heDirs[it] = (ref_verts[hef->vid].pos
						- ref_verts[he->vid].pos);
		heNorms[it] = (fNorms[he->fid]
						+ fNorms[hef->fid]).normalized();
		heTans[it] = QVector3D::crossProduct(
						heNorms[it], heDirs[it]).normalized();
		heNorms[it] = QVector3D::crossProduct(heDirs[it], heTans[it]).normalized();
		heCompMap[he->index] = heCompMap[hef->index] = it;

		heDirLens[it] = heDirs[it].length();
		heDirs[it] /= heDirLens[it];
		it++;
	}
	vector<vert_t> verts(refHeCount << 1);
	vector<he_t> hes(refHeCount << 1);
	vector<face_t> faces(refEdgeCount);

	hdsid_t outputOffset = 0;
	for (auto &he : ref_hes)
	{
		if (he.flip_offset < 0) continue;
		
		auto hef = he.flip();
		hdsid_t compID[5]{
			heCompMap.at(he.index),
			heCompMap.at(he.next()->index),
			heCompMap.at(he.prev()->index),
			heCompMap.at(hef->next()->index),
			heCompMap.at(hef->prev()->index)
		};
		QVector3D planeVecs[4]{
			QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[1]]),
			QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[2]]),
			QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[3]]),
			QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[4]]),
		};

		// Special Cases: normals are parallel to each other
		auto isZeroVec = [](const QVector3D &vec) {
			float threshold = 0.01f;
			return vec.lengthSquared() < sqr(threshold)/*(vec.x() < threshold && vec.x() > -threshold)
				&& (vec.y() < threshold && vec.y() > -threshold)
				&& (vec.z() < threshold && vec.z() > -threshold)*/;
		};
		if (isZeroVec(planeVecs[0]))
		{
			planeVecs[0] = heTans[compID[0]] - heDirs[compID[0]];
		}
		if (isZeroVec(planeVecs[1]))
		{
			planeVecs[1] = -heTans[compID[0]] - heDirs[compID[0]];
		}
		if (isZeroVec(planeVecs[2]))
		{
			planeVecs[2] = -heTans[compID[0]] + heDirs[compID[0]];
		}
		if (isZeroVec(planeVecs[3]))
		{
			planeVecs[3] = heTans[compID[0]] + heDirs[compID[0]];
		}

		planeVecs[1] *= -QVector3D::dotProduct(planeVecs[0], heTans[compID[0]])
			/ QVector3D::dotProduct(planeVecs[1], heTans[compID[0]]);
		planeVecs[3] *= -QVector3D::dotProduct(planeVecs[2], heTans[compID[0]])
			/ QVector3D::dotProduct(planeVecs[3], heTans[compID[0]]);

		float edgeLen = patchUniform? patchScale : heDirLens[compID[0]] * patchScale;

		float vecLen[2]{
			(planeVecs[0] + planeVecs[1]).length(),
			(planeVecs[2] + planeVecs[3]).length()
		};
		float scale[2]{
			edgeLen / (planeVecs[0] + planeVecs[1]).length(),
			edgeLen / (planeVecs[2] + planeVecs[3]).length()
		};
		if (vecLen[0] < 0.01f && vecLen[0] > -0.01f)
		{
			cout << "Null Vec: " << planeVecs[0] << ", " << planeVecs[1] << endl;
		}
		//printf("scale: %f, %f; len: %f, %f\n", scale[0], scale[1], vecLen[0], vecLen[1]);
		planeVecs[0] *= scale[0];
		planeVecs[1] *= scale[0];
		planeVecs[2] *= scale[1];
		//planeVecs[3] *= scale[1];

		// padded index for verts and HEs
		auto paddingIdx = outputOffset * 4;
		// Update vertex position

		// Edge Length
		verts[paddingIdx].pos = patchUniform? 
			heMid[compID[0]] + heDirs[compID[0]] * patchScale * 0.5f 
			: heMid[compID[0]] + heDirs[compID[0]] * edgeLen * 0.5f;

		verts[paddingIdx + 1].pos = verts[paddingIdx].pos + planeVecs[0];
		verts[paddingIdx + 2].pos = verts[paddingIdx + 1].pos + planeVecs[1];
		verts[paddingIdx + 3].pos = verts[paddingIdx + 2].pos + planeVecs[2];
		// Update vertex heid
		verts[paddingIdx    ].heid = paddingIdx;
		verts[paddingIdx + 1].heid = paddingIdx + 1;
		verts[paddingIdx + 2].heid = paddingIdx + 2;
		verts[paddingIdx + 3].heid = paddingIdx + 3;
		

		// Update edge connections
		hes[paddingIdx].prev_offset = 3;
		hes[paddingIdx + 1].prev_offset = hes[paddingIdx + 2].prev_offset
			= hes[paddingIdx + 3].prev_offset = -1;
		hes[paddingIdx].next_offset = hes[paddingIdx + 1].next_offset
			= hes[paddingIdx + 2].next_offset = 1;
		hes[paddingIdx + 3].next_offset = -3;

		// Update fid of edges
		hes[paddingIdx].fid
			= hes[paddingIdx + 1].fid
			= hes[paddingIdx + 2].fid
			= hes[paddingIdx + 3].fid
			= outputOffset;

		// Update vertex heid
		verts[paddingIdx    ].heid = hes[paddingIdx    ].vid = paddingIdx;
		verts[paddingIdx + 1].heid = hes[paddingIdx + 1].vid = paddingIdx + 1;
		verts[paddingIdx + 2].heid = hes[paddingIdx + 2].vid = paddingIdx + 2;
		verts[paddingIdx + 3].heid = hes[paddingIdx + 3].vid = paddingIdx + 3;

		// Update edge id of each face
		faces[outputOffset].heid = paddingIdx;
		
		outputOffset++;
		//normID[0] = heNorms[compID];
	}
	unordered_set<hdsid_t> exposedHEs(hes.size());
	for (hdsid_t i = 0; i < hes.size() ; i++)
	{
		exposedHEs.insert(i);
	}
	fillNullFaces(hes, faces, exposedHEs);
	return new HDS_Mesh(verts, hes, faces);
}
