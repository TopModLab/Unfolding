#include "MeshNeoWeaver.h"
#define LERP_BRIDGE 1
#define DOO_SABIN 1
HDS_Mesh* MeshNeoWeaver::create(const mesh_t* ref,
                                const confMap &conf)
{
    if (!ref) return nullptr;

	return createConicalWeaving(ref, conf);
	//return createClassicalWeaving(ref, conf);
	//return createWeaving(ref, conf);
	//return createOctWeaving(ref, conf);
}

HDS_Mesh* MeshNeoWeaver::createOctWeaving(const mesh_t* ref_mesh,
                                          const confMap &conf)
{
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
			constructHEPair(&verts[paddingIdx + i], &hes[paddingIdx + i]);
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
		if (!he.flip_offset) exposedHEs.insert(he.index);
	}
	fillNullFaces(newMesh->halfedges(), newMesh->faces(), exposedHEs);

	newMesh->updatePieceSet();

	return newMesh;
}

HDS_Mesh* MeshNeoWeaver::createWeaving(const mesh_t* ref_mesh,
                                       const confMap &conf)
{
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
	vector<bool> isConcaveEdge(refEdgeCount, false);
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
		// check if edge is concave
		isConcaveEdge[it] = !isSameDir(
			QVector3D::crossProduct(fNorms[he->fid], fNorms[hef->fid]), heDirs[it]);
		heDirLens[it] = heDirs[it].length();
		heDirs[it] /= heDirLens[it];

		heCompMap[he->index] = heCompMap[hef->index] = it;
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
			return vec.lengthSquared() < sqr(threshold);
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

		float edgeLen = patchUniform ? patchScale : heDirLens[compID[0]] * patchScale;

		Float v0x = QVector3D::dotProduct(planeVecs[0], heDirs[compID[0]]);
		Float v0y = QVector3D::dotProduct(planeVecs[0], heTans[compID[0]]);
		Float v1x = QVector3D::dotProduct(planeVecs[1], heDirs[compID[0]]);
		Float v1y = QVector3D::dotProduct(planeVecs[1], heTans[compID[0]]);
		Float v2x = QVector3D::dotProduct(planeVecs[2], heDirs[compID[0]]);
		Float v2y = QVector3D::dotProduct(planeVecs[2], heTans[compID[0]]);
		Float v3x = QVector3D::dotProduct(planeVecs[3], heDirs[compID[0]]);
		Float v3y = QVector3D::dotProduct(planeVecs[3], heTans[compID[0]]);
		Float ab[4]{
			-v1y*edgeLen / (v0y * v1x - v0x * v1y),
			 v0y*edgeLen / (v0y * v1x - v0x * v1y),
			-v3y*edgeLen / (v2y * v3x - v2x * v3y),
			 v2y*edgeLen / (v2y * v3x - v2x * v3y),
		};
		
		planeVecs[1] *= -QVector3D::dotProduct(planeVecs[0], heTans[compID[0]])
			/ QVector3D::dotProduct(planeVecs[1], heTans[compID[0]]);
		planeVecs[3] *= -QVector3D::dotProduct(planeVecs[2], heTans[compID[0]])
			/ QVector3D::dotProduct(planeVecs[3], heTans[compID[0]]);


		float vecLen[2]{
			(planeVecs[0] + planeVecs[1]).length(),
			(planeVecs[2] + planeVecs[3]).length()
		};
		float scale[2]{
			edgeLen / (planeVecs[0] + planeVecs[1]).length(),
			edgeLen / (planeVecs[2] + planeVecs[3]).length()
		};
#if 0
		if (vecLen[0] < 0.01f && vecLen[0] > -0.01f)
		{
			cout << "Null Vec: " << planeVecs[0] << ", " << planeVecs[1] << endl;
		}
		if (isConcaveEdge[compID[0]])
		{
			cout << "he: " << compID[0] << " is concave\n";
			cout << "v0*dir < 0: " << (QVector3D::dotProduct(planeVecs[0], heDirs[compID[0]]) < 0) << endl;
			cout << "v1*dir < 1: " << (QVector3D::dotProduct(planeVecs[1], heDirs[compID[0]]) < 0) << endl;
			cout << "v1*dir < 2: " << (QVector3D::dotProduct(planeVecs[2], heDirs[compID[0]]) > 0) << endl;
			cout << "v1*dir < 3: " << (QVector3D::dotProduct(planeVecs[3], heDirs[compID[0]]) > 0) << endl;
			cout << "scale(" <<  scale[0] << ", " << scale[1] << ")\n";
		}
#endif
		
		planeVecs[0] *= scale[0];
		planeVecs[1] *= scale[0];
		planeVecs[2] *= scale[1];
		//planeVecs[3] *= scale[1];

		// padded index for verts and HEs
		auto paddingIdx = outputOffset * edgeCount;
		// Update vertex position

		// Edge Length
		verts[paddingIdx].pos = patchUniform
			? heMid[compID[0]] + heDirs[compID[0]] * patchScale * 0.5f
			: heMid[compID[0]] + heDirs[compID[0]] * edgeLen * 0.5f;

		verts[paddingIdx + 1].pos = verts[paddingIdx].pos + planeVecs[0];
		verts[paddingIdx + 2].pos = verts[paddingIdx + 1].pos + planeVecs[1];
		verts[paddingIdx + 3].pos = verts[paddingIdx + 2].pos + planeVecs[2];
		
		// Construct edges
		for (int i = 0; i < edgeCount ; i++)
		{
			constructHEPair(&verts[paddingIdx + i], &hes[paddingIdx + i]);
		}
		
		//construct edge loop
		constructFace(&hes[paddingIdx], edgeCount, &faces[outputOffset]);

		//for testing bridger

		outputOffset++;
		//normID[0] = heNorms[compID];
	}
	
	mesh_t* newMesh = new HDS_Mesh(verts, hes, faces);

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

