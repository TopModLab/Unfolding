#include "HDS/HDS_Mesh.h"
#include "Utils/mathutils.h"
#include "Utils/utils.h"

HDS_Mesh::HDS_Mesh()
	: processType(HALFEDGE_PROC)
{
}

HDS_Mesh::HDS_Mesh(
	vector<vert_t> &verts,
	vector<he_t>   &hes,
	vector<face_t> &faces)
	: vertSet(std::move(verts))
	, heSet(std::move(hes))
	, faceSet(std::move(faces))
	, processType(HALFEDGE_PROC)
{
}

HDS_Mesh::HDS_Mesh(const HDS_Mesh &other)
	: vertSet(other.vertSet)
	, heSet(other.heSet)
	, faceSet(other.faceSet)
	, processType(other.processType)
	, pieceSet(other.pieceSet)
{
	if (other.bound.get()) bound.reset(new BBox3(*other.bound));

	if (!pieceSet.empty())
	{
		pieceSet.resize(other.pieceSet.size());
		for (size_t i = 0; i < pieceSet.size(); i++)
		{
			pieceSet[i] = other.pieceSet[i];
		}
	}
}

HDS_Mesh::~HDS_Mesh()
{
}

void HDS_Mesh::updatePieceSet()
{
	for (auto piece : pieceSet)
	{
		piece.clear();
	}
	pieceSet.clear();

	vector<bool> visitedFaces(faceSet.size(), false);

	// Find all faces
	for (size_t fid = 0; fid < faceSet.size(); fid++)
	{
		// If f has not been visited yet
		// Add to selected faces
		if (!visitedFaces[fid])
		{
			visitedFaces[fid] = true;
			// Find all linked faces except cut face
			auto curPiece = linkedFaces(fid);

			for (auto cf : curPiece)
			{
				visitedFaces[cf] = true;
			}
			pieceSet.push_back(curPiece);
		}
	}
#ifdef _DEBUG
	cout << "Piece Set Info:\n\t" << pieceSet.size()
		<< " pieces\n\tTotal faces " << faceSet.size()
		<< "\n\tface in pieces" << pieceSet.begin()->size() << endl;
#endif // _DEBUG
}

bool HDS_Mesh::validateEdge(hdsid_t heid)
{
	if (heid >= heSet.size()) return false;
	auto e = &heSet[heid];
	if (e->index != heid)
	{
		printf("edge id #%d doesn't match offset.\n", heid);
		return false;
	}
	if (e->flip()->flip() != e) { cout << "flip invalid" << endl; return false; }
	if (e->next()->prev() != e) { cout << "next invalid" << endl; return false; }
	if (e->prev()->next() != e) { cout << "prev invalid" << endl; return false; }
	if (e->fid == sInvalidHDS || e->fid >= faceSet.size())
	{
		cout << "face id invalid\n";
		return false;
	}
	if (e->vid == sInvalidHDS || e->vid >= vertSet.size())
	{
		cout << "vertex id invalid\n";
		return false;
	}
	return true;
}

bool HDS_Mesh::validateFace(hdsid_t fid)
{
	if (fid >= faceSet.size())
	{
		printf("face id #%d out of range\n", fid);
		return false;
	}
	auto f = &faceSet[fid];
	if (f->index != fid)
	{
		printf("face id #%d doesn't match offset.\n", fid);
		return false;
	}
	int maxEdges = 1000;
	he_t *he = &heSet[f->heid];
	he_t *curHe = he;
	int edgeCount = 0;
	do {
		curHe = curHe->next();
		++edgeCount;
		if (edgeCount > maxEdges)
		{
			printf("edge count exceed maximum\n");
			return false;
		}
	} while (curHe != he);
	return true;
}

bool HDS_Mesh::validateVertex(hdsid_t vid)
{
	if (vid >= vertSet.size()) return false;
	if (vertSet[vid].index != vid) return false;

	const int maxEdges = 100;
	auto he = heFromVert(vid);
	auto curHE = he;
	int edgeCount = 0;
	do {
		curHE = curHE->rotCW();
		++edgeCount;
		if (edgeCount > maxEdges) return false;
	} while (curHE != he);
	return true;
}

