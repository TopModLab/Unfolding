#ifndef BRIDGERPANEL_H
#define BRIDGERPANEL_H

#include <QWidget>
#include <map>

namespace Ui {
class BridgerPanel;
}

class BridgerPanel : public QWidget
{
	Q_OBJECT

public:
	explicit BridgerPanel(QWidget *parent = 0);
	~BridgerPanel();
	std::map<QString, double> getConfigValues();
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
	std::map<QString,double> bridgerConfig;



};

#endif // BRIDGERPANEL_H
