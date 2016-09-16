#ifndef HDS_MESH_H
#define HDS_MESH_H

#include "HDS/hds_halfedge.h"
#include "HDS/hds_vertex.h"
#include "HDS/hds_face.h"

#include "UI/colormap.h"

#include "BBox.h"

class HDS_Mesh
{
public:
	typedef QVector3D point_t;
	typedef HDS_Face face_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_HalfEdge he_t;

	enum PROCESS_TYPE : uint16_t
	{
		HALFEDGE_PROC,
		QUAD_PROC,
		WINGED_PROC,
		GRS_PROC,
		GES_PROC,
		FBWALK_PROC,
		WOVEN_PROC
	};
	enum SHOW_COMP : uint16_t
	{
		SHOW_NONE = 0,
		SHOW_VERT = 1 << 0,
		SHOW_FACE = 1 << 1,
		SHOW_EDGE = 1 << 2,
		SHOW_NORM = 1 << 3
	};
	/*enum ElementType
	{
		Face = 0,
		Edge,
		Vertex
	};*/

	HDS_Mesh();
	HDS_Mesh(vector<vert_t> &verts,
			 vector<he_t>   &hes,
			 vector<face_t> &faces);
	HDS_Mesh(const HDS_Mesh& other);
	~HDS_Mesh();

	HDS_Mesh operator=(const HDS_Mesh& rhs) = delete;
	//void updateSortedFaces();
	//void clearSortedFaces();

	void updatePieceSet();	/* Find linked faces and store in pieceSet */
	
	void printInfo(const string &msg = "");
	void printMesh(const string &msg = "");
	void releaseMesh();

	void setMesh(vector<face_t> &&faces,
				 vector<vert_t> &&verts,
				 vector<he_t>   &&hes);


	vector<vert_t> getReebPoints(const doubles_t &val = doubles_t(), const QVector3D &normdir = QVector3D(0, 0, 1));

	vector<vert_t> getSelectedVertices();
	vector<he_t*> getSelectedEdges(); //use one half edge to represent an edge
	vector<he_t> getSelectedHalfEdges(); //return all ispicked half edges
	vector<face_t> getSelectedFaces();

	void colorVertices(const doubles_t &val);

	/************************************************************************/
	/* Legacy Drawing Functions                                             */
	/************************************************************************/
	void draw(ColorMap cmap);
	void drawVertexIndices();
	void drawEdgeIndices();
	void drawFaceIndices();

	/************************************************************************/
	/* Modern Rendering Functions                                           */
	/************************************************************************/
	void exportVertVBO(floats_t* verts,	ui16s_t* vFLAGs = nullptr) const;
	void exportEdgeVBO(ui32s_t* heIBOs = nullptr,
		ui32s_t* heIDs = nullptr, ui16s_t* heFLAGs = nullptr) const;
	void exportFaceVBO(ui32s_t* fIBOs = nullptr,
		ui32s_t* fIDs = nullptr, ui16s_t* fFLAGs = nullptr) const;
	using ui32q_t = queue<uint32_t>;
	void exportSelection(ui32q_t* selVTX, ui32q_t* selHE, ui32q_t* selFACE);

	
	vector<he_t>& halfedges() { return heSet; }
	vector<face_t>& faces() { return faceSet; }
	vector<vert_t>& verts() { return vertSet; }

	const vector<he_t>& halfedges() const { return heSet; }
	const vector<face_t>& faces() const { return faceSet; }
	const vector<vert_t>& verts() const { return vertSet; }

	void addHalfEdge(he_t);
	void addVertex(vert_t);
	void addFace(face_t);
	void deleteFace(face_t);
	void deleteHalfEdge(he_t);

    static vector<face_t *> incidentFaces(vert_t *v);
    static vector<he_t *> incidentEdges(vert_t *v);
    static vector<face_t *> incidentFaces(face_t *f);

    static he_t* incidentEdge(face_t *f1, face_t *f2);
    static he_t* incidentEdge(vert_t *v1, vert_t *v2);

	static he_t* insertEdge(
		vector<he_t> &edges, vert_t* v1, vert_t* v2,
		he_t* he1 = nullptr, he_t* he2 = nullptr);

// 	template <typename T>
// 	void flipSelectionState(hdsid_t idx, unordered_map<hdsid_t, T> &m);
	void selectFace(hdsid_t idx);
	void selectEdge(hdsid_t idx);
	void selectVertex(hdsid_t idx);

	bool validate();

	void save(const string &filename);

	void setProcessType(int type){processType = type;}

private:
	bool validateVertex(const vert_t &v);
	bool validateFace(const face_t &f);
	bool validateEdge(const he_t &e);

protected:
	friend class ReebGraph;
	friend class MainWindow;
	friend class MeshCutter;
	friend class MeshViewerLegacy;
	friend class MeshViewer;
	friend class MeshManager;
	friend class MeshUnfolder;
	friend class MeshSmoother;
	friend class MeshExtender;
	friend class MeshRimFace;
	friend class MeshIterator;
	friend class MeshConnector;
private:
	vector<vert_t> vertSet;
	vector<he_t>   heSet;
	vector<face_t> faceSet;

	// pieces information
	vector<set<hdsid_t>> pieceSet;
	unique_ptr<BBox3> bound;
private:
	uint16_t processType;
#ifdef OPENGL_LEGACY
	uint8_t showComponent;
	bool showVert, showEdge, showFace, showNormals;
#endif
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