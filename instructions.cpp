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
    psw.n = (int16_t(res) < 0);
    psw.z = (int16_t(res) == 0);
    psw.v = (sign(int16_t(dst)) != sign(int16_t(src)) &&
            sign(int16_t(dst)) != 0 && sign(int16_t(src)) != 0 &&
            int16_t(sign(res)) == int16_t(sign(dst)));
    psw.c = (res > INT16_MAX);

    return true;
}

bool instr_cmpb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int16_t res = int8_t(src) - int8_t(dst);
    psw.n = (int16_t(res) < 0);
    psw.z = (int16_t(res) == 0);
    psw.v = (sign(int16_t(dst)) != sign(int16_t(src)) &&
            sign(int16_t(dst)) != 0 && sign(int16_t(src)) != 0 &&
            sign(int16_t(res)) == sign(int16_t(dst)));
    psw.c = (res > INT8_MAX);

    return true;
}

bool instr_add(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) + int16_t(dst);
    dst = int16_t(res);
    psw.n = (int16_t(res) < 0);
    psw.z = (int16_t(res) == 0);
    psw.v = (sign(int16_t(src)) == sign(int16_t(dst)) &&
             sign(int16_t(src)) != 0 && sign(int16_t(dst)) != 0 &&
            sign(int16_t(res)) == -sign(int16_t(dst)));
    psw.c = (res > INT16_MAX);

    return true;
}

bool instr_sub(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    int32_t res = int16_t(src) - int16_t(dst);
    dst = int16_t(res);
    psw.n = (int16_t(res) < 0);
    psw.z = (int16_t(res) == 0);
    psw.v = (sign(int16_t(dst)) != sign(int16_t(src)) &&
            sign(int16_t(dst)) != 0 && sign(int16_t(src)) != 0 &&
            sign(int16_t(res)) == sign(int16_t(src)));
    psw.c = (res > INT16_MAX);

    return true;
}

bool instr_bit(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = (src & dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (int16_t(dst) == 0);
    psw.v = false;

    return true;
}

bool instr_bitb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t(src) & uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (int16_t(dst) == 0);
    psw.v = false;

    return true;
}

bool instr_bic(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = ((~src) & dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (int16_t(dst) == 0);
    psw.v = false;

    return true;
}

bool instr_bicb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t(~src) & uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (int16_t(dst) == 0);
    psw.v = false;

    return true;
}

bool instr_bis(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst = (src | dst);
    psw.n = ((dst & (1<<15)) != 0);
    psw.z = (int16_t(dst) == 0);
    psw.v = false;

    return true;
}

bool instr_bisb(MemRegion& dst, MemRegion& src, VcpuPSW& psw)
{
    dst.set8(uint8_t(src) | uint8_t(dst));
    psw.n = ((dst & (1<<7)) != 0);
    psw.z = (int16_t(dst) == 0);
    psw.v = false;

    return true;
}

// VCPU_INSTR_TYPE_OPERAND_REGISTER

bool instr_mul(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw)
{
    int32_t result = int16_t(reg) * int16_t(src);
    if (onereg)
        reg = uint16_t (result);
    else
    {
        reg = uint16_t (uint16_t(result) >> 16);
        reg2 = uint16_t (result);
    }

    psw.n = (result < 0);
    psw.z = (result == 0);
    psw.v = false;
    psw.c = (result < -(1<<15) || result >= (1<<15));

    return true;
}

bool instr_div(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw)
{
    if (onereg)
        return false;

    if (src == 0 || abs(int16_t(reg)) > abs(int16_t(src))) // (INT32_MIN / -1) is covered
    {
        psw.n = false;
        psw.z = false;
        psw.v = true;
        psw.c = (src == 0);
        return true;
    }

    int32_t numerator = int32_t(reg2 + (reg << 16));

    int32_t quotient = numerator / src;
    int32_t remainder = numerator % src;
    if (remainder != 0)
    {
        if (quotient > 0 && remainder < 0)
            remainder += src;
        else if (quotient < 0 && remainder > 0)
            remainder -= src;
    }

    reg = int16_t(quotient);
    reg2 = int16_t(remainder);

    psw.n = (quotient < 0);
    psw.z = (quotient == 0);
    psw.v = false;
    psw.c = false;

    return true;
}

bool instr_ash(bool /*onereg*/, MemRegion& reg, MemRegion& /*reg2*/, MemRegion& src, VcpuPSW& psw)
{
    int shift = (src & 0x3F);
    if (shift >= 31)
        shift = shift - 64;

    uint16_t result = reg;
    if (shift > 0)
        result = (reg << shift);
    else if (shift < 0)
        result = (reg >> (-shift));

    psw.n = (int16_t(result) < 0);
    psw.z = (int16_t(result) == 0);
    psw.v = (sign(int16_t(result)) != 0 && sign(int16_t(reg)) != 0 && sign(int16_t(result)) != sign(int16_t(reg)));

    if (shift > 0)
    {
        if (shift == 1)
            psw.c = (GET_BIT(reg, 15) != 0);
        else
            psw.c = (GET_BIT(reg >> (shift - 1), 15) != 0);
    }
    else if (shift < 0)
    {
        if (shift == -1)
            psw.c = (GET_BIT(reg, 0) != 0);
        else
            psw.c = (GET_BIT(reg >> (-shift - 1), 0) != 0);
    }
    else
    {
        psw.c = false;
    }

    reg = result;

    return true;
}

bool instr_ashc(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw)
{
    int shift = (src & 0x3F);
    if (shift >= 31)
        shift = shift - 64;

    uint32_t regresult = onereg ? ((reg << 16) + reg) : ((reg << 16) + reg2);
    uint32_t result = regresult;
    if (shift > 0)
        result = (regresult << shift);
    else if (shift < 0)
        result = (regresult >> (-shift));

    psw.n = (int32_t(result) < 0);
    psw.z = (int32_t(result) == 0);
    psw.v = (sign(int32_t(result)) != 0 && sign(int16_t(reg)) != 0 && sign(int32_t(result)) != sign(int16_t(reg)));

    if (shift > 0)
    {
        if (shift == 1)
            psw.c = (GET_BIT(reg, 31) != 0);
        else
            psw.c = (GET_BIT(reg >> (shift - 1), 31) != 0);
    }
    else if (shift < 0)
    {
        if (shift == -1)
            psw.c = (GET_BIT(reg, 0) != 0);
        else
            psw.c = (GET_BIT(reg >> (-shift - 1), 0) != 0);
    }
    else
    {
        psw.c = false;
    }

    if (onereg)
        reg = uint16_t(result);
    else
    {
        reg = (uint16_t(result) >> 16);
        reg2 = uint16_t(result);
    }

    return true;
}

bool instr_xor(bool /*onereg*/, MemRegion& reg, MemRegion& /*reg2*/, MemRegion& src, VcpuPSW& psw)
{
    src = (src ^ reg);

    psw.n = (int16_t(src) < 0);
    psw.z = (int16_t(src) == 0);
    psw.v = false;

    return true;
}

// VCPU_INSTR_TYPE_SINGLE_OPERAND

// VCPU_INSTR_TYPE_BRANCH
