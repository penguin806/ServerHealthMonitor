#ifndef SENSORREADER_H
#define SENSORREADER_H

#include "config_container.h"
#include "log_management.h"
#include <QString>
#include <QTimer>

class SensorReader : QObject
{
    Q_OBJECT
public:
    SensorReader(QObject *parent, ConfigContainer &globalConfig, LogManagement &logMan);
    void startWorking();

private slots:
    void _readBmcSensor();

private:
    void _infoReceived(QString infoMsg);
    void _errorOccured(QString errorMsg);
    bool _parseIpmiUtilOutput(QByteArray ipmiUtilOutput, QMap<QString, QString> *parsedData);
    void _checkParsedValue(QMap<QString, QString> &parsedData);
    QTimer readSensorTimer;
    ConfigContainer &globalConfig;
    LogManagement &logMan;
};

#endif // SENSORREADER_H
