#include "connectdialog.h"
#include "ui_connectdialog.h"
#include <QMessageBox>
#include <QDebug>

ConnectDialog::ConnectDialog(InstrumentConnection *m_instrumentConnection, QWidget *parent) :
	QDialog(parent), ui(new Ui::ConnectDialog), instrumentConnection(m_instrumentConnection)
{
	ui->setupUi(this);
}

ConnectDialog::~ConnectDialog()
{
	delete ui;
}

void ConnectDialog::on_radioButtonVisa_toggled(bool checked)
{
	ui->listVisaAddr->setEnabled(true);
	ui->spinBoxGpibAddr->setEnabled(false);
	ui->cmdSearch->setEnabled(true);
}

void ConnectDialog::on_radioButtonGpib_toggled(bool checked)
{
	if(checked)
		ui->cmdConnect->setEnabled(true);
	else
		ui->cmdConnect->setEnabled(false);
	ui->spinBoxGpibAddr->setEnabled(true);
	ui->listVisaAddr->setEnabled(false);
	ui->cmdSearch->setEnabled(false);
}

void ConnectDialog::on_cmdCancel_clicked()
{
	this->reject();
}

void ConnectDialog::on_cmdSearch_clicked()
{
	ui->cmdConnect->setEnabled(false);
	ui->listVisaAddr->clear();
	ui->listVisaAddr->addItems(instrumentConnection->getAvailableVisaInstruments());
}

void ConnectDialog::on_cmdConnect_clicked()
{
	bool connectedState = false;
	if(ui->radioButtonVisa->isChecked())
	{
		connectedState = instrumentConnection->connectToInstrument(ui->listVisaAddr->currentItem()->text());
	}
	else
	{
		connectedState = instrumentConnection->connectToInstrument(ui->spinBoxGpibAddr->value());
	}
	if(connectedState)
	{
		QString idn = instrumentConnection->query("*IDN?");
		QMessageBox::information(this, "Connection successfull", QString("Successfully connected to instrument!\n"
																 "IDN: ") + idn);
		this->accept();
	}
	else
	{
		QMessageBox::warning(this, "Connection failed", "Connection to instrument failed!");
	}
}

void ConnectDialog::on_listVisaAddr_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	ui->cmdConnect->setEnabled(true);
}
