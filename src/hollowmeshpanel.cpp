#include "hollowmeshpanel.h"
#include "ui_hollowmeshpanel.h"

HollowMeshPanel::HollowMeshPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HollowMeshPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Hollow Mesh Panel"));
	ui->shift->hide();

	QButtonGroup *flapTypeGroup = new QButtonGroup(this);
	flapTypeGroup->addButton(ui->oneFlapButton,0);
	flapTypeGroup->addButton(ui->multFlapButon, 1);
	flapTypeGroup->setExclusive(true);

	connect(flapTypeGroup, SIGNAL(buttonClicked(int)), this, SLOT(slot_restrainSliders(int)));
	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(slot_saved()));
	connect(ui->okButton, SIGNAL(clicked()), this, SIGNAL(sig_saved()));
	connect(ui->connectorPanelButton, SIGNAL(clicked(bool)), this, SIGNAL(sig_setConnector(bool)));
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

void HollowMeshPanel::slot_saved()
{
	flapType = ui->oneFlapButton->isChecked()? 0:1;
	flapSize = ui->flapSizeSlider->value();
	connectorSize = ui->conSizeSlider->value();
	shiftAmount = ui->shiftSlider->value();
	close();
}
double HollowMeshPanel::getFlapSize()
{
	return (double)flapSize / (double)ui->flapSizeSlider->maximum();
}
double HollowMeshPanel::getConnectorSize()
{
	return 1.0 -(double)connectorSize / (double)ui->conSizeSlider->maximum();
}

double HollowMeshPanel::getShift()
{
	return (double)shiftAmount / (double)ui->shiftSlider->maximum();
}

int HollowMeshPanel::getType()
{
	return flapType;
}
