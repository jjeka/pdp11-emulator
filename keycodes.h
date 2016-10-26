#ifndef KEYCODES_H
#define KEYCODES_H

#define VCPU_KEYCODE(code) \
    VCPU_KEYCODE_ ## code,
#define VCPU_KEYCODE_(code, val) \
    VCPU_KEYCODE_ ## code = val,

enum VcpuKeyCode
{
    #include "keycodes_.h"
};

#undef VCPU_KEYCODE
#undef VCPU_KEYCODE_

#endif // KEYCODES_H
