// RichObjModel.cpp: implementation of the CRichModel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RichModel.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRichModel::CRichModel(const string& filename) : CBaseModel(filename)
{
	fBePreprocessed = false;
	fLocked = false;
}

void CRichModel::CreateEdgesFromVertsAndFaces()
{
	m_Edges.reserve(2 * (GetNumOfVerts() + GetNumOfFaces() - 2));
	map<pair<int, int>, int> pondOfUndeterminedEdges;
	int szFaces = GetNumOfFaces();
	for (int i = 0; i < szFaces; ++i)
	{		
		int threeIndices[3];
		for (int j = 0; j < 3; ++j)
		{
			int post = (j + 1) % 3;
			int pre = (j + 2) % 3;
			
			int leftVert = Face(i)[pre];
			int rightVert = Face(i)[j];

			map<pair<int, int>, int>::const_iterator it = pondOfUndeterminedEdges.find(make_pair(leftVert, rightVert));
			if (it != pondOfUndeterminedEdges.end())
			{
				int posInEdgeList = it->second;
				if (m_Edges[posInEdgeList].indexOfOppositeVert != -1)
				{
					throw "Repeated edges!";
				}
				threeIndices[j] = posInEdgeList;
				m_Edges[posInEdgeList].indexOfOppositeVert = Face(i)[post];
				m_Edges[posInEdgeList].indexOfFrontFace = i;				
			}
			else
			{
				CEdge edge;
				edge.indexOfLeftVert = leftVert;
				edge.indexOfRightVert = rightVert;
				edge.indexOfFrontFace = i;
				edge.indexOfOppositeVert = Face(i)[post];
				edge.indexOfReverseEdge = (int)m_Edges.size() + 1;
				edge.length = (Vert(leftVert) - Vert(rightVert)).Len();
				m_Edges.push_back(edge);
				pondOfUndeterminedEdges[make_pair(leftVert, rightVert)] = threeIndices[j] = (int)m_Edges.size() - 1;

				edge.indexOfLeftVert = rightVert;
				edge.indexOfRightVert = leftVert;
				edge.indexOfReverseEdge = (int)m_Edges.size() - 1;
				edge.indexOfOppositeVert = -1;
				m_Edges.push_back(edge);
				pondOfUndeterminedEdges[make_pair(rightVert, leftVert)] = (int)m_Edges.size() - 1;
			}
		}
		for (int j = 0; j < 3; ++j)
		{
			m_Edges[threeIndices[j]].indexOfLeftEdge = Edge(threeIndices[(j + 2) % 3]).indexOfReverseEdge;
			m_Edges[threeIndices[j]].indexOfRightEdge = Edge(threeIndices[(j + 1) % 3]).indexOfReverseEdge;
		}
	}
	m_Edges.swap(vector<CEdge>(m_Edges));
}

void CRichModel::CollectAndArrangeNeighs()
{
	m_nIsolatedVerts = 0;
	vector<int> sequenceOfDegrees(GetNumOfVerts(), 0);	
	m_NeighsAndAngles.resize(GetNumOfVerts());
	for (int i = 0; i < (int)m_NeighsAndAngles.size(); ++i)
	{
		m_NeighsAndAngles[i].resize(1, make_pair(-1, 0));
	}
	for (int i = 0; i < (int)GetNumOfEdges(); ++i)
	{
		const CEdge& edge = Edge(i);
		++sequenceOfDegrees[edge.indexOfLeftVert];
		int &indexOfStartEdge = m_NeighsAndAngles[edge.indexOfLeftVert][0].first;
		if (indexOfStartEdge == -1 || !IsStartEdge(indexOfStartEdge))
		{
			indexOfStartEdge = i;
		}
		else if (IsStartEdge(i))
		{
			m_NeighsAndAngles[edge.indexOfLeftVert].push_back(make_pair(i, 0));
		}
	}
	for (int i = 0; i < GetNumOfVerts(); ++i)
	{
		if (m_NeighsAndAngles[i][0].first == -1)
		{
			m_NeighsAndAngles[i].clear();
			m_nIsolatedVerts++;	
			continue;
		}
		vector<int> startEdges;
		for (int j = 0; j < (int)Neigh(i).size(); ++j)
		{
			startEdges.push_back(Neigh(i)[j].first);
		}	
		m_NeighsAndAngles[i].resize(sequenceOfDegrees[i], make_pair(0, 0));
		int num(0);
		for (int j = 0; j < (int)startEdges.size(); ++j)
		{
			int curEdge = startEdges[j];			
			while (1)
			{
				m_NeighsAndAngles[i][num].first = curEdge;
				++num;
				if (num >= sequenceOfDegrees[i])
					break;
				if (IsExtremeEdge(curEdge))
					break;
				curEdge = Edge(curEdge).indexOfLeftEdge;
				if (curEdge == startEdges[j])
				{
					break;
				}
			}
		}
		if (num != sequenceOfDegrees[i])
		{
			throw "Complex vertices";
		}
	}
}

