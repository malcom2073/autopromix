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

#include "mainwindow.h"

#include <QThread>
#include <QFile>
#include <QDebug>
#include <QMutex>
#include <QFileDialog>
#include <QMessageBox>
//#include <qglobal.h>
//#include "headers.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.listWidget,SIGNAL(currentRowChanged(int)),this,SLOT(currentItemRowChanged(int)));
	connect(ui.connectPushButton,SIGNAL(clicked()),this,SLOT(connectButtonClicked()));
	connect(ui.ripPushButton,SIGNAL(clicked()),this,SLOT(ripButtonClicked()));
	connect(ui.writeBinaryButton,SIGNAL(clicked()),this,SLOT(writeButtonClicked()));

	/*QFile file("/home/michael/EBL/EBL_V21.XDF");
	file.open(QIODevice::ReadOnly);
	QString filestring = file.readAll();
	file.close();
	QStringList filesplit = filestring.split("\n");
	bool initem = false;
	bool inheader = false;
	QVariantMap currentitem;

	foreach (QString fileline,filesplit)
	{
		if (initem || inheader)
		{
			if (fileline.startsWith("%%END%%"))
			{
				initem = false;
				inheader = false;
				itemlist.append(currentitem);
				currentitem.clear();
			}
			else if (initem)
			{
				if (fileline.split("=").size() > 1)
				{
					QString first = fileline.split("=")[0];
					first = first.trimmed().simplified();
					QString second = fileline.split("=")[1];
					currentitem[first.split(" ")[1]] = second.remove("\r");
				}
			}
			else if (inheader)
			{
			}
			else
			{

			}
		}
		else
		{
			if (fileline.startsWith("%%HEADER%%"))
			{
				inheader= true;
			}
			else if (fileline.startsWith("%%CONSTANT%%"))
			{
				initem = true;
			}
			else if (fileline.startsWith("%%CHECKSUM%%"))
			{
			}
			else
			{

			}
		}
	}

	for (int i=0;i<itemlist.size();i++)
	{
		QVariantMap map = itemlist[i].toMap();
		if (map.contains("Title"))
		{
			ui.listWidget->addItem(map["Title"].toString());
			itemmap[map["Title"].toString()] = map;
		}
	}
	int stopper = 1;
	//openPort();
	apuOne.openPort("/dev/ttyUSB1");
	qDebug() << "Version:" << apuOne.getVersion();
	currentBin = apuOne.get(0x40,0xC0);
	//qDebug() << "Verify:" << verify("output.txt");
	///home/michael/EBL/EBL.BIN
	//loadAll("/home/michael/EBL/EBL.BIN",0xC0); //This loads to 16k before the end (48k offset)
	//loadAll("output.txt");*/
	/*unsigned char buf[4];
	buf[0] = 'B';
	buf[1] = 'R';
	buf[2] = 'R';
	buf[3] = 0;
	for (int i=0;i<3;i++)
	{
		buf[3] += buf[i];
	}
	write(m_portHandle,buf,4);
	int num = read(m_portHandle,buf,4);
	qDebug() << num << (char)buf[0]<< buf[1];*/
	//saveAll("output.txt");
	//qDebug() << "Verify:" << verify("output.txt");
	//qDebug() << "Verify:" << verify("/home/michael/EBL/EBL.BIN",0x40,0xC0);
	connected = false;
}

void MainWindow::connectButtonClicked()
{
	if (!connected)
	{
		ui.connectPushButton->setText("Disconnect");
		apuOne.openPort(ui.portNameLineEdit->text());
		QString version = apuOne.getVersion();
		this->setWindowTitle(version);
		connected = true;
	}
	else
	{
		ui.connectPushButton->setText("Connect");
		apuOne.closePort();
		connected = false;
	}
}

void MainWindow::ripButtonClicked()
{
	QByteArray total = apuOne.read(0x10000,0);
	QByteArray midone = apuOne.read(0x1234,0);

	if (total.mid(0,0x1234) == midone)
	{
		qDebug() << "Success one";
	}
	else
	{
		qDebug() << "failure one";
	}
	QByteArray midtwo = apuOne.read(0x1234,0x123);
	if (total.mid(0x123,0x1234) == midtwo)
	{
		qDebug() << "Success two";
	}
	else
	{
		qDebug() << "failure two";
	}
	return;
	QString ripoffsettext = ui.ripOffsetLineEdit->text();
	QString ripsizetext = ui.ripSizeLineEdit->text();
	QString filename = QFileDialog::getSaveFileName(0,"Save Binary File");
	if (filename != "")
	{
		QByteArray getbytes = apuOne.getBulk();
		if (getbytes.size() == 0)
		{
			QMessageBox::information(0,"Error","Error getting binary from device");
			return;
		}
		QFile file(filename);
		file.open(QIODevice::ReadWrite | QIODevice::Truncate);
		file.write(getbytes);
		file.flush();
		file.close();
	}
}

