// ICHWithFurtherPriorityQueue.cpp: implementation of the CICHWithFurtherPriorityQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ICHWithFurtherPriorityQueue.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CICHWithFurtherPriorityQueue::CICHWithFurtherPriorityQueue(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts) : CImprovedCHWithEdgeValve(inputModel, indexOfSourceVerts)
{
	nameOfAlgorithm = "ICH2";
}

CICHWithFurtherPriorityQueue::CICHWithFurtherPriorityQueue(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts, const set<int>& destinations) : CImprovedCHWithEdgeValve(inputModel, indexOfSourceVerts), destinations(destinations)
{
	nameOfAlgorithm = "ICH2";
}


CICHWithFurtherPriorityQueue::~CICHWithFurtherPriorityQueue()
{
}

void CICHWithFurtherPriorityQueue::ExecuteLocally_Dis(double distThreshold, set<int> &fixedDests)
{
	if (fComputationCompleted)
		return;
	if (!fLocked)
	{
		fLocked = true;
		nCountOfWindows = 0;	
		nMaxLenOfWindowQueue = 0;
		depthOfResultingTree = 0;
		InitContainers();
		nTotalMilliSeconds = GetTickCount();	
		BuildSequenceTree_Dis(distThreshold, fixedDests);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
}

void CICHWithFurtherPriorityQueue::ExecuteLocally_SVG(double distThreshold, set<int> &fixedDests, int max_covered_points)
{
	if (fComputationCompleted)
		return;
	if (!fLocked)
	{
		fLocked = true;
		nCountOfWindows = 0;	
		nMaxLenOfWindowQueue = 0;
		depthOfResultingTree = 0;
		InitContainers();
		nTotalMilliSeconds = GetTickCount();	
		BuildSequenceTree_SVG(distThreshold, fixedDests,max_covered_points);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
}




double CICHWithFurtherPriorityQueue::ExecuteLocally_vertNum(int levelNum, set<int>& fixedDests)
{
	if (fComputationCompleted)
		return DBL_MAX;
	double dis = DBL_MAX;
	if (!fLocked)
	{
		fLocked = true;
		nCountOfWindows = 0;	
		nMaxLenOfWindowQueue = 0;
		depthOfResultingTree = 0;
		InitContainers();
		nTotalMilliSeconds = GetTickCount();	
		dis = BuildSequenceTree_vertNum(levelNum, fixedDests);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
	return dis;
}


void CICHWithFurtherPriorityQueue::InitContainers()
{
	//m_InfoAtAngles.resize(model.GetNumOfEdges());
}

void CICHWithFurtherPriorityQueue::ClearContainers()
{
	while (!m_QueueForWindows.empty())
	{
		delete m_QueueForWindows.top().pWindow;
		m_QueueForWindows.pop();
	}
	
	while (!m_QueueForPseudoSources.empty())
	{
		m_QueueForPseudoSources.pop();
	}	
}

double CICHWithFurtherPriorityQueue::BuildSequenceTree_vertNum(int levelNum, set<int> &fixedDests)
{
	fixedDests.clear();
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{	
		if ((int)m_QueueForWindows.size() > nMaxLenOfWindowQueue)
			nMaxLenOfWindowQueue = (int)m_QueueForWindows.size();
		if (m_QueueForPseudoSources.size() > nMaxLenOfPseudoSources)
			nMaxLenOfPseudoSources = m_QueueForPseudoSources.size();
		double minCurrentDis = DBL_MAX;
		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			minCurrentDis = m_QueueForPseudoSources.top().disUptodate;
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			minCurrentDis = quoteW.disUptodate;
		}

		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();			
			fixedDests.insert(indexOfVert);
			if (fixedDests.size() > levelNum)
			{
				return minCurrentDis;
			}			
			if (!model.IsConvexVert(indexOfVert))
				ComputeChildrenOfPseudoSource(indexOfVert);				
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			ComputeChildrenOfWindow(quoteW);		
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();		
	}
	return FLT_MAX;
}

void CICHWithFurtherPriorityQueue::BuildSequenceTree_Dis(double distThrehold, set<int> &fixedDests)
{
	fixedDests.clear();
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{	
		if ((int)m_QueueForWindows.size() > nMaxLenOfWindowQueue)
			nMaxLenOfWindowQueue = (int)m_QueueForWindows.size();
		if (m_QueueForPseudoSources.size() > nMaxLenOfPseudoSources)
			nMaxLenOfPseudoSources = m_QueueForPseudoSources.size();
		double minCurrentDis = DBL_MAX;
		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			minCurrentDis = m_QueueForPseudoSources.top().disUptodate;
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			minCurrentDis = quoteW.disUptodate;
		}

		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();	
			fixedDests.insert(indexOfVert);
			if (minCurrentDis > distThrehold)
			{
				return;
			}
			if (!model.IsConvexVert(indexOfVert))
				ComputeChildrenOfPseudoSource(indexOfVert);	
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			ComputeChildrenOfWindow(quoteW);		
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}

//------------By YingXiang---------------
void CICHWithFurtherPriorityQueue::BuildSequenceTree_SVG(double distThrehold, set<int> &fixedDests, int max_covered_points )
{
	fixedDests.clear();
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	numDirectWindow = 0;
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{	
		if(numDirectWindow <= 0)break; // for SVG
        if(fixedDests.size() >= max_covered_points) break;
		if ((int)m_QueueForWindows.size() > nMaxLenOfWindowQueue)
			nMaxLenOfWindowQueue = (int)m_QueueForWindows.size();
		if (m_QueueForPseudoSources.size() > nMaxLenOfPseudoSources)
			nMaxLenOfPseudoSources = m_QueueForPseudoSources.size();
		double minCurrentDis = DBL_MAX;
		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			minCurrentDis = m_QueueForPseudoSources.top().disUptodate;
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			minCurrentDis = quoteW.disUptodate;
		}
		//fprintf(stderr, "numDirectWindow=%d, minCurrentDis=%g\n", numDirectWindow, minCurrentDis);

		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();	
			fixedDests.insert(indexOfVert);

			if (minCurrentDis > distThrehold)
			{
				return;
			}
			if (!model.IsConvexVert(indexOfVert))
				ComputeChildrenOfPseudoSource(indexOfVert);	
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			if(quoteW.pWindow->indexOfRoot == indexOfSourceVerts[0]){
				numDirectWindow --;
			}
			ComputeChildrenOfWindow(quoteW);		
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}


void CICHWithFurtherPriorityQueue::BuildSequenceTree()
{
	set<int> tmpDests(destinations);
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{	
		if (!destinations.empty() && tmpDests.empty())
			break;
		if ((int)m_QueueForWindows.size() > nMaxLenOfWindowQueue)
			nMaxLenOfWindowQueue = (int)m_QueueForWindows.size();
		if (m_QueueForPseudoSources.size() > nMaxLenOfPseudoSources)
			nMaxLenOfPseudoSources = m_QueueForPseudoSources.size();
		double minCurrentDis = DBL_MAX;
		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			minCurrentDis = m_QueueForPseudoSources.top().disUptodate;
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			minCurrentDis = quoteW.disUptodate;
		}
		vector<int> tobedeleted;
		for (set<int>::iterator it = tmpDests.begin(); it != tmpDests.end(); ++it)
		{
			if (m_InfoAtVertices[*it].disUptodate <= minCurrentDis)
				tobedeleted.push_back(*it);
		}
		for (int i = 0; i < (int)tobedeleted.size(); ++i)
		{
			tmpDests.erase(tobedeleted[i]);
		}
		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();	
			if (!model.IsConvexVert(indexOfVert))
				ComputeChildrenOfPseudoSource(indexOfVert);	
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			ComputeChildrenOfWindow(quoteW);		
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}

double CICHWithFurtherPriorityQueue::GetExactGeoDisBetween(const CRichModel& inputModel, int indexOfSource, int indexOfDest, vector<CPoint3D>& path)
{
	vector<int> sources;
	sources.push_back(indexOfSource);
	set<int> dests;
	dests.insert(indexOfDest);
	CICHWithFurtherPriorityQueue alg(inputModel, sources, dests);
	alg.Execute();
	path.clear();
	alg.FindSourceVertex(indexOfDest, path);
	return alg.m_InfoAtVertices[indexOfDest].disUptodate;
}

double CICHWithFurtherPriorityQueue::GetExactGeoDisBetween(const CRichModel& inputModel, int indexOfSource, int indexOfDest)
{
	vector<int> sources;
	sources.push_back(indexOfSource);
	set<int> dests;
	dests.insert(indexOfDest);
	CICHWithFurtherPriorityQueue alg(inputModel, sources, dests);
	alg.Execute();
	return alg.m_InfoAtVertices[indexOfDest].disUptodate;
}