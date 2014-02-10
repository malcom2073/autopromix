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
	QThread(parent)
{
}
void ApuOne::openPort(QString portName)
{
	m_port = new QSerialPort(this);
	m_port->setPortName(portName);
	m_port->open(QIODevice::ReadWrite);
	m_port->setBaudRate(921600);
	m_port->setParity(QSerialPort::NoParity);
	m_port->setDataBits(QSerialPort::Data8);
	m_port->setStopBits(QSerialPort::OneStop);
	QByteArray buf;
	buf.append('B');
	buf.append('R');
	buf.append('R');
	unsigned char checksum = 0;
	for (int i=0;i<buf.size();i++)
	{
		checksum += ((unsigned char)buf.at(i));

	}
}
void ApuOne::closePort()
{
	m_port->close();
	delete m_port;
	m_port = 0;
}
void ApuOne::run()
{
	QList<RequestClass> privReqList;
	while (true)
	{
		m_reqListMutex.lock();
		for (int i=0;i<m_reqList.size();i++)
		{
			privReqList.append(m_reqList[i]);
		}
		m_reqList.clear();
		m_reqListMutex.unlock();
		for (int i=0;i<privReqList.size();i++)
		{
			if (privReqList[i].type == "GET_VERSION")
			{
				QString versionstr = getVersion();
				emit version(versionstr);
			}
			else if (privReqList[i].type == "WRITE_BYTES")
			{
				unsigned short offset = privReqList[i].args[0];
				unsigned short size = privReqList[i].bytes.size();
				write(privReqList[i].bytes,offset);
			}
			else if (privReqList[i].type == "READ_BYTES")
			{
			}
		}
		privReqList.clear();
	}
}