void MainWindow::currentItemRowChanged(int item)
{
	if (item != -1)
	{
		int counter = 0;
		ui.tableWidget->clear();
		ui.tableWidget->setRowCount(0);
		ui.tableWidget->setColumnCount(2);
		QString itemstr = ui.listWidget->item(item)->text();
		if (itemmap.contains(itemstr))
		{
			//ui.tableWidget->setRowCount(itemmap[itemstr].toMap().size());
			for (QVariantMap::const_iterator i=itemmap[itemstr].toMap().constBegin();i != itemmap[itemstr].toMap().constEnd();i++)
			{
				ui.tableWidget->setRowCount(ui.tableWidget->rowCount()+1);
				ui.tableWidget->setItem(counter,0,new QTableWidgetItem(i.key()));
				ui.tableWidget->setItem(counter,1,new QTableWidgetItem(i.value().toString()));
				counter++;
			}
			QString address = itemmap[itemstr].toMap()["Address"].toString();
			QString size = itemmap[itemstr].toMap()["SizeInBits"].toString();
			bool ok = false;
			unsigned int iaddress = address.split("x")[1].toInt(&ok,16);
			unsigned int isize = 8;
			if (size != "")
			{
				isize = size.split("x")[1].toInt(&ok,16);
			}
			unsigned int val = 0;
			for (int i=0;i<isize/8;i++)
			{
				val += ((unsigned char)currentBin[(iaddress+((isize/8)-1))-i]) << (i * 8);
			}
			QString equasion = itemmap[itemstr].toMap()["Equation"].toString();
			if (equasion != "")
			{
				equasion = equasion.split(",")[0];
				equasion = equasion.replace("X",QString::number(val));
				equasion = calc(equasion);
			}
			else
			{
				equasion = QString::number(val);
			}
			ui.lineEdit->setText(equasion);

		}
	}
}


void MainWindow::loadAll(QString filename,unsigned short offset)
{
	QFile output(filename);
	unsigned char buf[5];
	output.open(QIODevice::ReadOnly);
	QByteArray allbytes = output.readAll();

	unsigned char checksum = 0;
	for (int i=0;i<allbytes.size();i++)
	{
		checksum += (unsigned char)allbytes[i];
	}
	int len = allbytes.length();
	int loc = 0;
	QByteArray header;
	//int  = 0;
	int currblock = 0;
	currblock = offset;
	qDebug() << "Loc:" << loc << "Len:" << len;
	while (loc < len)
	{
		header.clear();
		if (len-loc >= (16*256))
		{
			header.append('Z');
			header.append('W');
			header.append((char)16);
			header.append((char)0);
			header.append(currblock);
			currblock+=16;
			checksum = 0;
			for (int i=0;i<(16*256);i++)
			{
				header.append(allbytes[loc]);
				//checksum += (unsigned char)allbytes[loc];
				loc++;
			}
			//checksum += (unsigned char)'Z';
			//checksum += (unsigned char)'W';
			//checksum += 1;
			//checksum += currblock;
			for (int i=0;i<header.size();i++)
			{
				checksum += (unsigned char)header[i];
			}
			header.append(checksum);
		}
		else
		{
			qDebug() << "Error sending last block" << len << loc;
			return;
			//header.append(0);
		}
		write(m_portHandle,header.constData(),header.size());
		//usleep(1000000);
		int readlen = read(m_portHandle,buf,5);
		//qDebug() << "Buf:" << (char)buf[0];
		if (buf[0] == '?')
		{
			qDebug() << "Command failure";
			return;
		}
		else if (buf[0] == 'O')
		{
			qDebug() << "Command success" << QString::number(currblock,16);
		}
		else
		{
			qDebug() << "Unknown return, command failure" << QString::number(buf[0],16) << "Len:" << readlen;
			return;
		}

	}


	header.append(len);
	allbytes.prepend('Z');
	allbytes.prepend('W');

	allbytes.append(checksum);
}


MainWindow::~MainWindow()
{
}