bool HDS_Mesh::validate()
{
	bool validated = true;
	// verify that the mesh has good topology, ie has loop
	for (int vid = 0; vid < vertSet.size(); vid++)
	{
		if (!validateVertex(vid))
		{
			cout << "vertex #" << vid << " is invalid.\n";
			validated = false;
		}
	}
	for (hdsid_t fid = 0; fid < faceSet.size(); fid++)
	{
		if (!validateFace(fid))
		{
			cout << "face #" << fid << " is invalid.\n";
			validated = false;
		}
	}
	for (hdsid_t heid = 0; heid < heSet.size(); heid++)
	{
		if (!validateEdge(heid))
		{
			cout << "half edge #" << heid << " is invalid.\n";
			validated = false;
		}
	}
	return validated;
}

void HDS_Mesh::printInfo(const string& msg)
{
	if( !msg.empty() ) {
		cout << msg << endl;
	}
	cout << "#vertices = " << vertSet.size() << endl;
	cout << "#faces = " << faceSet.size() << endl;
	cout << "#half edges = " << heSet.size() << endl;
}

void HDS_Mesh::printMesh(const string &msg)
{
	if( !msg.empty() ) {
		cout << msg << endl;
	}
	for(auto v : vertSet) {
		cout << v << endl;
	}

	for(auto f : faceSet) {
		cout << f << endl;
	}

	for(auto he : heSet) {
		cout << he << endl;
	}
}

void HDS_Mesh::releaseMesh()
{
	vertSet.clear();
	faceSet.clear();
	heSet.clear();
	pieceSet.clear();
	bound.release();
}

void HDS_Mesh::setMesh(
	vector<face_t> &&faces,
	vector<vert_t> &&verts,
	vector<he_t>   &&hes)
{
	releaseMesh();
	faceSet = faces;
	heSet   = hes;
	vertSet = verts;

	// reset the UIDs, hack
	HDS_Face::resetIndex();
	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();
}

void HDS_Mesh::exportVertVBO(
	floats_t* verts, ui16s_t* vFLAGs) const
{
	// vertex object buffer
	// If verts exist, copy vertex buffer and vertex flags
	if (verts != nullptr)
	{
		verts->clear();
		verts->reserve(vertSet.size());
		vFLAGs->clear();
		vFLAGs->reserve(vertSet.size());
		for (auto v : vertSet)
		{
			auto& pos = v.pos;
			verts->push_back(pos.x());
			verts->push_back(pos.y());
			verts->push_back(pos.z());
			vFLAGs->push_back(v.getFlag());
		}
	}
	// if verts is null, copy only vertex flags
	else
	{
		vFLAGs->clear();
		vFLAGs->reserve(vertSet.size());
		for (auto v : vertSet)
		{
			if (v.isPicked)
			{
				cout << "already picked!" << endl;
			}
			vFLAGs->push_back(v.getFlag());
		}
	}
}

void HDS_Mesh::exportEdgeVBO(
	ui32s_t* heIBOs, ui32s_t* heIDs, ui16s_t* heFLAGs) const
{
	size_t heSetSize = heSet.size();
	// if edge index buffer exists, copy edge index buffer,
	// edge ids, and edge flags
	// assume edge ids will never be null in this case
	if (heIBOs != nullptr)
	{
		// Hash table for tracking visited edges and their flip edges
		vector<bool> visitiedHE(heSetSize, false);

		heIBOs->clear();
		heIBOs->reserve(heSetSize);
		heIDs->clear();
		heIDs->reserve(heSetSize >> 1);
		heFLAGs->clear();
		heFLAGs->reserve(heSetSize >> 1);

		for (size_t i = 0; i < heSetSize; i++)
		{
			if (!visitiedHE[i])
			{
				auto &he = heSet[i];
				
				visitiedHE[i] = true;
				visitiedHE[he.flip()->index] = true;

				heIBOs->push_back(he.vid);
				heIBOs->push_back(he.flip()->vid);

				heIDs->push_back(he.index);
				heFLAGs->push_back(he.getFlag());
			}
		}
	}
	// if edge index buffer doesn't exist
	// copy edge flags only
	else if (heFLAGs != nullptr)
	{
		vector<bool> visitiedHE(heSetSize, false);

		heFLAGs->clear();
		heFLAGs->reserve(heSetSize >> 1);

		for (size_t i = 0; i < heSetSize; i++)
		{
			if (!visitiedHE[i])
			{
				auto &he = heSet[i];

				visitiedHE[i] = true;
				visitiedHE[he.flip()->index] = true;

				heFLAGs->push_back(he.getFlag());
			}
		}
	}
}

