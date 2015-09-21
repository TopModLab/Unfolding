#include "hollowmeshpanel.h"
#include "ui_hollowmeshpanel.h"

HollowMeshPanel::HollowMeshPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HollowMeshPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Hollow Mesh Panel"));

	connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(slot_saved()));
	connect(ui->pushButton, SIGNAL(clicked()), this, SIGNAL(sig_saved()));

}

HollowMeshPanel::~HollowMeshPanel()
{
	delete ui;
}

void HollowMeshPanel::slot_saved()
{
	flapSize = ui->flapSizeSlider->value();
	connectorSize = ui->conSizeSlider->value();
	close();
}
double HollowMeshPanel::getFlapSize()
{
	return (double)flapSize / (double)ui->flapSizeSlider->maximum();
}
double HollowMeshPanel::getConnectorSize()
{
	return (double)connectorSize / (double)ui->conSizeSlider->maximum();
}
