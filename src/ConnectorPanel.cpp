#include "ConnectorPanel.h"

ConnectorPanel::ConnectorPanel(int mesh_process_type)
	: ui(new Ui::ConnectorPanel)
	, meshType(mesh_process_type)
{
	ui->setupUi(this);

	//setMeshConfigure();
	initConnectorType();
	//setWindowTitle(tr("Mesh Export Panel"));
	connect(ui->file_button, SIGNAL(clicked()), this, SLOT(slot_setFileName()));
	connect(ui->save_button, SIGNAL(clicked(QAbstractButton*)), this, SLOT(slot_savePanelData(QAbstractButton*)));
	//connect(ui->scale_slider, SIGNAL(valueChanged(int)), this, SLOT(slot_setScaleToSpinbox(int)));
	//connect(ui->scale_val, SIGNAL(valueChanged(double)), this, SLOT(slot_setScaleToSlider(double)));
}

ConnectorPanel::~ConnectorPanel()
{
	delete ui;
}

void ConnectorPanel::setMeshConfigure()
{
	//meshType = mt;
	// Enable connector selection combobox
	/*switch (meshType)
	{
	case HDS_Mesh::REGULAR_PROC:
		ui->reg_conn_type->setDisabled(false);
		break;
	case HDS_Mesh::HOLLOWED_PROC:
		ui->hollow_conn_type->setDisabled(false);
		break;
	case HDS_Mesh::RIMMED_PROC:
		ui->rim_conn_type->setDisabled(false);
		break;
	default:
		break;
	}*/
}

QString ConnectorPanel::getFilename() const
{
	//cout << "File name is: " << ui->filename_text->text() << endl;
	return ui->filename_text->text();
}

double ConnectorPanel::getScale() const
{
	return ui->scale_val->value();
}

int ConnectorPanel::getConnectorType() const
{
	return ui->connector_type->currentIndex();
}

unordered_map<ConnectorConf, double> ConnectorPanel::getConfiguration() const
{
	unordered_map<ConnectorConf, double> ret;
	ret.insert(make_pair(ConnectorConf::SCALE, ui->scale_val->value()));
	ret.insert(make_pair(ConnectorConf::WIDTH, ui->width_val->value()));
	ret.insert(make_pair(ConnectorConf::LENGTH, ui->length_val->value()));

	return ret;
}

void ConnectorPanel::initConnectorType()
{
	switch (meshType)
	{
	case HDS_Mesh::REGULAR_PROC:
		ui->connector_type->addItem("Simple");
		ui->connector_type->addItem("Insert");
		ui->connector_type->addItem("Gear");
		ui->connector_type->addItem("Saw");
		ui->connector_type->addItem("Advanced Saw");
		break;
	case HDS_Mesh::HOLLOWED_PROC:
		ui->connector_type->addItem("Holes");
		ui->connector_type->addItem("Faces");
		break;
	case HDS_Mesh::RIMMED_PROC:
		ui->connector_type->addItem("Arch");
		ui->connector_type->addItem("Ring");
		break;
	default:
		break;
	}
}

void ConnectorPanel::slot_setFileName()
{
	QString filename = QFileDialog::getSaveFileName(this, "Export file as", "export/untitled.svg", tr("XML files (*.svg *.xml)"));
	ui->filename_text->setText(filename);
}

void ConnectorPanel::slot_savePanelData(QAbstractButton* button)
{
	this->close();
}
