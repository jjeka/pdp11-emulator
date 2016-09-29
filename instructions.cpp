#include "instructions.h"

int32_t sign(int32_t x) // TODO: utils
{
    return (x < 0) ? -1 : ((x > 0) ? 1 : 0);
}

//==================================
// VCPU_INSTR_TYPE_DOUBLE_OPERAND

void instr_mov(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    dst = src;
    psw.n = (int16_t(src) < 0);
    psw.z = (int16_t(src) == 0);
    psw.v = false;
}

void instr_movb(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    ((uint8_t&) dst) = uint8_t (src);
    psw.n = (int16_t(src) < 0);
    psw.z = (int16_t(src) == 0);
    psw.v = false;
}

void instr_cmp(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) - int16_t(dst);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(dst) != sign(src) && sign(dst) != 0 && sign(src) != 0 && sign(res) == sign(dst));
    psw.c = (res > INT16_MAX);
}

void instr_cmpb(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    int16_t res = int8_t(src) - int8_t(dst);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(dst) != sign(src) && sign(dst) != 0 && sign(src) != 0 && sign(res) == sign(dst));
    psw.c = (res > INT8_MAX);
}

void instr_add(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) + int16_t(dst);
    dst = int16_t(res);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(src) == sign(dst) && sign(src) != 0 && sign(dst) != 0 && sign(int16_t(res)) == -sign(dst));
    psw.c = (res > INT16_MAX);
}

void instr_sub(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) - int16_t(dst);
    dst = int16_t(res);
    psw.n = (res < 0);
    psw.z = (res == 0);
    psw.v = (sign(dst) != sign(src) && sign(dst) != 0 && sign(src) != 0 && sign(res) == sign(src));
    psw.c = (res > INT16_MAX);
}

void instr_bit(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    dst = (src & dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bitb(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    ((uint8_t&) dst) = (uint8_t(src) & uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bic(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    dst = ((~src) & dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bicb(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    ((uint8_t&) dst) = (uint8_t(~src) & uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bis(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    dst = (src | dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}

void instr_bisb(uint16_t& dst, uint16_t& src, VcpuPSW& psw)
{
    ((uint8_t&) dst) = (uint8_t(src) | uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (dst == 0);
    psw.v = false;
}
