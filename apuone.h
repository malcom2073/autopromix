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

#ifndef APUONE_H
#define APUONE_H


#include <QObject>

#ifdef Q_OS_WIN32
#include <windows.h>
#else
//#define HANDLE int
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif
#include <vector>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QMutex>

#include "qserialport.h"

class ApuOne : public QThread
{
	Q_OBJECT
public:
	explicit ApuOne(QObject *parent = 0);
	void openPort(QString portName);
	void closePort();
	QString getVersion();
	QByteArray getBulk(unsigned char size=0,unsigned char offset=0);
	QByteArray get(unsigned char size,unsigned short offset);
	bool verify(QString filename,unsigned char size=0,unsigned char offset=0);
	void saveAll(QString filename);
	void write(QByteArray buffer,unsigned short offset);
	QByteArray read(unsigned int size,unsigned short offset);
protected:
	void run();
private:
	class RequestClass
	{
	public:
		QString type;
		QByteArray bytes;
		QList<unsigned int> args;
	};

	QMutex m_reqListMutex;
	QList<RequestClass> m_reqList;
	int m_portHandle;
	QSerialPort *m_port;
	int readBytes(QByteArray *buf,int size, int timeout);
	QByteArray m_privBuffer;
signals:
	void version(QString version);
public slots:
	
};

#endif // APUONE_H
