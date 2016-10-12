#ifndef HDS_MESH_H
#define HDS_MESH_H

#include "HDS/HDS_HalfEdge.h"
#include "HDS/HDS_Vertex.h"
#include "HDS/HDS_Face.h"

#include "UI/colormap.h"

#include "GeomUtils/BBox.h"

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
		WOVEN_PROC,
		ORIGAMI_PROC
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
	HDS_Mesh(const HDS_Mesh &other);
	~HDS_Mesh() {}

	// Reset UID in each component
	// Mask(Bitwise Operation): face|edge|vertex
	//    e.g. All(111==3), Vertex Only(001==1), Vertex+Edge(011==3)
	static void resetIndex(uint8_t reset_mask = 7) {
		if (reset_mask & 1) vert_t::resetIndex();
		if (reset_mask & 2) he_t::resetIndex();
		if (reset_mask & 4) face_t::resetIndex();
	}

	void matchIndexToSize() {
		vert_t::matchIndexToSize(vertSet.size());
		he_t::matchIndexToSize(heSet.size());
		face_t::matchIndexToSize(faceSet.size());

	}
	void updatePieceSet();	/* Find linked faces and store in pieceSet */
	
	void printInfo(const string &msg = "");
	void printMesh(const string &msg = "");
	void releaseMesh();

	void setMesh(vector<face_t> &&faces,
				 vector<vert_t> &&verts,
				 vector<he_t>   &&hes);


	vector<vert_t*> getReebPoints(const doubles_t &val = doubles_t(), const QVector3D &normdir = QVector3D(0, 0, 1));

	vector<vert_t*> getSelectedVertices();
	vector<he_t*>   getSelectedEdges(); //use one half edge to represent an edge
	vector<he_t*>   getSelectedHalfEdges(); //return all ispicked half edges
	vector<face_t*> getSelectedFaces();

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
	//////////////////////////////////////////////////////////////////////////
	// Compute mesh properties
	vector<QVector3D> allVertNormal() const;
	QVector3D edgeVector(hdsid_t heid) const;
	QVector3D edgeVector(const he_t &he) const;
	QVector3D faceCenter(hdsid_t fid) const;
	QVector3D faceNormal(hdsid_t fid) const;
	vector<hdsid_t> faceCorners(hdsid_t fid) const;

	he_t* heFromFace(hdsid_t fid) { return &heSet[faceSet[fid].heid]; }
	he_t* heFromVert(hdsid_t vid) { return &heSet[vertSet[vid].heid]; }
	vert_t* vertFromHe(hdsid_t heid) { return &vertSet[heSet[heid].vid]; }
	face_t* faceFromHe(hdsid_t heid) { return &faceSet[heSet[heid].fid]; }

	const he_t* heFromFace(hdsid_t fid) const { return &heSet[faceSet[fid].heid]; }
	const he_t* heFromVert(hdsid_t vid) const { return &heSet[vertSet[vid].heid]; }
	const vert_t* vertFromHe(hdsid_t heid) const { return &vertSet[heSet[heid].vid]; }
	const face_t* faceFromHe(hdsid_t heid) const { return &faceSet[heSet[heid].fid]; }

	vector<hdsid_t> linkedFaces(hdsid_t fid) const;
	void splitHeFromFlip(hdsid_t heid);

	void addHalfEdge(he_t &he);
	void addVertex(vert_t &v);
	void addFace(face_t &f);

#ifdef USE_LEGACY_FACTORY
    vector<face_t*> incidentFaces(vert_t *v);
    vector<he_t*>   incidentEdges(vert_t *v);
    vector<face_t*> incidentFaces(face_t *f);

    he_t* incidentEdge(face_t *f1, face_t *f2);
    he_t* incidentEdge(vert_t *v1, vert_t *v2);

	static he_t* insertEdge(
		vector<he_t> &edges, vert_t* v1, vert_t* v2,
		he_t* he1 = nullptr, he_t* he2 = nullptr);
#endif // LEGACY_FACTORY

	vector<hdsid_t> incidentFaceIDs(hdsid_t fid);


	void selectFace(hdsid_t idx);
	void selectEdge(hdsid_t idx);
	void selectVertex(hdsid_t idx);

	bool validate();

	void save(const string &filename);

	void setProcessType(uint16_t type) { processType = type; }

private:
	bool validateVertex(hdsid_t vid);
	bool validateFace(hdsid_t fid);
	bool validateEdge(hdsid_t heid);

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
	vector<vector<hdsid_t>> pieceSet;
	unique_ptr<BBox3> bound;

	uint16_t processType;
};

inline ostream& operator<<(ostream &os, const HDS_Vertex& v) {
	os << v.index
		<< ": (" << v.pos.x() << ", " << v.pos.y() << ", " << v.pos.z() << ")"
		<< "\t"
		<< "he index: " << v.heid;
	return os;
}

inline ostream& operator<<(ostream &os, const HDS_HalfEdge& e) {
	os << e.index << "::"

		<< " prev: " << e.prev()->index
		<< " next: " << e.next()->index
		<< " flip: " << e.flip()->index
		<< " v: " << e.vid
		<< " f:" << e.fid;
	return os;
}

inline ostream& operator<<(ostream &os, const HDS_Face& f)
{
	os << "face #" << f.index << " cut: " << f.isCutFace
		<< "\n\tconnected edge: " << f.heID() << endl;

	return os;
}

#endif // HDS_MESH_H
