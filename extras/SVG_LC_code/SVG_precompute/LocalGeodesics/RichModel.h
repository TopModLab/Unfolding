// RichModel.h: interface for the CRichModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RICHOBJMODEL_H__EB74D2F8_BA58_480E_9050_6FBC1C83D98B__INCLUDED_)
#define AFX_RICHOBJMODEL_H__EB74D2F8_BA58_480E_9050_6FBC1C83D98B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BaseModel.h"
#include <cstdlib>
#include <cmath>

class CRichModel : virtual public CBaseModel 
{
public:
	struct CEdge
	{
		int indexOfLeftVert;
		int indexOfRightVert;
		int indexOfOppositeVert;
		int indexOfLeftEdge;
		int indexOfRightEdge;
		int indexOfReverseEdge;
		int indexOfFrontFace;
		double length;
		double xOfPlanarCoordOfOppositeVert;
		double yOfPlanarCoordOfOppositeVert;

		CEdge()
		{		
			indexOfOppositeVert = -1;	//key	
			indexOfLeftEdge = -1;
			indexOfRightEdge = -1;
		}
	};

protected:
	void CreateEdgesFromVertsAndFaces();
	void CollectAndArrangeNeighs();
	void ComputeAnglesAroundVerts();
	void ComputePlanarCoordsOfIncidentVertForEdges();
	void ComputeNumOfHoles();
public:
	//void LoadBunny();
	CRichModel(const string &filename);
	void Preprocess();	
	inline int GetSubindexToVert(int root, int neigh) const;
	inline const CEdge& Edge(int edgeIndex) const;	
	inline const vector<pair<int, double> >& Neigh(int root) const;	
	inline int IncidentVertex(int edgeIndex) const;
	inline double AngleSum(int vertIndex) const;
	inline double Curvature(int vertIndex) const;
	inline double ProportionOnEdgeByImage(int edgeIndex, double x, double y) const;
	inline double ProportionOnEdgeByImageAndPropOnLeftEdge(int edgeIndex, double x, double y, double proportion) const;
	inline double ProportionOnEdgeByImageAndPropOnRightEdge(int edgeIndex, double x, double y, double proportion) const;
	inline double ProportionOnLeftEdgeByImage(int edgeIndex, double x, double y, double proportion) const;
	inline double ProportionOnRightEdgeByImage(int edgeIndex, double x, double y, double proportion) const;
	inline double ProportionOnEdgeByImage(int edgeIndex, double x1, double y1, double x2, double y2) const;
	inline void GetPointByRotatingAround(int edgeIndex, double leftLen, double rightLen, double &xNew, double &yNew) const;
	inline void GetPointByRotatingAroundLeftChildEdge(int edgeIndex, double x, double y, double &xNew, double &yNew) const;	
	inline void GetPointByRotatingAroundRightChildEdge(int edgeIndex, double leftLen, double rightLen, double &xNew, double &yNew) const;	
	double DistanceToIncidentAngle(int edgeIndex, double x, double y) const;
	inline int GetNumOfEdges() const;
	inline int GetNumOfValidDirectedEdges() const;
	inline int GetNumOfTotalUndirectedEdges() const;
	inline int GetNumOfGenera() const;
	inline int GetNumOfIsolated() const;
	inline int GetNumOfComponents() const;
	inline int GetNumOfBoundries() const;
	inline bool IsConvexVert(int index) const;
	inline bool isBoundaryVert(int index) const;
	inline bool IsClosedModel() const;
	inline bool IsExtremeEdge(int edgeIndex) const;
	inline bool IsStartEdge(int edgeIndex) const;	
	inline bool HasBeenProcessed() const;
	inline int GetFirstEdgeIndex(int faceIndex) const;
	inline int GetSecondEdgeIndex(int faceIndex) const;
	inline int GetThirdEdgeIndex(int faceIndex) const;
	inline int GetEdgeIndexFromFace(int faceIndex, int subIndex) const;
	inline int GetEdgeIndexFromTwoVertices(int leftVert, int rightVert) const;
	inline CPoint3D ComputeShiftPoint(int indexOfVert) const;
	inline CPoint3D ComputeShiftPoint(int indexOfVert, double epsilon) const;
	static CPoint3D CombinePointAndNormalTo(const CPoint3D& pt, const CPoint3D& normal);
	static CPoint3D CombineTwoNormalsTo(const CPoint3D& pt1, double coef1, const CPoint3D& pt2, double coef2);	
	double GetGaussCurvature(int vertIndex) const;
protected:
	bool fLocked;
	bool fBePreprocessed;
	int m_nHoles;
	int m_nIsolatedVerts;
	vector<vector<pair<int, double> > > m_NeighsAndAngles;	
	vector<bool> m_FlagsForCheckingConvexVerts;
	vector<CEdge> m_Edges;
};

