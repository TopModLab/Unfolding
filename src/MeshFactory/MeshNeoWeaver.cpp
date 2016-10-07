#include "MeshNeoWeaver.h"

HDS_Mesh* MeshNeoWeaver::create(
	const mesh_t* ref, const confMap &conf)
{
	return createWeaving(ref, conf);
	//return createOctWeaving(ref, conf);
}

HDS_Mesh* MeshNeoWeaver::createOctWeaving(
	const mesh_t* ref_mesh, const confMap &conf)
{
	if (!ref_mesh) return nullptr;

	// scaling 
	const float patchScale = conf.at("patchScale");
	const bool patchUniform = (conf.at("patchUniform") == 1.0f);
	auto &ref_verts = ref_mesh->verts();
	auto &ref_hes = ref_mesh->halfedges();
	auto &ref_faces = ref_mesh->faces();
	size_t refHeCount = ref_hes.size();
	size_t refEdgeCount = refHeCount >> 1;
	size_t refFaceCount = ref_faces.size();

	mesh_t::resetIndex();
	// face normal buffer
	vector<QVector3D> fNorms(refFaceCount);
	// edge mid points
	vector<QVector3D> heMid(refEdgeCount);
	// Edge Directions
	vector<QVector3D> heDirs(refEdgeCount);
	// Edge Length Values
	vector<float> heDirLens(refEdgeCount);
	// Edge normals
	vector<QVector3D> heNorms(refEdgeCount);
	// Edge Tangent Directions
	vector<QVector3D> heTans(refEdgeCount);
	// Flag to Mark edges with face and flip face on the same plane
	vector<bool> heIsPlanar(refEdgeCount, false);
	// Hash Map from Original Edge to new Edge
	// key: original edge id, value: edge id in new mesh
	unordered_map<hdsid_t, hdsid_t> heCompMap;

	// Number of edges in each new patch
	const int edgeCount = 8;

	for (int i = 0; i < refFaceCount; i++)
	{
		fNorms[i] = ref_mesh->faceNormal(i);
	}

	// Pre-compute edge components
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

		// mark planer edge
		//heIsPlanar[it] = isParallel(fNorms[he->fid], fNorms[hef->fid]);
		if (isParallel(fNorms[he->fid], fNorms[hef->fid]))
		{
			cout << "parallel edge!\n";
			heIsPlanar[it] = true;
		}

		it++;
	}
	vector<vert_t> verts(refEdgeCount * edgeCount);
	vector<he_t> hes(refEdgeCount * edgeCount);
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
		auto &curHeDir = heDirs[compID[0]];
		auto &curHeTan = heTans[compID[0]];
		// sumVec, crossVec[4], fillVec[4]
		QVector3D planeVecs[9]/*{
			QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[1]]),
			QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[2]]),
			QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[3]]),
			QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[4]]),
		}*/;
		// from 1 to 4
		for (int i = 1; i < 5; i++)
		{
			planeVecs[i] = heIsPlanar[compID[i]]
				? ((i < 3) ? -curHeDir : curHeDir)
				: QVector3D::crossProduct(heNorms[compID[0]],
					heNorms[compID[i]]).normalized();
			planeVecs[0] += planeVecs[i];
		}
		if (QVector3D::dotProduct(curHeDir, planeVecs[1]) > 0.0f)
			cout << "dir x 1 wrong!" << endl;
		if (QVector3D::dotProduct(curHeDir, planeVecs[2]) > 0.0f)
			cout << "dir x 2 wrong!" << endl;
		if (QVector3D::dotProduct(curHeDir, planeVecs[3]) < 0.0f)
			cout << "dir x 3 wrong!" << endl;
		if (QVector3D::dotProduct(curHeDir, planeVecs[4]) < 0.0f)
			cout << "dir x 4 wrong!" << endl;
		if (QVector3D::dotProduct(curHeDir, planeVecs[1]) < 0.0f)
			cout << "tan x 1 wrong!" << endl;
		if (QVector3D::dotProduct(curHeTan, planeVecs[2]) > 0.0f)
			cout << "tan x 2 wrong!" << endl;
		if (QVector3D::dotProduct(curHeTan, planeVecs[3]) > 0.0f)
			cout << "tan x 3 wrong!" << endl;
		if (QVector3D::dotProduct(curHeTan, planeVecs[4]) < 0.0f)
			cout << "tan x 4 wrong!" << endl;
		// Special Cases: normals are parallel to each other
		float dx_2 = QVector3D::dotProduct(planeVecs[0], curHeDir);
		float dy_2 = QVector3D::dotProduct(planeVecs[0], curHeTan);
		const float gapLen = 0.0f;// tinny gap for non zero edges
		float lenX_gap5, lenX_gap6, lenX_gap7, lenX_gap8;// edge length 5, 6, 7, 8
		if (dx_2 > 0.0f)
		{
			lenX_gap5 = -gapLen;
			lenX_gap7 = dx_2 + gapLen;
		}
		else
		{
			lenX_gap5 = dx_2 - gapLen;
			lenX_gap7 = gapLen;
		}
		if (dy_2 > 0.0f)
		{
			lenX_gap6 = -gapLen;
			lenX_gap8 = dy_2 + gapLen;
		}
		else
		{
			lenX_gap6 = dy_2 - gapLen;
			lenX_gap8 = gapLen;
		}
		planeVecs[5] = curHeDir * lenX_gap5;
		planeVecs[6] = curHeTan * lenX_gap6;
		planeVecs[7] = curHeDir * lenX_gap7;
		planeVecs[8] = curHeTan * lenX_gap8;

		float xlen = -QVector3D::dotProduct(curHeDir, planeVecs[1] + planeVecs[2] + planeVecs[5]);
		float patchAbsLen = patchUniform
			? patchScale : heDirLens[compID[0]] * patchScale;
		float vecScale = patchAbsLen / xlen;
		//printf("dxdy (%f, %f), lenx gap(%f, %f, %f, %f), vecScale: %f\n",
		//	dx_2, dy_2, lenX_gap5, lenX_gap6, lenX_gap7, lenX_gap8, vecScale);

		// padded index for verts and HEs
		auto paddingIdx = outputOffset * edgeCount;
		// Update vertex position

		// Edge Length
		verts[paddingIdx].pos = heMid[compID[0]]
			+ curHeDir * patchAbsLen * 0.5f + planeVecs[8] * 0.5f * vecScale;

		verts[paddingIdx + 1].pos = verts[paddingIdx].pos + planeVecs[1] * vecScale;
		verts[paddingIdx + 2].pos = verts[paddingIdx + 1].pos + planeVecs[5] * vecScale;
		verts[paddingIdx + 3].pos = verts[paddingIdx + 2].pos + planeVecs[2] * vecScale;
		verts[paddingIdx + 4].pos = verts[paddingIdx + 3].pos + planeVecs[6] * vecScale;
		verts[paddingIdx + 5].pos = verts[paddingIdx + 4].pos + planeVecs[3] * vecScale;
		verts[paddingIdx + 6].pos = verts[paddingIdx + 5].pos + planeVecs[7] * vecScale;
		verts[paddingIdx + 7].pos = verts[paddingIdx + 6].pos + planeVecs[4] * vecScale;

		// Construct edges
		for (int i = 0; i < edgeCount; i++)
		{
			constructHE(&verts[paddingIdx + i], &hes[paddingIdx + i]);
		}

		//construct edge loop
		constructFace(hes.data() + paddingIdx, edgeCount, faces.data() + outputOffset);

		//for testing bridger

		outputOffset++;
	}

	mesh_t* newMesh = new HDS_Mesh(verts, hes, faces);
	/*for (int i = 0; i < refHeCount; i++)
	{
	auto he = &ref_hes[i];
	auto he_next = he->next();

	hdsid_t he1id, he2id;
	he1id = he->flip_offset > 0
	? heCompMap.at(he->index) * edgeCount
	: heCompMap.at(he->flip()->index) * edgeCount + 2;
	he2id = he_next->flip_offset > 0
	? heCompMap.at(he_next->index) * edgeCount
	: heCompMap.at(he_next->flip()->index) * edgeCount + 2;
	}*/

	unordered_set<hdsid_t> exposedHEs;
	for (auto &he : newMesh->halfedges())
	{
		if (!he.flip_offset)
			exposedHEs.insert(he.index);
	}
	fillNullFaces(newMesh->halfedges(), newMesh->faces(), exposedHEs);

	newMesh->updatePieceSet();

	return newMesh;
}

