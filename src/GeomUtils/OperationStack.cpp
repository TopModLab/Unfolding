#include "GeomUtils/OperationStack.h"
#include <cstdlib>

OperationStack::OperationStack()
	: curFlag(Undefined)
	, canUnfold(true)
	, canGRS(true)
	, canCut(true)
	, canGES(true)
	, canRim(true)
	, canQuad(true)
{
}

void OperationStack::undo()
{
	if (opStack.size() > 1){
		redoStack.push(opStack.top());
		opStack.pop();
		getCurrentMesh()->matchIndexToSize();

		setCurrentFlag(getCurrentFlag());

		cout<<"undo to flag "<<getCurrentFlag()<<endl;

	}else {
		cout<<"no more undo"<<endl;
	}

}

void OperationStack::redo()
{
	if (!redoStack.empty()){
		opStack.push(redoStack.top());
		setCurrentFlag(getCurrentFlag());
		redoStack.pop();

	}else {
		cout<<"no more redo"<<endl;
	}
}

/*call setCurrentFlag in mainWindow before mesh operation*/
/*call push in meshManger*/
void OperationStack::push(HDS_Mesh* curMesh)
{
	opStack.push(QSharedPointer<Operation>(new Operation(curMesh, curFlag)));
	setCurrentFlag(curFlag);
	if (curFlag == Original)
		ori_mesh = opStack.top()->mesh;
	if (curFlag == Unfolded)
		unfolded_mesh = opStack.top()->mesh;
	
	while (!redoStack.empty())
	{
		//delete redoStack.top();
		redoStack.pop();
	}

	curFlag = Undefined;
}

void OperationStack::reset()
{
	while (!redoStack.empty())
		redoStack.pop();
	while (opStack.size() > 1)
		opStack.pop();
	unfolded_mesh.reset();
	getCurrentMesh()->matchIndexToSize();
	setCurrentFlag(OperationStack::Original);
}

void OperationStack::clear()
{
	while (!redoStack.empty())
		redoStack.pop();
	while (!opStack.empty())
		opStack.pop();
	ori_mesh.reset();
	unfolded_mesh.reset();
	setCurrentFlag(OperationStack::Original);

}

/*call setCurrentFlag in mainWindow before mesh operation*/
/*call push in meshManger*/
void OperationStack::setCurrentFlag(Flag flag)
{
	curFlag = flag;

	//check operation availability
	switch(flag) {
	case Original:
		canUnfold = false;
		canGRS = true;
		canCut = true;
		canGES = true;
		canRim = true;
		canQuad = true;
		break;
	case Cutted:
		canUnfold = true;
		canCut = false;
		canGES = false;
		canRim = false;
		canQuad = false;
		break;
	case GES:
	case GRS:
	case Rimmed:
	case QuadEdge:
	case Woven:
	case Origami:
		canUnfold = true;
		canGRS = false;
		canCut = false;
		canGES = false;
		canRim = false;
		canQuad = false;
		break;
	case Unfolded:
		canUnfold = false;
		canGRS = false;
		canCut = false;
		canGES = false;
		canRim = false;
		canQuad = false;
		break;
	}
}

HDS_Mesh* OperationStack::getCurrentMesh()
{
	return opStack.top()->mesh.data();
}

OperationStack::Flag OperationStack::getCurrentFlag()
{
	return opStack.top()->flag;
}
