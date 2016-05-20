#include "hds_mesh.h"
#include "glutils.hpp"
#include "mathutils.hpp"
#include "utils.hpp"
//#include "meshviewer.h"
#include <GL/glu.h>
#include<iostream>
using namespace std;

HDS_Mesh::HDS_Mesh()
	: showComponent(SHOW_FACE | SHOW_EDGE)
	, showFace(true), showVert(true)
	, showEdge(true), showNormals(false)
	, processType(REGULAR_PROC)
	, bound(nullptr)
{
}

HDS_Mesh::HDS_Mesh(const HDS_Mesh &other)
	: showFace(other.showFace), showEdge(other.showEdge)
	, showVert(other.showVert), showNormals(other.showNormals)
	, processType(other.processType)
{
	// need a deep copy

	// copy the vertices set
	vertSet.clear();
	vertMap.clear();
	for( auto v : other.vertSet ) {
		// he is not set for this vertex
		vert_t *nv = new vert_t(*v);
		vertSet.insert(nv);
		vertMap.insert(make_pair(v->index, nv));
	}

	faceSet.clear();
	faceMap.clear();
	for( auto f : other.faceSet ) {
		face_t *nf = new face_t(*f);
		faceSet.insert(nf);
		faceMap.insert(make_pair(f->index, nf));
	}

	heSet.clear();
	heMap.clear();
	for( auto he : other.heSet ) {
		// face, vertex, prev, next, and flip are not set yet
		he_t *nhe = new he_t(*he);
		heSet.insert(nhe);
		heMap.insert(make_pair(he->index, nhe));
	}

	// fill in the pointers
	for( auto &he : heSet ) {
		auto he_ref = other.heMap.at(he->index);
		//cout << he_ref->index << endl;
		if (he_ref->flip != nullptr)
			he->flip = heMap.at(he_ref->flip->index);
		if (he_ref->prev != nullptr)
			he->prev = heMap.at(he_ref->prev->index);
		if (he_ref->next != nullptr)
			he->next = heMap.at(he_ref->next->index);

		if (he_ref->cutTwin != nullptr)
			he->cutTwin = heMap.at(he_ref->cutTwin->index);

		if (he_ref->f != nullptr)
			he->f = faceMap.at(he_ref->f->index);
		if (he_ref->v != nullptr)
			he->v = vertMap.at(he_ref->v->index);
	}
	// set the half edges for faces
	for( auto &f : faceSet ) {
		auto f_ref = other.faceMap.at(f->index);
		f->he = heMap.at(f_ref->he->index);
	}

	// set the half edges for vertices
	for( auto &v : vertSet ) {
		auto v_ref = other.vertMap.at(v->index);
		v->he = heMap.at(v_ref->he->index);
	}

	// Copy piece set information
	this->pieceSet = other.pieceSet;

	bound = other.bound != nullptr ?
		new BBox3(*other.bound) : nullptr;
}

HDS_Mesh::~HDS_Mesh()
{
	releaseMesh();
}

void HDS_Mesh::updateSortedFaces()
{
	// create the sorted face set
	cout<<"updating sorted faces"<<endl;
	sortedFaces.assign(faceSet.begin(), faceSet.end());
	std::sort(sortedFaces.begin(), sortedFaces.end(), [](const face_t *fa, const face_t *fb) {
		auto ca = fa->corners();
		auto cb = fb->corners();
		float minZa = 1e9, minZb = 1e9;
		for (auto va : ca) {
			minZa = std::min(va->pos.z(), minZa);
		}
		for (auto vb : cb) {
			minZb = std::min(vb->pos.z(), minZb);
		}
		return minZa < minZb;
	});
}

void HDS_Mesh::clearSortedFaces()
{
	sortedFaces.clear();
}


void HDS_Mesh::updatePieceSet()
{
	for (auto piece : pieceSet)
	{
		piece.clear();
	}
	pieceSet.clear();

	unordered_set<hdsid_t> visitedFaces;

	int progressIndex = 0;
	// Find all faces
	for (auto f : this->faceSet)
	{
		// If f has not been visited yet
		// Add to selected faces
		if (visitedFaces.find(f->index) == visitedFaces.end())
		{
			visitedFaces.insert(f->index);
			// Find all linked faces except cut face
			set<HDS_Face*> linkedFaces = f->linkedFaces();

			set<hdsid_t> curPiece;
			for (auto cf : linkedFaces)
			{
				curPiece.insert(cf->index);
				visitedFaces.insert(cf->index);
			}
			pieceSet.insert(curPiece);
		}
	}
#ifdef _DEBUG
	cout << "Piece Set Info:\n\t" << pieceSet.size()
		<< " pieces\n\tTotal faces " << faceSet.size()
		<< "\n\tface in pieces" << pieceSet.begin()->size() << endl;
#endif // _DEBUG
}

