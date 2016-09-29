#include "utils.h"

int32_t sign(int32_t x)
{
    return (x < 0) ? -1 : ((x > 0) ? 1 : 0);
}
