#include "instructions.h"
#include "vcpu.h"

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

bool instr_clr(MemRegion& data, VcpuPSW& psw)
{
    data = 0;

    psw.n = false;
    psw.z = true;
    psw.v = false;
    psw.z = false;

    return true;
}

bool instr_clrb(MemRegion& data, VcpuPSW& psw)
{
    data.set8(0);

    psw.n = false;
    psw.z = true;
    psw.v = false;
    psw.z = false;

    return true;
}

bool instr_com(MemRegion& data, VcpuPSW& psw)
{
    data = ~data;

    psw.n = (GET_BIT(data, 15) != 0);
    psw.z = (int16_t(data) == 0);
    psw.v = false;
    psw.z = true;

    return true;
}

bool instr_comb(MemRegion& data, VcpuPSW& psw)
{
    data.set8(uint8_t(~data));

    psw.n = (GET_BIT(data, 7) != 0);
    psw.z = (int16_t(data) == 0);
    psw.v = false;
    psw.z = true;

    return true;
}

bool instr_inc(MemRegion& data, VcpuPSW& psw)
{
    data = data + 1;

    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = (data == INT16_MIN);

    return true;
}

bool instr_incb(MemRegion& data, VcpuPSW& psw)
{
    data.set8(int8_t(data) + 1);

    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = (int8_t(data) == INT8_MIN);

    return true;
}

bool instr_dec(MemRegion& data, VcpuPSW& psw)
{
    data = data - 1;

    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = (data == INT16_MAX);

    return true;
}

bool instr_decb(MemRegion& data, VcpuPSW& psw)
{
    data.set8(int8_t(data) - 1);

    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = (int8_t(data) == INT8_MAX);

    return true;
}

bool instr_neg(MemRegion& data, VcpuPSW& psw)
{
    if (int16_t(data) != INT16_MIN)
        data = -int16_t(data);

    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = (int16_t(data) == INT16_MIN);
    psw.c = (int16_t(data) != 0);

    return true;
}

bool instr_negb(MemRegion& data, VcpuPSW& psw)
{
    if (int8_t(data) != INT8_MIN)
        data.set8(-int8_t(data));

    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = (int8_t(data) == INT8_MIN);
    psw.c = (int8_t(data) != 0);

    return true;
}

bool instr_tst(MemRegion& data, VcpuPSW& psw)
{
    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = false;
    psw.c = false;

    return true;
}

bool instr_tstb(MemRegion& data, VcpuPSW& psw)
{
    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = false;
    psw.c = false;

    return true;
}

bool instr_asr(MemRegion& data, VcpuPSW& psw)
{
    bool bit = (GET_BIT(data, 0) != 0);

    data = (((data & 0x7FFF) >> 1) | (data & 0x8000));

    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = psw.n ^ bit;
    psw.c = bit;

    return true;
}

bool instr_asrb(MemRegion& data, VcpuPSW& psw)
{
    bool bit = (GET_BIT(data, 0) != 0);

    data = (((data & 0x7F) >> 1) | (data & 0x80));

    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = psw.n ^ bit;
    psw.c = bit;

    return true;
}

bool instr_asl(MemRegion& data, VcpuPSW& psw)
{
    bool bit = (GET_BIT(data, 15) != 0);

    data = (data << 1);

    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = psw.n ^ bit;
    psw.c = bit;

    return true;
}

bool instr_aslb(MemRegion& data, VcpuPSW& psw)
{
    bool bit = (GET_BIT(data, 7) != 0);

    data = (data << 1);

    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = psw.n ^ bit;
    psw.c = bit;

    return true;
}

bool instr_ror(MemRegion& data, VcpuPSW& psw)
{
    bool bit = (GET_BIT(data, 0) != 0);

    data = ((data >> 1) + (GET_BIT(data, 0) << 15));

    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = psw.n ^ bit;
    psw.c = bit;

    return true;
}

bool instr_rorb(MemRegion& data, VcpuPSW& psw)
{
    bool bit = (GET_BIT(data, 0) != 0);

    data.set8(((int8_t(data) >> 1) + (GET_BIT(data, 0) << 7)));

    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = psw.n ^ bit;
    psw.c = bit;

    return true;
}

bool instr_rol(MemRegion& data, VcpuPSW& psw)
{
    bool bit = (GET_BIT(data, 15) != 0);

    data = ((data << 1) + (GET_BIT(data, 15)));

    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = psw.n ^ bit;
    psw.c = bit;

    return true;
}