bool HDS_Mesh::validateEdge(he_t *e) {

	if( heMap.find(e->index) == heMap.end() ){cout<<"heMap invalid"<<endl; return false;}
	if( e->flip->flip != e ){cout<<"flip invalid"<<endl; return false;}
	if( e->next->prev != e ){cout<<"next invalid"<<endl; return false;}
	if( e->prev->next != e ){cout<<"prev invalid"<<endl; return false;}
	if( e->f == nullptr ){cout<<"f invalid"<<endl; return false;}
	if( e->v == nullptr ){cout<<"v invalid"<<endl; return false;}
	if( faceSet.find(e->f) == faceSet.end() ){cout<<"->f invalid"<<endl; return false;}
	if( vertSet.find(e->v) == vertSet.end() ) {cout<<"->v invalid"<<endl;return false;}
	return true;
}

bool HDS_Mesh::validateFace(face_t *f)
{
	if( faceMap.find(f->index) == faceMap.end() ) {
        cout<<"cant find in face map"<<endl;
		return false;
	}
	int maxEdges = 1000;
	he_t *he = f->he;
	he_t *curHe = he;
	int edgeCount = 0;
	do {
		curHe = curHe->next;
		++edgeCount;
		if (edgeCount > maxEdges)
		{
            cout<<"edge count error"<<endl;
			return false;
		}
	} while( curHe != he );
	return true;
}

bool HDS_Mesh::validateVertex(vert_t *v) {
	if( vertMap.find(v->index) == vertMap.end() ) return false;

	int maxEdges =100;
	he_t *he = v->he;
	he_t *curHe = he;
	int edgeCount = 0;
	do {
		curHe = curHe->flip->next;
		++edgeCount;
		if( edgeCount > maxEdges ) return false;
	} while( curHe != he );
	return true;
}