void HDS_Mesh::exportFaceVBO(
	ui32s_t* fIBOs, ui32s_t* fIDs, ui16s_t* fFLAGs) const
{
	// face index buffer
	auto inTriangle = [](const QVector3D& p, const QVector3D& v0, const QVector3D& v1, const QVector3D& v2)->bool {
		auto area = QVector3D::crossProduct(v1 - v0, v2 - v0);
		auto v1p = v1 - p;
		auto v2p = v2 - p;
		if (QVector3D::dotProduct(QVector3D::crossProduct(v1p, v2p), area) < 0)
		{
			return false;
		}
		auto v0p = v0 - p;
		if (QVector3D::dotProduct(QVector3D::crossProduct(v2p, v0p), area) < 0)
		{
			return false;
		}
		if (QVector3D::dotProduct(QVector3D::crossProduct(v0p, v1p), area) < 0)
		{
			return false;
		}
		return true;
	};

	size_t fSetSize = faceSet.size();
	if (fIBOs != nullptr)
	{
		// triangulated face index buffer
		fIBOs->clear();
		fIBOs->reserve(fSetSize * 3);
		// original face index, for query
		fIDs->clear();
		fIDs->reserve(fSetSize * 2);
		fFLAGs->clear();
		fFLAGs->reserve(fSetSize * 2);
		for (auto &face : faceSet)
		{
			if (face.isCutFace)
			{
				continue;
			}
			ui32s_t vid_array;
			auto fid = static_cast<uint32_t>(face.index);
			uint16_t flag = face.getFlag();
			auto he = &heSet[face.heid];
			auto curHE = he;
			do
			{
				vid_array.push_back(curHE->vid);
				curHE = curHE->next();
			} while (curHE != he);

			// Operate differently depending on edge number
			size_t vidCount = vid_array.size();
			switch (vidCount)
			{
			case 3:
			{
				// Index buffer
				fIBOs->insert(fIBOs->end(), vid_array.begin(), vid_array.end());
				// face attribute
				//fid_array->push_back(fid);
				//fflag_array->push_back(flag);
				break;
			}
			/*case 4:
			{
			// P3 in Triangle012
			if (inTriangle(
			vertMap.at(vid_array[3])->pos,
			vertMap.at(vid_array[0])->pos,
			vertMap.at(vid_array[1])->pos,
			vertMap.at(vid_array[2])->pos))
			{
			// Index buffer 013
			fib_array->insert(fib_array->end(),
			vid_array.begin(), vid_array.begin() + 2);
			fib_array->push_back(vid_array.back());

			// Index buffer 123
			fib_array->insert(fib_array->end(),
			vid_array.begin() + 1, vid_array.end());
			}
			else// P3 outside Triangle012
			{
			// Index buffer 012
			fib_array->insert(fib_array->end(),
			vid_array.begin(), vid_array.begin() + 3);

			// Index buffer 230
			fib_array->insert(fib_array->end(),
			vid_array.begin() + 2, vid_array.end());
			fib_array->push_back(vid_array.front());

			}
			// face attribute
			//fid_array->insert(fid_array->end(), 2, fid);
			//fflag_array->insert(fflag_array->end(), 2, flag);
			}*/
			case 6:
			{
				if (!face.isBridger)
				{
					/*********************/
					/* Non-Bridger Faces */
					/* From Multi-Hollow */
					/*              1    */
					/*           2 /|    */
					/*   4 _______|*|    */
					/*    /_________|    */
					/*   5          0    */
					/*********************/
					// Index buffer
					fIBOs->push_back(vid_array[0]);
					fIBOs->push_back(vid_array[1]);
					fIBOs->push_back(vid_array[2]);

					fIBOs->push_back(vid_array[0]);
					fIBOs->push_back(vid_array[2]);
					fIBOs->push_back(vid_array[3]);

					fIBOs->push_back(vid_array[0]);
					fIBOs->push_back(vid_array[3]);
					fIBOs->push_back(vid_array[4]);

					fIBOs->push_back(vid_array[0]);
					fIBOs->push_back(vid_array[4]);
					fIBOs->push_back(vid_array[5]);
				}
				break;
			}
			case 8:
			{
				if (!face.isBridger)
				{
					/*********************/
					/* Non-Bridger Faces */
					/* From Multi-Hollow */
					/*      5       1    */
					/*   6 /|    2 /|    */
					/*    |*|_____|*|    */
					/*    |_________|    */
					/*    7         0    */
					/*********************/
					// Index buffer
					fIBOs->push_back(vid_array[0]);
					fIBOs->push_back(vid_array[1]);
					fIBOs->push_back(vid_array[2]);
					
					fIBOs->push_back(vid_array[0]);
					fIBOs->push_back(vid_array[2]);
					fIBOs->push_back(vid_array[3]);

					fIBOs->push_back(vid_array[0]);
					fIBOs->push_back(vid_array[3]);
					fIBOs->push_back(vid_array[4]);

					fIBOs->push_back(vid_array[0]);
					fIBOs->push_back(vid_array[4]);
					fIBOs->push_back(vid_array[7]);

					fIBOs->push_back(vid_array[7]);
					fIBOs->push_back(vid_array[4]);
					fIBOs->push_back(vid_array[5]);

					fIBOs->push_back(vid_array[7]);
					fIBOs->push_back(vid_array[5]);
					fIBOs->push_back(vid_array[6]);
				}
				break;
			}
			default: // n-gons
			{
				// Triangle Fan
				for (size_t i = 1; i < vidCount - 1; i++)
				{
					fIBOs->push_back(vid_array[0]);
					fIBOs->push_back(vid_array[i]);
					fIBOs->push_back(vid_array[i + 1]);
				}
				break;
			}
			}
			fIDs->insert(fIDs->end(), vidCount - 2, fid);
			fFLAGs->insert(fFLAGs->end(), vidCount - 2, flag);
		}
	}
	else if (fFLAGs != nullptr)
	{
		// re-export face flag
		fFLAGs->clear();
		fFLAGs->reserve(fSetSize * 2);
		for (auto face : faceSet)
		{
			if (face.isCutFace)
			{
				continue;
			}
			uint16_t flag = face.getFlag();
			auto he = &heSet[face.heid];
			auto curHE = he;
			size_t vidCount = 0;
			do
			{
				vidCount++;
				curHE = curHE->next();
			} while (curHE != he);
			fFLAGs->insert(fFLAGs->end(), vidCount - 2, flag);
		}
	}
}

