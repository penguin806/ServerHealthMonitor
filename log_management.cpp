#include "log_management.h"
#include <QCoreApplication>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

LogManagement::LogManagement(ConfigContainer &globalConfig) : globalConfig(globalConfig)
{
    this->_initLoggingSystem();
}

LogManagement::~LogManagement()
{
    this->_destroyLoggingSystem();
}

void LogManagement::sensorDataReceived(QMap<QString, QString> &sensorData)
{
    qDebug() << sensorData;
}

void LogManagement::infoReceived(QString infoMsg)
{
    puts(infoMsg.toLocal8Bit().data());
}

void LogManagement::errorOccured(QString errorMsg, int errorLevel)
{
    puts(errorMsg.toLocal8Bit().data());
    this->_writeLog(errorMsg, "ERROR");
    if(globalConfig.getShutdownAfterThresholdExceeded())
    {
        this->_writeLog("Shutting down...", "INFO");
        this->_destroyLoggingSystem();
        system("shutdown /s /f /t 1 /c ServerHealthMonitor");
    }
}

void LogManagement::_writeLog(QString log, QString type)
{
    if(this->globalConfig.getLogMode() == ConfigContainer::LOG_MODE_LOCAL)
    {
        QTextStream stream(this->logFile);
        stream.setCodec("UTF-8");
        if(this->logFile)
        {
            stream << '[' + type + "] " +
                      QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + log + '\n';
        }
    }
}

void LogManagement::_initLoggingSystem()
{
    if(this->globalConfig.getLogMode() == ConfigContainer::LOG_MODE_LOCAL)
    {
        const QString logFilePath = "./server_health_monitor.log";
        this->logFile = new QFile(logFilePath);
        if(!this->logFile->open(QIODevice::Append | QIODevice::Text))
        {
            printf_s("Unable to write log file: %s", logFilePath.toLocal8Bit().data());
            getchar();
            QCoreApplication::quit();
        }
    }
    else
    {
        // remote logging mode
    }
}

void LogManagement::_destroyLoggingSystem()
{
    if(this->logFile)
    {
        this->logFile->close();
    }
}