bool HDS_Mesh::validate() {
	bool validated = true;
	// verify that the mesh has good topology, ie has loop
	for( auto v : vertSet ) {
		if( !validateVertex(v) ) {
			cout << "vertex #" << v->index << " is invalid." << endl;
			validated = false;
		}
	}

	for( auto f : faceSet ) {
		if( !validateFace(f) ) {
			cout << "face #" << f->index << " is invalid." << endl;
			validated = false;
		}
	}
	for( auto e : heSet ) {
		if( !validateEdge(e) ) {
			cout << "half edge #" << e->index << " is invalid." << endl;
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
	cout << "#sorted Faces = "<< sortedFaces.size() << endl;
}

void HDS_Mesh::printMesh(const string &msg)
{
	if( !msg.empty() ) {
		cout << msg << endl;
	}
	for(auto v : vertSet) {
		cout << *v << endl;
	}

	for(auto f : faceSet) {
		//if (f->isCutFace)
		{
			cout << *f << endl;
		}
	}

	for(auto he : heSet) {
		cout << *he << endl;
	}
}

void HDS_Mesh::releaseMesh() {
	for(auto v : vertSet)
		delete v;
	vertSet.clear();

	for(auto f : faceSet)
		delete f;
	faceSet.clear();

	for(auto he : heSet)
		delete he;
	heSet.clear();

	delete bound;
}

void HDS_Mesh::setMesh(
		const vector<HDS_Face *> &faces,
		const vector<HDS_Vertex *> &verts,
		const vector<HDS_HalfEdge *> &hes)
{
	releaseMesh();

	// reset the UIDs, hack
	HDS_Face::resetIndex();
	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();

	for (auto &f : faces)
	{
		int faceIdx = HDS_Face::assignIndex();
		f->setRefId(faceIdx);
		faceMap[faceIdx] = f;
		f->index = faceIdx;
		faceSet.insert(f);
	}

	for (auto &v : verts)
	{
		int vertIdx = HDS_Vertex::assignIndex();
		v->setRefId(vertIdx);
		vertMap[vertIdx] = v;
		v->index = vertIdx;
		vertSet.insert(v);
	}

	heSet.insert(hes.begin(), hes.end());
	for(auto &he : heSet)
	{
		if( he->index >= 0 ) continue;

		int heIdx = HDS_HalfEdge::assignIndex();
		he->setRefId(heIdx);
		heMap[heIdx] = he;
		he->index = heIdx;

		int hefIdx = HDS_HalfEdge::assignIndex();
		he->flip->setRefId(heIdx);
		he->flip->index = hefIdx;
		heMap[hefIdx] = he->flip;
	}

	updateSortedFaces();
}



//usage unknown
#define MAX_CHAR        128

#ifdef OPENGL_LEGACY
void drawString(const char* str, int numb)
{
	static int isFirstCall = 1;
	static GLuint lists;

	if (isFirstCall) {
		isFirstCall = 0;

		lists = glGenLists(MAX_CHAR);

		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}

	//for(int i=0; i<numb; i++){

	glCallList(lists + *str);
	cout << "ok" << *str << endl;

	//}
}
void display(int num)
{

	glColor3f(1.0f, 1.0f, 1.0f);
	//    glRasterPos2f(0.0f, 0.0f);
	//     glScalef(100,100,100);
	char ss[20];
	itoa(num, ss, 10);
	drawString(ss, num);
	num = 0;

}
#endif
void HDS_Mesh::draw(ColorMap cmap)
{
#ifdef OPENGL_LEGACY
	if (showFace)
	{
		GLfloat face_mat_diffuse[4] = { 0.75, 0.75, 0.75, 1 };
		GLfloat face_mat_diffuse_selected[4] = { 1, 0, 0.5, 1 };
		GLfloat face_mat_diffuse_bridger[4] = { 0.75, 0.75, 0.95, 1 };


		// traverse the mesh and render every single face
		for (auto fit = sortedFaces.begin(); fit != sortedFaces.end(); fit++)
		{
			face_t* f = (*fit);
			if (f->isCutFace || f->isHole) continue; //invisible face
													 // render the faces
			he_t* he = f->he;
			he_t* hen = he->next;
			he_t* hep = he->prev;


			he_t* curHe = he;

			if (f->isPicked) {
				glColor4f(1, 0, 0.5, 1);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, face_mat_diffuse_selected);

			}
			else if (f->isBridger) {
				glColor4f(0.75, 0.75, 0.95, 1);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, face_mat_diffuse_bridger);
			}

			else {
				glColor4f(0.75, 0.75, 0.75, 1);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, face_mat_diffuse);
			}

			int vcount = 0;
			glBegin(GL_POLYGON);
			do
			{
				++vcount;
				vert_t* v = curHe->v;

				// interpolation
				if (!f->isBridger) {
					//QColor clr = cmap.getColor_discrete(v->colorVal);
					//GLUtils::setColor(QColor(0.75,0.75,0.75), 0.3); //commented out face color variation
				}
				GLUtils::useNormal(v->normal);
				GLUtils::useVertex(v->pos);
				curHe = curHe->next;
			} while (curHe != he);
			glEnd();


		}
	}

	if (showEdge)
	{

		glColor4f(0.25, 0.25, 0.25, 1);
		GLfloat line_mat_diffuse[4] = { 0.25, 0.25, 0.25, 1 };
		GLfloat line_mat_diffuse_selected[4] = { 1, 0, 0.5, 1 };
		GLfloat line_mat_diffuse_cutEdge[4] = { 0, 1, 0, 1 };


		glLineWidth(2.0);
		// render the boundaires
		for (auto eit = heSet.begin(); eit != heSet.end(); eit++)
		{
			he_t* e = (*eit);
			he_t* en = e->next;

			QColor c = Qt::black;
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_mat_diffuse);

			if (e->isPicked) {
				c.setRgbF(1.0, 0.0, 0.5, 1.0);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_mat_diffuse_selected);
			}
			else if (e->isCutEdge) {
				c = Qt::green;
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_mat_diffuse_cutEdge);

			}

			GLUtils::drawLine(e->v->pos, en->v->pos, c);
		}
	}

	if (showVert || showNormals) {
		glColor4f(0, 0, 1, 1);
		GLfloat vert_mat_diffuse[4] = { 1, 1, 0, 1 };
		GLfloat vert_mat_diffuse_selected[4] = { 1, 0, 0.5, 1 };


		// render the boundaires
		for (auto vit = vertSet.begin(); vit != vertSet.end(); vit++)
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vert_mat_diffuse);
			vert_t* v = (*vit);
			glPushMatrix();
			glTranslatef(v->x(), v->y(), v->z());
