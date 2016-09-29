#ifndef UTILS_H
#define UTILS_H

#include <cinttypes>

int32_t sign(int32_t x);

#define GET_BIT(a,n) ((a >> n) & 1)
#define SET_BIT(a,n,val) a ^= ((-val ^ a) & (1 << n))

#endif // UTILS_H