HDS_Mesh * MeshNeoWeaver::createCrossWeaving(const mesh_t* ref_mesh,
                                             const confMap &conf)
{
#if 0
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
    vector<QVector3D> fNorms(refFaceCount);
    vector<QVector3D> heMid(refHeCount);
    vector<QVector3D> heDirs(refHeCount);
    vector<float> heDirLens(refHeCount);
    vector<QVector3D> heNorms(refHeCount);
    vector<QVector3D> heTans(refHeCount);
    vector<bool> isConcaveEdge(refHeCount, false);

    // Number of edges in each new patch
    int edgeCount = 4;

    for (int i = 0; i < refFaceCount; i++)
    {
        fNorms[i] = ref_mesh->faceNormal(i);
    }

    vector<bool> visitedHE(refHeCount, false);
    for (hdsid_t i = 0; i < refHeCount; i++)
    {
        auto he = &ref_hes[i];
        if (visitedHE[i]) continue;
        //if (he->flip_offset < 0) continue;
        auto hef = he->flip();
        hdsid_t hefIdx = hef->index;
        // calc dir
        heMid[i] = heMid[hefIdx] = (ref_verts[hef->vid].pos
            + ref_verts[he->vid].pos) * 0.5f;
        heDirs[i] = (ref_verts[hef->vid].pos
            - ref_verts[he->vid].pos);
        heDirs[hefIdx] = -heDirs[i];
        if (QVector3D::dotProduct(QVector3D::crossProduct(
            fNorms[he->fid], fNorms[hef->fid]), heDirs[i]) > 0)
        {
            heNorms[i] = heNorms[hefIdx]
                = (fNorms[he->fid] + fNorms[hef->fid]).normalized();
            heTans[i] = QVector3D::crossProduct(
                heNorms[i], heDirs[i]).normalized();
            heTans[hefIdx] = -heTans[i];
        }
        else
        {
            heNorms[i]
                = (fNorms[he->fid] - fNorms[hef->fid]).normalized();
            heNorms[hefIdx] = -heNorms[i];

            heTans[i] = heTans[hefIdx] = QVector3D::crossProduct(
                heNorms[i], heDirs[i]).normalized();
        }

        heDirLens[i] = heDirLens[hefIdx] = heDirs[i].length();
        heDirs[i] /= heDirLens[i];
        heDirs[hefIdx] /= heDirLens[i];

        visitedHE[i] = visitedHE[hefIdx] = true;
    }
    vector<vert_t> verts(refHeCount << 1);
    vector<he_t> hes(refHeCount << 1);
    vector<face_t> faces(refEdgeCount);

    hdsid_t outputOffset = 0;
    for (auto &he : ref_hes)
    {
        if (he.flip_offset < 0) continue;

        auto hef = he.flip();
        /*hdsid_t compID[5]{
            heCompMap.at(he.index),
            heCompMap.at(he.next()->index),
            heCompMap.at(he.prev()->index),
            heCompMap.at(hef->next()->index),
            heCompMap.at(hef->prev()->index)
        };*/
        QVector3D planeVecs[4]{
            QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[1]]),
            QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[2]]),
            QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[3]]),
            QVector3D::crossProduct(heNorms[compID[0]], heNorms[compID[4]]),
        };

        // Special Cases: normals are parallel to each other
        auto isZeroVec = [](const QVector3D &vec) {
            float threshold = 0.01f;
            return vec.lengthSquared() < sqr(threshold);
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

        float edgeLen = patchUniform ? patchScale : heDirLens[compID[0]] * patchScale;

        Float v0x = QVector3D::dotProduct(planeVecs[0], heDirs[compID[0]]);
        Float v0y = QVector3D::dotProduct(planeVecs[0], heTans[compID[0]]);
        Float v1x = QVector3D::dotProduct(planeVecs[1], heDirs[compID[0]]);
        Float v1y = QVector3D::dotProduct(planeVecs[1], heTans[compID[0]]);
        Float v2x = QVector3D::dotProduct(planeVecs[2], heDirs[compID[0]]);
        Float v2y = QVector3D::dotProduct(planeVecs[2], heTans[compID[0]]);
        Float v3x = QVector3D::dotProduct(planeVecs[3], heDirs[compID[0]]);
        Float v3y = QVector3D::dotProduct(planeVecs[3], heTans[compID[0]]);
        Float ab[4]{
            -v1y*edgeLen / (v0y * v1x - v0x * v1y),
            v0y*edgeLen / (v0y * v1x - v0x * v1y),
            -v3y*edgeLen / (v2y * v3x - v2x * v3y),
            v2y*edgeLen / (v2y * v3x - v2x * v3y),
        };

        planeVecs[1] *= -QVector3D::dotProduct(planeVecs[0], heTans[compID[0]])
            / QVector3D::dotProduct(planeVecs[1], heTans[compID[0]]);
        planeVecs[3] *= -QVector3D::dotProduct(planeVecs[2], heTans[compID[0]])
            / QVector3D::dotProduct(planeVecs[3], heTans[compID[0]]);


        float vecLen[2]{
            (planeVecs[0] + planeVecs[1]).length(),
            (planeVecs[2] + planeVecs[3]).length()
        };
        float scale[2]{
            edgeLen / (planeVecs[0] + planeVecs[1]).length(),
            edgeLen / (planeVecs[2] + planeVecs[3]).length()
        };

        planeVecs[0] *= scale[0];
        planeVecs[1] *= scale[0];
        planeVecs[2] *= scale[1];
        //planeVecs[3] *= scale[1];

        // padded index for verts and HEs
        auto paddingIdx = outputOffset * edgeCount;
        // Update vertex position

        // Edge Length
        verts[paddingIdx].pos = patchUniform
            ? heMid[compID[0]] + heDirs[compID[0]] * patchScale * 0.5f
            : heMid[compID[0]] + heDirs[compID[0]] * edgeLen * 0.5f;

        verts[paddingIdx + 1].pos = verts[paddingIdx].pos + planeVecs[0];
        verts[paddingIdx + 2].pos = verts[paddingIdx + 1].pos + planeVecs[1];
        verts[paddingIdx + 3].pos = verts[paddingIdx + 2].pos + planeVecs[2];

        // Construct edges
        for (int i = 0; i < edgeCount; i++)
        {
            constructHEPair(&verts[paddingIdx + i], &hes[paddingIdx + i]);
        }

        //construct edge loop
        constructFace(&hes[paddingIdx], edgeCount, &faces[outputOffset]);

        //for testing bridger

        outputOffset++;
        //normID[0] = heNorms[compID];
    }

    mesh_t* newMesh = new HDS_Mesh(verts, hes, faces);

    unordered_set<hdsid_t> exposedHEs;
    for (auto &he : newMesh->halfedges())
    {
        if (!he.flip_offset)
            exposedHEs.insert(he.index);
    }
    fillNullFaces(newMesh->halfedges(), newMesh->faces(), exposedHEs);

    newMesh->updatePieceSet();

    return newMesh;

#endif
	return nullptr;
}

HDS_Mesh* MeshNeoWeaver::createClassicalWeaving(const mesh_t* ref_mesh,
                                                const confMap &conf)
{
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
	vector<vert_t> verts(refHeCount << 2);
	vector<he_t> hes(refHeCount << 2);
	vector<face_t> faces(refHeCount);

	vector<QVector3D> fNorms(refFaceCount);
	vector<QVector3D> fCenters(refFaceCount);
	// Edge local axis
	vector<QVector3D> heDirs(refEdgeCount);
	vector<QVector3D> heNorms(refEdgeCount);
	vector<QVector3D> heTans(refEdgeCount);
	vector<float> heDirLens(refEdgeCount);
	// Edge plane
	vector<QVector3D> hePlane(refEdgeCount * 5);
	//
	const float r1 = 1.0f;
	const float r2 = 0.5f;
	const float r3 = 0.8f;
	// key: original edge id, value: edge id in new mesh
	unordered_map<hdsid_t, hdsid_t> heCompMap;

	/************************************************************************/
	/* Caching face centers                                                 */
	/************************************************************************/
	// face center cache array
	for (int i = 0; i < refFaceCount; i++)
	{
		fNorms[i] = ref_mesh->faceNormal(i);
		fCenters[i] = ref_mesh->faceCenter(i);
	}

	for (hdsid_t i = 0, it = 0; i < refHeCount; i++)
	{
		auto he = &ref_hes[i];
		if (he->flip_offset < 0) continue;
		auto hef = he->flip();
		// calc dir
		heDirs[it] = (ref_verts[hef->vid].pos
			- ref_verts[he->vid].pos);
		heNorms[it] = (fNorms[he->fid]
			+ fNorms[hef->fid]).normalized();
		heTans[it] = QVector3D::crossProduct(
			heNorms[it], heDirs[it]).normalized();
		heNorms[it] = QVector3D::crossProduct(heDirs[it], heTans[it]).normalized();
		// check if edge is concave
		heDirLens[it] = heDirs[it].length();
		heDirs[it] /= heDirLens[it];

		heCompMap[he->index] = heCompMap[hef->index] = it++;
	}

	QVector3D p0, p1, p2, p3, p4, p5, heC;
	QVector3D v1, v2, v3, v4, v5, v6, v7, v8;
	for (auto &he : ref_hes)
	{
		if (he.flip_offset < 0) continue;
		hdsid_t edgeIt = heCompMap.at(he.index);
		auto hef = he.flip();
		p0 = ref_verts[he.vid].pos;
		p1 = ref_verts[hef->vid].pos;
		QVector3D fcV = fCenters[he.fid] - p0;
		QVector3D fcV_f = fCenters[hef->fid] - p0;
		float x2 = QVector3D::dotProduct(fcV, heDirs[edgeIt]);
		float y2 = QVector3D::dotProduct(fcV, heTans[edgeIt]);
		float x3 = QVector3D::dotProduct(fcV_f, heDirs[edgeIt]);
		float y3 = QVector3D::dotProduct(fcV_f, heTans[edgeIt]);

		p2 = p0 + x2 * heDirs[edgeIt] + y2 * heTans[edgeIt];
		p3 = p0 + x3 * heDirs[edgeIt] + y3 * heTans[edgeIt];
		heC = Utils::Lerp(p2, p3, y2 / (y2 - y3));
		p4 = Utils::Lerp(heC, p2, r1);
		p5 = Utils::Lerp(heC, p3, r1);

		v1 = Utils::Lerp(p0, p4, r2);
		v4 = Utils::Lerp(p0, p4, r3);
		v5 = Utils::Lerp(p0, p5, r2);
		v6 = Utils::Lerp(p0, p5, r3);
		v3 = Utils::Lerp(p1, p5, r2);
		v2 = Utils::Lerp(p1, p5, r3);
		v7 = Utils::Lerp(p1, p4, r2);
		v8 = Utils::Lerp(p1, p4, r3);

		vert_t* curVerts = verts.data() + edgeIt * 8;
		he_t* curHEs = hes.data() + edgeIt * 8;
		face_t* curFaces = faces.data() + edgeIt * 2;
		constructHERing(curVerts, curHEs, 8);
		constructFace(curHEs, 4, curFaces);
		constructFace(curHEs + 4, 4, curFaces + 1);
		curVerts->pos = v1;
		(curVerts + 1)->pos = v2;
		(curVerts + 2)->pos = v3;
		(curVerts + 3)->pos = v4;
		(curVerts + 4)->pos = v5;
		(curVerts + 5)->pos = v6;
		(curVerts + 6)->pos = v7;
		(curVerts + 7)->pos = v8;
	}

	unordered_set<hdsid_t> exposedHEs;
	for (hdsid_t i = 0; i < hes.size(); i++)
	{
		exposedHEs.insert(i);
	}
	fillNullFaces(hes, faces, exposedHEs);
	mesh_t* newMesh = new HDS_Mesh(verts, hes, faces);

	newMesh->updatePieceSet();

	return newMesh;
}

