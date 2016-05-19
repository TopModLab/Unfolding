#include "ConnectorPanel.h"

QFont ConnectorPanel::fontfamily = QFont("Arial");

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

confMap ConnectorPanel::getConfiguration() const
{
	confMap ret;
	fontfamily = ui->font_val->currentFont();

	ret.insert(make_pair(ConnectorConf::SCALE, ui->scale_val->value()));
	ret.insert(make_pair(ConnectorConf::WIDTH, ui->width_val->value()));
	ret.insert(make_pair(ConnectorConf::LENGTH, ui->length_val->value()));

	ret.insert(make_pair(ConnectorConf::PINHOLE_UNIT, (double)ui->pinholeunit_type->currentIndex()));
	ret.insert(make_pair(ConnectorConf::PINHOLESIZE, ui->pinholesize_val->value()));
	ret.insert(make_pair(ConnectorConf::PINHOLECOUNT_TYPE,
		(double)ui->pinholecount_type->currentIndex()));

	ret.insert(make_pair(ConnectorConf::ETCHSEG, (double)ui->etchseg_val->value()));
	ret.insert(make_pair(ConnectorConf::SCORE_TYPE, (double)ui->score_type->currentIndex()));
	ret.insert(make_pair(ConnectorConf::DASH_LEN, ui->scoredash_len->value()));
	ret.insert(make_pair(ConnectorConf::DASH_GAP, ui->scoredash_gap->value()));
	ret.insert(make_pair(ConnectorConf::DASH_UNIT, (double)ui->scoredash_unit->currentIndex()));
	return ret;
}

void ConnectorPanel::initConnectorType()
{
	ui->mesh_type->setCurrentIndex(meshType);
	switch (meshType)
	{
	case HDS_Mesh::REGULAR_PROC:
		ui->connector_type->addItem("Simple");
		ui->connector_type->addItem("Insert");
		ui->connector_type->addItem("Gear");
		ui->connector_type->addItem("Saw");
		ui->connector_type->addItem("Advanced Saw");
		break;
	case HDS_Mesh::QUAD_PROC:
	case HDS_Mesh::WINGED_PROC:
		//ui->pinholeunit_label->setDisabled(false);
		ui->pinholeunit_type->setDisabled(false);
		ui->pinholesize_label->setDisabled(false);
		ui->pinholesize_val->setDisabled(false);
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
	QString filename = QFileDialog::getSaveFileName(this, "Export file as", "export/untitled.svg", tr("SVG files (*.svg)"));
	ui->filename_text->setText(filename);
}

void ConnectorPanel::slot_savePanelData(QAbstractButton* button)
{
	this->close();
}
