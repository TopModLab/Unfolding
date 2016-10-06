#include "HDS/hds_mesh.h"
#include "Utils/mathutils.h"
#include "Utils/utils.h"
//#include "UI/MeshViewer.h"
#ifdef OPENGL_LEGACY
#include <GL/glu.h>
#include "UI/glutilsLegacy.h"
#else
#include "UI/glutils.h"
#endif
#include<iostream>
using namespace std;

HDS_Mesh::HDS_Mesh()
	: processType(HALFEDGE_PROC)
#ifdef OPENGL_LEGACY
	, showComponent(SHOW_FACE | SHOW_EDGE)
	, showVert(true), showEdge(true)
	, showFace(true), showNormals(false)
#endif
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
#ifdef OPENGL_LEGACY
	, showComponent(SHOW_FACE | SHOW_EDGE)
	, showVert(true), showEdge(true)
	, showFace(true), showNormals(false)
#endif
{
}

HDS_Mesh::HDS_Mesh(const HDS_Mesh &other)
	: vertSet(other.vertSet)
	, heSet(other.heSet)
	, faceSet(other.faceSet)
	, processType(other.processType)
	, pieceSet(other.pieceSet)
#ifdef OPENGL_LEGACY
	, showFace(other.showFace), showEdge(other.showEdge)
	, showVert(other.showVert), showNormals(other.showNormals)
#endif
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

