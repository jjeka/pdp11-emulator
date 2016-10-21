#include "utils.h"
#include <QThread>

int32_t sign(int32_t x)
{
    return (x < 0) ? -1 : 1;
}

uint16_t leconvert(uint16_t x)
{
#ifdef __ORDER_LITTLE_ENDIAN__
    return x;
#elif __ORDER_BIG_ENDIAN__
    return (x << 8) | (x >> 8);
#else
    #error "Unable to detect hos endianness"
#endif
}

//TODO: c++11 sleep
void sleepMS(int ms)
{
    return QThread::msleep(ms);
}