void HDS_Mesh::exportSelection(
	selSeq_t* selVTX, selSeq_t* selHE, selSeq_t* selFACE)
{
	if (selVTX != nullptr)
	{
		for (auto v : vertSet)
		{
			if (v.isPicked) selVTX->insert(v.index);
		}
	}
	if (selHE != nullptr)
	{
		for (auto he : heSet)
		{
			if (he.isPicked) selHE->insert(he.index);
		}
	}
	if (selFACE != nullptr)
	{
		for (auto f : faceSet)
		{
			if (f.isPicked) selFACE->insert(f.index);
		}
	}
}

vector<hdsid_t> HDS_Mesh::faceCorners(hdsid_t fid) const
{
	vector<hdsid_t> corners;

	auto he = heFromFace(fid);
	auto curHE = he;
	do
	{
		corners.push_back(curHE->vid);
		curHE = curHE->next();
	} while (curHE != he);

	return corners;
}

QVector3D HDS_Mesh::faceCenter(hdsid_t fid) const
{
	auto corners = faceCorners(fid);
	QVector3D c;
	for (auto vid : corners) {
		c += vertSet[vid].pos;
	}
	c /= (qreal)corners.size();

	return c;
}

QVector3D HDS_Mesh::faceNormal(hdsid_t fid) const
{
	auto corners = faceCorners(fid);
	QVector3D c;
	for (auto vid : corners) {
		c += vertSet[vid].pos;
	}
	c /= (qreal)corners.size();

	QVector3D n = QVector3D::crossProduct(
		vertSet[corners[0]].pos - c,
		vertSet[corners[1]].pos - c);

	return n.normalized();
}

QVector3D HDS_Mesh::edgeVector(hdsid_t heid) const
{
	return vertSet[heSet[heid].flip()->vid].pos
		- vertSet[heSet[heid].vid].pos;
}

QVector3D HDS_Mesh::edgeVector(const he_t &he) const
{
	return vertSet[he.next()->vid].pos - vertSet[he.vid].pos;
}

