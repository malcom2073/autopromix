#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

#include "apuone.h"
class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();



	void loadAll(QString filename,unsigned short offset);
private slots:
	void currentItemRowChanged(int item);
	void connectButtonClicked();
	void ripButtonClicked();
private:
	bool connected;
	ApuOne apuOne;
	QString calc(QString str);
	QVariantList itemlist;
	QByteArray currentBin;
	QVariantMap itemmap;
	Ui::MainWindow ui;

	int m_portHandle;
};

#endif // MAINWINDOW_H