bool instr_rolb(MemRegion& data, VcpuPSW& psw)
{
    bool bit = (GET_BIT(data, 7) != 0);

    data.set8((data << 1) + (GET_BIT(data, 7)));

    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = psw.n ^ bit;
    psw.c = bit;

    return true;
}

bool instr_swab(MemRegion& data, VcpuPSW& psw)
{
    data = (((data & 256) << 8) | (data >> 8));

    psw.n = (GET_BIT(data, 7) != 0);
    psw.z = (uint8_t(data) == 0);
    psw.v = false;
    psw.c = false;

    return true;
}

bool instr_adc(MemRegion& data, VcpuPSW& psw)
{
    int16_t prev = int16_t(data);
    data = int16_t(data) + (psw.c ? 1 : 0);

    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = (int16_t(prev) == INT16_MAX && psw.c);
    psw.c = (int16_t(prev) == -1 && psw.c);

    return true;
}

bool instr_adcb(MemRegion& data, VcpuPSW& psw)
{
    int8_t prev = int8_t(data);
    data.set8(int8_t(data) + (psw.c ? 1 : 0));

    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = (int8_t(prev) == INT8_MAX && psw.c);
    psw.c = (int8_t(prev) == -1 && psw.c);

    return true;
}

bool instr_sbc(MemRegion& data, VcpuPSW& psw)
{
    int16_t prev = int16_t(data);
    data = int16_t(data) - (psw.c ? 1 : 0);

    psw.n = (int16_t(data) < 0);
    psw.z = (int16_t(data) == 0);
    psw.v = (int16_t(prev) == INT16_MIN && psw.c);
    psw.c = (int16_t(prev) == 0 && psw.c);

    return true;
}

bool instr_sbcb(MemRegion& data, VcpuPSW& psw)
{
    int8_t prev = int8_t(data);
    data.set8(int8_t(data) - (psw.c ? 1 : 0));

    psw.n = (int8_t(data) < 0);
    psw.z = (int8_t(data) == 0);
    psw.v = (int8_t(prev) == INT8_MIN && psw.c);
    psw.c = (int8_t(prev) == 0 && psw.c);

    return true;
}

bool instr_sxt(MemRegion& data, VcpuPSW& psw)
{
    data = (psw.n ? -1 : 0);

    psw.z = !psw.n;

    return true;
}

// VCPU_INSTR_TYPE_BRANCH

bool instr_br(uint16_t& pc, int8_t offset, VcpuPSW& /*psw*/)
{
    pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bne(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (!psw.z)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_beq(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (psw.z)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bpl(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (!psw.n)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bmi(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (psw.n)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bvc(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (!psw.v)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bvs(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (psw.v)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bcc(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (!psw.c)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bcs(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (psw.c)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bge(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if ((psw.n ^ psw.v) == false)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_blt(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if ((psw.n ^ psw.v) == true)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bgt(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if ((psw.n ^ psw.v) == false && psw.z == false)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_ble(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if ((psw.n ^ psw.v) == true || psw.z == true)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_bhi(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (!psw.c && !psw.z)
        pc = pc + int16_t(offset) * 2;
    return true;
}

bool instr_blos(uint16_t& pc, int8_t offset, VcpuPSW& psw)
{
    if (psw.c || psw.z)
        pc = pc + int16_t(offset) * 2;
    return true;
}

// Other

bool instr_nop(uint16_t /*instr*/, Vcpu& /*cpu*/)
{
    return true;
}

bool instr_condition_code_operation(uint16_t instr, Vcpu& cpu)
{
    if (instr & 10)
    {
        if (instr & 1)
            cpu.setCarryFlag(true);
        if (instr & 2)
            cpu.setOverflowFlag(true);
        if (instr & 4)
            cpu.setZeroFlag(true);
        if (instr & 8)
            cpu.setNegativeFlag(true);
    }
    else
    {
        if (instr & 1)
            cpu.setCarryFlag(false);
        if (instr & 2)
            cpu.setOverflowFlag(false);
        if (instr & 4)
            cpu.setZeroFlag(false);
        if (instr & 8)
            cpu.setNegativeFlag(false);
    }

    return true;
}

bool instr_halt(uint16_t /*instr*/, Vcpu& cpu)
{
    cpu.onHalt_();
    return true;
}