int CRichModel::IncidentVertex(int edgeIndex) const
{
	return Edge(edgeIndex).indexOfOppositeVert;
}

int CRichModel::GetNumOfValidDirectedEdges() const
{
	return (int)m_Faces.size() * 3;
}

int CRichModel::GetNumOfTotalUndirectedEdges() const
{
	return (int)m_Edges.size() / 2;
}

int CRichModel::GetNumOfGenera() const
{
	return int(GetNumOfTotalUndirectedEdges() - (GetNumOfVerts() - m_nIsolatedVerts) - GetNumOfFaces() - GetNumOfBoundries()) / 2 + 1;
}

int CRichModel::GetNumOfComponents() const
{
	return int(GetNumOfVerts() - m_nIsolatedVerts + GetNumOfFaces() + GetNumOfBoundries() - GetNumOfTotalUndirectedEdges()) / 2; 
}

int CRichModel::GetNumOfBoundries() const
{
	return m_nHoles;
}

bool CRichModel::IsClosedModel() const
{
	return GetNumOfValidDirectedEdges() ==  GetNumOfEdges();
}

int CRichModel::GetNumOfIsolated() const
{
	return m_nIsolatedVerts;
}

int CRichModel::GetNumOfEdges() const
{
	return (int)m_Edges.size();
}

bool CRichModel::isBoundaryVert(int index) const
{
	return IsStartEdge(Neigh(index).front().first);
}

bool CRichModel::IsConvexVert(int index) const
{
	return m_FlagsForCheckingConvexVerts[index];
}

bool CRichModel::IsExtremeEdge(int edgeIndex) const
{
	return Edge(edgeIndex).indexOfOppositeVert == -1;
}

bool CRichModel::IsStartEdge(int edgeIndex) const
{
	return Edge(Edge(edgeIndex).indexOfReverseEdge).indexOfOppositeVert == -1;
}

const CRichModel::CEdge& CRichModel::Edge(int edgeIndex) const
{
	return m_Edges[edgeIndex];
}

const vector<pair<int, double> >& CRichModel::Neigh(int root) const
{
	return m_NeighsAndAngles[root];
}

double CRichModel::ProportionOnEdgeByImageAndPropOnLeftEdge(int edgeIndex, double x, double y, double proportion) const
{
	double x1 = Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * proportion;
	double y1 = Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * proportion;
	return ProportionOnEdgeByImage(edgeIndex, x1, y1, x, y);
}

double CRichModel::ProportionOnEdgeByImageAndPropOnRightEdge(int edgeIndex, double x, double y, double proportion) const
{
	double x1 = Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * (1 - proportion)
		+ Edge(edgeIndex).length * proportion;
	double y1 = Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * (1 - proportion);
	return ProportionOnEdgeByImage(edgeIndex, x1, y1, x, y);
}

double CRichModel::ProportionOnEdgeByImage(int edgeIndex, double x, double y) const
{
	double res = Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * x;
	return res / ((y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert) * Edge(edgeIndex).length);
}

double CRichModel::ProportionOnEdgeByImage(int edgeIndex, double x1, double y1, double x2, double y2) const
{
	double res = x1 * y2 - x2 * y1;
	return res / ((y2 - y1) * Edge(edgeIndex).length);
}

