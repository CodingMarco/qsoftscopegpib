#include "connectdialog.h"
#include "ui_connectdialog.h"
#include <QMessageBox>
#include <QDebug>

ConnectDialog::ConnectDialog(GpibInstrument& m_instr, QWidget *parent) :
	QDialog(parent), ui(new Ui::ConnectDialog), instr(m_instr)
{
	ui->setupUi(this);
}

ConnectDialog::~ConnectDialog()
{
	delete ui;
}

void ConnectDialog::on_cmdCancel_clicked()
{
	this->reject();
}

void ConnectDialog::on_cmdSearch_clicked()
{
	ui->cmdConnect->setEnabled(false);
	ui->listVisaAddr->clear();
}

void ConnectDialog::on_cmdConnect_clicked()
{
	bool connectedState = false;
	connectedState = instr.openInstrument(ui->spinBoxGpibAddr->value());
	if(connectedState)
	{
		QString idn = instr.query("*IDN?");
		QMessageBox::information(this, "Connection successfull", QString("Successfully connected to instrument!\n"
																 "IDN: ") + idn);
		this->accept();
	}
	else
	{
		QMessageBox::warning(this, "Connection failed", "Connection to instrument failed!");
	}
}

void ConnectDialog::on_listVisaAddr_currentItemChanged()
{
	ui->cmdConnect->setEnabled(true);
}
