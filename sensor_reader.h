#ifndef SENSORREADER_H
#define SENSORREADER_H

#include <QString>
#include <QTimer>

class SensorReader : QObject
{
    Q_OBJECT
public:
    SensorReader(QObject *parent);
    void startWorking();

private slots:
    void _readBmcSensor();

private:
    void _errorOccured(QString errorMsg);
    QTimer readSensorTimer;
};

#endif // SENSORREADER_H
