#include "utils.h"
#include <QThread>

int32_t sign(int32_t x)
{
    return (x < 0) ? -1 : 1;
}

void sleepMS(int ms)
{
    return QThread::msleep(ms);
}
