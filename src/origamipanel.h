#pragma once
#ifndef ORIGAMIPANEL_H
#define ORIGAMIPANEL_H

#include "common.h"
#include <QWidget>

namespace Ui {
class OrigamiPanel;
}

class OrigamiPanel : public QWidget
{
	Q_OBJECT

public:
	explicit OrigamiPanel(QWidget *parent = 0);
	~OrigamiPanel();
	//std::map<QString, double> getConfigValues();
	confMap getConfigValues() const { return origamiConfig; }
	void setSaveMode(bool);

signals:
	void sig_save2extend();
	void sig_saved();
	void sig_canceled();

public slots:
	void slot_saved();
	void slot_restrainSliders(int);

private:
	Ui::OrigamiPanel *ui;
	//std::map<QString, double> origamiConfig;
	confMap origamiConfig;
};

#endif // BRIDGERPANEL_H
