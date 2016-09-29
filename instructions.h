#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "utils.h"
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

typedef void vcpu_instr_double_operand_callback(uint16_t& dst, uint16_t& src, VcpuPSW& psw);

// VCPU_INSTR_TYPE_DOUBLE_OPERAND
void instr_mov(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_movb(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_cmp(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_cmpb(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_bit(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_bitb(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_bic(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_bicb(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_bis(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_bisb(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_add(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_addb(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_sub(uint16_t& dst, uint16_t& src, VcpuPSW& psw);
void instr_subb(uint16_t& dst, uint16_t& src, VcpuPSW& psw);

#endif // INSTRUCTIONS_H
