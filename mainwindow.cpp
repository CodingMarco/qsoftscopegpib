#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gpib.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	gpib g;
}

MainWindow::~MainWindow()
{
	delete ui;
}

