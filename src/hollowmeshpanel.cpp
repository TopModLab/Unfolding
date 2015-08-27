#include "hollowmeshpanel.h"
#include "ui_hollowmeshpanel.h"

HollowMeshPanel::HollowMeshPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HollowMeshPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Hollow Mesh Panel"));

	connect(ui->pushButton, SIGNAL(clicked()), this, SIGNAL(sig_saved()));
	connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(slot_saved()));

}

HollowMeshPanel::~HollowMeshPanel()
{
	delete ui;
}

void HollowMeshPanel::slot_saved()
{
	flapSize = ui->horizontalSlider->value();
	close();
}
double HollowMeshPanel::getSize()
{
	return (double)flapSize / (double)ui->horizontalSlider->maximum();
}