#if 0 // Legacy code
void HDS_Mesh::updateSortedFaces()
{
	// create the sorted face set
	cout<<"updating sorted faces"<<endl;
	sortedFaces.assign(faceSet.begin(), faceSet.end());
	std::sort(sortedFaces.begin(), sortedFaces.end(), [](const face_t *fa, const face_t *fb) {
		auto ca = fa->corners();
		auto cb = fb->corners();
		Float minZa = 1e9, minZb = 1e9;
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
#endif


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
	//for (auto f : faceSet)
	{
		//auto &f = faceSet[i];
		// If f has not been visited yet
		// Add to selected faces
		if (!visitedFaces[fid])
		//if (visitedFaces.find(f.index) == visitedFaces.end())
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

bool HDS_Mesh::validateEdge(const he_t &e)
{
	//if( heMap.find(e->index) == heMap.end() ){cout<<"heMap invalid"<<endl; return false;}
	//if( e.index >= heSet.size()) {cout<<"he index out of range"<<endl; return false;}
	if (e.flip()->flip() != &e) { cout << "flip invalid" << endl; return false; }
	if (e.next()->prev() != &e) { cout << "next invalid" << endl; return false; }
	if (e.prev()->next() != &e) { cout << "prev invalid" << endl; return false; }
	if (e.fid < 0) { cout << "f invalid" << endl; return false; }
	if (e.vid < 0) { cout << "v invalid" << endl; return false; }
	//if( e.f->index >= faceSet.size() || e.f != &faceSet[e.f->index] ){cout<<"->f invalid"<<endl; return false;}
	//if( e.v->index >= vertSet.size() || e.v != &vertSet[e.v->index] ) {cout<<"->v invalid"<<endl;return false;}
	return true;
}

bool HDS_Mesh::validateFace(const face_t &f)
{
	if( f.index >= faceSet.size() || &f != &faceSet[f.index] ) {
        cout<<"cant find in face map"<<endl;
		return false;
	}
	int maxEdges = 1000;
	he_t *he = &heSet[f.heid];
	he_t *curHe = he;
	int edgeCount = 0;
	do {
		curHe = curHe->next();
		++edgeCount;
		if (edgeCount > maxEdges)
		{
            cout<<"edge count error"<<endl;
			return false;
		}
	} while( curHe != he );
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
		curHE = curHE->flip()->next();
		++edgeCount;
		if (edgeCount > maxEdges) return false;
	} while(curHE != he);
	return true;
}

bool HDS_Mesh::validate()
{
	bool validated = true;
	// verify that the mesh has good topology, ie has loop
	for (int vid = 0; vid < vertSet.size(); vid++)
	{
		if (!validateVertex(vid)) {
			cout << "vertex #" << vid << " is invalid." << endl;
			validated = false;
		}
	}
	for( auto f : faceSet ) {
		if( !validateFace(f) ) {
			cout << "face #" << f.index << " is invalid." << endl;
			validated = false;
		}
	}
	for( auto e : heSet ) {
		if( !validateEdge(e) ) {
			cout << "half edge #" << e.index << " is invalid." << endl;
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
		//if (f->isCutFace)
		{
			cout << f << endl;
		}
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
			if (f->isCutFace) continue; //invisible face
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
			else
			{
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

		Float r, g, b;
		encodeIndex<Float>(v->index, r, g, b);
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

		Float r, g, b;
		encodeIndex<Float>(e->index, r, g, b);
		glLineWidth(2.0);
		GLUtils::drawLine(e->v->pos, en->v->pos, QColor::fromRgbF(r, g, b));
	}
#endif
}

void HDS_Mesh::drawFaceIndices()
{
#ifdef OPENGL_LEGACY
	for (auto &f : faceSet) {
		Float r, g, b;
		encodeIndex<Float>(f->index, r, g, b);
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

void HDS_Mesh::exportSelection(ui32q_t* selVTX, ui32q_t* selHE, ui32q_t* selFACE)
{
	if (selVTX != nullptr)
	{
		for (auto v : vertSet)
		{
			if (v.isPicked) selVTX->push(v.index);
		}
	}
	if (selHE != nullptr)
	{
		for (auto he : heSet)
		{
			if (he.isPicked) selHE->push(he.index);
		}
	}
	if (selFACE != nullptr)
	{
		for (auto f : faceSet)
		{
			if (f.isPicked) selFACE->push(f.index);
		}
	}
}

vector<hdsid_t> HDS_Mesh::faceCorners(hdsid_t fid) const
{
	vector<hdsid_t> corners;

	auto he = &heSet[faceSet[fid].heid];
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
	ProcQueue.push(inFaceId);
	// Input face should be marked as visited
	visitedFaces[inFaceId] = true;
	while (!ProcQueue.empty())
	{
		auto cur_fid = ProcQueue.front();
		ProcQueue.pop();

		// If CutFace is not the last face,
		// move it to the end of the queue
		// TODO: Potential issue: current piece can have multiple CutFace
		if (faceSet[cur_fid].isCutFace && !ProcQueue.empty())
		{
			ProcQueue.push(cur_fid);
			continue;
		}
		// Otherwise, add to result
		else
		{
			retFaceSet.push_back(cur_fid);
		}

		// Loop face and record all unvisited face into queue
		auto fhe = heFromFace(cur_fid);
		auto curHE = fhe;
		do {
			auto adj_fid = curHE->flip()->fid;
			curHE = curHE->next();
			if (!visitedFaces[adj_fid])
			{
				visitedFaces[adj_fid] = true;
				ProcQueue.push(adj_fid);
			}
		} while (curHE != fhe);
	}
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

/*
void HDS_Mesh::deleteFace(face_t face)
{
	faceSet.erase(faceSet.begin() + face.index);
	//faceMap.erase(face->index);
	//delete face;
}

void HDS_Mesh::deleteHalfEdge(he_t edge)
{
	heSet.erase(heSet.begin() + edge.index);
	//heMap.erase(edge->index);
	//delete edge;
}*/

vector<HDS_Face*> HDS_Mesh::incidentFaces(vert_t *v)
{
	vector<face_t*> faces;

	he_t *he = &heSet[v->heid];
	he_t *curHe = he;
	do {
		faces.push_back(&faceSet[curHe->fid]);
		curHe = curHe->flip()->next();
	} while( curHe != he );

	return faces;
}

std::vector<hdsid_t> HDS_Mesh::incidentFaceIDs(hdsid_t fid)
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

// all outgoing half edges of vertex v
vector<HDS_HalfEdge*> HDS_Mesh::incidentEdges(vert_t *v)
{
	he_t *he = &heSet[v->heid];
	he_t *curHe = he;
	vector<he_t*> hes;
	do {
		hes.push_back(curHe);
		curHe = curHe->flip()->next();
	} while( curHe != he );
	return hes;
}

vector<HDS_Face*> HDS_Mesh::incidentFaces(face_t *f)
{
	he_t *he = &heSet[f->heid];
	he_t *curHe = he;
	vector<face_t*> faces;
	do {
		faces.push_back(&faceSet[curHe->flip()->fid]);
		curHe = curHe->next();
	} while( curHe != he );
	return faces;
}

HDS_HalfEdge* HDS_Mesh::incidentEdge(face_t *f1, face_t *f2)
{
	if(f1 == f2) return nullptr;
	he_t *he = &heSet[f1->heid];
	he_t *curHe = he;
	do {
		if(curHe->flip()->fid == f2->index)
			return curHe;
		curHe = curHe->next();
	} while( curHe != he );

	return nullptr;
}

HDS_HalfEdge* HDS_Mesh::incidentEdge(vert_t *v1, vert_t *v2)
{
	if(v1 == v2) return nullptr;
	if(v1->heid == sInvalidHDS || v2->heid == sInvalidHDS) return nullptr;

	he_t * he = &heSet[v1->heid];
	he_t * curHE = he;
	do {
		if(curHE->flip()->vid == v2->index)
			return curHE;
		curHE = curHE->flip()->next();
	} while (curHE != he);

	return nullptr;
}

HDS_HalfEdge* HDS_Mesh::insertEdge(
	vector<he_t> &edges, vert_t* v1, vert_t* v2, he_t* he1, he_t* he2)
{
#ifdef USE_LEGACY_FACTORY
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
#endif
	return nullptr;
}

/*
template <typename T>
void HDS_Mesh::flipSelectionState(hdsid_t idx, unordered_map<hdsid_t, T> &m) {
	auto it = m.find(idx);

	if( it != m.end() ) {
		it->second->setPicked( !it->second->isPicked );
	}
}*/

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

void HDS_Mesh::save(const string &filename)
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