double CRichModel::ProportionOnLeftEdgeByImage(int edgeIndex, double x, double y, double proportion) const
{
	double xBalance = proportion * Edge(edgeIndex).length;
	double res = Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * (x - xBalance);
	return xBalance * y / res;
}

double CRichModel::ProportionOnRightEdgeByImage(int edgeIndex, double x, double y, double proportion) const
{
	double part1 = Edge(edgeIndex).length * y;
	double part2 = proportion * Edge(edgeIndex).length * Edge(edgeIndex).yOfPlanarCoordOfOppositeVert;
	double part3 = Edge(edgeIndex).yOfPlanarCoordOfOppositeVert * x - Edge(edgeIndex).xOfPlanarCoordOfOppositeVert * y;	
	return (part3 + proportion * part1 - part2) / (part3 + part1 - part2);
}

void CRichModel::GetPointByRotatingAround(int edgeIndex, double leftLen, double rightLen, double &xNew, double &yNew) const
{
	xNew = ((leftLen * leftLen - rightLen * rightLen) / Edge(edgeIndex).length + Edge(edgeIndex).length) / 2.0;
    yNew = -sqrt(std::max(leftLen * leftLen - xNew * xNew, 0.0));
}

void CRichModel::GetPointByRotatingAroundLeftChildEdge(int edgeIndex, double x, double y, double &xNew, double &yNew) const
{
	//double leftLen = sqrt(x * x + y * y);
	//double detaX = x - Edge(edgeIndex).xOfPlanarCoordOfOppositeVert;
	//double detaY = y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert;
	//double rightLen = sqrt(detaX * detaX + detaY * detaY);
	//GetPointByRotatingAround(Edge(edgeIndex).indexOfLeftEdge, leftLen, rightLen, xNew, yNew);

	//The followings are also okay:

	double leftX = 0;
	double leftY = 0;
	int reverseEdge = Edge(Edge(edgeIndex).indexOfLeftEdge).indexOfReverseEdge;
	double rightX = Edge(reverseEdge).length - Edge(reverseEdge).xOfPlanarCoordOfOppositeVert;
	double rightY = -Edge(reverseEdge).yOfPlanarCoordOfOppositeVert;

	double detaX = rightX - leftX;
	double detaY = rightY - leftY;
	double scale = abs(detaX) + abs(detaY);
	detaX /= scale;
	detaY /= scale;
	double len = sqrt(detaX * detaX + detaY * detaY);
	double unitX = detaX / len;
	double unitY = detaY / len;
	// |unitX   -unitY|
	// |unitY    unitX|
	xNew = unitX * x - unitY * y + leftX;
	yNew = unitY * x + unitX * y + leftY;
}

void CRichModel::GetPointByRotatingAroundRightChildEdge(int edgeIndex, double x, double y, double &xNew, double &yNew) const
{
	//double detaX = x - Edge(edgeIndex).xOfPlanarCoordOfOppositeVert;
	//double detaY = y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert;
	//double leftLen = sqrt(detaX * detaX + detaY * detaY);
	//detaX = x - Edge(edgeIndex).length;
	//double rightLen = sqrt(detaX * detaX + y * y);
	//GetPointByRotatingAround(Edge(edgeIndex).indexOfRightEdge, leftLen, rightLen, xNew, yNew);

	//The followings are also okay:

	int reverseEdge = Edge(Edge(edgeIndex).indexOfRightEdge).indexOfReverseEdge;
	double rightX = Edge(reverseEdge).length;
	double rightY = 0;
	double leftX = Edge(reverseEdge).length - Edge(reverseEdge).xOfPlanarCoordOfOppositeVert;
	double leftY = -Edge(reverseEdge).yOfPlanarCoordOfOppositeVert;

	double detaX = rightX - leftX;
	double detaY = rightY - leftY;
	double scale = abs(detaX) + abs(detaY);
	detaX /= scale;
	detaY /= scale;
	double len = sqrt(detaX * detaX + detaY * detaY);
	double unitX = detaX / len;
	double unitY = detaY / len;
	// |unitX   -unitY|
	// |unitY    unitX|
	xNew = unitX * x - unitY * y + leftX;
	yNew = unitY * x + unitX * y + leftY;
}