HDS_Mesh* MeshNeoWeaver::createWeaving(
	const mesh_t* ref_mesh, const confMap &conf)
{
	if (!ref_mesh) return nullptr;

	// scaling 
	const float patchScale =  conf.at("patchScale");
	const bool patchUniform = (conf.at("patchUniform") == 1.0f);
	auto &ref_verts = ref_mesh->verts();
	auto &ref_hes = ref_mesh->halfedges();
	auto &ref_faces = ref_mesh->faces();
	size_t refHeCount = ref_hes.size();
	size_t refEdgeCount = refHeCount >> 1;
	size_t refFaceCount = ref_faces.size();

	mesh_t::resetIndex();
	vector<QVector3D> fNorms(refFaceCount);
	vector<QVector3D> heMid(refEdgeCount);
	vector<QVector3D> heDirs(refEdgeCount);
	vector<float> heDirLens(refEdgeCount);
	vector<QVector3D> heNorms(refEdgeCount);
	vector<QVector3D> heTans(refEdgeCount);
	// key: original edge id, value: edge id in new mesh
	unordered_map<hdsid_t, hdsid_t> heCompMap;

	// Number of edges in each new patch
	int edgeCount = 4;

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

		float edgeLen = patchUniform ? patchScale : heDirLens[compID[0]] * patchScale;

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
		auto paddingIdx = outputOffset * edgeCount;
		// Update vertex position

		// Edge Length
		verts[paddingIdx].pos = patchUniform ?
			heMid[compID[0]] + heDirs[compID[0]] * patchScale * 0.5f
			: heMid[compID[0]] + heDirs[compID[0]] * edgeLen * 0.5f;

		verts[paddingIdx + 1].pos = verts[paddingIdx].pos + planeVecs[0];
		verts[paddingIdx + 2].pos = verts[paddingIdx + 1].pos + planeVecs[1];
		verts[paddingIdx + 3].pos = verts[paddingIdx + 2].pos + planeVecs[2];
		
		// Construct edges
		for (int i = 0; i < edgeCount ; i++)
		{
			constructHE(&verts[paddingIdx + i], &hes[paddingIdx + i]);
		}
		
		//construct edge loop
		constructFace(&hes[paddingIdx], edgeCount, &faces[outputOffset]);

		//for testing bridger

		outputOffset++;
		//normID[0] = heNorms[compID];
	}
	
	mesh_t* newMesh = new HDS_Mesh(verts, hes, faces);
	/*for (int i = 0; i < refHeCount; i++)
	{
		auto he = &ref_hes[i];
		auto he_next = he->next();

		hdsid_t he1id, he2id;
		he1id = he->flip_offset > 0
			? heCompMap.at(he->index) * edgeCount
			: heCompMap.at(he->flip()->index) * edgeCount + 2;
		he2id = he_next->flip_offset > 0
			? heCompMap.at(he_next->index) * edgeCount
			: heCompMap.at(he_next->flip()->index) * edgeCount + 2;
	}*/

	unordered_set<hdsid_t> exposedHEs;
	for (auto &he: newMesh->halfedges())
	{
		if (!he.flip_offset)
			exposedHEs.insert(he.index);
	}
	fillNullFaces(newMesh->halfedges(), newMesh->faces(), exposedHEs);

	newMesh->updatePieceSet();

	return newMesh;
}