QString ApuOne::getVersion()
{
	QByteArray buf;
	buf.append('V');
	buf.append('V');
	m_port->write(buf);
	QByteArray bufret;
	int count = readBytes(&bufret,3,1000);
	qDebug() << "Read:" << count << bufret[0] << bufret[1] << bufret[2];
	QString retval = "";
	if (bufret[2] == 'A')
	{
		//APU 1
		retval = "AutoProm Version " + QString::number(bufret[0]) + "." + QString::number(bufret[1]);
	}
	else if (bufret[2] == 'O')
	{
		//Ostrich
		retval = "Ostrich Version " + QString::number(bufret[0]) + "." + QString::number(bufret[1]);
	}
	return retval;
}
void ApuOne::write(QByteArray buffer,unsigned short offset)
{
	qDebug() << "Writing buffer:" << buffer.size() << "to offset" << offset;
	QByteArray header;
	QByteArray packet;
	QByteArray sizedpacket;
	int bufpos = 0;
	int currblock = offset;
	QByteArray readBuf;
	while (bufpos < (buffer.size() - packet.size()))
	{
		sizedpacket.clear();
		packet.clear();
		packet = buffer.mid(bufpos,16*256);
		bufpos += packet.size();
		//Header has the current size, bufpos has the current buffer position (in buffer);
		int chunkcount = packet.size() / 0x100; //This is hte number of 256 byte chunks that are currently in header.
		header.clear();
		header.append('Z');
		header.append('W');
		header.append((char)chunkcount);
		header.append((char)0);
		header.append(currblock);
		sizedpacket = packet.mid(0,chunkcount*0x100);
		packet = packet.mid(chunkcount*0x100);
		header.append(sizedpacket);

		unsigned char checksum = 0;
		for (int i=0;i<header.size();i++)
		{
			checksum += ((unsigned char)header.at(i));
		}
		header.append(checksum);
		qDebug() << "Total packet size to write:" << header.size();
		qDebug() << "Packet bytes left over:" << packet.size();
		qDebug() << "Offset (divided by 256):" << currblock;
		qDebug() << "Total bytes left:" << buffer.size() - bufpos;
		currblock += chunkcount;
		m_port->write(header);
		readBuf.clear();
		int count = readBytes(&readBuf,1,500);
		if (readBuf.at(0) == 'O')
		{
			//Ok!
		}
		else
		{
			qDebug() << "Invalid write";
			return;
		}
		//Header is a full packet now, ready to be sent.
		//buffer may have some left in it, if not, then packet will have the remainder of the less than 256 bytes in it, needing to be sent.
	}
	//packet may still have some in it, if so, send it.
	if (packet.size() > 0)
	{
		header.clear();
		header.append('W');
		header.append(packet.size());
		unsigned short location = currblock * 0x100;
		header.append((char)((location >> 8) & 0xFF));
		header.append(location & 0xFF);
		header.append(packet);
		unsigned short checksum = 0;
		for (int i=0;i<header.size();i++)
		{
			checksum += ((unsigned char)header.at(i));
		}
		header.append(checksum);
		m_port->write(header);
		readBuf.clear();
		int count = readBytes(&readBuf,1,500);
		if (readBuf.at(0) == 'O')
		{
			//Ok!
		}
		else
		{
			qDebug() << "Invalid write";
			return;
		}

	}
	QByteArray checkbuf;
	unsigned char chunks = buffer.size() / (0x100 * 16);
	//chunks is the number of 4k chunks there are.
	for (int i=0;i<chunks;i++)
	{
		checkbuf.append(getBulk(16,i * 16));
	}
	//Need to get the remainder.
	//buffer.size() - chunks * 16
	int newsize = (buffer.size() - (chunks * (0x100 * 16)));
	if (newsize > 0)
	{
		checkbuf.append(get(newsize,chunks * 16));
	}
	//checkbuf.append(getBulk(buffer.size() - (chunks * 0x100 * 16),(chunks * 16)));
	//QByteArray checkbuf = get();
	if (checkbuf == buffer)
	{
		qDebug() << "Everything is happy!";
	}
	else
	{
		qDebug() << "Invalid write!";
	}
	return;
	if ((buffer.size() - bufpos) > 0xFF)
	{
		//Do a bulk write
		int len = buffer.size();
		int loc = 0;
		int currblock = offset;
		while (loc < len)
		{
			int size = buffer.size() / 0x100;
			if (size > 16)
			{
				size = 16;
			}
			//Size is how many 256 byte blocks there are.
			header.clear();
			header.append('Z');
			header.append('W');
			header.append((char)size);
			header.append((char)0);
			header.append(currblock);
			header.append(packet);
			unsigned short checksum = 0;
			for (int i=0;i<header.size();i++)
			{
				checksum += ((unsigned char)header.at(i));
			}
			header.append(checksum);
			//Header is a full packet now, ready to be sent.

		}
	}
	//QFile output(filename);
	//unsigned char buf[5];
	//output.open(QIODevice::ReadOnly);
	//QByteArray allbytes = output.readAll();

	/*unsigned char checksum = 0;
	for (int i=0;i<allbytes.size();i++)
	{
		checksum += (unsigned char)allbytes[i];
	}
	int len = allbytes.length();
	int loc = 0;

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

	allbytes.append(checksum);*/
}
QByteArray ApuOne::read(unsigned int size,unsigned short offset)
{
	qDebug() << "read" << size << offset;
	QByteArray checkbuf;
	unsigned char chunks = size / (0x100 * 16);
	qDebug() << "read chunks" << chunks;
	//chunks is the number of 4k chunks there are.
	if (offset % 0x100)
	{
		qDebug() << "Offset is odd";
		//Offset is not evenly divisible,
		//need to read from offset, up to the closest 0x100 line.
		int nextoffset = 0x1000 * ((int)((offset+0x1000) / 0x1000));
		//Read from offset to nextoffset,
		//and bulk read from nextoffset to size
		unsigned int smallchunks = (nextoffset - offset) / 0x100;
		if (offset + size < nextoffset)
		{
			qDebug() << "Offset to nextoffset is small";
			//Need a series of reads to read to the next 4k boundry.
			//only need a single read here, since
			//size is actually less than 0x100.
			smallchunks = size / 0x100;
			for (int i=0;i<smallchunks;i++)
			{
				checkbuf.append(get(0x100,offset + (i * 0x100)));
			}
			checkbuf.append(get(size - (smallchunks * 0x100),offset + (smallchunks * 0x100)));
			return checkbuf;
		}
		else
		{
			qDebug() << "Offset to nextoffset has multiple chunks << smallchunks";
			for (int i=0;i<smallchunks;i++)
			{
				checkbuf.append(get(0,offset + (i*0x100)));
			}
			int newsize = (nextoffset - offset) - (smallchunks * 0x100);
			//checkbuf.append
			checkbuf.append(get(newsize,offset + (smallchunks * 0x100)));
			offset = (offset + (smallchunks * 0x100) + newsize);
			size = size - ((smallchunks * 0x100) + newsize);
		}
	}
	qDebug() << "Chunks was:" << chunks;
	chunks = size / (0x100 * 16);
	qDebug() << "Chunks is now:" << chunks;
	for (int i=0;i<chunks;i++)
	{
		checkbuf.append(getBulk(16,(offset / 0x100) + (i * 16)));
	}
	//Need to get the remainder.
	//buffer.size() - chunks * 16

	int newsize = (size - (chunks * (0x100 * 16)));
	unsigned int smallchunks = newsize / 0x100;
	for (int i=0;i<smallchunks;i++)
	{
		checkbuf.append(get(0,(offset) + (chunks * 16 * 0x100) + (i * 0x100)));
	}
	newsize = (size - (chunks * (0x100 * 16)) - (smallchunks * 0x100));
	if (newsize > 0)
	{
		checkbuf.append(get(newsize,(offset) + (chunks * 16 * 0x100) + (smallchunks * 0x100)));
	}
	return checkbuf;
}

