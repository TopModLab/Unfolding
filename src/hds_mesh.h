#ifndef HDS_MESH_H
#define HDS_MESH_H

#include <QtGui/QVector3D>

#include "common.h"
#include "hds_halfedge.h"
#include "hds_vertex.h"
#include "hds_face.h"

#include "colormap.h"

#include "BBox.h"

class HDS_Mesh
{
public:
	typedef QVector3D point_t;
	typedef HDS_Face face_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_HalfEdge he_t;


	enum ElementType {
		Face = 0,
		Edge,
		Vertex
	};

	HDS_Mesh();
	HDS_Mesh(const HDS_Mesh& other);
	~HDS_Mesh();

	HDS_Mesh operator=(const HDS_Mesh& rhs);
	void updateSortedFaces();
	void clearSortedFaces();

	//////////////////////////////////////////////////////////////////////////
	void updatePieceSet();
	//////////////////////////////////////////////////////////////////////////
	void printInfo(const string &msg = "");
	void printMesh(const string &msg = "");
	void releaseMesh();

	void setMesh(const vector<face_t*> &faces,
					const vector<vert_t*> &verts,
					const vector<he_t*> &hes);


	unordered_set<vert_t*> getReebPoints(const vector<double> &val = vector<double>(), const QVector3D &normdir = QVector3D(0, 0, 1));

	unordered_set<vert_t*> getSelectedVertices();
	unordered_set<he_t*> getSelectedEdges(); //use one half edge to represent an edge
	unordered_set<he_t*> getSelectedHalfEdges(); //return all ispicked half edges
	unordered_set<face_t*> getSelectedFaces();

	void colorVertices(const vector<double> &val);

	void draw(ColorMap cmap);
	void drawFaceIndices();
	void drawVertexIndices();
	void drawEdgeIndices();
	void flipShowEdges();
	void flipShowFaces();
	void flipShowVertices();
	void flipShowNormals();

	const unordered_set<he_t*>& halfedges() const { return heSet; }
	unordered_set<he_t*>& halfedges() { return heSet; }
	const unordered_set<face_t*>& faces() const { return faceSet; }
	unordered_set<face_t*>& faces() { return faceSet; }
	const unordered_set<vert_t*>& verts() const { return vertSet; }
	unordered_set<vert_t*>& verts() { return vertSet; }

	void addHalfEdge(he_t*);
	void addVertex(vert_t*);
	void addFace(face_t*);
	void deleteFace(face_t*);
	void deleteHalfEdge(he_t*);

	vector<face_t *> incidentFaces(vert_t *v);
	vector<he_t *> incidentEdges(vert_t *v);
	vector<face_t *> incidentFaces(face_t *f);

	he_t* incidentEdge(face_t *f1, face_t *f2);
	he_t* incidentEdge(vert_t *v1, vert_t *v2);

	void linkToCutFace(he_t* he, face_t* cutFace);
	face_t * bridging(he_t* h1, he_t* h2, face_t* cutFace); //create a bridge between two boundary half edges
	he_t* insertEdge(vert_t* v1, vert_t* v2);

	template <typename T>
	void flipSelectionState(int idx, unordered_map<int, T> &m);
	void selectFace(int idx);
	void selectEdge(int idx);
	void selectVertex(int idx);

	void validate();

	void save(const string &filename);

private:
	bool validateVertex(vert_t *v);
	bool validateFace(face_t *f);
	bool validateEdge(he_t *e);

protected:
	friend class ReebGraph;
	friend class MeshCutter;
	friend class MeshViewer;
	friend class MeshManager;
	friend class MeshUnfolder;
	friend class MeshSmoother;
	friend class MeshExtender;
	friend class MeshHollower;
	friend class MeshBinder;
	friend class MeshIterator;
	friend class MeshConnector;
private:
	unordered_set<he_t*> heSet;
	unordered_set<face_t*> faceSet;
	unordered_set<vert_t*> vertSet;

	vector<face_t*> sortedFaces;

	unordered_map<int, he_t*> heMap;
	unordered_map<int, face_t*> faceMap;
	unordered_map<int, vert_t*> vertMap;

	// pieces information
	set<set<int>> pieceSet;
private:
	bool isHollowed;
	bool showFace, showEdge, showVert, showNormals;
};

inline ostream& operator<<(ostream &os, const HDS_Vertex& v) {
	os << v.index
		<< ": (" << v.pos.x() << ", " << v.pos.y() << ", " << v.pos.z() << ")"
		<< "\t"
		<< v.he;
	return os;
}

inline ostream& operator<<(ostream &os, const HDS_HalfEdge& e) {
	os << e.index << "::"

		<< " prev: " << e.prev->index
		<< " next: " << e.next->index
		<< " flip: " << e.flip->index
		<< " v: " << e.v->index
		<< " f:" << e.f->index;
	return os;
}

inline ostream& operator<<(ostream &os, const HDS_Face& f) {
	os << "face #" << f.index << " " << f.n << " cut: " << f.isCutFace << endl;
	HDS_HalfEdge *he = f.he;
	HDS_HalfEdge *curHE = he;
	do {
		os << "(" << curHE->index << ", " << curHE->v->index << ") ";
		curHE = curHE->next;
	} while( curHE != he );
	return os;
}

#endif // HDS_MESH_H