QString MainWindow::calc(QString str)
{
	//This is where the magic happens...
	//qDebug() << str;
	int start=-1;
	int stop=-1;
	for (int i=0;i<str.length();i++)
	{
		if (str[i] == '(')
		{
			if (start == -1)
				start = i+1;
		}
		if (str[i] == ')')
		{
				stop = i;
			//i = str.length();
		}
	}
	if (start != -1 && stop != -1)
	{
		QString tmp = calc(str.mid(start,stop-start));
		//qDebug() << "Old:" << str;
		str.replace(start-1,(stop-start)+2,tmp);
		//qDebug() << "New:" << str;
	}
	else
	{
		//qDebug() << "Inner function:" << str;
	}
	start = 0;
	stop = -1;
	//int op = 0;
	//int current;
	//"5+3*2+2-5*4/8;
	//5 3*2 2-5*4/8   + split
	//2 5*4/8     - split
	//5*4 8    / split
	//5 4   * split

	//"5+3*2+2-5*4/8;
	//5+3*2+2 5*4/8  - split
	//5 3*2 2-5*4/8   + split
	//5+3 2+2-5 4/8  * split

	for (int k=0;k<2;k++)
	{
	for (int i=0;i<str.length();i++)
	{
		if (k == 1)
		{
		if (str[i] == '+')
		{
			stop = -1;
			start = i;
			for (int j=i-1;j>=0;j--)
			{
				if (str[j] == '+' || str[j] == '-' || str[j] == '/' || str[j] == '*')
				{
					stop = j;
					j=0;
				}
			}
			if (stop == -1)
			{
				stop = 0;
			}
			int replacer = stop;
			QString tmp = str.mid(stop,start-stop);
			//qDebug() << "Found:" <<i<<stop<< tmp;
			stop = -1;
			for (int j=i+1;j<str.length();j++)
			{
				if (str[j] == '+' || str[j] == '-' || str[j] == '/' || str[j] == '*')
				{
					stop = j;
					j = str.length();
				}
			}
			if (stop == -1)
			{
				stop = str.length();
			}
			QString tmp2 = str.mid(start+1,stop-(start+1));
			//qDebug() << "Found2:" <<start<<stop<< tmp2;
			float first = tmp.toFloat();
			float second = tmp2.toFloat();
			str = str.replace(replacer,tmp.length()+tmp2.length()+1,QString::number(first+second));
			//qDebug() << str;
			i = 0;
		}
		if (str[i] == '-')
		{
			stop = -1;
			start = i;
			for (int j=i-1;j>=0;j--)
			{
				if (str[j] == '+' || str[j] == '-' || str[j] == '/' || str[j] == '*')
				{
					stop = j;
					j=0;
				}
			}
			if (stop == -1)
			{
				stop = 0;
			}
			int replacer = stop;
			QString tmp = str.mid(stop,start-stop);
			//qDebug() << "Found:" <<i<<stop<< tmp;
			stop = -1;
			for (int j=i+1;j<str.length();j++)
			{
				if (str[j] == '+' || str[j] == '-' || str[j] == '/' || str[j] == '*')
				{
					stop = j;
					j = str.length();
				}
			}
			if (stop == -1)
			{
				stop = str.length();
			}
			QString tmp2 = str.mid(start+1,stop-(start+1));
			//qDebug() << "Found2:" << tmp2;
			float first = tmp.toFloat();
			float second = tmp2.toFloat();
			str = str.replace(replacer,tmp.length()+tmp2.length()+1,QString::number(first-second));
			//qDebug() << str;
			i = 0;
		}
		}
		if (k == 0)
		{
		if (str[i] == '*')
		{
			stop = -1;
			start = i;
			for (int j=i-1;j>=0;j--)
			{
				if (str[j] == '+' || str[j] == '-' || str[j] == '/' || str[j] == '*')
				{
					stop = j;
					j=0;
				}
			}
			if (stop == -1)
			{
				stop = 0;
			}
			int replacer = stop;
			QString tmp = str.mid(stop,start-stop);
			//qDebug() << "Found:" << tmp;
			stop = -1;
			for (int j=i+1;j<str.length();j++)
			{
				if (str[j] == '+' || str[j] == '-' || str[j] == '/' || str[j] == '*')
				{
					stop = j;
					j = str.length();
				}
			}
			if (stop == -1)
			{

				stop = str.length();
			}
			QString tmp2 = str.mid(start+1,stop-(start+1));
			//qDebug() << "Found2:" << tmp2;
			float first = tmp.toFloat();
			float second = tmp2.toFloat();
			str = str.replace(replacer,tmp.length()+tmp2.length()+1,QString::number(first*second));
			//qDebug() << str;
			i = 0;
		}
		if (str[i] == '/')
		{
			stop = -1;
			start = i;
			for (int j=i-1;j>=0;j--)
			{
				if (str[j] == '+' || str[j] == '-' || str[j] == '/' || str[j] == '*')
				{
					stop = j;
					j=0;
				}
			}
			if (stop == -1)
			{
				stop = 0;
			}
			int replacer = stop;
			QString tmp = str.mid(stop,start-stop);
			//qDebug() << "Found:" << tmp;
			stop = -1;
			for (int j=i+1;j<str.length();j++)
			{
				if (str[j] == '+' || str[j] == '-' || str[j] == '/' || str[j] == '*')
				{
					stop = j;
					j = str.length();
				}
			}
			if (stop == -1)
			{
				stop = str.length();
			}
			QString tmp2 = str.mid(start+1,stop-(start+1));
			//qDebug() << "Found2:" << tmp2;
			float first = tmp.toFloat();
			float second = tmp2.toFloat();
			str = str.replace(replacer,tmp.length()+tmp2.length()+1,QString::number(first/second));
			//qDebug() << str;
			i = 0;
		}
		}
		//usleep(100000);
	}
	}
	return str;
}
void MainWindow::writeButtonClicked()
{
	QString filename = QFileDialog::getOpenFileName();
	if (filename == "")
	{
		return ;
	}
	QFile hexfile(filename);
	hexfile.open(QIODevice::ReadOnly);
	QByteArray hexbytes = hexfile.readAll();
	hexfile.close();
	apuOne.write(hexbytes,0);
}
