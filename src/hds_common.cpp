#include "hds_common.h"

int HDS_Common::assignRef_ID(int id, int type)
{
	return (id << 2) + type;
}

QString HDS_Common::ref_ID2String(int refid)
{
	QString ret;
	int newrid = refid & 3;
	switch (newrid)
	{
	case ORIGIN:
		ret = "R";
		break;
	case FROM_VERTEX:
		ret = "V";
		break;
	case FROM_EDGE:
		ret = "E";
		break;
	case FROM_FACE:
		ret = "F";
		break;
	default:
		break;
	}
	return ret.append(QString::number(refid >> 2));
}
