#pragma once
#ifndef _SelectByRefIdPanel_
#define _SelectByRefIdPanel_
#include "Utils/common.h"
#include "ui_selbyrefid.h"

class SelectByRefIdPanel : public QDialog
{
	Q_OBJECT
public:
	SelectByRefIdPanel();

	int type();
	int id();
private:
	QScopedPointer<Ui::selbyrefid_dialog> ui;
	//confMap conf;
};
#endif