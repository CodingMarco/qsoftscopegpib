#pragma once

#include <QDialog>
#include <QListWidgetItem>
#include "gpibInstrument.h"

namespace Ui {
	class ConnectDialog;
}

class ConnectDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ConnectDialog(GpibInstrument& m_instr, QWidget *parent = nullptr);
	~ConnectDialog();

private slots:
	void on_cmdCancel_clicked();
	void on_cmdSearch_clicked();
	void on_cmdConnect_clicked();
	void on_listVisaAddr_currentItemChanged();

private:
	Ui::ConnectDialog* ui;
	GpibInstrument& instr;
};

