#include <QCoreApplication>
#include "log_management.h"
#include "sensor_reader.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ServerHealthMonitor");
    QCoreApplication::setApplicationVersion("0.0.1");
    QCoreApplication::setOrganizationDomain("xuefeng.space");

    LogManagement logMan;

    SensorReader sensorReader(&app);
    sensorReader.startWorking();

    return app.exec();
}
