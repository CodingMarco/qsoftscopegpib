#pragma once

#include <QDialog>
#include <QListWidgetItem>
#include "instrumentConnection.h"

namespace Ui {
	class ConnectDialog;
}

class ConnectDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ConnectDialog(InstrumentConnection* m_instrumentConnection, QWidget *parent = nullptr);
	~ConnectDialog();

private slots:
	void on_radioButtonVisa_toggled(bool checked);
	void on_radioButtonGpib_toggled(bool checked);
	void on_cmdCancel_clicked();

	void on_cmdSearch_clicked();

	void on_cmdConnect_clicked();

	void on_listVisaAddr_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
	Ui::ConnectDialog* ui;
	InstrumentConnection* instrumentConnection;
};

