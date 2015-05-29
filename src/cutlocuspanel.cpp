#include "cutlocuspanel.h"
#include "ui_cutlocuspanel.h"

CutLocusPanel::CutLocusPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CutLocusPanel)
{
    ui->setupUi(this);

    connect(ui->methodComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sig_methodChanged(int)));
    connect(ui->MinmaxRadioButton, SIGNAL(toggled(bool)), this, SIGNAL(sig_displayMinMax(bool)));
    connect(ui->CutRadioButton, SIGNAL(toggled(bool)), this, SIGNAL(sig_displayCut(bool)));

}

CutLocusPanel::~CutLocusPanel()
{
    delete ui;
}


void CutLocusPanel::closeEvent(QCloseEvent *e)
{
    close();
    emit closedSignal();
}
