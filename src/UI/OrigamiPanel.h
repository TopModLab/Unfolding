#ifndef ORIGAMIPANEL_H
#define ORIGAMIPANEL_H
#include "Utils/common.h"
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
	confMap getConfig() const { return config; }

signals:
	void sig_saved();

public:
	void setConfig();

private:
	Ui::OrigamiPanel *ui;
	confMap config;
};

#endif // ORIGAMIPANEL_H
