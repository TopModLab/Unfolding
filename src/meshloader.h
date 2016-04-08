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
	size_t size;
	ui32s_t v, uv, n;
	void push_back(uint32_t* ids)
	{
		v.push_back(ids[0] - 1);
		if (ids[1] > 0) uv.push_back(ids[1] - 1);
		if (ids[2] > 0) uv.push_back(ids[2] - 1);
	}
};

class MeshLoader
{
public:
	typedef QVector3D vert_t;
	/*struct face_t {
		face_t() {
			// at least 3 vertices
			v.reserve(8);
			n.reserve(8);
			t.reserve(8);
		}
		vector<int> v, n, t;
		QVector3D normal;
	};*/
	/*typedef QVector2D texcoord_t;
	typedef QVector3D norm_t;*/

	virtual bool load(const string &filename) = 0;

	/*const vector<vert_t>& getVerts() const { return verts; }
	const vector<face_t>& getFaces() const { return faces; }
	const vector<norm_t>& getNormals() const { return normals; }
	const vector<texcoord_t>& getTexcoords() const { return texcoords; }*/
	const floats_t& getVerts() const { return vertices; }
	const vector<PolyIndex*>& getFaces() const { return polys; }
	const floats_t& getNormals() const { return normals; }
	const floats_t& getTexcoords() const { return uvs; }

protected:

	bool triangulated;
	bool hasVertexTexCoord;
	bool hasVertexNormal;
	/*vector<vert_t> verts;
	vector<face_t> faces;
	vector<texcoord_t> texcoords;
	vector<norm_t> normals;*/


	floats_t vertices;
	floats_t uvs;
	floats_t normals;
	vector<PolyIndex*> polys;

protected:
	void clear();
	void triangulate();
	void estimateNormals();
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
