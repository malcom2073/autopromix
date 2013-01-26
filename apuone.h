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
#include <QFile>

class ApuOne : public QObject
{
	Q_OBJECT
public:
	explicit ApuOne(QObject *parent = 0);
	void openPort(QString portName);
	void closePort();
	QString getVersion();
	QByteArray get(unsigned char size=0,unsigned char offset=0);
	bool verify(QString filename,unsigned char size=0,unsigned char offset=0);
	void saveAll(QString filename);
private:
	int m_portHandle;
signals:
	
public slots:
	
};

#endif // APUONE_H