bool CRichModel::HasBeenProcessed() const
{
	return fBePreprocessed;
}

int CRichModel::GetSubindexToVert(int root, int neigh) const
{
	for (int i = 0; i < (int)Neigh(root).size(); ++i)
	{
		if (Edge(Neigh(root)[i].first).indexOfRightVert == neigh)
			return i;
	}
	return -1;
}

CPoint3D CRichModel::ComputeShiftPoint(int indexOfVert) const
{
	return Vert(indexOfVert) + Normal(indexOfVert) * RateOfNormalShift / m_scale;
}

CPoint3D CRichModel::ComputeShiftPoint(int indexOfVert, double epsilon) const
{
	return Vert(indexOfVert) +  Normal(indexOfVert) * epsilon;
}

double CRichModel::AngleSum(int vertIndex) const
{
	double angleSum(0);
	for (int j = 0; j < (int)m_NeighsAndAngles[vertIndex].size(); ++j)
	{		
		angleSum += m_NeighsAndAngles[vertIndex][j].second;			
	}
	return angleSum;
}

int CRichModel::GetFirstEdgeIndex(int faceIndex) const
{
	int root = m_Faces[faceIndex][0];
	int subIndex = GetSubindexToVert(root, m_Faces[faceIndex][1]);
	return Neigh(root)[subIndex].first;
}
int CRichModel::GetSecondEdgeIndex(int faceIndex) const
{
	int root = m_Faces[faceIndex][1];
	int subIndex = GetSubindexToVert(root, m_Faces[faceIndex][2]);
	return Neigh(root)[subIndex].first;
}
int CRichModel::GetThirdEdgeIndex(int faceIndex) const
{
	int root = m_Faces[faceIndex][2];
	int subIndex = GetSubindexToVert(root, m_Faces[faceIndex][0]);
	return Neigh(root)[subIndex].first;
}
int CRichModel::GetEdgeIndexFromFace(int faceIndex, int subIndex) const
{
	if (subIndex == 0)
	{
		int edgeIndex = GetFirstEdgeIndex(faceIndex);
		assert (Edge(edgeIndex).indexOfFrontFace == faceIndex);
		return edgeIndex;
	}
	if (subIndex == 1)
	{
		int edgeIndex = GetSecondEdgeIndex(faceIndex);
		assert (Edge(edgeIndex).indexOfFrontFace == faceIndex);
		return edgeIndex;
	}
	if (subIndex == 2)
	{
		int edgeIndex = GetThirdEdgeIndex(faceIndex);
		assert (Edge(edgeIndex).indexOfFrontFace == faceIndex);
		return edgeIndex;
	}
	assert(false);
	return -1;
}

int CRichModel::GetEdgeIndexFromTwoVertices(int leftVert, int rightVert) const
{
	int subIndex = GetSubindexToVert(leftVert, rightVert);
	assert (subIndex != -1);
	return Neigh(leftVert)[subIndex].first;
}

double CRichModel::Curvature(int vertIndex) const
{	
	double meanCurvature(0);
	for (int i = 0; i < (int)Neigh(vertIndex).size(); ++i)
	{
		if (!IsExtremeEdge(Neigh(vertIndex)[i].first) && !IsStartEdge(Neigh(vertIndex)[i].first))
		{
			CPoint3D detaVec = Vert(Edge(Neigh(vertIndex)[i].first).indexOfRightVert) - Vert(vertIndex);
			meanCurvature += abs(detaVec ^ Normal(vertIndex)) / (detaVec ^ detaVec) * detaVec.Len();
		}
	}
	meanCurvature /= Neigh(vertIndex).size();
	return meanCurvature;
}
#endif // !defined(AFX_RICHOBJMODEL_H__EB74D2F8_BA58_480E_9050_6FBC1C83D98B__INCLUDED_)

