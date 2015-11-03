#pragma once
#include "common.h"
#include <QString>

class HDS_Common
{
public:
	enum ReferenceID_type
	{
		ORIGIN = 0,
		FROM_VERTEX = 1,
		FROM_EDGE = 2,
		FROM_FACE = 3
	};
	static int assignRef_ID(int id, int type);
	
	static QString ref_ID2String(int refid);
};
