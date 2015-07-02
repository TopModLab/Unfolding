#include "connectorpanel.h"
#include "ui_connectorpanel.h"
#include "connectorpanelviewer.h"

ConnectorPanel::ConnectorPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ConnectorPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Connector Panel"));

	viewer = new ConnectorPanelViewer(this);
	ui->viewerLayout->addWidget(viewer);

	connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(close()));

	connect(ui->curvSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setCurvature(int)));
	connect(ui->shapeComboBox, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setShape(int)));
	connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setSamples(int)));
	connect(ui->sizeSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setSize(int)));
	connect(ui->convergeSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setConvergingPoint(int)));
	connect(ui->adhesiveComboBox, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setOpeningType(int)));

}

ConnectorPanel::~ConnectorPanel()
{
	delete ui;
}