void CRichModel::ComputeAnglesAroundVerts()
{	
	m_FlagsForCheckingConvexVerts.resize(GetNumOfVerts());
	for (int i = 0; i < (int)m_NeighsAndAngles.size(); ++i)
	{
		m_NeighsAndAngles[i].resize(Neigh(i).size());
	}
	for (int i = 0; i < (int)m_NeighsAndAngles.size(); ++i)
	{
		double angleSum(0);
		for (int j = 0; j < (int)m_NeighsAndAngles[i].size(); ++j)
		{
			if (IsExtremeEdge(Neigh(i)[j].first))
				m_NeighsAndAngles[i][j].second = 2 * PI + 0.1;
			else
			{
				int next = j + 1;
				if (next >= (int)m_NeighsAndAngles[i].size())
				{
					next = 0;
				}
				double l = Edge(Neigh(i)[j].first).length;
				double r = Edge(Neigh(i)[next].first).length;
				double b = Edge(Edge(Neigh(i)[j].first).indexOfRightEdge).length;				
				m_NeighsAndAngles[i][j].second = acos((l * l + r * r - b * b) / (2 * l * r));
			}
			angleSum += m_NeighsAndAngles[i][j].second;			
		}
		m_FlagsForCheckingConvexVerts[i] = (angleSum < 2 * PI - ToleranceOfConvexAngle);
	}
}

void CRichModel::ComputePlanarCoordsOfIncidentVertForEdges()
{
	for (int i = 0; i < GetNumOfEdges(); ++i)
	{
		if (IsExtremeEdge(i))
			continue;
		double bottom = Edge(i).length;
		double leftLen = Edge(Edge(i).indexOfLeftEdge).length;
		double squareOfLeftLen = leftLen * leftLen;
		double rightLen = Edge(Edge(i).indexOfRightEdge).length;
		double x = (squareOfLeftLen - rightLen * rightLen) / bottom + bottom;
		x /= 2.0;
		m_Edges[i].xOfPlanarCoordOfOppositeVert = x;		
		m_Edges[i].yOfPlanarCoordOfOppositeVert = sqrt(max(0.0, squareOfLeftLen - x * x));
	}
}

void CRichModel::Preprocess()
{
	if (fBePreprocessed)
		return;	
	if (!m_fBeLoaded)
	{
		LoadModel();
	}

	if (!fLocked)
	{
		fLocked = true;
		CreateEdgesFromVertsAndFaces();
		CollectAndArrangeNeighs();	
		ComputeNumOfHoles();
		ComputeAnglesAroundVerts();
		ComputePlanarCoordsOfIncidentVertForEdges();
		fBePreprocessed = true;
		fLocked = false; 		
	}
}

double CRichModel::DistanceToIncidentAngle(int edgeIndex, double x, double y) const
{
	double detaX = x - Edge(edgeIndex).xOfPlanarCoordOfOppositeVert;
	double detaY = y - Edge(edgeIndex).yOfPlanarCoordOfOppositeVert;
	return sqrt(detaX * detaX + detaY * detaY);
}

CPoint3D CRichModel::CombinePointAndNormalTo(const CPoint3D& pt, const CPoint3D& normal)
{
	return pt + normal * RateOfNormalShift;
}

CPoint3D CRichModel::CombineTwoNormalsTo(const CPoint3D& pt1, double coef1, const CPoint3D& pt2, double coef2)
{
	return coef1 * pt1 + coef2 * pt2;
}

void CRichModel::ComputeNumOfHoles()
{
	m_nHoles = 0;
	if (IsClosedModel())
	{
		return;
	}
	set<int> extremeEdges;
	for (int i = 0; i < (int)m_Edges.size(); ++i)
	{
		if (m_Edges[i].indexOfOppositeVert != -1)
			continue;
		extremeEdges.insert(i);
	}		

	while (!extremeEdges.empty())
	{
		++m_nHoles;
		int firstEdge = *extremeEdges.begin();
		int edge = firstEdge;
		do
		{			
			extremeEdges.erase(edge);
			int root = Edge(edge).indexOfRightVert;
			int index = GetSubindexToVert(root, Edge(edge).indexOfLeftVert);
			edge  = Neigh(root)[(index - 1 + (int)Neigh(root).size()) % (int)Neigh(root).size()].first;		
		} while (edge != firstEdge && !extremeEdges.empty());
	}
}

double CRichModel::GetGaussCurvature(int vertIndex) const
{
	double angleSum(0);
	for (int i = 0; i < (int)Neigh(vertIndex).size(); ++i)
	{
		angleSum += Neigh(vertIndex)[i].second;
	}
	return 2 * M_PI - angleSum;
}
