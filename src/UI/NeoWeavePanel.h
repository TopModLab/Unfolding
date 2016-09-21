#ifndef NEOWEAVEPANEL_H
#define NEOWEAVEPANEL_H
#include "Utils/common.h"
#include <QWidget>

namespace Ui {
class NeoWeavePanel;
}

class NeoWeavePanel : public QWidget
{
	Q_OBJECT

public:
	explicit NeoWeavePanel(QWidget *parent = 0);
	~NeoWeavePanel();
	confMap getConfig() const { return config; }

signals:
	void sig_saved();

public:
	void setConfig();

private:
	Ui::NeoWeavePanel *ui;
	confMap config;

};

#endif // NEOWEAVEPANEL_H
