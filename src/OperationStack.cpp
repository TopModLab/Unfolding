#include "OperationStack.h"
#include <cstdlib>

OperationStack::OperationStack()
{
	curFlag = Undefined;

	canUnfold = false;
	canExtend = true;
	canCut = true;
	canBind = true;
	canRim = true;
	canHollow = true;
}

OperationStack::~OperationStack()
{

}

void OperationStack::undo()
{
	if (opStack.size() > 1){
		redoStack.push(opStack.top());
		opStack.pop();
		cout<<"undo to flag "<<getCurrentFlag()<<endl;

	}else {
		cout<<"no more undo"<<endl;
	}

}

void OperationStack::redo()
{
	if (!redoStack.empty()){
		opStack.push(redoStack.top());
		redoStack.pop();

	}else {
		cout<<"no more redo"<<endl;
	}
}

/*call setCurrentFlag in mainWindow before mesh operation*/
/*call push in meshManger*/
void OperationStack::push(HDS_Mesh* curMesh)
{
	if (curFlag == Original)
		ori_mesh.reset(new HDS_Mesh(*curMesh));
	if (curFlag == Unfolded)
		unfolded_mesh.reset(new HDS_Mesh(*curMesh));
	Operation* op = new Operation(curMesh, curFlag);
	opStack.push(op);
	while(!redoStack.empty()) redoStack.pop();
	curFlag = Undefined;
}

void OperationStack::reset()
{
	while(!redoStack.empty()) redoStack.pop();
	while(opStack.size() > 1) opStack.pop();
	unfolded_mesh.reset();

	canUnfold = false;
	canExtend = true;
	canCut = true;
	canBind = true;
	canRim = true;
	canHollow = true;
}

void OperationStack::clear()
{
	while(!redoStack.empty()) redoStack.pop();
	while(!opStack.empty()) opStack.pop();
	ori_mesh.reset();
	unfolded_mesh.reset();

	canUnfold = false;
	canExtend = true;
	canCut = true;
	canBind = true;
	canRim = true;
	canHollow = true;
}

/*call setCurrentFlag in mainWindow before mesh operation*/
/*call push in meshManger*/
void OperationStack::setCurrentFlag(Flag flag)
{
	curFlag = flag;

	//check operation availability
	switch(flag) {
	case Cutted:
		canUnfold = true;
		canCut = false;
		canBind = false;
		canRim = false;
		canHollow = false;
		break;
	case Binded:
	case Rimmed:
	case Hollowed:
		canUnfold = true;
		canExtend = false;
		canCut = false;
		canBind = false;
		canRim = false;
		canHollow = false;
		break;
	case Unfolded:
		canUnfold = false;
		canExtend = false;
		canCut = false;
		canBind = false;
		canRim = false;
		canHollow = false;
		break;
	}
}

HDS_Mesh* OperationStack::getCurrentMesh()
{
	Operation* op = opStack.top();
	return op->mesh.data();
}

OperationStack::Flag OperationStack::getCurrentFlag()
{
	Operation* op = opStack.top();
	return op->flag;
}
