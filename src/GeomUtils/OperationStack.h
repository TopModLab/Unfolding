#ifndef OPERATIONSTACK_H
#define OPERATIONSTACK_H

#include <QScopedPointer>
#include <QSharedPointer>
#include "HDS/hds_mesh.h"

class OperationStack
{
public:
	/*list out all the possible mesh operations here*/
	enum Flag{
		Undefined = -1,
		Original = 0,
		GRS = 1,
		Cutted = 2,
		GES = 3,
		Rimmed = 4,
		QuadEdge = 5,
		Woven = 6,
		DForm = 7,
		Origami = 8,
		Unfolded
	};

	struct Operation{
		QSharedPointer<HDS_Mesh> mesh;
		Flag flag = Undefined;
		Operation() = delete;
		Operation(HDS_Mesh* op_mesh, Flag op_flag){
			mesh.reset(op_mesh);
			flag = op_flag;
		}
	};

	OperationStack();

	void undo();
	void redo();
	void reset();
	void clear();

	void push(HDS_Mesh* curMesh);


	HDS_Mesh* getCurrentMesh();
	HDS_Mesh* getOriMesh(){return ori_mesh.data();}
	HDS_Mesh* getUnfoldedMesh(){return unfolded_mesh.data();}

	void setCurrentFlag(Flag flag);
	Flag getCurrentFlag();

	bool canUnfold;
	bool canGRS;
	bool canCut;
	bool canGES;
	bool canRim;
	bool canQuad;

private:
	stack<QSharedPointer<Operation>> opStack;
	stack<QSharedPointer<Operation>> redoStack;
	Flag curFlag;

	//QSharedPointer<HDS_Mesh> ori_mesh;
	QSharedPointer<HDS_Mesh> ori_mesh, unfolded_mesh;

};

#endif // OPERATIONSTACK_H
