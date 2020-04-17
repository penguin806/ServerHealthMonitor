#ifndef LOGMANAGEMENT_H
#define LOGMANAGEMENT_H

#include "config_container.h"
#include <QMap>
#include <QFile>

class LogManagement
{
public:
    LogManagement(ConfigContainer &globalConfig);
    ~LogManagement();
    void sensorDataReceived(QMap<QString, QString> &sensorData);
    void infoReceived(QString infoMsg);
    void errorOccured(QString errorMsg, int errorLevel = 0);

private:
    void _writeLog(QString log, QString type);
    void _initLoggingSystem();
    void _destroyLoggingSystem();

    ConfigContainer &globalConfig;
    QFile *logFile;
};

#endif // LOGMANAGEMENT_H
