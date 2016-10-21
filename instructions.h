#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "utils.h"
#include "memregion.h"
#include <string>

enum InstructionType
{
    VCPU_INSTR_TYPE_NOT_INITIALIZED,
    VCPU_INSTR_TYPE_NOT_IMPLEMENTED,
    VCPU_INSTR_TYPE_INVALID_OPCODE,
    VCPU_INSTR_TYPE_DOUBLE_OPERAND,
    VCPU_INSTR_TYPE_OPERAND_REGISTER,
    VCPU_INSTR_TYPE_OPERAND_REGISTER_EX,
    VCPU_INSTR_TYPE_SINGLE_OPERAND,
    VCPU_INSTR_TYPE_SINGLE_OPERAND_EX,
    VCPU_INSTR_TYPE_SINGLE_REGISTER,
    VCPU_INSTR_TYPE_WITHOUT_PARAMETERS,
    VCPU_INSTR_TYPE_BRANCH,
    VCPU_INSTR_TYPE_NUMBER,
    VCPU_INSTR_TYPE_REGISTER_NUMBER,
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

typedef bool vcpu_instr_double_operand_16_callback(MemRegion16& dst, MemRegion16& src, VcpuPSW& psw);
typedef bool vcpu_instr_double_operand_8_callback(MemRegion8& dst, MemRegion8& src, VcpuPSW& psw);

typedef bool vcpu_instr_operand_register_callback(bool onereg, MemRegion16& reg, MemRegion16& reg2, MemRegion16& src, VcpuPSW& psw);
typedef bool vcpu_instr_operand_register_ex_callback(uint16_t instr, MemRegion16& reg, MemRegion16& dst, Vcpu& cpu);

typedef bool vcpu_instr_single_operand_16_callback(MemRegion16& data, VcpuPSW& psw);
typedef bool vcpu_instr_single_operand_8_callback(MemRegion8& data, VcpuPSW& psw);

typedef bool vcpu_instr_single_register_callback(MemRegion16& reg, Vcpu& cpu);
typedef bool vcpu_instr_without_parameters_and_ex_callback(uint16_t instr, Vcpu& cpu);
typedef bool vcpu_instr_branch_callback(uint16_t& pc, int8_t offset, VcpuPSW& psw);
typedef bool vcpu_instr_number_callback(uint8_t n, Vcpu& cpu);
typedef bool vcpu_instr_register_number_callback(MemRegion16& reg, uint8_t n, Vcpu& cpu);

// VCPU_INSTR_TYPE_DOUBLE_OPERAND
bool instr_mov(MemRegion16& dst, MemRegion16& src, VcpuPSW& psw);
bool instr_movb(MemRegion8& dst, MemRegion8& src, VcpuPSW& psw);
bool instr_cmp(MemRegion16& dst, MemRegion16& src, VcpuPSW& psw);
bool instr_cmpb(MemRegion8& dst, MemRegion8& src, VcpuPSW& psw);
bool instr_bit(MemRegion16& dst, MemRegion16& src, VcpuPSW& psw);
bool instr_bitb(MemRegion8& dst, MemRegion8& src, VcpuPSW& psw);
bool instr_bic(MemRegion16& dst, MemRegion16& src, VcpuPSW& psw);
bool instr_bicb(MemRegion8& dst, MemRegion8& src, VcpuPSW& psw);
bool instr_bis(MemRegion16& dst, MemRegion16& src, VcpuPSW& psw);
bool instr_bisb(MemRegion8& dst, MemRegion8& src, VcpuPSW& psw);
bool instr_add(MemRegion16& dst, MemRegion16& src, VcpuPSW& psw);
bool instr_addb(MemRegion8& dst, MemRegion8& src, VcpuPSW& psw);
bool instr_sub(MemRegion16& dst, MemRegion16& src, VcpuPSW& psw);
bool instr_subb(MemRegion8& dst, MemRegion8& src, VcpuPSW& psw);

// VCPU_INSTR_TYPE_OPERAND_REGISTER
bool instr_mul(bool onereg, MemRegion16& reg, MemRegion16& reg2, MemRegion16& src, VcpuPSW& psw);
bool instr_div(bool onereg, MemRegion16& reg, MemRegion16& reg2, MemRegion16& src, VcpuPSW& psw);
bool instr_ash(bool onereg, MemRegion16& reg, MemRegion16& reg2, MemRegion16& src, VcpuPSW& psw);
bool instr_ashc(bool onereg, MemRegion16& reg, MemRegion16& reg2, MemRegion16& src, VcpuPSW& psw);
bool instr_xor(bool onereg, MemRegion16& reg, MemRegion16& reg2, MemRegion16& src, VcpuPSW& psw);

// VCPU_INSTR_TYPE_SINGLE_OPERAND
bool instr_clr(MemRegion16& data, VcpuPSW& psw);
bool instr_clrb(MemRegion8& data, VcpuPSW& psw);
bool instr_com(MemRegion16& data, VcpuPSW& psw);
bool instr_comb(MemRegion8& data, VcpuPSW& psw);
bool instr_inc(MemRegion16& data, VcpuPSW& psw);
bool instr_incb(MemRegion8& data, VcpuPSW& psw);
bool instr_dec(MemRegion16& data, VcpuPSW& psw);
bool instr_decb(MemRegion8& data, VcpuPSW& psw);
bool instr_neg(MemRegion16& data, VcpuPSW& psw);
bool instr_negb(MemRegion8& data, VcpuPSW& psw);
bool instr_tst(MemRegion16& data, VcpuPSW& psw);
bool instr_tstb(MemRegion8& data, VcpuPSW& psw);
bool instr_asr(MemRegion16& data, VcpuPSW& psw);
bool instr_asrb(MemRegion8& data, VcpuPSW& psw);
bool instr_asl(MemRegion16& data, VcpuPSW& psw);
bool instr_aslb(MemRegion8& data, VcpuPSW& psw);
bool instr_ror(MemRegion16& data, VcpuPSW& psw);
bool instr_rorb(MemRegion8& data, VcpuPSW& psw);
bool instr_rol(MemRegion16& data, VcpuPSW& psw);
bool instr_rolb(MemRegion8& data, VcpuPSW& psw);
bool instr_swab(MemRegion16& data, VcpuPSW& psw);
bool instr_adc(MemRegion16& data, VcpuPSW& psw);
bool instr_adcb(MemRegion8& data, VcpuPSW& psw);
bool instr_sbc(MemRegion16& data, VcpuPSW& psw);
bool instr_sbcb(MemRegion8& data, VcpuPSW& psw);
bool instr_sxt(MemRegion16& data, VcpuPSW& psw);

// VCPU_INSTR_TYPE_BRANCH
bool instr_br(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bne(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_beq(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bpl(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bmi(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bvc(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bvs(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bcc(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bcs(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bge(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_blt(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bgt(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_ble(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_bhi(uint16_t& pc, int8_t offset, VcpuPSW& psw);
bool instr_blos(uint16_t& pc, int8_t offset, VcpuPSW& psw);

// Other
bool instr_nop(uint16_t instr, Vcpu& cpu);
bool instr_condition_code_operation(uint16_t instr, Vcpu& cpu);
bool instr_halt(uint16_t instr, Vcpu& cpu);
bool instr_jmp(uint16_t instr, Vcpu& cpu);

bool instr_jsr(uint16_t instr, MemRegion16& reg, MemRegion16& dst, Vcpu& cpu);
bool instr_rts(MemRegion16& reg, Vcpu& cpu);
bool instr_mark(uint8_t n, Vcpu& cpu);
bool instr_sob(MemRegion16& reg, uint8_t n, Vcpu& cpu);

#endif // INSTRUCTIONS_H