#if 0
			glutSolidSphere(0.125, 16, 16);
#else
			glPointSize(5.0);
			if (v->isPicked) {
				glColor4f(1, 0, 0.5, 1);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vert_mat_diffuse_selected);

			}


			else {
				double c = .5 - clamp(v->curvature, -Pi / 2, Pi / 2) / Pi; //[0, 1]
																		   //cout << v->index << ":" << v->curvature << ", " << c << endl;
																		   //        if( c < 0.5 ) {
																		   //          glColor4f(0.0, (0.5 - c)*2, c*2.0, 1.0);
																		   //        }
																		   //        else {
																		   //          glColor4f(c, (c-0.5, 0.0, 1.0);
																		   //        }
																		   //glColor4f(c, 1-c, 1-c, 1.0); //commented out show colors depanding on curvature
				glColor4f(0.55, 0.55, 0.55, 0.0);
			}

			if (showVert)
			{
				glBegin(GL_POINTS);
				glVertex3f(0, 0, 0);
				glEnd();
			}
			if (showNormals)
			{
				glBegin(GL_LINES);


				GLUtils::setColor(Qt::green);
				GLUtils::useVertex(QVector3D(0, 0, 0));
				GLUtils::useVertex(v->normal);

				glEnd();
			}
#endif
			glPopMatrix();
		}
	}
#endif
}

void HDS_Mesh::drawVertexIndices()
{
#ifdef OPENGL_LEGACY
	glColor4f(0, 0, 1, 1);
	GLfloat line_mat_diffuse[4] = { 1, 0, 0, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_mat_diffuse);

	// render the boundaires
	for (auto vit = vertSet.begin(); vit != vertSet.end(); vit++)
	{
		vert_t* v = (*vit);
		glPushMatrix();
		glTranslatef(v->x(), v->y(), v->z());
#if 0
		glutSolidSphere(0.125, 16, 16);
#else
		glPointSize(15.0);

		float r, g, b;
		encodeIndex<float>(v->index, r, g, b);
		glColor4f(r, g, b, 1.0);

		glBegin(GL_POINTS);
		glVertex3f(0, 0, 0);
		glEnd();
#endif
		glPopMatrix();
	}
#endif
}

void HDS_Mesh::drawEdgeIndices()
{
#ifdef OPENGL_LEGACY
	for (auto eit = heSet.begin(); eit != heSet.end(); eit++)
	{
		he_t* e = (*eit);
		he_t* en = e->next;

		// draw only odd index half edges
		if (e->index & 0x1) continue;

		float r, g, b;
		encodeIndex<float>(e->index, r, g, b);
		glLineWidth(2.0);
		GLUtils::drawLine(e->v->pos, en->v->pos, QColor::fromRgbF(r, g, b));
	}
#endif
}

void HDS_Mesh::drawFaceIndices()
{
#ifdef OPENGL_LEGACY
	for (auto &f : faceSet) {
		float r, g, b;
		encodeIndex<float>(f->index, r, g, b);
		glColor4f(r, g, b, 1.0);

		he_t* he = f->he;
		he_t* curHe = he;

		glBegin(GL_POLYGON);
		do
		{
			vert_t* v = curHe->v;
			GLUtils::useVertex(v->pos);
			curHe = curHe->next;
		} while (curHe != he);
		glEnd();
	}
#endif
}

void HDS_Mesh::flipShowEdges()
{
	showEdge = !showEdge;
}

void HDS_Mesh::flipShowFaces()
{
	showFace = !showFace;
}

void HDS_Mesh::flipShowVertices()
{
	showVert = !showVert;
}

void HDS_Mesh::flipShowNormals()
{
	showNormals = !showNormals;
}

