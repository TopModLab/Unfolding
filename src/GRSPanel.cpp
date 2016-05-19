#include "GRSPanel.h"
#include "ui_GRSPanel.h"

GRSPanel::GRSPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::GRSPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("GRS Panel"));

	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(slot_saved()));
	connect(ui->okButton, SIGNAL(clicked()), this, SIGNAL(sig_saved()));
	connect(ui->bridgerPanelButton, SIGNAL(clicked()), this, SIGNAL(sig_setBridger()));
	connect(ui->sizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setSize(int)));
}

GRSPanel::~GRSPanel()
{
	delete ui;
}

void
GRSPanel::setPanelType(int type)
{
	panelType = type;
	//adjust panel ui according to panelType
}

void
GRSPanel::slot_setSize(int value)
{
	ui->sizeText->setText(QString::number((float)value/ui->sizeSlider->maximum()));
}

void
GRSPanel::slot_saved()
{
	bridgerSize = ui->sizeSlider->value();
	close();
}

double
GRSPanel::getBridgerSize()
{
	return (double)bridgerSize / (double)ui->sizeSlider->maximum() /2.0;
}
