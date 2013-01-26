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