vector<QVector3D> HDS_Mesh::allVertNormal() const
{
	vector<QVector3D> ret(vertSet.size(), QVector3D());

	for ( hdsid_t fid = 0; fid < faceSet.size(); fid++)
	{
		auto corners = faceCorners(fid);
		QVector3D c;
		for (auto vid : corners) {
			c += vertSet[vid].pos;
		}
		c /= (qreal)corners.size();

		QVector3D n = QVector3D::crossProduct(
			vertSet[corners[0]].pos - c,
			vertSet[corners[1]].pos - c);
		n.normalize();

		for (auto vid : corners)
		{
			ret[vid] += n;
		}
	}
	for (auto n : ret) n.normalize();

	return ret;
}


// Find all connected faces from input face
// Input face must NOT be CutFace
// BFS Tree
vector<hdsid_t> HDS_Mesh::linkedFaces(hdsid_t inFaceId) const
{
	// Return face indices
	vector<hdsid_t> retFaceSet;
	// Hash table to record visited faces
	vector<bool> visitedFaces(faceSet.size(), false);

	queue<hdsid_t> ProcQueue;
	vector<hdsid_t> CutFaces;
	ProcQueue.push(inFaceId);
	// Input face should be marked as visited
	visitedFaces[inFaceId] = true;
	while (!ProcQueue.empty())
	{
		auto cur_fid = ProcQueue.front();
		ProcQueue.pop();

		// If CutFace is not the last face,
		// move it to the end of the queue
		if (faceSet[cur_fid].isCutFace)
		{
			CutFaces.push_back(cur_fid);
			continue;
		}
		// Otherwise, add to result
		retFaceSet.push_back(cur_fid);

		// Loop face and record all unvisited face into queue
		auto fhe = heFromFace(cur_fid);
		auto curHE = fhe;
		do
		{
			auto adj_fid = curHE->flip()->fid;
			curHE = curHE->next();
			if (!visitedFaces[adj_fid])
			{
				visitedFaces[adj_fid] = true;
				ProcQueue.push(adj_fid);
			}
		} while (curHE != fhe);
	}
	// Move cut face to the end of the result
	retFaceSet.insert(retFaceSet.end(), CutFaces.begin(), CutFaces.end());
	return retFaceSet;
}

// Functionality:
//	Split he and he->flip so that they become cut edges
// Input:
//	heid: the id of the he that needs to be split up
void HDS_Mesh::splitHeFromFlip(hdsid_t heid)
{
	auto &he = heSet[heid];
	//duplicate verts
	int oriSize = vertSet.size();
	vertSet.resize(oriSize + 2);
	auto &v = vertSet[he.vertID()];
	auto &flipv = vertSet[he.flip()->vertID()];
	auto fliphe = he.flip();
	vertSet[oriSize].pos = flipv.pos;
	vertSet[oriSize + 1].pos = v.pos;

	//assign them to he->flip
	flipv.heid = he.next_offset + heid;
	v.heid = heid;
	fliphe->vid = oriSize;
	fliphe->next()->vid = oriSize + 1;
	vertSet[oriSize].heid = fliphe->index;
	vertSet[oriSize + 1].heid = fliphe->next()->index;
	//reset flips
	he.flip_offset = 0;
	fliphe->flip_offset = 0;
}

void HDS_Mesh::addHalfEdge(he_t &he)
{
	heSet.push_back(he);
}

void HDS_Mesh::addVertex(vert_t &vert)
{
	vertSet.push_back(vert);
}

void HDS_Mesh::addFace(face_t &face)
{
	faceSet.push_back(face);
}

std::vector<hdsid_t> HDS_Mesh::incidentFacesFromFace(hdsid_t fid) const
{
	vector<hdsid_t> ret;
	auto he = heFromFace(fid);
	auto curHE = he;
	do 
	{
		ret.push_back(curHE->flip()->fid);
		curHE = curHE->next();
	} while (curHE != he);

	return ret;
}

vector<hdsid_t> HDS_Mesh::incidentFacesFromVert(hdsid_t vid) const
{
	vector<hdsid_t> ret;
	auto he = heFromVert(vid);
	auto curHE = he;
	do
	{
		ret.push_back(curHE->fid);
		curHE = curHE->rotCW();
	} while (curHE != he);

	return ret;
}
// all outgoing half edges of vertex v
vector<hdsid_t> HDS_Mesh::incidentEdgesFromVert(hdsid_t vid) const
{
	vector<hdsid_t> ret;
	auto he = heFromVert(vid);
	auto curHE = he;
	do
	{
		ret.push_back(curHE->index);
		curHE = curHE->rotCW();
	} while (curHE != he);

	return ret;
}