void HDS_Mesh::exportVertVBO(
	floats_t* verts, ui16s_t* vFLAGs) const
{
	// vertex object buffer
	if (verts != nullptr)
	{
		verts->clear();
		verts->reserve(vertSet.size());
		vFLAGs->clear();
		vFLAGs->reserve(vertSet.size());
		for (int i = 0; i < vertMap.size(); i++)
		{
			auto vert = vertMap.at(i);
			auto& pos = vert->pos;
			verts->push_back(pos.x());
			verts->push_back(pos.y());
			verts->push_back(pos.z());
			vFLAGs->push_back(vert->getFlag());
		}
	}
	else
	{
		vFLAGs->clear();
		vFLAGs->reserve(vertSet.size());
		for (int i = 0; i < vertMap.size(); i++)
		{
			if (vertMap.at(i)->isPicked)
			{
				cout << "already picked!" << endl;
			}
			vFLAGs->push_back(vertMap.at(i)->getFlag());
		}
	}
}

void HDS_Mesh::exportEdgeVBO(
	ui32s_t* heIBOs, ui32s_t* heIDs, ui16s_t* heFLAGs) const
{
	size_t heSetSize = heSet.size();
	if (heIBOs != nullptr)
	{
		unordered_set<he_t*> visitiedHE;
		visitiedHE.reserve(heSetSize);


		heIBOs->clear();
		heIBOs->reserve(heSetSize);
		heFLAGs->clear();
		heFLAGs->reserve(heSetSize >> 1);

		for (auto he : heSet)
		{
			if (visitiedHE.find(he) == visitiedHE.end())
			{
				visitiedHE.insert(he);
				visitiedHE.insert(he->flip);

				heIBOs->push_back(he->v->index);
				heIBOs->push_back(he->flip->v->index);

				heIDs->push_back(static_cast<uint32_t>(he->index));
				heFLAGs->push_back(he->getFlag());
			}
		}
	}
	else if (heFLAGs != nullptr)
	{
		unordered_set<he_t*> visitiedHE;
		visitiedHE.reserve(heSetSize);

		heFLAGs->clear();
		heFLAGs->reserve(heSetSize >> 1);

		for (auto he : heSet)
		{
			if (visitiedHE.find(he) == visitiedHE.end())
			{
				visitiedHE.insert(he);
				visitiedHE.insert(he->flip);

				heFLAGs->push_back(he->getFlag());
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
		// original face idex, for query
		fIDs->clear();
		fIDs->reserve(fSetSize * 2);
		fFLAGs->clear();
		fFLAGs->reserve(fSetSize * 2);
		for (auto face : faceSet)
		{
			if (face->isCutFace)
			{
				continue;
			}
			ui32s_t vid_array;
			auto fid = static_cast<uint32_t>(face->index);
			uint16_t flag = face->getFlag();
			auto he = face->he;
			auto curHE = he;
			do
			{
				vid_array.push_back(curHE->v->index);
				curHE = curHE->next;
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
				if (!face->isBridger)
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
				if (!face->isBridger)
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
			if (face->isCutFace)
			{
				continue;
			}
			uint16_t flag = face->getFlag();
			auto he = face->he;
			auto curHE = he;
			size_t vidCount = 0;
			do
			{
				vidCount++;
				curHE = curHE->next;
			} while (curHE != he);
			fFLAGs->insert(fFLAGs->end(), vidCount - 2, flag);
		}
	}
}

void HDS_Mesh::addHalfEdge(he_t* he)
{
	heSet.insert(he);
	heMap.insert(make_pair(he->index, he));
}

void HDS_Mesh::addVertex(vert_t* vert)
{
	vertSet.insert(vert);
	vertMap.insert(make_pair(vert->index, vert));
}

void HDS_Mesh::addFace(face_t* face)
{
	faceSet.insert(face);
	faceMap.insert(make_pair(face->index, face));
}

void HDS_Mesh::deleteFace(face_t* face)
{
	faceSet.erase(face);
	faceMap.erase(face->index);
	//delete face;
}

void HDS_Mesh::deleteHalfEdge(he_t* edge)
{
	heSet.erase(edge);
	heMap.erase(edge->index);
	//delete edge;
}

vector<HDS_Mesh::face_t *> HDS_Mesh::incidentFaces(vert_t *v)
{
	he_t *he = v->he;
	he_t *curHe = he;
	vector<face_t*> faces;
	do {
		faces.push_back(curHe->f);
		curHe = curHe->flip->next;
	} while( curHe != he );
	return faces;
}

// all outgoing half edges of vertex v
vector<HDS_Mesh::he_t *> HDS_Mesh::incidentEdges(vert_t *v)
{
	he_t *he = v->he;
	he_t *curHe = he;
	vector<he_t*> hes;
	do {
		hes.push_back(curHe);
		curHe = curHe->flip->next;
	} while( curHe != he );
	return hes;
}

vector<HDS_Mesh::face_t *> HDS_Mesh::incidentFaces(HDS_Mesh::face_t *f)
{
	he_t *he = f->he;
	he_t *curHe = he;
	vector<face_t*> faces;
	do {
		faces.push_back(curHe->flip->f);
		curHe = curHe->next;
	} while( curHe != he );
	return faces;
}

HDS_Mesh::he_t* HDS_Mesh::incidentEdge(HDS_Mesh::face_t *f1, HDS_Mesh::face_t *f2)
{
	if(f1 == f2) return nullptr;
	he_t *he = f1->he;
	he_t *curHe = he;
	do {
		if(curHe->flip->f == f2)
			return curHe;
		curHe = curHe->next;
	} while( curHe != he );
	return nullptr;
}

HDS_Mesh::he_t* HDS_Mesh::incidentEdge(HDS_Mesh::vert_t *v1, HDS_Mesh::vert_t *v2)
{
	if(v1 == v2) return nullptr;
	if(v1->he == nullptr || v2->he == nullptr) return nullptr;

	he_t * he = v1->he;
	he_t * curHE = he;
	do {
		if(curHE->flip->v == v2)
			return curHE;
		curHE = curHE->flip->next;
	}while (curHE != he);
	return nullptr;
}

HDS_Mesh::he_t * HDS_Mesh::insertEdge(HDS_Mesh::vert_t* v1, HDS_Mesh::vert_t* v2, HDS_Mesh::he_t* he1, HDS_Mesh::he_t* he2)
{
	bool v1_isNew = false, v2_isNew = false;

	if (v1->he == nullptr) v1_isNew = true;
	if (v2->he == nullptr) v2_isNew = true;

	he_t* he = new he_t;
	he_t* he_flip = new he_t;
	he->setFlip(he_flip);

	//link edge and vertices
	he->v = v1;
	he_flip->v = v2;
	if (v1_isNew) v1->he = he;
	if (v2_isNew) v2->he = he_flip;

	//link edge loop
	he->next = he_flip;
	he->prev = he_flip;
	he_flip->next = he;
	he_flip->prev = he;

	if(!v1_isNew) {
		he_t* prevHE, *nextHE;
		if (he1 != nullptr){
			nextHE = he1->next;
			prevHE = he1;
		}else{
			nextHE = v1->he->flip->next;
			prevHE = v1->he->flip;
		}
		he_flip->next = nextHE;
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
		he_flip->prev = prevHE;
		prevHE->next = he_flip;

	}
	return he;
}

template <typename T>
void HDS_Mesh::flipSelectionState(hdsid_t idx, unordered_map<hdsid_t, T> &m) {
	auto it = m.find(idx);

	if( it != m.end() ) {
		it->second->setPicked( !it->second->isPicked );
	}
}

void HDS_Mesh::selectFace(hdsid_t idx)
{
	flipSelectionState(idx, faceMap);
}

void HDS_Mesh::selectEdge(hdsid_t idx)
{
	flipSelectionState(idx, heMap);
}

void HDS_Mesh::selectVertex(hdsid_t idx)
{
	flipSelectionState(idx, vertMap);
}

unordered_set<HDS_Mesh::vert_t*> HDS_Mesh::getSelectedVertices()
{
	unordered_set<vert_t*> pickedVerts;
	for(auto v: vertSet) {
		if (v->isPicked) {
			pickedVerts.insert(v);
		}
	}
	return pickedVerts;
}

unordered_set<HDS_Mesh::he_t*> HDS_Mesh::getSelectedEdges()
{
	unordered_set<he_t*> pickedHEs;
	unordered_set<he_t*> heSetTmp = heSet;
	for(auto he: heSetTmp) {
		if (he->isPicked) {
			pickedHEs.insert(he);
			heSetTmp.erase(he);
			heSetTmp.erase(he->flip);
		}
	}
	return pickedHEs;
}

unordered_set<HDS_Mesh::he_t*> HDS_Mesh::getSelectedHalfEdges()
{
	unordered_set<he_t*> pickedHEs;
	for(auto he: heSet) {
		if (he->isPicked) {
			pickedHEs.insert(he);
		}
	}
	return pickedHEs;
}

unordered_set<HDS_Mesh::face_t*> HDS_Mesh::getSelectedFaces()
{
	unordered_set<face_t*> pickedFaces;
	for(auto f: faceSet) {
		if (f->isPicked) {
			pickedFaces.insert(f);
		}
	}
	return pickedFaces;
}


unordered_set<HDS_Mesh::vert_t*> HDS_Mesh::getReebPoints(const doubles_t &funcval, const QVector3D &normdir)
{
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
	auto isReebPoint = [&](vert_t* v) {


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




			auto neighbors = v->neighbors();

			// if all neighbors have smaller z-values
			bool allSmaller = std::all_of(neighbors.begin(), neighbors.end(), [&](vert_t* n) {

					//    cout<<"moorseFunc(n"<<n->index<<", a, b, c) = "<<moorseFunc(n, a, b, c)<<endl;
					//    cout<<"moorseFunc(v"<<v->index<<", a, b, c) = "<<moorseFunc(v, a, b, c)<<endl;
					return moorseFunc(n, a, b, c) > moorseFunc(v, a, b, c);

		});

			// if all neighbors have larger z-values
			bool allLarger = std::all_of(neighbors.begin(), neighbors.end(), [&](vert_t* n) {
					return moorseFunc(n, a, b, c) < moorseFunc(v, a, b, c);
		});
			//   cout<<" moorseFunc("<<v->index<<", a, b, c) = "<< moorseFunc(v, a, b, c);
			// if this is a saddle point
			bool isSaddle = false;
			doubles_t diffs;

			if(!allSmaller&&!allLarger)                 //later added;
			{

				for (auto n : neighbors) {
					double st= moorseFunc(n, a, b, c) - moorseFunc(v, a, b, c);
					//     if(abs(st)<1e-5) st=0;                                             //later changed;
					diffs.push_back(st);
					//   diffs[n->index]=st;
					if(v->index==6||v->index==7||v->index==5||v->index==8||v->index==100){
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
					v->rtype = HDS_Vertex::Saddle;
					v->sdegree = (ngroups - 2) / 2;
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
				v->rtype = HDS_Vertex::Minimum;
				//       cout << "Minimum: " << v->index << ",moorseFunc(v, a, b, c) =  " << moorseFunc(v, a, b, c) << endl;
				//for (auto neighbor : neighbors) {
				//  cout << moorseFunc(neighbor, a, b, c) << " ";
				//}
				//cout << endl;
				s2+=1;
				//    cout<<"minimum  !!!!!!!!!"<<s2<<endl;
			}


			if (allLarger) {
				v->rtype = HDS_Vertex::Maximum;
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
				v->rtype = HDS_Vertex::Regular;
				return false;
			}
		}
		if(s1==3)s11+=v->sdegree;
		if(s2==3)s22+=1;
		if(s3==3)s33+=1;
		cout<<"maximum = "<<s33<<endl;
		cout<<"minimum = "<<s22<<endl;

		cout<<"saddle = "<< s11<<endl;
		return true;
	};

	return Utils::filter_set(vertSet, isReebPoint);

}

void HDS_Mesh::colorVertices(const doubles_t &val)
{
#if 1
	for (int i = 0; i < vertSet.size(); ++i) {
		vertMap[i]->colorVal = val[i];
	}

#else
	int i = 0;
	for (auto v : vertSet) {
		v->colorVal = val[i++];
	}
#endif
}

void HDS_Mesh::save(const string &filename)
{
	// the vertices and faces are saved in the same order they are loaded in
	stringstream ss;

	// save the vertices first
	for (int i = 0; i < vertMap.size(); ++i) {
		auto v = vertMap[i];
		ss << "v " << v->pos.x() << ' ' << v->pos.y() << ' ' << v->pos.z() << endl;
	}

	// save the faces
	for (int i = 0; i < faceMap.size(); ++i) {
		auto corners = faceMap[i]->corners();
		ss << "f ";
		for (auto v : corners) {
			ss << v->index + 1 << ' ';
		}
		ss << endl;
	}

	ofstream fout(filename);
	fout << ss.str();
	fout.close();
}