QByteArray ApuOne::get(unsigned char size,unsigned short offset)
{
	unsigned short newsize = size;
	if (size == 0)
	{
		newsize = 256;
	}
	qDebug() << "get" << newsize << offset;
	QByteArray sendbuf;
	sendbuf.append('R');
	sendbuf.append(size);
	sendbuf.append((char)(offset >> 8) & 0xFF);
	sendbuf.append((char)offset & 0xFF);
	unsigned char checksum=0;
	for (int i=0;i<sendbuf.size();i++)
	{
		checksum += ((unsigned char)sendbuf.at(i));
	}
	sendbuf.append(checksum);
	m_port->write(sendbuf);
	QByteArray newbuf;
	int count = readBytes(&newbuf,newsize+1,1000);
	if (count != newsize+1)
	{
		qDebug() << "Failure to read";
		return QByteArray();
	}
	checksum = 0;
	for (int i=0;i<newbuf.size()-1;i++)
	{
		checksum += ((unsigned char)newbuf.at(i));
	}
	if (((unsigned char)newbuf.at(newbuf.length()-1)) != checksum)
	{
		qDebug() << "Invalid checksum. Expected: " <<  checksum << "got: " << ((unsigned char)newbuf.at(newbuf.length()-1));
		return QByteArray();
	}
	qDebug() << "Valid checksum";
	return newbuf.mid(0,newbuf.length()-1);
}

QByteArray ApuOne::getBulk(unsigned char size,unsigned char offset)
{
	qDebug() << "getBulk" << size << offset;
	QByteArray sendbuf;
	sendbuf.append('Z');
	sendbuf.append('R');
	sendbuf.append(size);
	sendbuf.append((char)0x0);
	sendbuf.append(offset);
	unsigned char checksum=0;
	for (int i=0;i<sendbuf.size();i++)
	{
		checksum += ((unsigned char)sendbuf.at(i));
	}
	sendbuf.append(checksum);
	m_port->write(sendbuf);

	int count = 0;
	int pos = 0;
	QByteArray array;
	while (pos < (size * 0x100))
	{
		QByteArray newbuf;
		int count = readBytes(&newbuf,0xFF,500);
		array.append(newbuf);
		pos += count;
	}
	checksum = 0;
	for (int i=0;i<array.size()-1;i++)
	{
		checksum += ((unsigned char)array.at(i));
	}
	if (((unsigned char)array.at(array.length()-1)) != checksum)
	{
		qDebug() << "Invalid checksum. Expected: " <<  checksum << "got: " << ((unsigned char)array.at(array.length()-1));
		return QByteArray();
	}
	qDebug() << "Valid checksum for offset" << offset << "Size:" << size;
	return array.mid(0,array.length()-1);
}
bool ApuOne::verify(QString filename,unsigned char size,unsigned char offset)
{
	QFile output(filename);
	output.open(QIODevice::ReadOnly);
	QByteArray allbytes = output.readAll();
	output.close();
	QByteArray device = getBulk(size,offset);
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
	QByteArray array = getBulk();
	QFile output(filename);
	output.open(QIODevice::ReadWrite | QIODevice::Truncate);
	output.write(array.data(),array.length());
	output.flush();
	output.close();
}
int ApuOne::readBytes(QByteArray *buf,int size, int timeout)
{
	if (m_privBuffer.size() >= size)
	{
		*buf = m_privBuffer.mid(0,size);
		m_privBuffer = m_privBuffer.remove(0,size);
		return size;
	}
	while (m_port->waitForReadyRead(timeout))
	{
		m_privBuffer.append(m_port->readAll());
		if (m_privBuffer.size() >= size)
		{
			*buf = m_privBuffer.mid(0,size);
			m_privBuffer = m_privBuffer.remove(0,size);
			return size;
		}
	}
	if (m_privBuffer.size() > 0)
	{
		*buf = m_privBuffer;
		m_privBuffer.clear();
	}
	return buf->size();
}
