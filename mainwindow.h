/**************************************************************************
*   Copyright (C) 2013 by Michael Carpenter (malcom2073)                  *
*   malcom2073@gmail.com                                                  *
*                                                                         *
*   This file is a part of autopromix                                     *
*                                                                         *
*   autopromix is free software: you can redistribute it and/or modify    *
*   it under the terms of the GNU Lesser General Public License           *
*   vversion 2 as published by the Free Software Foundation               *
*                                                                         *
*   autopromix is distributed in the hope that it will be useful,         *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with autopromix.  If not, see <http://www.gnu.org/licenses/>.   *
***************************************************************************/

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
	void writeButtonClicked();
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
