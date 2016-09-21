#include "UI/QuadEdgePanel.h"
#include "ui_QuadEdgePanel.h"

QuadEdgePanel::QuadEdgePanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::QuadEdgePanel)
{
	ui->setupUi(this);
	ui->bridgerSizeLabel->setText(QString::number((float)ui->bridgerSizeSlider->value()/ui->bridgerSizeSlider->maximum()));
	ui->flapSizeLabel->setText(QString::number((float)ui->flapSizeSlider->value()/ui->flapSizeSlider->maximum()));
	ui->shiftLabel->setText(QString::number((float)ui->shiftSlider->value()/ui->shiftSlider->maximum()));


	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(slot_saved()));
	connect(ui->okButton, SIGNAL(clicked()), this, SIGNAL(sig_saved()));
	connect(ui->bridgerPanelButton, SIGNAL(clicked()), this, SIGNAL(sig_setBridger()));

	connect(ui->bridgerSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setbridgerLabel(int)));
	connect(ui->flapSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setflapLabel(int)));
	connect(ui->shiftSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setshiftLabel(int)));

}

QuadEdgePanel::~QuadEdgePanel()
{
	delete ui;
}

void QuadEdgePanel::setPanelType(int type)
{
	if (type == 0) {
		ui->shift->hide();
		flapType = 0;
		setWindowTitle(tr("Quad Edge Panel"));
	}
	else {
		ui->shift->show();
		flapType = 1;
		setWindowTitle(tr("Winged Edge Panel"));
	}
}

void QuadEdgePanel::slot_setbridgerLabel(int value)
{
	ui->bridgerSizeLabel->setText(QString::number(
		value/static_cast<Float>(ui->bridgerSizeSlider->maximum())));
}

void QuadEdgePanel::slot_setflapLabel(int value)
{
	ui->flapSizeLabel->setText(QString::number((float)value/ui->flapSizeSlider->maximum()));
}

void QuadEdgePanel::slot_setshiftLabel(int value)
{
	ui->shiftLabel->setText(QString::number((float)value/ui->shiftSlider->maximum()));
}

void QuadEdgePanel::slot_saved()
{
	flapSize = ui->flapSizeSlider->value();
	bridgerSize = ui->bridgerSizeSlider->value();
	shiftAmount = ui->shiftSlider->value();
	close();
}
float QuadEdgePanel::getFlapSize()
{
	return flapSize / static_cast<float>(ui->flapSizeSlider->maximum());
}
float QuadEdgePanel::getBridgerSize()
{
	return bridgerSize / static_cast<float>(ui->bridgerSizeSlider->maximum());
}

float QuadEdgePanel::getShift()
{
	return shiftAmount / static_cast<float>(ui->shiftSlider->maximum());
}

int QuadEdgePanel::getType()
{
	return flapType;
}
