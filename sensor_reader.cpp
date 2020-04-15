#include "sensor_reader.h"
#include <QProcess>
#include <QDebug>

SensorReader::SensorReader(QObject *parent) : QObject(parent)
{

}

void SensorReader::startWorking()
{
    this->_readBmcSensor();

    QObject::connect(&this->readSensorTimer, SIGNAL(timeout()), this, SLOT(_readBmcSensor()));
    this->readSensorTimer.setInterval(20000);
    this->readSensorTimer.start();
}

void SensorReader::_readBmcSensor()
{
    QProcess ipmiUtilProcess;
    ipmiUtilProcess.start(
                QString::fromLocal8Bit("./ipmiutil-3.1.6-win64/ipmiutil.exe"),
                QStringList() << "sensor" << "-i 3e-4c",
                QIODevice::ReadOnly
            );
    bool rv = ipmiUtilProcess.waitForStarted(2000);
    if(!rv)
    {
        this->_errorOccured(QString("Start Fail! Code: ") + char(ipmiUtilProcess.error()));
        return;
    }

    rv = ipmiUtilProcess.waitForFinished(5000);
    if(!rv)
    {
        this->_errorOccured(QString("Crashed! Code: ") + char(ipmiUtilProcess.exitStatus()));
    }

    QByteArray outputData = ipmiUtilProcess.readAll();
    printf_s(outputData.data());
}

void SensorReader::_errorOccured(QString errorMsg)
{
    qDebug() << errorMsg;
    this->readSensorTimer.stop();
}
