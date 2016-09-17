#pragma once
#ifndef WEAVEPANEL_H
#define WEAVEPANEL_H
#include "Utils/common.h"
#include <QWidget>

namespace Ui {
class WeavePanel;
}

class WeavePanel : public QWidget
{
	Q_OBJECT

public:
	explicit WeavePanel(QWidget *parent = 0);
	~WeavePanel();

	confMap getConfig() const { return config; }

signals:
	void sig_saved();
	void sig_setBridger();

public slots:
	void slot_setType();
	void slot_setThickness(int);
	void slot_setRoundness(int);
	void slot_setPivot(int);
	void slot_setFlap(int);

private:
	Ui::WeavePanel *ui;
	confMap config;
};

#endif // WEAVEPANEL_H
