#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "utils.h"
#include "memregion.h"
#include <string>

enum InstructionType
{
    VCPU_INSTR_TYPE_NOT_INITIALIZED     = 0,
    VCPU_INSTR_TYPE_NOT_IMPLEMENTED     = 1,
    VCPU_INSTR_TYPE_DOUBLE_OPERAND      = 2,
    VCPU_INSTR_TYPE_OPERAND_REGISTER    = 3,
    VCPU_INSTR_TYPE_SINGLE_OPERAND      = 4,
    VCPU_INSTR_TYPE_BRANCH              = 5
};

struct InstructionInfo
{
    uint16_t begin;
    uint16_t end;
    std::string name;
    void* callback;
    InstructionType type;
};

extern InstructionInfo VCPU_INSTRUCTIONS[];
extern int VCPU_NUM_INSTRUCTIONS_IN_TABLE;

struct VcpuPSW
{
    bool n;
    bool z;
    bool v;
    bool c;
};

typedef bool vcpu_instr_double_operand_callback(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
typedef bool vcpu_instr_operand_register_callback(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw);
typedef bool vcpu_instr_single_operand_callback(MemRegion& data, VcpuPSW& psw);
typedef bool vcpu_instr_branch_callback(uint16_t& pc, uint8_t offset, VcpuPSW& psw);

// VCPU_INSTR_TYPE_DOUBLE_OPERAND
bool instr_mov(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_movb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_cmp(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_cmpb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_bit(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_bitb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_bic(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_bicb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_bis(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_bisb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_add(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_addb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_sub(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
bool instr_subb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);

// VCPU_INSTR_TYPE_OPERAND_REGISTER
bool instr_mul(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw);
bool instr_div(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw);
bool instr_ash(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw);
bool instr_ashc(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw);
bool instr_xor(bool onereg, MemRegion& reg, MemRegion& reg2, MemRegion& src, VcpuPSW& psw);

// VCPU_INSTR_TYPE_SINGLE_OPERAND

// VCPU_INSTR_TYPE_BRANCH

#endif // INSTRUCTIONS_H
