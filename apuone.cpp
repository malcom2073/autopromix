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


#include "apuone.h"

ApuOne::ApuOne(QObject *parent) :
	QObject(parent)
{
}
void ApuOne::openPort(QString portName)
{
#ifdef Q_OS_WIN32
	m_portHandle=CreateFileA(portName.toAscii(), GENERIC_READ|GENERIC_WRITE,0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_portHandle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	COMMCONFIG Win_CommConfig;

	unsigned long confSize = sizeof(COMMCONFIG);
	Win_CommConfig.dwSize = confSize;
	GetCommConfig(m_portHandle, &Win_CommConfig, &confSize);
	if (oddparity)
	{
		qDebug() << "SerialPort Odd parity selected";
		Win_CommConfig.dcb.Parity = 1; //Odd parity
	}
	else
	{
		qDebug() << "SerialPort No parity selected";
		Win_CommConfig.dcb.Parity = 0; //No parity
	}
	Win_CommConfig.dcb.fRtsControl = RTS_CONTROL_DISABLE;
	Win_CommConfig.dcb.fOutxCtsFlow = FALSE;
	Win_CommConfig.dcb.fOutxDsrFlow = FALSE;
	Win_CommConfig.dcb.fDtrControl = DTR_CONTROL_DISABLE;
	Win_CommConfig.dcb.fDsrSensitivity = FALSE;
	Win_CommConfig.dcb.fNull=FALSE;
	Win_CommConfig.dcb.fTXContinueOnXoff = FALSE;
	Win_CommConfig.dcb.fInX=FALSE;
	Win_CommConfig.dcb.fOutX=FALSE;
	Win_CommConfig.dcb.fBinary=TRUE;
	Win_CommConfig.dcb.DCBlength = sizeof(DCB);
	if (baudrate != -1)
	{
		Win_CommConfig.dcb.BaudRate = baudrate;
	}
	else
	{
		Win_CommConfig.dcb.BaudRate = 115200;
	}
	Win_CommConfig.dcb.ByteSize = 8;
	COMMTIMEOUTS Win_CommTimeouts;
	Win_CommTimeouts.ReadIntervalTimeout = MAXDWORD; //inter-byte timeout value (Disabled)
	Win_CommTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD; //Multiplier
	Win_CommTimeouts.ReadTotalTimeoutConstant = 10; //Total timeout, 1 second to match *nix
	Win_CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	Win_CommTimeouts.WriteTotalTimeoutConstant = 0;
	SetCommConfig(m_portHandle, &Win_CommConfig, sizeof(COMMCONFIG));
	SetCommTimeouts(m_portHandle,&Win_CommTimeouts);
	SetCommMask(m_portHandle,EV_RXCHAR);
	return 0;
#else

	m_portHandle = open(portName.toAscii(),O_RDWR | O_NOCTTY | O_NDELAY); //Should open the port non blocking
	if (m_portHandle < 0)
	{
		//printf("Error opening Com: %s\n",portName);
		//debug(obdLib::DEBUG_ERROR,"Error opening com port %s",portName);
		perror("Error opening COM port Low Level");
		qDebug() << "Port:" << portName;
		return ;
	}
	if (flock(m_portHandle,LOCK_EX | LOCK_NB))
	{
		qDebug() << "Unable to maintain lock on serial port" << portName;
		qDebug() << "This port is likely open in another process";
		return ;
	}
	//printf("Com Port Opened %i\n",portHandle);
	//debug(obdLib::DEBUG_VERBOSE,"Com Port Opened %i",portHandle);
	//fcntl(m_portHandle, F_SETFL, FASYNC); //Set it to blocking. This is required? Wtf?
	//struct termios oldtio;
	struct termios newtio;
	//bzero(&newtio,sizeof(newtio));
	tcgetattr(m_portHandle,&newtio);
	long BAUD = B115200;
	int baudrate = 115200;
	switch (baudrate)
	{
		case 38400:
			BAUD = B38400;
			break;
		case 115200:
			BAUD  = B115200;
			break;
		case 19200:
			BAUD  = B19200;
			break;
		case 9600:
			BAUD  = B9600;
			break;
		case 4800:
			BAUD  = B4800;
			break;
		default:
			BAUD = B115200;
			break;
	}  //end of switch baud_rate
	if (strspn("/dev/pts",portName.toAscii()) >= 8)
	{
		//debug(obdLib::DEBUG_WARN,"PTS Detected... disabling baud rate selection on: %s",portName);
		//printf("PTS detected... disabling baud rate selection: %s\n",portName);
		baudrate = -1;
	}
	else
	{
	}
	newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS8;
	newtio.c_cflag |= CLOCAL | CREAD;
	bool oddparity = false;
	if (oddparity)
	{
		newtio.c_cflag |= (PARENB | PARODD);
	}
	else
	{
		newtio.c_cflag &= ~(PARENB | PARODD);
	}
	newtio.c_cflag &= ~CRTSCTS;
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_iflag=IGNBRK;
	newtio.c_iflag &= ~(IXON|IXOFF|IXANY);
	newtio.c_lflag=0;
	newtio.c_oflag=0;
	newtio.c_cc[VTIME]=10; //1 second timeout, to allow for quitting the read thread
	newtio.c_cc[VMIN]=0; //We want a pure timer timeout
	if (baudrate != -1)
	{
		if(cfsetispeed(&newtio, BAUD))
		{
			//perror("cfsetispeed");
		}

		if(cfsetospeed(&newtio, BAUD))
		{
			//perror("cfsetospeed");
		}
		//debug(obdLib::DEBUG_VERBOSE,"Setting baud rate to %i on port %s\n",baudrate,portName);
	}
	fcntl(m_portHandle, F_SETFL, 0); //Set to blocking
	tcsetattr(m_portHandle,TCSANOW,&newtio);
	tcflush(m_portHandle,TCIOFLUSH);
	//return;
#endif //Q_OS_WIN32

	//read
}
void ApuOne::closePort()
{
	close(m_portHandle);
	m_portHandle = 0;
}

QString ApuOne::getVersion()
{
	tcflush(m_portHandle,TCIOFLUSH);
	unsigned char buf[2];
	buf[0] = 'V';
	buf[1] = 'V';
	write(m_portHandle,buf,2);
	unsigned char bufret[3];
	int count = read(m_portHandle,&bufret,3);
	qDebug() << "Read:" << count << bufret[0] << bufret[1] << bufret[2];
	QString retval = "";
	if ((char)bufret[2] == 'A')
	{
		//APU 1
		retval = "AutoProm Version " + QString::number(bufret[0]) + "." + QString::number(bufret[1]);
	}
	tcflush(m_portHandle,TCIOFLUSH);
	return retval;
}
QByteArray ApuOne::get(unsigned char size,unsigned char offset)
{
	tcflush(m_portHandle,TCIOFLUSH);
	unsigned char buf[5];
	buf[0] = 'Z';
	buf[1] = 'R';
	buf[2] = size;
	buf[3] = 0x0;
	buf[4] = offset;
	buf[5] = 0x0;
	for (int i=0;i<5;i++)
	{
		buf[5]+=buf[i];
	}
	write(m_portHandle,buf,6);
	//usleep(1000000);
	unsigned char newbuf[0x100];
	int count = 0;
	int pos = offset << 8;
	QByteArray array;
	while (pos < (0x10001))
	{
		int count = read(m_portHandle,newbuf,0xFF);
		array.append((char*)newbuf,count);
		pos += count;
		qDebug() << "Done:" << count << QString::number(pos,16).toUpper();
	}

	//int count = read(m_portHandle,newbuf,65);
	qDebug() << "Read:" << count;
	unsigned char checksum = 0;
	for (int i=0;i<array.size()-1;i++)
	{
		checksum += (unsigned char)array.at(i);
	}
	if (((unsigned char)array.at(array.length()-1)) != checksum)
	{
		qDebug() << "Invalid checksum";
		return QByteArray();
	}
	else
	{
		qDebug() << "valid checksum";
	}
	return array.mid(0,array.length()-1);
}
bool ApuOne::verify(QString filename,unsigned char size,unsigned char offset)
{
	QFile output(filename);
	output.open(QIODevice::ReadOnly);
	QByteArray allbytes = output.readAll();
	output.close();
	QByteArray device = get(size,offset);
	if (allbytes == device)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void ApuOne::saveAll(QString filename)
{
	QByteArray array = get();
	QFile output(filename);
	output.open(QIODevice::ReadWrite | QIODevice::Truncate);
	output.write(array.data(),array.length());
	output.flush();
	output.close();
}
