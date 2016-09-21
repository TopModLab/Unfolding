#pragma once
#ifndef BRIDGERPANEL_H
#define BRIDGERPANEL_H

#include "Utils/common.h"
#include <QWidget>

namespace Ui {
class BridgerPanel;
}

class BridgerPanel : public QWidget
{
	Q_OBJECT

public:
	explicit BridgerPanel(QWidget *parent = 0);
	~BridgerPanel();
	//std::map<QString, Float> getConfigValues();
	confMap getConfigValues() const { return bridgerConfig; }
	void setSaveMode(bool);

signals:
	void sig_save2extend();
	void sig_saved();
	void sig_canceled();

public slots:
	void slot_saved();
	void slot_restrainSliders(int);

private:
	Ui::BridgerPanel *ui;
	//std::map<QString, Float> bridgerConfig;
	confMap bridgerConfig;
};

#endif // BRIDGERPANEL_H
