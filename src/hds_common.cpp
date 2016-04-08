#include "hds_common.h"

int HDS_Common::assignRef_ID(hdsid_t id, hdsid_t type)
{
	return (id << 2) + type;
}

QString HDS_Common::ref_ID2String(hdsid_t refid)
{
	QString ret;
	int type = refid & 3;
	switch (type)
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
	//return ret.append(QString::number(refid >> 2));
	return QString::number(refid >> 2);
}

void HDS_Common::setFlag(uint16_t flgComp)
{
	flag |= flgComp;
}

void HDS_Common::unsetFlag(uint16_t flgComp)
{
	flag &= ~flgComp;
}

void HDS_Common::reverseFlag(uint16_t flgComp)
{
	flag ^= flgComp;
}

