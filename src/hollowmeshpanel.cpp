#include "hollowmeshpanel.h"
#include "ui_hollowmeshpanel.h"

HollowMeshPanel::HollowMeshPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HollowMeshPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Hollow Mesh Panel"));
	ui->shift->hide();

	ui->bridgerSizeLabel->setText(QString::number((float)ui->bridgerSizeSlider->value()/ui->bridgerSizeSlider->maximum()));
	ui->flapSizeLabel->setText(QString::number((float)ui->flapSizeSlider->value()/ui->flapSizeSlider->maximum()));
	ui->shiftLabel->setText(QString::number((float)ui->shiftSlider->value()/ui->shiftSlider->maximum()));

	QButtonGroup *flapTypeGroup = new QButtonGroup(this);
	flapTypeGroup->addButton(ui->oneFlapButton,0);
	flapTypeGroup->addButton(ui->multFlapButon, 1);
	flapTypeGroup->setExclusive(true);

	connect(flapTypeGroup, SIGNAL(buttonClicked(int)), this, SLOT(slot_restrainSliders(int)));
	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(slot_saved()));
	connect(ui->okButton, SIGNAL(clicked()), this, SIGNAL(sig_saved()));
	connect(ui->bridgerPanelButton, SIGNAL(clicked()), this, SIGNAL(sig_setBridger()));

	connect(ui->bridgerSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setbridgerLabel(int)));
	connect(ui->flapSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setflapLabel(int)));
	connect(ui->shiftSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setshiftLabel(int)));

}

HollowMeshPanel::~HollowMeshPanel()
{
	delete ui;
}

void HollowMeshPanel::slot_restrainSliders(int button)
{
	if (button == 0) {
		ui->shift->hide();
	}
	else {
		ui->shift->show();
	}
}

void HollowMeshPanel::slot_setbridgerLabel(int value)
{
	ui->bridgerSizeLabel->setText(QString::number((float)value/ui->bridgerSizeSlider->maximum()));
}

void HollowMeshPanel::slot_setflapLabel(int value)
{
	ui->flapSizeLabel->setText(QString::number((float)value/ui->flapSizeSlider->maximum()));
}

void HollowMeshPanel::slot_setshiftLabel(int value)
{
	ui->shiftLabel->setText(QString::number((float)value/ui->shiftSlider->maximum()));
}

void HollowMeshPanel::slot_saved()
{
	flapType = ui->oneFlapButton->isChecked()? 0:1;
	flapSize = ui->flapSizeSlider->value();
	bridgerSize = ui->bridgerSizeSlider->value();
	shiftAmount = ui->shiftSlider->value();
	close();
}
double HollowMeshPanel::getFlapSize()
{
	return (double)flapSize / (double)ui->flapSizeSlider->maximum();
}
double HollowMeshPanel::getBridgerSize()
{
	return (double)bridgerSize / (double)ui->bridgerSizeSlider->maximum();
}

double HollowMeshPanel::getShift()
{
	return (double)shiftAmount / (double)ui->shiftSlider->maximum();
}

int HollowMeshPanel::getType()
{
	return flapType;
}
