#include "UI/GESPanel.h"
#include "ui_GESPanel.h"

GESPanel::GESPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::GESPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("GES Panel"));

	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(slot_saved()));
	connect(ui->okButton, SIGNAL(clicked()), this, SIGNAL(sig_saved()));
	connect(ui->bridgerPanelButton, SIGNAL(clicked()), this, SIGNAL(sig_setBridger()));
	connect(ui->sizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setSize(int)));
}

GESPanel::~GESPanel()
{
	delete ui;
}

void
GESPanel::setPanelType(int type)
{
	panelType = type;
	//adjust panel ui according to panelType
}

void
GESPanel::slot_setSize(int value)
{
	ui->sizeText->setText(QString::number(
		value/static_cast<Float>(ui->sizeSlider->maximum())));
}

void
GESPanel::slot_saved()
{
	bridgerSize = ui->sizeSlider->value();
	close();
}

Float
GESPanel::getBridgerSize()
{
	return bridgerSize / static_cast<Float>(ui->sizeSlider->maximum());
}