hdsid_t HDS_Mesh::sharedEdgeByFaces(hdsid_t fid1, hdsid_t fid2) const
{
	if (fid1 == fid2) return sInvalidHDS;

	auto he = heFromFace(fid1);
	auto curHE = he;
	do
	{
		if (curHE->flip()->fid == fid2)
		{
			return curHE->index;
		}
		curHE = curHE->next();
	} while (curHE != he);

	return sInvalidHDS;
}

hdsid_t HDS_Mesh::sharedEdgeByVerts(hdsid_t vid1, hdsid_t vid2) const
{
	if (vid1 == vid2) return sInvalidHDS;

	auto he = heFromVert(vid1);
	auto curHE = he;
	do {
		if (curHE->flip()->vid == vid2)
		{
			return curHE->index;
		}
		curHE = curHE->rotCW();
	} while (curHE != he);

	return sInvalidHDS;
}

bool HDS_Mesh::checkPlanarFace(hdsid_t fid)
{
	auto &f = faceSet[fid];
	auto fVerts = faceCorners(fid);
	QVector3D normal = QVector3D::crossProduct(
		vertSet[fVerts[1]].pos - vertSet[fVerts[0]].pos,
		vertSet[fVerts[2]].pos - vertSet[fVerts[0]].pos
	);

	for (int i = 3; i < fVerts.size(); i++)
	{
		Float dot = QVector3D::dotProduct(normal,
			vertSet[fVerts[i]].pos - vertSet[fVerts[0]].pos);
		if (fabsf(dot) > 0.3)
		{
			f.isNonPlanar = true;
			return false;
		}
	}
	return true;
}

void HDS_Mesh::updatePlanarFlag()
{
	for (hdsid_t i = 0; i < faceSet.size(); i++)
	{
		checkPlanarFace(i);
	}
}

#ifdef USE_LEGACY_FACTORY
HDS_HalfEdge* HDS_Mesh::insertEdge(
	vector<he_t> &edges, vert_t* v1, vert_t* v2, he_t* he1, he_t* he2)
{
	bool v1_isNew = false, v2_isNew = false;

	if (v1->he == nullptr) v1_isNew = true;
	if (v2->he == nullptr) v2_isNew = true;

	edges.emplace_back();
	he_t* he = &edges.back();
	edges.emplace_back();
	he_t* he_flip = &edges.back();
	//he_t* he_flip = new he_t;
	he->setFlip(he_flip);

	//link edge and vertices
	he->v = v1;
	he_flip()->v = v2;
	if (v1_isNew) v1->he = he;
	if (v2_isNew) v2->he = he_flip;

	//link edge loop
	he->next = he_flip;
	he->prev = he_flip;
	he_flip()->next = he;
	he_flip()->prev = he;

	if(!v1_isNew) {
		he_t* prevHE, *nextHE;
		if (he1 != nullptr){
			nextHE = he1->next;
			prevHE = he1;
		}else{
			nextHE = v1->he->flip()->next;
			prevHE = v1->he->flip;
		}
		he_flip()->next = nextHE;
		nextHE->prev = he_flip;
		he->prev = prevHE;
		prevHE->next = he;
	}
	if(!v2_isNew) {
		he_t* prevHE, *nextHE;
		if (he2 != nullptr) {
			nextHE = he2;
			prevHE = he2->prev;
		}else {
		nextHE = v2->he;
		prevHE = v2->he->prev;
		}
		he->next = nextHE;
		nextHE->prev = he;
		he_flip()->prev = prevHE;
		prevHE->next = he_flip;

	}
	return he;
}

#endif // USE_LEGACY_FACTORY

void HDS_Mesh::selectFace(hdsid_t idx)
{
	faceSet[idx].setPicked(!faceSet[idx].isPicked);
}

void HDS_Mesh::selectEdge(hdsid_t idx)
{
	heSet[idx].setPicked(!heSet[idx].isPicked);
}

void HDS_Mesh::selectVertex(hdsid_t idx)
{
	vertSet[idx].setPicked(!vertSet[idx].isPicked);
}

