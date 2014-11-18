// BaseModel.h: interface for the CBaseModel class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "Point3d.h"
class CBaseModel  
{
public:	
	CBaseModel(const string& filename);
public:
	struct CFace
	{
		int verts[3];
		CFace(){}
		CFace(int x, int y, int z)
		{
			verts[0] = x;
			verts[1] = y;
			verts[2] = z;
		}
		int& operator[](int index)
		{
			return verts[index];
		}
		int operator[](int index) const
		{
			return verts[index];
		} 
		bool operator==(const CFace& f)const
		{
			int a[3];
			int b[3];
			a[0]=verts[0];a[1]=verts[1];a[2]=verts[2];
			b[0] = f[0];b[1]=f[1];b[2]=f[2];
			sort(a,a+3);
			sort(b,b+3);
			if( a[0] != b[0] || a[1] != b[1] || a[2] != b[2] ) return false;
			return true;
		}
	};
	
	
protected:	
	void FastReadObjFile(const string& filename);
	void ReadMFile(const string& filename);
	void ReadFile(const string& filename);
	void ReadObjFile(const string& filename);
	void ReadOffFile(const string& filename);
public:
	void AdjustScaleAndComputeNormalsToVerts();
	void SaveMFile(const string& filename) const;
	void SaveOffFile(const string& filename) const;
	void SaveObjFile(const string& filename) const;
	void FastSaveObjFile(const string& filename) const;

	void RenderWithColor(vector<CPoint3D> colorsOfTriangle );

	inline int GetNumOfVerts() const;
	inline int GetNumOfFaces() const;
	void LoadModel();
	string GetFileName() const;
	string GetFullPathAndFileName() const;
	inline const CPoint3D& Vert(int vertIndex) const;
	inline const CPoint3D& Normal(int vertIndex) const;
	inline const CFace& Face(int faceIndex) const;
	inline bool HasBeenLoad() const;
	vector<CPoint3D> m_Verts;
	vector<CFace> m_Faces;
protected:
	vector<CPoint3D> m_NormalsToVerts;
public:
	bool m_fBeLoaded;
	string m_filename;
	CPoint3D m_center;
	double m_scale;
	CPoint3D m_ptUp;
	CPoint3D m_ptDown;
};

int CBaseModel::GetNumOfVerts() const
{
	return (int)m_Verts.size();
}

int CBaseModel::GetNumOfFaces() const
{
	return (int)m_Faces.size();
}

const CPoint3D& CBaseModel::Vert(int vertIndex) const
{
	return m_Verts[vertIndex];
}

const CPoint3D& CBaseModel::Normal(int vertIndex) const
{
	return m_NormalsToVerts[vertIndex];
}

const CBaseModel::CFace& CBaseModel::Face(int faceIndex) const
{
	return m_Faces[faceIndex];
}

bool CBaseModel::HasBeenLoad() const
{
	return m_fBeLoaded;
}

