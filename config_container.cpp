#include "config_container.h"
#include <QSettings>
#include <QTextCodec>

ConfigContainer::ConfigContainer()
{
    this->loadConfig();
}

void ConfigContainer::loadConfig()
{
    QSettings settings("./config.ini", QSettings::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    settings.beginGroup("log");
    this->logMode = LOGMODE(
                settings.value(QString("log_mode"), LOG_MODE_LOCAL).toInt()
            );
    settings.endGroup();

    settings.beginGroup("sensor");
    this->fanSpeedThreshold = settings.value(QString("fan_speed_threshold"), 40).toDouble();
    this->shutdownAfterThresholdExceeded = settings.value(QString("shutdown_after_threshold_exceeded"), 1).toBool();
    settings.endGroup();
}

void ConfigContainer::printConfig()
{
    puts("------ServerHealthMonitor------");
    printf("LogMode: %s\n", this->getLogMode() == LOG_MODE_LOCAL? "local" : "remote");
    printf("FanSpeedThreshold: %lf\n", this->getFanSpeedThreshold());
    printf("ShutdownAfterThresholdExceeded: %s\n", this->getShutdownAfterThresholdExceeded() == true ? "true" : "false");
}

ConfigContainer::LOGMODE ConfigContainer::getLogMode() const
{
    return logMode;
}

double ConfigContainer::getFanSpeedThreshold() const
{
    return fanSpeedThreshold;
}

bool ConfigContainer::getShutdownAfterThresholdExceeded() const
{
    return shutdownAfterThresholdExceeded;
}