vector<HDS_Vertex*> HDS_Mesh::getSelectedVertices()
{
	vector<vert_t*> pickedVerts;
	for(auto &v : vertSet) {
		if (v.isPicked) {
			pickedVerts.push_back(&v);
		}
	}
	return pickedVerts;
}

vector<HDS_HalfEdge*> HDS_Mesh::getSelectedEdges()
{
	vector<he_t*> pickedHEs;
	vector<bool>  unvisitedHE(heSet.size(), true);
	for(auto &he : heSet)
	{
		if (he.isPicked && unvisitedHE[he.index])
		{
			unvisitedHE[he.index] = unvisitedHE[he.flip()->index] = false;
			pickedHEs.push_back(&he);
		}
	}
	return pickedHEs;
}

vector<HDS_HalfEdge*> HDS_Mesh::getSelectedHalfEdges()
{
	vector<he_t*> pickedHEs;
	for(auto &he : heSet) {
		if (he.isPicked) {
			pickedHEs.push_back(&he);
		}
	}
	return pickedHEs;
}

vector<HDS_Face*> HDS_Mesh::getSelectedFaces()
{
	vector<face_t*> pickedFaces;
	for(auto &f : faceSet) {
		if (f.isPicked) {
			pickedFaces.push_back(&f);
		}
	}
	return pickedFaces;
}


