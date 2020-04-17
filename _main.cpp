#include <QCoreApplication>
#include "config_container.h"
#include "log_management.h"
#include "sensor_reader.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ServerHealthMonitor");
    QCoreApplication::setApplicationVersion("0.0.1");
    QCoreApplication::setOrganizationDomain("xuefeng.space");

    ConfigContainer globalConfig;
    globalConfig.printConfig();

    LogManagement logMan(globalConfig);

    SensorReader sensorReader(&app, globalConfig, logMan);
    sensorReader.startWorking();

    return app.exec();
}