HDS_Mesh* MeshNeoWeaver::createConicalWeaving(const mesh_t* ref_mesh,
                                              const confMap &conf)
{

	// scaling 
	const Float patchScale = conf.at("patchScale");
	const bool patchUniform = (conf.at("patchUniform") == 1.0f);
    const Float layerOffset = conf.at("layerOffset");
    const Float patchStripScale = conf.at("patchStripScale"); // 0.25 by default
	const uint32_t patchSeg = 2;// static_cast<uint32_t>(conf.at("patchSeg"));
    const uint32_t patchCurvedSample = 3;
	auto &ref_verts = ref_mesh->verts();
	auto &ref_hes = ref_mesh->halfedges();
	auto &ref_faces = ref_mesh->faces();
	size_t refHeCount = ref_hes.size();
	size_t refEdgeCount = refHeCount >> 1;
	size_t refFaceCount = ref_faces.size();

	mesh_t::resetIndex();
	vector<vert_t> verts;
	vector<he_t> hes;
	vector<face_t> faces;

#ifdef DOO_SABIN
	//evaluate doo-sabin subd
	vector<QVector3D> heCorners(refHeCount*2);
	for (int i = 0; i < refFaceCount; i++) {
		//loop each face and evaluate new positions
		QVector3D fc = ref_mesh->faceCenter(i);
		auto curHE = ref_mesh->heFromFace(i);
		QVector3D prevEc = ref_mesh->edgeCenter(curHE->prev()->index);
		do 
		{
			QVector3D curEc = ref_mesh->edgeCenter(curHE->index);
			heCorners[curHE->index<<1] = heCorners[(curHE->prev()->index<<1)+1]
				= (fc + prevEc + curEc + ref_mesh->vertFromHe(curHE->index)->pos) / 4;
			prevEc = curEc;
			curHE = curHE->next();
		} while (curHE != ref_mesh->heFromFace(i));
	}
#endif
	//calculate new edge normal and center
	
	vector<QVector3D> fNorms(refFaceCount);
	//vector<QVector3D> fCenters(refFaceCount);
	// Edge local axis
	// half length of he directions
	vector<QVector3D> heDirs(refHeCount);
	vector<QVector3D> heCenters(refHeCount);
	vector<QVector3D> heNorms(refHeCount);
	vector<QVector3D> heTans(refHeCount);
	vector<QVector3D> heCross(refHeCount * 2);
	vector<float>     heDirLens(refHeCount);
    vector<uint8_t>   heCurvatureType(refHeCount, 0);// 1 for convex, 0: planar, 2: concave
    vector<bool>      heCornerConsistent(refHeCount, false);//edge at corner has same type

    // Cache out face normals for Edge Normals(thickness)
    for (int i = 0; i < refFaceCount; i++) fNorms[i] = ref_mesh->faceNormal(i);
    
    // Cache out Edge Directions and Edge Centers
	for (hdsid_t i = 0; i < refHeCount; i++)
	{
		const he_t* he = &ref_hes[i];
		if (he->flip_offset < 0) continue;
		const he_t* hef = he->flip();
		hdsid_t hefId = hef->index;
#ifdef DOO_SABIN
		//get center point, 
		heCenters[i] = heCenters[hefId]
			= (heCorners[i<<1] + heCorners[(i<<1)+1]
				+ heCorners[hefId<<1] + heCorners[(hefId<<1)+1]) / 4;
		//calculate normal of v1, v2, cp and normal of v3, v4, cp, 
		QVector3D n1 = QVector3D::normal(heCenters[i], heCorners[(i<<1)+1], heCorners[i<<1]);
		QVector3D n2 = QVector3D::normal(heCenters[i], heCorners[(hefId<<1)+1], heCorners[hefId<<1]);

		//and get the average normal
		heNorms[i] = heNorms[hefId] = (n1 + n2).normalized();
		
#else
		heNorms[i] = heNorms[hefId]
			= (fNorms[he->fid] + fNorms[hef->fid]).normalized();
		heCenters[i] = heCenters[hefId] = ref_mesh->edgeCenter(i);
		heDirs[i] = ref_mesh->edgeVector(i) * 0.5f;// half length of he
		heDirs[hefId] = -heDirs[i];
#endif
	}

#ifdef DOO_SABIN
	//project heCorners to this plane
	for (hdsid_t i = 0; i < refHeCount*2; i++)
	{
		heCorners[i] = Utils::LinePlaneIntersect(
			heCorners[i], heCorners[i] + heNorms[i >> 1],
			heCenters[i >> 1], heNorms[i >> 1]);
	}
#endif

#ifdef DOO_SABIN

	//////////////////////
	//get approximate heCross
	for (hdsid_t i = 0; i < refHeCount; i++)
	{
		const he_t* he = &ref_hes[i];
		const he_t* hef = he->flip();
		hdsid_t hefId = hef->index;
		heDirs[i] = (heCorners[(i << 1) + 1] - heCorners[i << 1])*0.5f;

		// 0 (planar):  cross product of face normals is zero
		// 1 (convex):  cross product of face normals is in the same dir with edge dir
		// 2 (concave): cross product of face normals is opposite to edge dir
		QVector3D crossVec = QVector3D::crossProduct(fNorms[he->fid], fNorms[hef->fid]);
		heCurvatureType[i] = crossVec.isNull() ? 0
			: QVector3D::dotProduct(crossVec, heDirs[i]) > 0 ? 1 : 2;
	}
	// update corner flag
	// true:  both edge are convex or concave
	// false: edges has different flag, or one of them is planar
	for (auto &he : ref_hes)
	{
		heCornerConsistent[he.index] = heCurvatureType[he.index]
			& heCurvatureType[he.prev()->index];
	}
	// Cache out Edge Cross Vectors for generating patches on edge.
	// If corner has same flag on each edge, aka flag consistent,
	// use cross product of edge normals to get corner vectors;
	// Otherwise, use average of edge vectors
	for (auto &he : ref_hes)
	{
		heCross[he.index << 1] = heCross[(he.prev()->index << 1) + 1]
			= heCornerConsistent[he.index]
			? QVector3D::crossProduct(heNorms[he.prev()->index],
				heNorms[he.index])
			: heDirs[he.index] - heDirs[he.prev()->index];
	}

	//find new edge endpoint
	for (hdsid_t i = 0; i < refHeCount; i++)
	{
		const he_t* he = &ref_hes[i];
		const he_t* hef = he->flip();
		hdsid_t hefId = hef->index;
		//get intersecting point of two heCross
		QVector3D vs;
		Utils::LineLineIntersect(
			heCorners[i<<1],heCorners[i<<1]+heCross[i<<1],
			heCorners[(hefId<<1)+1], heCorners[(hefId << 1) + 1]+heCross[(hefId<<1)+1], 
			&vs);
		heCorners[i << 1] = heCorners[(hefId << 1) + 1] = vs;
	}
	for (hdsid_t i = 0; i < refHeCount; i++)
	{
		heDirs[i] = (heCorners[(i << 1) + 1] - heCorners[i << 1])*0.5f;
	}
#endif // DOO_SABIN

	for (hdsid_t i = 0; i < refHeCount; i++)
	{
		const he_t* he = &ref_hes[i];
		const he_t* hef = he->flip();
		hdsid_t hefId = hef->index;

		heDirLens[i] = heDirs[i].length();
		heTans[i] = QVector3D::crossProduct(heNorms[i], heDirs[i]).normalized();

        // 0 (planar):  cross product of face normals is zero
        // 1 (convex):  cross product of face normals is in the same dir with edge dir
        // 2 (concave): cross product of face normals is opposite to edge dir
        QVector3D crossVec = QVector3D::crossProduct(fNorms[he->fid], fNorms[hef->fid]);
        heCurvatureType[i] = crossVec.isNull() ? 0
                           : QVector3D::dotProduct(crossVec, heDirs[i]) > 0 ? 1 : 2;
    }
    // update corner flag
    // true:  both edge are convex or concave
    // false: edges has different flag, or one of them is planar
	for (auto &he : ref_hes)
	{
        heCornerConsistent[he.index] = heCurvatureType[he.index]
                                     & heCurvatureType[he.prev()->index];
	}
    // Cache out Edge Cross Vectors for generating patches on edge.
    // If corner has same flag on each edge, aka flag consistent,
    // use cross product of edge normals to get corner vectors;
    // Otherwise, use average of edge vectors
	for (auto &he : ref_hes)
	{
        heCross[he.index << 1] = heCross[(he.prev()->index << 1) + 1]
                               = heCornerConsistent[he.index]
                               ? QVector3D::crossProduct(heNorms[he.prev()->index],
                                                         heNorms[he.index])
                               : heDirs[he.index] - heDirs[he.prev()->index];
	}


    // Solve linear equation to scale he cross vectors
    // to make the sum of current and next cross vectors match edge length
    for (int i = 0; i < refHeCount; i++)
    {
        hdsid_t nextHeID = ref_hes[i].next()->index;
        QVector3D &v1 = heCross[i << 1];
        QVector3D &v2 = heCross[(i << 1) + 1];
        float v1x = QVector3D::dotProduct(v1, heTans[i]);
        float v2x = QVector3D::dotProduct(v2, heTans[i]);
        v2 *= v1x / v2x;
        QVector3D vy = v1 - v2;
        const QVector3D &dir = heDirs[i];
        int maxExtAxis = maxExtent(dir);
        float scalor = dir[maxExtAxis] / vy[maxExtAxis] * 2;
        v1 *= scalor;
        v2 *= scalor;
    }
    // Check cross vector length at the same corner
    // Use the shorter vector to avoid intersection at face center
    for (int i = 0; i < refHeCount; i++)
    {
        QVector3D &v1 = heCross[i << 1];
        QVector3D &v2 = heCross[(ref_hes[i].prev()->index << 1) + 1];

        int maxExtAxis = maxExtent(v1);
        if (abs(v1[maxExtAxis]) < abs(v2[maxExtAxis])) v2 = v1;
        else                                           v1 = v2;
    }
    // Cache out four points for each edge
    // vector<QVector3D> heToPatchPos(refHeCount * 4);
    vector<QVector3D> heToPatchPos(refHeCount * 4);
	vector<QVector3D> heToPatchVec(refHeCount * 2);
	for (int i = 0; i < refHeCount; i++)
	{
		const he_t* he = &ref_hes[i];
		hdsid_t he_flip_idx = he->flip()->index;
		hdsid_t he_prev_Idx = he->prev()->index;
		hdsid_t he_next_Idx = he->next()->index;
		hdsid_t he_fnext_Idx = he->flip()->next()->index;
		hdsid_t he_fprev_Idx = he->flip()->prev()->index;

		//			EDGE PATCH:						||  EDGE:
		//			   v2	pc  v3					||	
		//		vs ------------------ vs+heCross	||	vs
		//		   |	|		|	|				||	 \
		//	   v0h ------------------ v1			||	   \
		//		   |	|		|	|				||	    \
		//		   ------------------ pc_next		||		  \
		//		   |	|		|	|				||		   \
		//	   v1h ------------------ v0			||	         \
		//		   |	|		|	|				||		      \
		//ve+heCro ------------------ ve			||			    ve
		//				v3h		v2h					||

		//get centerVec
		QVector3D cVec = heCenters[he_prev_Idx] - heCenters[he_fprev_Idx];
		QVector3D cVec_next = heCenters[he_next_Idx] - heCenters[he_fnext_Idx];
		//project to edge plane
		QVector3D cVec_he = 
			heDirs[i].normalized() * QVector3D::dotProduct(heDirs[i].normalized(), cVec)
			+ heTans[i] * QVector3D::dotProduct(heTans[i], cVec);
		QVector3D cVec_he_next = 
			heDirs[i].normalized() * QVector3D::dotProduct(heDirs[i].normalized(), cVec_next)
			+ heTans[i] * QVector3D::dotProduct(heTans[i], cVec_next);
		//get centerLine which passes the edge center
		//find intersection with heCrosses
		QVector3D pc, pc_next;
#ifdef DOO_SABIN
		QVector3D vs = heCorners[i<<1];
		QVector3D ve = heCorners[(i<<1)+1];
#else
		QVector3D vs = ref_mesh->vertFromHe(i)->pos;
		QVector3D ve = ref_mesh->vertFromHe(he_next_Idx)->pos;
#endif
		Utils::LineLineIntersect(
			heCenters[i], heCenters[i] + cVec_he, 
			vs, vs + heCross[i<<1], &pc);
		Utils::LineLineIntersect(
			heCenters[i], heCenters[i] + cVec_he_next,
			ve, ve + heCross[he_next_Idx<<1], 
			&pc_next);
		//scale the strip based on centerLine
		heToPatchPos[i * 4 + 2] = Utils::Lerp(pc, vs, patchScale);
		heToPatchPos[i * 4 + 3] = Utils::Lerp(pc, vs + heCross[i << 1], patchScale);
		heToPatchPos[i * 4 + 0] = Utils::Lerp(pc_next, ve, patchScale);
		heToPatchPos[i * 4 + 1] = Utils::Lerp(pc_next, ve + heCross[he_next_Idx << 1], patchScale);
		//record the centerLines
		heToPatchVec[i * 2 + 0] = cVec_he_next;
		heToPatchVec[i * 2 + 1] = cVec_he;
	}
	//scale the strip inwards while keeping the vectors parallel


	for (int i = 0; i < refHeCount; i++)
	{
		const he_t* he = &ref_hes[i];
		hdsid_t he_flip_idx = he->flip()->index;
		if (he->flip_offset > 0) continue;
		QVector3D v0, v0h, v1, v1h, v2, v2h, v3, v3h;
		v0 = heToPatchPos[i * 4 + 0]; v0h = heToPatchPos[he_flip_idx * 4 + 0];
		v1 = heToPatchPos[i * 4 + 1]; v1h = heToPatchPos[he_flip_idx * 4 + 1];
		v2 = heToPatchPos[i * 4 + 2]; v2h = heToPatchPos[he_flip_idx * 4 + 2];
		v3 = heToPatchPos[i * 4 + 3]; v3h = heToPatchPos[he_flip_idx * 4 + 3];
		//scale with edge center as pivot

		//find intersection of two crossing pieces
		//scale the other ends accordingly
		QVector3D v2_v3h, v3_v2h, v1_v0h, v0_v1h;
		Utils::LineLineIntersect(
			heCenters[i], heCenters[i] + heToPatchVec[i * 2 + 0],
			v2, v3h, &v2_v3h);
		Utils::LineLineIntersect(
			heCenters[i], heCenters[i] + heToPatchVec[i * 2 + 0],
			v3, v2h, &v3_v2h);
		Utils::LineLineIntersect(
			heCenters[i], heCenters[i] + heToPatchVec[i * 2 + 1],
			v1, v0h, &v1_v0h);
		Utils::LineLineIntersect(
			heCenters[i], heCenters[i] + heToPatchVec[i * 2 + 1],
			v0, v1h, &v0_v1h);

		if ((v2-v2_v3h).length() > (v3 - v3_v2h).length())
		{
#ifdef PATCH_LENGTH
			heToPatchPos[i * 4 + 3] = Utils::Lerp((v3 + v2h) / 2, v3, patchLength);
			heToPatchPos[he_flip_idx * 4 + 2] = Utils::Lerp((v3 + v2h) / 2, v2h, patchLength);
#else
			Utils::LineLineIntersect(v1, v0h, v3, v2h, &heToPatchPos[i * 4 + 3]);
			Utils::LineLineIntersect(v0, v1h, v2, v3h, &heToPatchPos[he_flip_idx * 4 + 3]);

#endif
			Utils::LineLineIntersect(
				heToPatchPos[i * 4 + 3], heToPatchPos[i * 4 + 3] + (v2 - v3),
				v2, v3h, &heToPatchPos[i * 4 + 2]);
			Utils::LineLineIntersect(
				heToPatchPos[he_flip_idx * 4 + 3], heToPatchPos[he_flip_idx * 4 + 3] + (v2h - v3h),
				v3, v2h, &heToPatchPos[he_flip_idx * 4 + 2]);
		}
		else {
#ifdef PATCH_LENGTH
			heToPatchPos[i * 4 + 2] = Utils::Lerp((v2 + v3h) / 2, v2, patchLength);
			heToPatchPos[he_flip_idx * 4 + 3] = Utils::Lerp((v2 + v3h) / 2, v3h, patchLength);
#else
			Utils::LineLineIntersect(v1, v0h, v2, v3h, &heToPatchPos[i * 4 + 2]);
			Utils::LineLineIntersect(v0, v1h, v3, v2h, &heToPatchPos[he_flip_idx * 4 + 2]);
#endif
			Utils::LineLineIntersect(
				heToPatchPos[i * 4 + 2], heToPatchPos[i * 4 + 2] + (v3 - v2),
				v3, v2h, &heToPatchPos[i * 4 + 3]);
			Utils::LineLineIntersect(
				heToPatchPos[he_flip_idx * 4 + 2], heToPatchPos[he_flip_idx * 4 + 2] + (v3h - v2h),
				v2, v3h, &heToPatchPos[he_flip_idx * 4 + 3]);
		}

		if ((v0 - v0_v1h).length() > (v1 - v1_v0h).length())
		{
#ifdef PATCH_LENGTH
			heToPatchPos[i * 4 + 1] = Utils::Lerp((v1 + v0h) / 2, v1, patchLength);
			heToPatchPos[he_flip_idx * 4 + 0] = Utils::Lerp((v1 + v0h) / 2, v0h, patchLength);
#else
			Utils::LineLineIntersect(v3, v2h, v1, v0h, &heToPatchPos[i * 4 + 1]);
			Utils::LineLineIntersect(v2, v3h, v0, v1h, &heToPatchPos[he_flip_idx * 4 + 1]);
#endif
			Utils::LineLineIntersect(
				heToPatchPos[i * 4 + 1], heToPatchPos[i * 4 + 1] + (v0 - v1),
				v0, v1h, &heToPatchPos[i * 4 + 0]);
			Utils::LineLineIntersect(
				heToPatchPos[he_flip_idx * 4 + 1], heToPatchPos[he_flip_idx * 4 + 1] + (v0h - v1h),
				v1, v0h, &heToPatchPos[he_flip_idx * 4 + 0]);
			
		}
		else {
#ifdef PATCH_LENGTH
			heToPatchPos[i * 4 + 0] = Utils::Lerp((v0 + v1h) / 2, v0, patchLength);
			heToPatchPos[he_flip_idx * 4 + 1] = Utils::Lerp((v0 + v1h) / 2, v1h, patchLength);
#else
			Utils::LineLineIntersect(v3, v2h, v0, v1h, &heToPatchPos[i * 4 + 0]);
			Utils::LineLineIntersect(v2, v3h, v1, v0h, &heToPatchPos[he_flip_idx * 4 + 0]);
#endif
			Utils::LineLineIntersect(
				heToPatchPos[i * 4 + 0], heToPatchPos[i * 4 + 0] + (v1 - v0),
				v1, v0h, &heToPatchPos[i * 4 + 1]);
			Utils::LineLineIntersect(
				heToPatchPos[he_flip_idx * 4 + 0], heToPatchPos[he_flip_idx * 4 + 0] + (v1h - v0h),
				v0, v1h, &heToPatchPos[he_flip_idx * 4 + 1]);
			
		}
	}
/*
    Float vecScaleShort = (0.5f - patchScale * 0.5f) * patchScale;
    Float vecScaleLong = (0.5f + patchScale * 0.5f) * patchScale;
    for (int i = 0; i < refHeCount; i++)
    {
        const he_t* he = &ref_hes[i];
        hdsid_t he_Idx = he->index;
        hdsid_t he_prev_Idx = he->prev()->index;
        QVector3D p0 = heCenters[he_Idx] - heDirs[he_Idx] * patchScale;
        QVector3D p1 = heCenters[he_prev_Idx] + heDirs[he_prev_Idx] * patchScale;
        QVector3D av = heCross[he->index << 1] * vecScaleShort;
        QVector3D bv = heCross[he->index << 1] * vecScaleLong;
        heToPatchPos[he_Idx * 4 + 2] = p0 + av;
        heToPatchPos[he_Idx * 4 + 3] = p0 + bv;
        heToPatchPos[he_prev_Idx * 4 + 1] = p1 + bv;
        heToPatchPos[he_prev_Idx * 4] = p1 + av;
        // v1 = p0+av;
        // v2 = p0+bv;
        // v4 = p1+bv;
        // v3 = p1+av;
        // connect v1-v2-v4-v3
    }
*/
    // Construct bridge patch
    const int sPatchMaxVertices = 256;
    const int sPatchFaceCount = 21;
    const int sPatchHeCount = sPatchFaceCount * 4;
    const int sPatchVertCount = sPatchFaceCount * 2 + 2;
    const int sPatchCount = refEdgeCount;
    faces.resize(sPatchCount * sPatchFaceCount);
    hes.resize(sPatchCount * sPatchHeCount);
    verts.resize(sPatchCount * sPatchVertCount);
    //mesh_t* newMesh = new HDS_Mesh(verts, hes, faces);
    for (int i = 0; i < refEdgeCount; i++)
    {
        int patchFaceOffset = i * sPatchFaceCount;
        int patchHEOffset = patchFaceOffset * 4;
        int patchVertOffset = i * sPatchVertCount;
        auto curHE = hes.data() + patchHEOffset;
        auto curVert = verts.data() + patchVertOffset;
        auto curFace = faces.data() + patchFaceOffset;
        for (int j = 0; j < sPatchFaceCount; j++)
        {
            constructFace(curHE, 4, curFace);
            if (j != sPatchFaceCount - 1)
            {
                (curHE + 2)->flip_offset = 2;
                (curHE + 4)->flip_offset = -2;
            }
            constructHEPair(curVert, curHE++);
            constructHEPair(curVert + 1, curHE++);
            constructHEPair(curVert + 3, curHE++);
            constructHEPair(curVert + 2, curHE++);
            curFace++;
            curVert += 2;
        }
    }

    // start from one edge and move to next woven edge to update bridge up/down
    vector<bool> visitedHE(refHeCount, false);
    vector<bool> heOnTopFlag(refHeCount, false);
    vector<hdsid_t> heToPatch(refHeCount);
    hdsid_t curPatchID = 0;
    QVector3D patchPos[sPatchVertCount];// [sPatchMaxVertices];
    for (int i = 0; i < refHeCount; i++)
    {
        if (visitedHE[i]) continue;

        auto he = &ref_hes[i];
        auto curHE = he;

        do
        {
            // neighboring edge ids
            hdsid_t neiEdgeIDs[]{
                curHE->prev()->flip()->index,
                curHE->prev()->index,
                curHE->index,
                curHE->flip()->index,
                curHE->flip()->prev()->index,
                curHE->flip()->prev()->flip()->index
            };
			//get Edge id from HE id
			hdsid_t preEdge = neiEdgeIDs[0] < neiEdgeIDs[1] ? neiEdgeIDs[0] : neiEdgeIDs[1];
			hdsid_t curEdge = neiEdgeIDs[2] < neiEdgeIDs[3] ? neiEdgeIDs[2] : neiEdgeIDs[3];
			hdsid_t nxtEdge = neiEdgeIDs[4] < neiEdgeIDs[5] ? neiEdgeIDs[4] : neiEdgeIDs[5];

            // Update position and set refid
            auto patchVerts = &verts[curPatchID * sPatchVertCount];
			(patchVerts + 0)->setRefId(preEdge);
			(patchVerts + 11)->setRefId(preEdge);
            patchPos[0] = heToPatchPos[neiEdgeIDs[0] * 4];
            patchPos[1] = heToPatchPos[neiEdgeIDs[0] * 4 + 1];
            patchPos[10] = heToPatchPos[neiEdgeIDs[1] * 4 + 1];
            patchPos[11] = heToPatchPos[neiEdgeIDs[1] * 4];

			(patchVerts + 17)->setRefId(curEdge);
			(patchVerts + 26)->setRefId(curEdge);
            patchPos[16] = heToPatchPos[neiEdgeIDs[2] * 4 + 3] + heNorms[neiEdgeIDs[2]] * layerOffset;
            patchPos[17] = heToPatchPos[neiEdgeIDs[2] * 4 + 2] + heNorms[neiEdgeIDs[2]] * layerOffset;
            patchPos[26] = heToPatchPos[neiEdgeIDs[3] * 4 + 2] + heNorms[neiEdgeIDs[2]] * layerOffset;
            patchPos[27] = heToPatchPos[neiEdgeIDs[3] * 4 + 3] + heNorms[neiEdgeIDs[2]] * layerOffset;

			(patchVerts + 32)->setRefId(nxtEdge);
			(patchVerts + 43)->setRefId(nxtEdge);
            patchPos[32] = heToPatchPos[neiEdgeIDs[4] * 4] - heNorms[neiEdgeIDs[4]] * layerOffset;
            patchPos[33] = heToPatchPos[neiEdgeIDs[4] * 4 + 1] - heNorms[neiEdgeIDs[4]] * layerOffset;
            patchPos[42] = heToPatchPos[neiEdgeIDs[5] * 4 + 1] - heNorms[neiEdgeIDs[4]] * layerOffset;
            patchPos[43] = heToPatchPos[neiEdgeIDs[5] * 4] - heNorms[neiEdgeIDs[4]] * layerOffset;

            //////////////////////////////////////////////////////////////////////////
//#define LERP_BRIDGE
            // Linear interpolate positions for patch boundary
            auto lerpPatchPos = [](QVector3D* ptr, int srcBegin, int srcEnd, int seg) {
                Float increment = 2.0f / seg;
                Float wgt = increment;
                for (int targ = srcBegin + 2; targ < srcEnd; targ += 2, wgt += increment)
                {
                    ptr[targ] = Utils::Lerp(ptr[srcBegin], ptr[srcEnd], wgt);
                }
            };
			auto parallelPatchPos = [](QVector3D* ptr, int ref, int target, int srcBoundS, int srcBoundE) {
				Utils::LineLineIntersect(
					ptr[target], ptr[target] - (ptr[ref] - ptr[ref+1]),
					ptr[srcBoundS], ptr[srcBoundE], &ptr[target+1]);
			};
            // Calculate difference vectors from different sides on an edge
            // Generate triangle region in the center of the patch
            // Linear interpolate between triangle and cross vectors on each side
            //     (the lerp operation is handled outside the function below)
			
			auto edgePatchEval = [](QVector3D* ptr, int srcBegin, int srcEnd) {
				QVector3D vec1 = ptr[srcBegin + 1] - ptr[srcBegin];
				QVector3D vec2 = ptr[srcEnd + 1] - ptr[srcEnd];
				float len1 = vec1.length();
				float len2 = vec2.length();
				float lenAvg = (len1 + len2) * 0.5f;
				vec1 *= lenAvg / len1;
				vec2 *= lenAvg / len2;
				int seg = (srcEnd - srcBegin - 2) >> 1;
				Float increment = 2.0 / seg;
				Float wgt = increment;
				if (QVector3D::dotProduct(vec2 - vec1, ptr[srcEnd] - ptr[srcBegin]) > 0)
				{
					ptr[srcBegin + seg] = ptr[srcEnd - seg]
						= Utils::Lerp(ptr[srcBegin], ptr[srcEnd], 0.5f);

					ptr[srcBegin + seg + 1] = ptr[srcBegin + seg] + vec1;
					ptr[srcEnd - seg + 1] = ptr[srcEnd - seg] + vec2;
				}
				else
				{
					ptr[srcBegin + seg + 1] = ptr[srcEnd - seg + 1]
						= Utils::Lerp(ptr[srcBegin + 1],
							ptr[srcEnd + 1],
							0.5f);

					ptr[srcBegin + seg] = ptr[srcBegin + seg + 1] - vec1;
					ptr[srcEnd - seg] = ptr[srcEnd - seg + 1] - vec2;
				}
				for (int i = 2; i < seg; i += 2, wgt += increment)
				{
					ptr[srcBegin + i] = Utils::Lerp(ptr[srcBegin], ptr[srcBegin + seg], wgt);
					ptr[srcEnd - i] = Utils::Lerp(ptr[srcEnd], ptr[srcEnd - seg], wgt);
					ptr[srcBegin + i + 1] = Utils::Lerp(ptr[srcBegin + 1], ptr[srcBegin + seg + 1], wgt);
					ptr[srcEnd - i + 1] = Utils::Lerp(ptr[srcEnd + 1], ptr[srcEnd - seg + 1], wgt);
				}

			};

			//evaluate bridge control points
			auto bridgeEval = [](QVector3D* ptr, QVector3D heTBegin, int srcBegin, QVector3D heTEnd, int srcEnd, float patchStripScale) {
				//WARNING: intersection might not exist
				QVector3D mid0, mid1;

				Utils::LineLineIntersect(ptr[srcBegin], ptr[srcBegin] + heTBegin, ptr[srcEnd], ptr[srcEnd] + heTEnd, &mid0);
				Utils::LineLineIntersect(ptr[srcBegin + 1], ptr[srcBegin + 1] + heTBegin, ptr[srcEnd + 1], ptr[srcEnd + 1] + heTEnd, &mid1);
				//if the intersection is within any edge patches
				if (QVector3D::dotProduct((mid0 - ptr[srcBegin]), (ptr[srcBegin] - ptr[srcBegin - 10])) < 0
					|| QVector3D::dotProduct((mid0 - ptr[srcEnd]), (ptr[srcEnd] - ptr[srcEnd + 10])) < 0)
				{
					ptr[srcBegin + 2] = Utils::Lerp((ptr[srcBegin] + ptr[srcEnd]) / 2, ptr[srcBegin], patchStripScale);
					ptr[srcBegin + 3] = Utils::Lerp((ptr[srcBegin + 1] + ptr[srcEnd + 1]) / 2, ptr[srcBegin + 1], patchStripScale);
					ptr[srcBegin + 4] = Utils::Lerp((ptr[srcBegin] + ptr[srcEnd]) / 2, ptr[srcEnd], patchStripScale);
					ptr[srcBegin + 5] = Utils::Lerp((ptr[srcBegin + 1] + ptr[srcEnd + 1]) / 2, ptr[srcEnd + 1], patchStripScale);
				}
				else {
					ptr[srcBegin + 2] = Utils::Lerp(mid0, ptr[srcBegin], patchStripScale);
					ptr[srcBegin + 3] = Utils::Lerp(mid1, ptr[srcBegin + 1], patchStripScale);
					ptr[srcBegin + 4] = Utils::Lerp(mid0, ptr[srcEnd], patchStripScale);
					ptr[srcBegin + 5] = Utils::Lerp(mid1, ptr[srcEnd + 1], patchStripScale);
				}

			};

			//eval edge patch
#ifdef LERP_BRIDGE
			
            lerpPatchPos(patchPos, 0, 10, 10);
			//make outer edges parallel
			parallelPatchPos(patchPos, 0, 2, 1,11);
			parallelPatchPos(patchPos, 10, 8, 1,11);

            lerpPatchPos(patchPos, 3, 9, 6);			
#else
            edgePatchEval(patchPos, 0, 10);
#endif // LERP_BRIDGE
/*

            patchPos[12] = Utils::Lerp(patchPos[10], patchPos[16], patchStripScale);
            patchPos[13] = Utils::Lerp(patchPos[11], patchPos[17], patchStripScale);
            patchPos[14] = Utils::Lerp(patchPos[16], patchPos[10], patchStripScale);
            patchPos[15] = Utils::Lerp(patchPos[17], patchPos[11], patchStripScale);*/

#ifdef LERP_BRIDGE
            lerpPatchPos(patchPos, 16, 26, 10);
			parallelPatchPos(patchPos, 16, 18, 17,27);
			parallelPatchPos(patchPos, 26, 24, 17,27);
			
            lerpPatchPos(patchPos, 19, 25, 6);
#else
            edgePatchEval(patchPos, 16, 26);
#endif // LERP_BRIDGE
           /* patchPos[28] = Utils::Lerp(patchPos[26], patchPos[32], patchStripScale);
            patchPos[29] = Utils::Lerp(patchPos[27], patchPos[33], patchStripScale);
            patchPos[30] = Utils::Lerp(patchPos[32], patchPos[26], patchStripScale);
            patchPos[31] = Utils::Lerp(patchPos[33], patchPos[27], patchStripScale);*/

#ifdef LERP_BRIDGE
            lerpPatchPos(patchPos, 32, 42, 10);
			//make outer edges parallel
			parallelPatchPos(patchPos, 32, 34, 33, 43);
			parallelPatchPos(patchPos, 42, 40, 33, 43);
            lerpPatchPos(patchPos, 35, 41, 6);
#else
            edgePatchEval(patchPos, 32, 42);
#endif // LERP_BRIDGE

			//eval bridges
			bridgeEval(patchPos, 
				QVector3D::crossProduct(patchPos[11]-patchPos[10], heNorms[neiEdgeIDs[1]]), 10, 
				QVector3D::crossProduct(patchPos[17] - patchPos[16], heNorms[neiEdgeIDs[2]]), 16,
				patchStripScale);
			bridgeEval(patchPos, 
				QVector3D::crossProduct(patchPos[27] - patchPos[26], heNorms[neiEdgeIDs[3]]), 26, 
				QVector3D::crossProduct(patchPos[33] - patchPos[32], heNorms[neiEdgeIDs[4]]), 32, 
				patchStripScale);
            auto bezierPos = [](QVector3D* ptr) {
                *ptr = *(ptr - 2) * 0.25f + *ptr * 0.5f + *(ptr + 2) * 0.25f;
            };
            bezierPos(&patchPos[10]);
            bezierPos(&patchPos[11]);
            bezierPos(&patchPos[16]);
            bezierPos(&patchPos[17]);
            bezierPos(&patchPos[26]);
            bezierPos(&patchPos[27]);
            bezierPos(&patchPos[32]);
            bezierPos(&patchPos[33]);

            // Assign evaluated positions to patch
            for (int j = 0; j < sPatchVertCount; j++)
            {
                (patchVerts + j)->pos = patchPos[j];
            }
            // Update he-to-patch lookup table
            heToPatch[neiEdgeIDs[3]] = -curPatchID;
            heToPatch[neiEdgeIDs[2]] = curPatchID++;
            visitedHE[neiEdgeIDs[2]] = visitedHE[neiEdgeIDs[3]] = true;

            heOnTopFlag[neiEdgeIDs[1]] = true;
            // Move to he corresponding to next connected trip
            curHE = curHE->rotCCW()->next()->flip();
        } while (curHE != he);
    }
    
    // Finalize Mesh
	unordered_set<hdsid_t> exposedHEs;
	for (auto &he : hes)
	{
		if (he.flip_offset == 0) exposedHEs.insert(he.index);
	}
	fillNullFaces(hes, faces, exposedHEs);
    HDS_Mesh* ret = new HDS_Mesh(verts, hes, faces);
	//ret->updatePieceSet();

	return ret;
}

