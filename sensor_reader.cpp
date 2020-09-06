#include "sensor_reader.h"
#include "config_container.h"
#include <QProcess>
#include <QTextStream>
#include <QMap>
#include <QRegularExpression>
#include <QDebug>

SensorReader::SensorReader(QObject *parent, ConfigContainer &globalConfig, LogManagement &logMan)
    : QObject(parent),  globalConfig(globalConfig), logMan(logMan)
{

}

void SensorReader::startWorking()
{
    this->_readBmcSensor();
    //puts("Start in 15 seconds...");

    QObject::connect(&this->readSensorTimer, SIGNAL(timeout()), this, SLOT(_readBmcSensor()));
    this->readSensorTimer.setInterval(15000);
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
        this->_errorOccured(QString("IpmiUtil start failed! Code: ") + QString::number(ipmiUtilProcess.error()));
        return;
    }

    rv = ipmiUtilProcess.waitForFinished(100000);
    if(!rv)
    {
        this->_errorOccured(QString("IpmiUtil crashed! Code: ") + QString::number(ipmiUtilProcess.exitStatus()));
    }

    const QByteArray outputData = ipmiUtilProcess.readAll();
    QMap<QString, QString> parsedData;
    if(!this->_parseIpmiUtilOutput(outputData, &parsedData))
    {
        this->_errorOccured("IpmiUtil failed!");
        return;
    }

    logMan.sensorDataReceived(parsedData);
    this->_checkParsedValue(parsedData);
}

void SensorReader::_infoReceived(QString infoMsg)
{
    logMan.infoReceived(infoMsg);
}

void SensorReader::_errorOccured(QString errorMsg)
{
    this->readSensorTimer.stop();
    logMan.errorOccured(errorMsg);
}

bool SensorReader::_parseIpmiUtilOutput(QByteArray ipmiUtilOutput, QMap<QString, QString> *parsedData)
{
    if(!parsedData)
    {
        return false;
    }

    QTextStream outputStream(ipmiUtilOutput);
    QString line;
    int i = 1;

    for(; outputStream.readLineInto(&line); i++)
    {
        if(1 == i)
        {
            QRegularExpression ipmiUtilVersionExp("^ipmiutil sensor version ([\\d\\.]+)$");
            QRegularExpressionMatch match = ipmiUtilVersionExp.match(line);
            if(!match.hasMatch() || !match.capturedLength(1))
            {
                this->_infoReceived("Unknown ipmiutil version, raw data:\n" + ipmiUtilOutput);
                return false;
            }
            else
            {
                // KEY1: IPMIUTIL Version
                parsedData->insert("ipmiutilVersion", match.captured(1));
            }
        }
        else if(2 == i)
        {
            QString idRangeLeft = line.split(" = ", QString::SkipEmptyParts).at(1);
            if(idRangeLeft != QString("0x3e"))
            {
                this->_infoReceived("IdRangeLeft do not match: " + idRangeLeft);
                return false;
            }
        }
        else if(3 == i)
        {
            QString idRangeRight = line.split(" = ", QString::SkipEmptyParts).at(1);
            if(idRangeRight != QString("0x4c"))
            {
                this->_infoReceived("IdRangeRight do not match: " + idRangeRight);
                return false;
            }
        }
        else if(4 == i)
        {
            QRegularExpression bmcIpmiVersionExp("^-- BMC version ([\\d\\.]+), IPMI version ([\\d\\.]+)");
            QRegularExpressionMatch match = bmcIpmiVersionExp.match(line);
            if(!match.hasMatch() || !match.capturedLength(1) || !match.capturedLength(2))
            {
                this->_infoReceived("Unknown ipmiutil/bmc version: \n" + line);
                return false;
            }
            else
            {
                // KEY2: BMC/IPMI Version
                parsedData->insert("bmcIpmiVersion", match.captured(1) + '/' + match.captured(2));
            }
        }
        else if(5 == i)
        {
            if(line != QString("_ID_ SDR_Type_xx ET Own Typ S_Num   Sens_Description   Hex & Interp Reading"))
            {
                this->_infoReceived("Header do not match: " + line);
                return false;
            }
        }
        else if(6 <= i && i <= 13)
        {
            //Fan 1-8
            int fanID = i - 5;
            QRegularExpression fanInfoExp("^(\\w{4}) SDR Full .{13} snum (\\w{2}) Fan Block (\\d) = \\w{2} (\\d{1,3}\\.\\d{1,2}) %, (.*)$");
            QRegularExpressionMatch match = fanInfoExp.match(line);
            if(
                    !match.hasMatch() || !match.capturedLength(1) || !match.capturedLength(2)
                    || match.captured(3) != QString::number(fanID) || match.captured(4).toDouble() == 0.0
                    || !match.capturedLength(5)
            )
            {
                this->_infoReceived("Fan " + QString::number(fanID) + " read error: \n" + line);
                return false;
            }
            else
            {
                // KEY3-10: Fan 1-8 Speed
                parsedData->insert("fan" + QString::number(fanID), match.captured(4));
            }
        }
        else if(14 <= i && i <= 18)
        {
            continue;
        }
        else if(19 == i)
        {
            QRegularExpression isRedundant("^004b SDR Full .* Fully Redundant$");
            QRegularExpressionMatch match = isRedundant.match(line);
            if(!match.hasMatch())
            {
                this->_infoReceived("Fan redundancy lost: \n" + line);
                return false;
            }
        }
        else if(20 == i)
        {
            if(line != QString("ipmiutil sensor, completed successfully"))
            {
                this->_infoReceived("Complete line do not match: " + line);
                return false;
            }
        }
    }
    if(21 != i)
    {
        this->_infoReceived("Incomplete output:\n" + ipmiUtilOutput);
        return false;
    }

    return true;
}

void SensorReader::_checkParsedValue(QMap<QString, QString> &parsedData)
{
    if(10 != parsedData.size())
    {
        this->_errorOccured("Incomplete data map: " + QString::number(parsedData.size()));
        return;
    }
    for(int fanNum = 1; fanNum <= 8; fanNum++)
    {
        QString keyName = "fan" + QString::number(fanNum);
        if(parsedData.value(keyName).toDouble() > globalConfig.getFanSpeedThreshold())
        {
            this->_errorOccured("Fan " + QString::number(fanNum) + " speed exceeds threshold: " + parsedData.value(keyName));
            return;
        }
    }
}
