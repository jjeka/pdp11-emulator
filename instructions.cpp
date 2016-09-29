#include "instructions.h"

// VCPU_INSTR_TYPE_DOUBLE_OPERAND

void instr_mov(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = src;
    psw.n = (int16_t(src) < 0);
    psw.z = (int16_t(src) == 0);
    psw.v = false;
}

void instr_movb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t (src));
    psw.n = (int16_t(src) < 0);
    psw.z = (int16_t(src) == 0);
    psw.v = false;
}

void instr_cmp(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) - int16_t(dst);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(dst) != sign(src) && sign(dst) != 0 && sign(src) != 0 && sign(res) == sign(dst));
    psw.c = (res > INT16_MAX);
}

void instr_cmpb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int16_t res = int8_t(src) - int8_t(dst);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(dst) != sign(src) && sign(dst) != 0 && sign(src) != 0 && sign(res) == sign(dst));
    psw.c = (res > INT8_MAX);
}

void instr_add(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) + int16_t(dst);
    dst = int16_t(res);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(src) == sign(dst) && sign(src) != 0 && sign(dst) != 0 && sign(int16_t(res)) == -sign(dst));
    psw.c = (res > INT16_MAX);
}

void instr_sub(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) - int16_t(dst);
    dst = int16_t(res);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(dst) != sign(src) && sign(dst) != 0 && sign(src) != 0 && sign(res) == sign(src));
    psw.c = (res > INT16_MAX);
}

void instr_bit(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = (src & dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bitb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t(src) & uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bic(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = ((~src) & dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bicb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t(~src) & uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bis(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = (src | dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bisb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t(src) | uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

// VCPU_INSTR_TYPE_OPERAND_REGISTER

// VCPU_INSTR_TYPE_SINGLE_OPERAND

// VCPU_INSTR_TYPE_BRANCH
