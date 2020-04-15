#ifndef LOGMANAGEMENT_H
#define LOGMANAGEMENT_H

#include <QArrayData>

class LogManagement
{
public:
    LogManagement();
    void sensorDataReceived(QArrayData data);
};

#endif // LOGMANAGEMENT_H
