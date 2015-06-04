#include "cutlocuspanel.h"
#include "ui_cutlocuspanel.h"

CutLocusPanel::CutLocusPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CutLocusPanel)
{
	ui->setupUi(this);

	connect(ui->methodComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sig_methodChanged(int)));
	connect(ui->MinmaxCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(sig_displayMinMax(int)));
	connect(ui->CutCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slot_toggleCut(int)));
	//connect(ui->OneCutRadioButton, SIGNAL(toggled(bool)), this, SIGNAL(sig_displayOneCut(bool)));
	//connect(ui->MultCutRadioButton, SIGNAL(toggled(bool)), this, SIGNAL(sig_displayMultCut(bool)));
	ui->buttonGroup->connect(ui->buttonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(sig_toggleCutMode(int)));


}

CutLocusPanel::~CutLocusPanel()
{
	delete ui;
}


void CutLocusPanel::closeEvent(QCloseEvent *e)
{
	close();
	ui->MinmaxCheckBox->setChecked(false);
	emit sig_closedSignal();
}

void CutLocusPanel::slot_toggleCut(int state)
{
	if (state == Qt::Unchecked) {
		ui->OneCutRadioButton->setEnabled(false);
		ui->MultCutRadioButton->setEnabled(false);

	}else {
		ui->OneCutRadioButton->setEnabled(true);
		ui->MultCutRadioButton->setEnabled(true);
	}
	emit sig_toggleCut();
}
