#include "instructions.h"

// VCPU_INSTR_TYPE_DOUBLE_OPERAND

bool instr_mov(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = src;
    psw.n = (int16_t(src) < 0);
    psw.z = (int16_t(src) == 0);
    psw.v = false;

    return true;
}

bool instr_movb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t (src));
    psw.n = (int16_t(src) < 0);
    psw.z = (int16_t(src) == 0);
    psw.v = false;

    return true;
}

bool instr_cmp(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) - int16_t(dst);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(dst) != sign(src) && sign(dst) != 0 && sign(src) != 0 && sign(res) == sign(dst));
    psw.c = (res > INT16_MAX);

    return true;
}

bool instr_cmpb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int16_t res = int8_t(src) - int8_t(dst);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(dst) != sign(src) && sign(dst) != 0 && sign(src) != 0 && sign(res) == sign(dst));
    psw.c = (res > INT8_MAX);

    return true;
}

bool instr_add(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) + int16_t(dst);
    dst = int16_t(res);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(src) == sign(dst) && sign(src) != 0 && sign(dst) != 0 && sign(int16_t(res)) == -sign(dst));
    psw.c = (res > INT16_MAX);

    return true;
}

bool instr_sub(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) - int16_t(dst);
    dst = int16_t(res);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(dst) != sign(src) && sign(dst) != 0 && sign(src) != 0 && sign(res) == sign(src));
    psw.c = (res > INT16_MAX);

    return true;
}

bool instr_bit(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = (src & dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (dst == 0);
    psw.v = false;

    return true;
}

bool instr_bitb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t(src) & uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (dst == 0);
    psw.v = false;

    return true;
}

bool instr_bic(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = ((~src) & dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (dst == 0);
    psw.v = false;

    return true;
}

bool instr_bicb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t(~src) & uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (dst == 0);
    psw.v = false;

    return true;
}

bool instr_bis(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = (src | dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (dst == 0);
    psw.v = false;

    return true;
}

bool instr_bisb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t(src) | uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (dst == 0);
    psw.v = false;

    return true;
}

// VCPU_INSTR_TYPE_OPERAND_REGISTER

bool instr_mul(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw)
{
    int32_t result = int16_t(reg) * int16_t(src);
    reg = uint16_t (result);
    if (!onereg)
        reg2 = uint16_t (uint16_t(result) >> 16);

    psw.n = (result < 0);
    psw.z = (result == 0);
    psw.v = false;
    psw.c = (result < -(1<<15) || result >= (1<<15));

    return true;
}

bool instr_div(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw)
{
    /*int32_t numerator
    int32_t result = int16_t(reg) * int16_t(src);
    reg = uint16_t (result);
    if (!onereg)
        reg2 = uint16_t (uint16_t(result) >> 16);

    psw.n = (result < 0);
    psw.z = (result == 0);
    psw.v = false;
    psw.c = (result < -(1<<15) || result >= (1<<15));*/

    return true;
}

// VCPU_INSTR_TYPE_SINGLE_OPERAND

// VCPU_INSTR_TYPE_BRANCH
