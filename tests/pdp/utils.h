#ifndef PDP_UTILS
#define PDP_UTILS

#define DEBUG_PRINT_VAR(reg, n)	\
	{ unsigned __debug = n; asm volatile("mov %0, " reg "\nmov $012345, r0\nhalt\nmov $0, r0" :: "r"(__debug)); }
#define DEBUG_PRINT_VAR_MULTIPLE(reg, n)	\
	{ unsigned __debug = n; asm volatile("mov %0, " reg "\nmov $012345, r0\n" :: "r"(__debug)); }
#define DEBUG_HALT()	\
	asm volatile("halt");

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef uint16_t size_t;

#define NULL 0

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) < (b)) ? (b) : (a))
#define abs(a) (((a) < 0) ? (-(a)) : (a))

void sleep(int t);
void* memset(void* s, int c, size_t n);

#endif // PDP_UTILS
