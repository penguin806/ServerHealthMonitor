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
}

void ConfigContainer::printConfig()
{
    puts("------ServerHealthMonitor------");
    printf("LogMode: %s\n", this->getLogMode() == LOG_MODE_LOCAL? "local" : "remote");
}

ConfigContainer::LOGMODE ConfigContainer::getLogMode() const
{
    return logMode;
}
