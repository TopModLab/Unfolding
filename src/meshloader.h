#ifdef _MSC_VER
#pragma once
#pragma warning (disable:4996)
#endif // _MSVC
#ifndef MESHLOADER_H
#define MESHLOADER_H

#include "common.h"
#include <QVector2D>
#include <QVector3D>


struct PolyIndex
{
	PolyIndex(size_t sz = 3) : size(sz)
	{
		v.reserve(sz);
		uv.reserve(sz);
		n.reserve(sz);
	}
	void push_back(uint32_t* ids)
	{
		v.push_back(ids[0] - 1);
		if (ids[1] > 0) uv.push_back(ids[1] - 1);
		if (ids[2] > 0) uv.push_back(ids[2] - 1);
	}

	size_t size;
	ui32s_t v, uv, n;
};

class MeshLoader
{
public:
	MeshLoader() : triangulated(false),
		hasVertexTexCoord(false), hasVertexNormal(false) {}
	virtual bool load(const string &filename) = 0;

	virtual ui32s_t* getTriangulatedIndices() const;
	const doubles_t& getVerts() const { return m_verts; }
	const vector<PolyIndex*>& getFaces() const { return m_polys; }
	const doubles_t& getNormals() const { return m_norms; }
	const doubles_t& getTexcoords() const { return m_uvs; }

protected:
	void clear();
	void triangulate();
	void estimateNormals();

protected:

	bool triangulated;
	bool hasVertexTexCoord;
	bool hasVertexNormal;

	doubles_t m_verts;
	doubles_t m_uvs;
	doubles_t m_norms;
	vector<PolyIndex*> m_polys;

	string m_filename;
};

class PLYLoader : public MeshLoader
{
public:
	virtual bool load(const string& filename);

protected:
	struct vert_t {
		float x, y, z;
		unsigned char r, g, b;
	};

	struct face_t {
		unsigned char nVerts;
		vector<int> verts;
	};
};

class OBJLoader : public MeshLoader
{
public:
	bool load(const string &filename);
private:
	enum index_t : uint8_t
	{
		V = 1 << 0,
		UV = 1 << 1,
		NORM = 1 << 2,
		VT = V | UV,
		VN = V | NORM,
		VTN = V | UV | NORM
	};
	index_t facetype(const char* str, uint32_t* val);
private:
	bool load_from_string(const string &filename);
	bool load_from_file(const string &filename);
};


#endif // MESHLOADER_H