vector<HDS_Vertex*> HDS_Mesh::getReebPoints(const doubles_t &funcval, const QVector3D &normdir)
{
	//TODO: currently not used
	auto moorseFunc = [&](vert_t* v, double a, double b, double c) -> double{
		if (!funcval.empty()) {
			// assign the function value to the vertex
			v->morseFunctionVal = funcval[v->index];
			return (funcval[v->index]);
			cout<<"v->index="<<v->index<<endl;
		}
		else {
			return a * v->pos.x() + b * v->pos.y() + c * v->pos.z();
			cout<<"a="<<a<<endl;      //later added;
		}
	};

	const int n = 3;
	vector<tuple<double, double, double>> randvals;
	for (int i = 0; i < 10; ++i) {
		double a = (rand() / (double)RAND_MAX - 0.5) * 1e-8 + normdir.x();
		double b = (rand() / (double)RAND_MAX - 0.5) * 1e-8 + normdir.y();
		double c = (rand() / (double)RAND_MAX - 0.5) * 1e-8 + normdir.z();
		randvals.push_back(make_tuple(a, b, c));
	}

	int s11=0,s22=0,s33=0;
	auto isReebPoint = [&](vert_t v) {


		int s1=0,s2=0,s3=0;
		for (int tid = 0; tid < n; ++tid) {

			// perform n tests

#if 1
			double a = std::get<0>(randvals[tid]);
			double b = std::get<1>(randvals[tid]);
			double c = std::get<2>(randvals[tid]);
#else
			double a = 0, b = 0, c = 0;
#endif




			auto neighbors = v.neighbors();

			// if all neighbors have smaller z-values
			bool allSmaller = std::all_of(neighbors.begin(), neighbors.end(), [&](vert_t* n) {

					//    cout<<"moorseFunc(n"<<n->index<<", a, b, c) = "<<moorseFunc(n, a, b, c)<<endl;
					//    cout<<"moorseFunc(v"<<v->index<<", a, b, c) = "<<moorseFunc(v, a, b, c)<<endl;
					return moorseFunc(n, a, b, c) > moorseFunc(&v, a, b, c);

		});

			// if all neighbors have larger z-values
			bool allLarger = std::all_of(neighbors.begin(), neighbors.end(), [&](vert_t* n) {
					return moorseFunc(n, a, b, c) < moorseFunc(&v, a, b, c);
		});
			//   cout<<" moorseFunc("<<v->index<<", a, b, c) = "<< moorseFunc(v, a, b, c);
			// if this is a saddle point
			bool isSaddle = false;
			doubles_t diffs;

			if(!allSmaller&&!allLarger)                 //later added;
			{

				for (auto n : neighbors) {
					double st= moorseFunc(n, a, b, c) - moorseFunc(&v, a, b, c);
					//     if(abs(st)<1e-5) st=0;                                             //later changed;
					diffs.push_back(st);
					//   diffs[n->index]=st;
					if(v.index==6||v.index==7||v.index==5||v.index==8||v.index==100){
						//          cout<<"moorseFunc("<<n->index<<", a, b, c) - moorseFunc("<<v->index<<", a, b, c) = "<<st<<endl;
					}

				}
				//      cout<<"diffs[0]="<<diffs[0]<<"diffs[1]="<<diffs[1]<<"diffs[2]="<<diffs[2]<<endl; //later added;

				//      cout<<"diffs.size()"<<diffs.size()<<endl;                                    //later added;
				//      cout<<"diffs.front()"<<diffs.front()<<endl;                                   //later added;
				int ngroups = 1, sign = diffs.front() > 0 ? 1 : -1;

				for (int i = 1; i < diffs.size(); ++i) {
					if (diffs[i] * sign >= 0) continue;
					else {
						sign = -sign;
						++ngroups;
						if (i == diffs.size() - 1 && sign == diffs.front()) {
							--ngroups;
						}
					}

				}


				if (ngroups % 2 == 1) {
					//cout << "error in computing groups!" << endl;
					ngroups = ngroups - 1;
				}
				//   cout<<v->index<<"ngroups = "<<ngroups<<endl;
				isSaddle = (ngroups >= 4 && ngroups % 2 == 0);

				if (isSaddle) {
					v.rtype = HDS_Vertex::Saddle;
					v.sdegree = (ngroups - 2) / 2;
					//     cout << "Saddle: " << v->index << ",moorseFunc(v, a, b, c) =  " << moorseFunc(v, a, b, c) << endl;
					//for (auto neighbor : neighbors) {
					//  cout << moorseFunc(neighbor, a, b, c) << " ";
					//}
					//cout << endl;
					s1+=1;
					//    cout<<"saddle !!!!!!!!!"<<s1<<endl;
				}

			}

			if (allSmaller) {
				v.rtype = HDS_Vertex::Minimum;
				//       cout << "Minimum: " << v->index << ",moorseFunc(v, a, b, c) =  " << moorseFunc(v, a, b, c) << endl;
				//for (auto neighbor : neighbors) {
				//  cout << moorseFunc(neighbor, a, b, c) << " ";
				//}
				//cout << endl;
				s2+=1;
				//    cout<<"minimum  !!!!!!!!!"<<s2<<endl;
			}


			if (allLarger) {
				v.rtype = HDS_Vertex::Maximum;
				//     cout << "Maximum: " << v->index << ",moorseFunc(v, a, b, c) =  " << moorseFunc(v, a, b, c) << endl;
				//for (auto neighbor : neighbors) {
				//  cout << moorseFunc(neighbor, a, b, c) << " ";
				//}
				//cout << endl;
				s3+=1;
				//    cout<<"maximum  !!!!!!!!!"<<s3<<endl;

			}


			if (allSmaller || allLarger || isSaddle) {}
			else {
				v.rtype = HDS_Vertex::Regular;
				return false;
			}
		}
		if(s1==3)s11+=v.sdegree;
		if(s2==3)s22+=1;
		if(s3==3)s33+=1;
		cout<<"maximum = "<<s33<<endl;
		cout<<"minimum = "<<s22<<endl;

		cout<<"saddle = "<< s11<<endl;
		return true;
	};

	//return Utils::filter(vertSet, isReebPoint);
	vector<vert_t*> ret;
	for (auto &x : vertSet) {
		if (isReebPoint(x))
			ret.push_back(&x);
	}
	return ret;
}

void HDS_Mesh::colorVertices(const doubles_t &val)
{
#if 1
	for (int i = 0; i < vertSet.size(); ++i) {
		vertSet[i].colorVal = val[i];
	}

#else
	int i = 0;
	for (auto v : vertSet) {
		v->colorVal = val[i++];
	}
#endif
}

void HDS_Mesh::save(const string &filename) const
{
	// the vertices and faces are saved in the same order they are loaded in
	stringstream ss;

	// save the vertices first
	for (auto v : vertSet) {
		ss << "v " << v.pos.x() << ' ' << v.pos.y() << ' ' << v.pos.z() << endl;
	}

	// save the faces
	for (auto &curFace : faceSet) {
		if (curFace.isCutFace) continue;

		auto corners = faceCorners(curFace.index);
		ss << "f ";
		for (auto vid : corners) {
			ss << vid + 1 << ' ';
		}
		ss << endl;
	}

	ofstream fout(filename);
	fout << ss.str();
	fout.close();
}
