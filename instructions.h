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
    VCPU_INSTR_TYPE_SINGLE_OPERAND      = 3,
    VCPU_INSTR_TYPE_OPERAND_REGISTER    = 4,
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

typedef void vcpu_instr_double_operand_callback(MemRegion& dst, MemRegion& src, VcpuPSW& psw);

// VCPU_INSTR_TYPE_DOUBLE_OPERAND
void instr_mov(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_movb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_cmp(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_cmpb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_bit(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_bitb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_bic(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_bicb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_bis(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_bisb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_add(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_addb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_sub(MemRegion& dst, MemRegion& src, VcpuPSW& psw);
void instr_subb(MemRegion& dst, MemRegion& src, VcpuPSW& psw);

#endif // INSTRUCTIONS_H
