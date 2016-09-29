#include "SelectByRefIdPanel.h"

SelectByRefIdPanel::SelectByRefIdPanel()
	: ui(new Ui::selbyrefid_dialog)
{
	ui->setupUi(this);
}

int SelectByRefIdPanel::type()
{
	return ui->src_val->currentIndex();
}

int SelectByRefIdPanel::id()
{
	return ui->refid_val->value();
}
