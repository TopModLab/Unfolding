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

public slots:
	void slot_setConfig();
	void slot_setPatchSize(int);

private:
	Ui::NeoWeavePanel *ui;
	confMap config;

};

#endif // NEOWEAVEPANEL_H
