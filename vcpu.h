#ifndef VCPU_H
#define VCPU_H

#include <string>
#include <cassert>
#include <array>
#include <vector>
#include <cstring>
#include <set>
#include <functional>

#include "instructions.h"

#define VCPU_SP_REGISTER            6
#define VCPU_PC_REGISTER            7
#define VCPU_NUM_REGISTERS          8
#define VCPU_MEM_SIZE               (1 << 16)
#define VCPU_BUFFER_SIZE            (VCPU_MEM_SIZE + VCPU_NUM_REGISTERS * sizeof (uint16_t))
#define VCPU_FLAG_REGISTER_OFFSET   (VCPU_MEM_SIZE - sizeof (uint8_t))

#define VCPU_DISPLAY_WIDTH          100
#define VCPU_DISPLAY_HEIGHT         100

#define VCPU_ROM_OFFSET             0
#define VCPU_ROM_SIZE               (1 << 15)
#define VCPU_FB_OFFSET              (VCPU_ROM_OFFSET + VCPU_ROM_SIZE)
#define VCPU_FB_SIZE                (VCPU_DISPLAY_WIDTH * VCPU_DISPLAY_HEIGHT)
#define VCPU_RAM_OFFSET             (VCPU_FB_OFFSET + VCPU_FB_SIZE)
#define VCPU_RAM_SIZE               (VCPU_MEM_SIZE - VCPU_RAM_OFFSET - 1)
// ||--- rom ---|--- fb ---|--- ram ---| psw ||

#define VCPU_NEGATIVE_FLAG_BIT      4
#define VCPU_ZERO_FLAG_BIT          5
#define VCPU_OVERFLOW_FLAG_BIT      6
#define VCPU_CARRY_FLAG_BIT         7

#define VCPU_NUM_INSTRUCTIONS       (1 << 16)
#define VCPU_MAX_INSTRUCTION_NAMES  1024
#define VCPU_INSTRUCTION_NAME_SIZE  512

#define VCPU_ADDR_MODE_REGISTER                 0
#define VCPU_ADDR_MODE_REGISTER_DEFFERED        1
#define VCPU_ADDR_MODE_AUTOINCREMENT            2
#define VCPU_ADDR_MODE_AUTOINCREMENT_DEFFERED   3
#define VCPU_ADDR_MODE_AUTODECREMENT            4
#define VCPU_ADDR_MODE_AUTODECREMENT_DEFFERED   5
#define VCPU_ADDR_MODE_INDEX                    6
#define VCPU_ADDR_MODE_INDEX_DEFERRED           7

#define VCPU_GET_BIT(a,n) ((a >> n) & 1)
#define VCPU_SET_BIT(a,n,val) a ^= ((-val ^ a) & (1 << n))

#define VCPU_GET_REG(instr,begin) ((instr >> begin) & 7)
#define VCPU_GET_ADDR_MODE(instr,begin) ((instr >> (begin + 3)) & 7)

enum VcpuStatus
{
    VCPU_STATUS_OK,
    VCPU_STATUS_FAIL_OPEN_ROM,
    VCPU_STATUS_WRONG_ROM_SIZE
};

class Vcpu
{
public:
    Vcpu(std::string romFile, std::function<void()> executionStoppedCallback);
    ~Vcpu();

    VcpuStatus status();

    std::string getRegisterName(unsigned n);
    uint16_t& getRegister(unsigned n);
    uint16_t& getPC();
    uint16_t& getSP();
    uint8_t& getPSW();
    unsigned getNRegisters();
    unsigned getDisplayWidth();
    unsigned getDisplayHeight();
    void* getFramebuffer();
	unsigned getMemSize();
    std::string instrAtAddress(uint16_t address);
    void start();
    void reset();
    void pause();
    void step();

    void addBreakpoint(uint16_t address);
    void removeBreakpoint(uint16_t address);
    bool breakpointExists(uint16_t address);
    bool breakpointHit();

    bool getNegativeFlag();
    void setNegativeFlag(bool flag);
    bool getZeroFlag();
    void setZeroFlag(bool flag);
    bool getOverflowFlag();
    void setOverflowFlag(bool flag);
    bool getCarryFlag();
    void setCarryFlag(bool flag);

private:

    VcpuStatus status_;
    std::array<std::string, VCPU_NUM_REGISTERS> registerNames_;
    uint8_t memory_[VCPU_BUFFER_SIZE];
    std::set<uint16_t> breakpoints_;

    struct Instruction
    {
        InstructionType type;
        void* callback;
        std::string* name;
    };

    std::array<Instruction, VCPU_NUM_INSTRUCTIONS> instructions_;

    std::array<std::string, VCPU_MAX_INSTRUCTION_NAMES> instructionNames_;
    int numInstructionNames_;

    std::function<void()> executionStoppedCallback_;

    void addInstruction_(uint16_t begin, uint16_t end, std::string name, void* callback, InstructionType type);
    std::string getOperand_(uint16_t instr, int begin, uint16_t data);
    std::string getRegisterByInstr_(uint16_t instr, int begin);
    bool readonlyAddr_(uint16_t addr);
    void executeInstruction_();
    uint16_t& getMemoryWord_(uint16_t addr);
    uint16_t& getAddrByAddrMode_(int r, int mode, uint16_t incrementSize);

};

#endif // VCPU_H