HDS_Mesh* MeshNeoWeaver::createBiTriWeaving(const mesh_t* ref_mesh,
                                            const confMap &conf)
{
    // scaling 
    const float patchScale = conf.at("patchScale");
    const bool patchUniform = (conf.at("patchUniform") == 1.0f);
    const Float layerOffset = conf.at("layerOffset");// 0.1 by default
    const Float patchStripScale = conf.at("patchStripScale"); // 0.25 by default
    const uint32_t patchSeg = 2;// static_cast<uint32_t>(conf.at("patchSeg"));
    //const uint32_t patchCurvedSample = 3;

    auto &ref_verts = ref_mesh->verts();
    auto &ref_hes = ref_mesh->halfedges();
    auto &ref_faces = ref_mesh->faces();
    size_t refHeCount = ref_hes.size();
    size_t refEdgeCount = refHeCount >> 1;
    size_t refFaceCount = ref_faces.size();

    mesh_t::resetIndex();
    vector<vert_t> verts;
    vector<he_t> hes;
    vector<face_t> faces;

    vector<QVector3D> fNorms(refFaceCount);
    //vector<QVector3D> fCenters(refFaceCount);
    // Edge local axis
    // half length of he directions
    vector<QVector3D> heDirs(refHeCount);
    vector<QVector3D> heCenters(refHeCount);
    vector<QVector3D> heNorms(refHeCount);
    vector<QVector3D> heTans(refHeCount);
    vector<QVector3D> heCross(refHeCount * 2);
    vector<float>     heDirLens(refHeCount);
    vector<uint8_t>   heCurvatureType(refHeCount, 0);// 1 for convex, 0: planar, 2: concave
    vector<bool>      heCornerConsistent(refHeCount, false);//edge at corner has same type
                                                            // Cache out face normals for Edge Normals(thickness)
    for (int i = 0; i < refFaceCount; i++) fNorms[i] = ref_mesh->faceNormal(i);

    // Cache out Edge properties (direction, center, normal, tangent, length)
    //           Edge Type(Convex or Concave)
    for (hdsid_t i = 0; i < refHeCount; i++)
    {
        const he_t* he = &ref_hes[i];
        if (he->flip_offset < 0) continue;
        const he_t* hef = he->flip();
        hdsid_t hefId = hef->index;
        heNorms[i] = heNorms[hefId]
            = (fNorms[he->fid] + fNorms[hef->fid]).normalized();
        heDirs[i] = ref_mesh->edgeVector(i) * 0.5f;// half length of he
        heDirLens[i] = heDirLens[hefId] = heDirs[i].length();
        heCenters[i] = heCenters[hefId] = ref_mesh->edgeCenter(i);
        heTans[i] = QVector3D::crossProduct(heNorms[i], heDirs[i]).normalized();
        heDirs[hefId] = -heDirs[i];
        heTans[hefId] = -heTans[i];

        // 0 (planar):  cross product of face normals is zero
        // 1 (convex):  cross product of face normals is in the same dir with edge dir
        // 2 (concave): cross product of face normals is opposite to edge dir
        QVector3D crossVec = QVector3D::crossProduct(fNorms[he->fid], fNorms[hef->fid]);
        heCurvatureType[i] = heCurvatureType[hefId]
                           = crossVec.isNull() ? 0
                           : QVector3D::dotProduct(crossVec, heDirs[i]) > 0 ? 1 : 2;
    }
    // update corner flag
    // true:  both edge are convex or concave
    // false: edges has different flag, or one of them is planar
    for (auto &he : ref_hes)
    {
        heCornerConsistent[he.index] = heCurvatureType[he.index]
            & heCurvatureType[he.prev()->index];
    }
    // Cache out Edge Cross Vectors for generating patches on edge.
    // If corner has same flag on each edge, aka flag consistent,
    // use cross product of edge normals to get corner vectors;
    // Otherwise, use average of edge vectors
    for (auto &he : ref_hes)
    {
        heCross[he.index << 1] = heCross[(he.prev()->index << 1) + 1]
            = heCornerConsistent[he.index]
            ? QVector3D::crossProduct(heNorms[he.prev()->index],
                                      heNorms[he.index])
            : heDirs[he.index] - heDirs[he.prev()->index];
    }
    // Scale edge cross product vectors to get max coverage
    for (int i = 0; i < refHeCount; i++)
    {
        hdsid_t nextHeID = ref_hes[i].next()->index;
        QVector3D &v1 = heCross[i << 1];
        QVector3D &v2 = heCross[(i << 1) + 1];
        float v1x = QVector3D::dotProduct(v1, heTans[i]);
        float v2x = QVector3D::dotProduct(v2, heTans[i]);
        v2 *= v1x / v2x;
        QVector3D vy = v1 - v2;
        const QVector3D &dir = heDirs[i];
        int maxExtAxis = maxExtent(dir);
        float scalor = dir[maxExtAxis] / vy[maxExtAxis] * 2;
        v1 *= scalor;
        v2 *= scalor;
    }
    // check vector length on each corner side to avoid intersection
    for (int i = 0; i < refHeCount; i++)
    {
        QVector3D &v1 = heCross[i << 1];
        QVector3D &v2 = heCross[(ref_hes[i].prev()->index << 1) + 1];

        int maxExtAxis = maxExtent(v1);
        if (abs(v1[maxExtAxis]) < abs(v2[maxExtAxis])) v2 = v1;
        else                                           v1 = v2;
    }
    // cache out four points for each half-edge
    vector<QVector3D> heToPatchPos(refHeCount * 4);
    Float vecScaleShort = (0.5f - patchScale * 0.5f) * patchScale;
    Float vecScaleLong = (0.5f + patchScale * 0.5f) * patchScale;
    for (int i = 0; i < refHeCount; i++)
    {
        const he_t* he = &ref_hes[i];
        hdsid_t he_Idx = he->index;
        hdsid_t he_prev_Idx = he->prev()->index;
        QVector3D p0 = heCenters[he_Idx] - heDirs[he_Idx] * patchScale;
        QVector3D p1 = heCenters[he_prev_Idx] + heDirs[he_prev_Idx] * patchScale;
        QVector3D av = heCross[he->index << 1] * vecScaleShort;
        QVector3D bv = heCross[he->index << 1] * vecScaleLong;
        heToPatchPos[he_Idx * 4 + 2] = p0 + av;
        heToPatchPos[he_Idx * 4 + 3] = p0 + bv;
        heToPatchPos[he_prev_Idx * 4 + 1] = p1 + bv;
        heToPatchPos[he_prev_Idx * 4] = p1 + av;
        // v1 = p0+av;
        // v2 = p0+bv;
        // v4 = p1+bv;
        // v3 = p1+av;
        // connect v1-v2-v4-v3
    }
    // Construct bridge patch
    const int sPatchCtrlVertices = 24;
    const int sPatchMaxVertices = 256;
    const int sPatchFaceCount = 21;
    const int sPatchHeCount = sPatchFaceCount * 4;
    const int sPatchVertCount = sPatchFaceCount * 2 + 2;
    const int sPatchCount = refEdgeCount;
    faces.resize(sPatchCount * sPatchFaceCount);
    hes.resize(sPatchCount * sPatchHeCount);
    verts.resize(sPatchCount * sPatchVertCount);

    for (int i = 0; i < refEdgeCount; i++)
    {
        int patchFaceOffset = i * sPatchFaceCount;
        int patchHEOffset = patchFaceOffset * 4;
        int patchVertOffset = i * sPatchVertCount;
        auto curHE = hes.data() + patchHEOffset;
        auto curVert = verts.data() + patchVertOffset;
        auto curFace = faces.data() + patchFaceOffset;
        for (int j = 0; j < sPatchFaceCount; j++)
        {
            constructFace(curHE, 4, curFace);
            if (j != sPatchFaceCount - 1)
            {
                (curHE + 2)->flip_offset = 2;
                (curHE + 4)->flip_offset = -2;
            }
            constructHEPair(curVert, curHE++);
            constructHEPair(curVert + 1, curHE++);
            constructHEPair(curVert + 3, curHE++);
            constructHEPair(curVert + 2, curHE++);
            curFace++;
            curVert += 2;
        }
    }

    // start from one edge and move to next woven edge to update bridge up/down
    vector<bool> visitedHE(refHeCount, false);
    vector<bool> heOnTopFlag(refHeCount, false);
    vector<hdsid_t> heToPatch(refHeCount);
    hdsid_t curPatchID = 0;
    QVector3D patchPos[sPatchVertCount];// [sPatchMaxVertices];
    for (int i = 0; i < refHeCount; i++)
    {
        if (visitedHE[i]) continue;

        const he_t* he = &ref_hes[i];
        const he_t* curHE = he;

        do
        {
            // neighboring edge ids
            hdsid_t neiEdgeIDs[]{
                curHE->prev()->flip()->index,
                curHE->prev()->index,
                curHE->index,
                curHE->flip()->index,
                curHE->flip()->prev()->index,
                curHE->flip()->prev()->flip()->index
            };
            // Update position
            auto patchVerts = &verts[curPatchID * sPatchVertCount];
            patchPos[0] = heToPatchPos[neiEdgeIDs[0] * 4];
            patchPos[1] = heToPatchPos[neiEdgeIDs[0] * 4 + 1];
            patchPos[10] = heToPatchPos[neiEdgeIDs[1] * 4 + 1];
            patchPos[11] = heToPatchPos[neiEdgeIDs[1] * 4];

            patchPos[16] = heToPatchPos[neiEdgeIDs[2] * 4 + 3] + heNorms[neiEdgeIDs[2]] * layerOffset;
            patchPos[17] = heToPatchPos[neiEdgeIDs[2] * 4 + 2] + heNorms[neiEdgeIDs[2]] * layerOffset;
            patchPos[26] = heToPatchPos[neiEdgeIDs[3] * 4 + 2] + heNorms[neiEdgeIDs[2]] * layerOffset;
            patchPos[27] = heToPatchPos[neiEdgeIDs[3] * 4 + 3] + heNorms[neiEdgeIDs[2]] * layerOffset;

            patchPos[32] = heToPatchPos[neiEdgeIDs[4] * 4] - heNorms[neiEdgeIDs[4]] * layerOffset;
            patchPos[33] = heToPatchPos[neiEdgeIDs[4] * 4 + 1] - heNorms[neiEdgeIDs[4]] * layerOffset;
            patchPos[42] = heToPatchPos[neiEdgeIDs[5] * 4 + 1] - heNorms[neiEdgeIDs[4]] * layerOffset;
            patchPos[43] = heToPatchPos[neiEdgeIDs[5] * 4] - heNorms[neiEdgeIDs[4]] * layerOffset;

            //////////////////////////////////////////////////////////////////////////
            //#define LERP_BRIDGE
            auto lerpPatchPos = [](QVector3D* ptr, int srcBegin, int srcEnd, int seg) {
                Float increment = 1.0f / seg;
                Float wgt = increment;
                for (int targ = srcBegin + 2; targ < srcEnd; targ += 2, wgt += increment)
                {
                    ptr[targ] = Utils::Lerp(ptr[srcBegin], ptr[srcEnd], wgt);
                }
            };
            auto edgePatchEval = [](QVector3D* ptr, int srcBegin, int srcEnd) {
                QVector3D vec1 = ptr[srcBegin + 1] - ptr[srcBegin];
                QVector3D vec2 = ptr[srcEnd + 1] - ptr[srcEnd];
                float len1 = vec1.length();
                float len2 = vec2.length();
                float lenAvg = (len1 + len2) * 0.5f;
                vec1 *= lenAvg / len1;
                vec2 *= lenAvg / len2;
                int seg = (srcEnd - srcBegin - 2) >> 1;
                Float increment = 2.0 / seg;
                Float wgt = increment;
                if (QVector3D::dotProduct(vec2 - vec1, ptr[srcEnd] - ptr[srcBegin]) > 0)
                {
                    ptr[srcBegin + seg] = ptr[srcEnd - seg]
                        = Utils::Lerp(ptr[srcBegin], ptr[srcEnd], 0.5f);

                    ptr[srcBegin + seg + 1] = ptr[srcBegin + seg] + vec1;
                    ptr[srcEnd - seg + 1] = ptr[srcEnd - seg] + vec2;
                }
                else
                {
                    ptr[srcBegin + seg + 1] = ptr[srcEnd - seg + 1]
                        = Utils::Lerp(ptr[srcBegin + 1],
                                      ptr[srcEnd + 1],
                                      0.5f);

                    ptr[srcBegin + seg] = ptr[srcBegin + seg + 1] - vec1;
                    ptr[srcEnd - seg] = ptr[srcEnd - seg + 1] - vec2;
                }
                for (int i = 2; i < seg; i += 2, wgt += increment)
                {
                    ptr[srcBegin + i] = Utils::Lerp(ptr[srcBegin], ptr[srcBegin + seg], wgt);
                    ptr[srcEnd - i] = Utils::Lerp(ptr[srcEnd], ptr[srcEnd - seg], wgt);
                    ptr[srcBegin + i + 1] = Utils::Lerp(ptr[srcBegin + 1], ptr[srcBegin + seg + 1], wgt);
                    ptr[srcEnd - i + 1] = Utils::Lerp(ptr[srcEnd + 1], ptr[srcEnd - seg + 1], wgt);
                }

            };
#ifdef LERP_BRIDGE
            lerpPatchPos(patchPos, 0, 10, 10);
            lerpPatchPos(patchPos, 1, 11, 10);
#else
            edgePatchEval(patchPos, 0, 10);
#endif // LERP_BRIDGE

            patchPos[12] = Utils::Lerp(patchPos[10], patchPos[16], patchStripScale);
            patchPos[13] = Utils::Lerp(patchPos[11], patchPos[17], patchStripScale);
            patchPos[14] = Utils::Lerp(patchPos[16], patchPos[10], patchStripScale);
            patchPos[15] = Utils::Lerp(patchPos[17], patchPos[11], patchStripScale);

#ifdef LERP_BRIDGE
            lerpPatchPos(patchPos, 16, 26, 10);
            lerpPatchPos(patchPos, 17, 27, 10);
#else
            edgePatchEval(patchPos, 16, 26);
#endif // LERP_BRIDGE

            patchPos[28] = Utils::Lerp(patchPos[26], patchPos[32], patchStripScale);
            patchPos[29] = Utils::Lerp(patchPos[27], patchPos[33], patchStripScale);
            patchPos[30] = Utils::Lerp(patchPos[32], patchPos[26], patchStripScale);
            patchPos[31] = Utils::Lerp(patchPos[33], patchPos[27], patchStripScale);

#ifdef LERP_BRIDGE
            lerpPatchPos(patchPos, 32, 42, 10);
            lerpPatchPos(patchPos, 33, 43, 10);
#else
            edgePatchEval(patchPos, 32, 42);
#endif // LERP_BRIDGE
            auto bezierPos = [](QVector3D* ptr) {
                *ptr = *(ptr - 2) * 0.25f + *ptr * 0.5f + *(ptr + 2) * 0.25f;
            };
            bezierPos(&patchPos[10]);
            bezierPos(&patchPos[11]);
            bezierPos(&patchPos[16]);
            bezierPos(&patchPos[17]);
            bezierPos(&patchPos[26]);
            bezierPos(&patchPos[27]);
            bezierPos(&patchPos[32]);
            bezierPos(&patchPos[33]);

            for (int j = 0; j < sPatchVertCount; j++)
            {
                (patchVerts + j)->pos = patchPos[j];
            }

            heToPatch[neiEdgeIDs[3]] = -curPatchID;
            heToPatch[neiEdgeIDs[2]] = curPatchID++;
            visitedHE[neiEdgeIDs[2]] = visitedHE[neiEdgeIDs[3]] = true;

            heOnTopFlag[neiEdgeIDs[1]] = true;
            curHE = curHE->rotCCW()->next()->flip();
        } while (curHE != he);
    }

    // Finalize Mesh
    unordered_set<hdsid_t> exposedHEs;
    for (auto &he : hes)
    {
        if (he.flip_offset == 0) exposedHEs.insert(he.index);
    }
    fillNullFaces(hes, faces, exposedHEs);
    HDS_Mesh* ret = new HDS_Mesh(verts, hes, faces);
    //ret->updatePieceSet();

    return ret;
}

