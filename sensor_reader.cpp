#include "sensor_reader.h"
#include <QProcess>
#include <QTextStream>
#include <QMap>
#include <QRegularExpression>
#include <QDebug>

SensorReader::SensorReader(QObject *parent) : QObject(parent)
{

}

void SensorReader::startWorking()
{
    this->_readBmcSensor();

    QObject::connect(&this->readSensorTimer, SIGNAL(timeout()), this, SLOT(_readBmcSensor()));
    this->readSensorTimer.setInterval(30000);
    this->readSensorTimer.start();
}

void SensorReader::_readBmcSensor()
{
    QString ipmiUtilPath = QString::fromLocal8Bit("./ipmiutil-3.1.6-win64/ipmiutil.exe");
    QProcess ipmiUtilProcess;

    ipmiUtilProcess.start(
                ipmiUtilPath,
                QStringList() << "sensor" << "-i 3e-4c",
                QIODevice::ReadOnly
            );

    bool rv = ipmiUtilProcess.waitForStarted(3000);
    if(!rv)
    {
        this->_errorOccured(QString("Start Fail! Code: ") + QString::number(ipmiUtilProcess.error()));
        return;
    }

    rv = ipmiUtilProcess.waitForFinished(100000);
    if(!rv)
    {
        this->_errorOccured(QString("Crashed! Code: ") + QString::number(ipmiUtilProcess.exitStatus()));
    }

    QByteArray outputData = ipmiUtilProcess.readAll();
    qDebug() << outputData.data();
}

void SensorReader::_errorOccured(QString errorMsg)
{
    qDebug() << errorMsg;
    this->readSensorTimer.stop();
}

bool SensorReader::_parseIpmiUtilOutput(QByteArray ipmiUtilOutput)
{
    QTextStream outputStream(ipmiUtilOutput);
    QString line;
    QMap<QString, QString> parsedData;
    int i = 1;

    for(; outputStream.readLineInto(&line); i++)
    {
        if(1 == i)
        {
            QRegularExpression ipmiUtilVersionExp("^ipmiutil sensor version ([\\d\\.]+)$");
            QRegularExpressionMatch match = ipmiUtilVersionExp.match(line);
            if(!match.hasMatch() || !match.capturedLength(1))
            {
                this->_errorOccured("Unknown ipmiutil version, raw data:\n" + ipmiUtilOutput);
                return false;
            }
            else
            {
                parsedData.insert("ipmiutilVersion", match.captured(1));
            }
        }
        else if(2 == i)
        {
            QString idRangeLeft = line.split(" = ", QString::SkipEmptyParts).at(1);
            if(idRangeLeft != QString("0x3e"))
            {
                this->_errorOccured("IdRangeLeft do not match: " + idRangeLeft);
                return false;
            }
        }
        else if(3 == i)
        {
            QString idRangeRight = line.split(" = ", QString::SkipEmptyParts).at(1);
            if(idRangeRight != QString("0x4c"))
            {
                this->_errorOccured("IdRangeRight do not match: " + idRangeRight);
                return false;
            }
        }
        else if(4 == i)
        {
            QRegularExpression bmcIpmiVersionExp("^-- BMC version ([\\d\\.]+), IPMI version ([\\d\\.]+)$");
            QRegularExpressionMatch match = bmcIpmiVersionExp.match(line);
            if(!match.hasMatch() || !match.capturedLength(1) || !match.capturedLength(2))
            {
                this->_errorOccured("Unknown ipmiutil/bmc version: \n" + line);
                return false;
            }
            else
            {
                parsedData.insert("bmcIpmiVersion", match.captured(1) + '/' + match.captured(2));
            }
        }
        else if(5 == i)
        {
            if(line != QString("_ID_ SDR_Type_xx ET Own Typ S_Num   Sens_Description   Hex & Interp Reading"))
            {
                this->_errorOccured("Header do not match: " + line);
                return false;
            }
        }
        else if(6 <= i && i <= 13)
        {
            //Fan 1-8
            int fanID = i - 5;
            QRegularExpression fanInfoExp("^(\\w{4}) SDR Full .{13} snum (\\w{2}) Fan Block (\\d) = \\d{1,3} (\\d{1,3}\\.\\d{1,2}) %, (.*)$");
            QRegularExpressionMatch match = fanInfoExp.match(line);
            if(
                    !match.hasMatch() || !match.capturedLength(1) || !match.capturedLength(2)
                    || match.captured(3) != QString::number(fanID) || match.captured(4).toDouble() == 0.0
                    || !match.capturedLength(5)
            )
            {
                this->_errorOccured("Fan " + QString::number(fanID) + " read error: \n" + line);
                return false;
            }
            else
            {
                parsedData.insert("fan" + QString::number(fanID), match.captured(4));
            }
        }
        else if(14 <= i && i <= 18)
        {
            continue;
        }
        else if(19 ==i)
        {

        }
        else if(20 == i)
        {
            if(line != QString("ipmiutil sensor, completed successfully"))
            {
                this->_errorOccured("Complete line do not match: " + line);
                return false;
            }
        }
    }
    if(21 != i)
    {
        this->_errorOccured("Incomplete output:\n" + ipmiUtilOutput);
        return false;
    }

    return true;
}
