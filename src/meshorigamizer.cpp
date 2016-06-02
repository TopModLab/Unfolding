#include "meshorigamizer.h"



bool MeshOrigamizer::origamiMesh( HDS_Mesh * mesh )
{
	initiate();
	cur_mesh = mesh;
	seperateFaces();

	//get bridge pairs
	unordered_map<hdsid_t, he_t*> refidMap;
	for (auto he : hes_new) {
		if (refidMap.find(he->flip->refid) == refidMap.end()) {
			refidMap.insert(make_pair(he->refid, he));
		}
		else {
			he->flip->setBridgeTwin(refidMap[he->refid]->flip);
		}
	}



	updateNewMesh();
	cur_mesh->setProcessType(HDS_Mesh::ORIGAMI_PROC);
#ifdef _DEBUG
	cout << "origami succeed............." << endl;
#endif
	return true;
}

void MeshOrigamizer::seperateFaces() {
	for (auto f : cur_mesh->faces()) {
		if (!f->isCutFace) {
			auto fCorners = f->corners();
			vector<vert_t*> vertices;
			for (int i = 0; i < fCorners.size(); ++i) {
				vert_t* v_new = new vert_t;
				v_new->pos = fCorners[i]->pos;
				v_new->refid = fCorners[i]->refid;
				vertices.push_back(v_new);
				verts_new.push_back(v_new);
			}

			face_t* newFace = createFace(vertices);
			newFace->refid = f->refid;
			newFace->isCutFace = false;
			newFace->isBridger = false;
			faces_new.push_back(newFace);

			he_t* newHE = newFace->he;
			he_t* curHE = f->he;

			do {
				newHE->refid = curHE->refid;
				newHE->flip->refid = curHE->flip->refid;
				newHE->setCutEdge(curHE->isCutEdge);
				if (newHE->isCutEdge) {
					face_t* newCutFace = new face_t(*(curHE->flip->f));
					newHE->flip->f = newCutFace;
					faces_new.push_back(newCutFace);
				}
				newHE = newHE->next;
				newHE = curHE->next;
			} while (newHE != newFace->he);
		}
	}
}