void MeshNeoWeaver::createStripTopo(vector<vert_t> &verts,
                                    vector<he_t>   &hes,
                                    vector<face_t> &faces,
                                    size_t          nStrip, /*Total strip count*/
                                    size_t          nSegFace,/*Face count of each strip*/
                                    size_t          nSegVert,/*vertex count in each strip*/
                                    size_t          nSegHeOfs,/*reserved offset of each strip*/
                                    size_t          nSegFaceOfs)
{
    //const int nVert = nSegFace * 2 + 2;
    for (int i = 0; i < nStrip; i++)
    {
        int patchFaceOffset = i * nSegFace;
        int patchHEOffset = patchFaceOffset * 4;

        int patchVertOffset = i * nSegVert;
        auto curHE = hes.data() + patchHEOffset;
        auto curVert = verts.data() + patchVertOffset;
        auto curFace = faces.data() + patchFaceOffset;
        for (int j = 0; j < nSegFace; j++)
        {
            constructFace(curHE, 4, curFace);
            if (j != nSegFace - 1)
            {
                (curHE + 2)->flip_offset = 2;
                (curHE + 4)->flip_offset = -2;
            }
            constructHEPair(curVert, curHE++);
            constructHEPair(curVert + 1, curHE++);
            constructHEPair(curVert + 3, curHE++);
            constructHEPair(curVert + 2, curHE++);
            curFace++;
            curVert += 2;
        }
    }
}
