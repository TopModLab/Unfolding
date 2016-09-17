#include "UI/criticalpointspanel.h"

CriticalPointsPanel::CriticalPointsPanel(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	// connect the slider and the spinbox
	connect(ui.smoothingSlider, SIGNAL(valueChanged(int)), ui.smoothingSpinBox, SLOT(setValue(int)));
	connect(ui.smoothingSpinBox, SIGNAL(valueChanged(int)), ui.smoothingSlider, SLOT(setValue(int)));
	connect(ui.smoothingSlider, SIGNAL(valueChanged(int)), this, SIGNAL(sig_smoothingTimesChanged(int)));
	connect(ui.methodComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sig_methodChanged(int)));
	connect(ui.smoothingTypeCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(sig_smoothingTypeChanged(int)));
}

CriticalPointsPanel::~CriticalPointsPanel()
{

}

void CriticalPointsPanel::closeEvent(QCloseEvent *e)
{
	close();
	emit closedSignal();
}
