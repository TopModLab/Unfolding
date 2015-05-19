#include "cutlocuspanel.h"
#include "ui_cutlocuspanel.h"

CutLocusPanel::CutLocusPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CutLocusPanel)
{
    ui->setupUi(this);

    connect(ui->methodComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sig_methodChanged(int)));
    connect(ui->minmaxPushButton, SIGNAL(clicked()), this, SIGNAL(sig_displayMinMax()));
    connect(ui->cutPushButton, SIGNAL(clicked()), this, SIGNAL(sig_displayCut()));

}

CutLocusPanel::~CutLocusPanel()
{
    delete ui;
}
