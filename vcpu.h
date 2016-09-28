#ifndef VCPU_H
#define VCPU_H

#include <string>
#include <cassert>
#include <array>
#include <vector>
#include <cstring>

#define VCPU_SP_REGISTER            6
#define VCPU_PC_REGISTER            7
#define VCPU_NUM_REGISTERS          8
#define VCPU_MEM_SIZE               (1 << 16)
#define VCPU_BUFFER_SIZE            (VCPU_MEM_SIZE + VCPU_NUM_REGISTERS * sizeof (uint16_t))
#define VCPU_FLAG_REGISTER_OFFSET   (VCPU_MEM_SIZE - sizeof (uint8_t))

#define VCPU_DISPLAY_WIDTH          300
#define VCPU_DISPLAY_HEIGHT         300
#define VCPU_FB_OFFSET              8000

#define VCPU_NEGATIVE_FLAG_BIT      4
#define VCPU_ZERO_FLAG_BIT          5
#define VCPU_OVERFLOW_FLAG_BIT      6
#define VCPU_CARRY_FLAG_BIT         7

#define VCPU_NUM_INSTRUCTIONS       (1 << 16)
#define VCPU_MAX_INSTRUCTION_NAMES  1024
#define VCPU_MAX_INSTRUCTION_NAME   512

#define VCPU_GET_BIT(a,n) ((a >> n) & 1)
#define VCPU_SET_BIT(a,n,val) ((-val ^ n) && (1 << a))

class Vcpu
{
public:
    Vcpu(std::string romFile);
    ~Vcpu();

    std::string getRegisterName(unsigned n);
    uint16_t& getRegister(unsigned n);
    uint16_t& getPC();
    uint16_t& getSP();
    uint8_t& getPSW();
    unsigned getNRegisters();
    unsigned getDisplayWidth();
    unsigned getDisplayHeight();
    void* getFramebuffer();
    std::string instrAtAddress(uint16_t address);
    void start();
    void pause();
    void step();
    void addBreakpoint(uint16_t address);

    bool getNegativeFlag();
    void setNegativeFlag(bool flag);
    bool getZeroFlag();
    void setZeroFlag(bool flag);
    bool getOverflowFlag();
    void setOverflowFlag(bool flag);
    bool getCarryFlag();
    void setCarryFlag(bool flag);

private:

    std::array<std::string, VCPU_NUM_REGISTERS> registerNames_;
    uint8_t memory_[VCPU_BUFFER_SIZE];

    enum InstructionType
    {
        TYPE_NOT_INITIALIZED = 0,
        TYPE_NOT_IMPLEMENTED = 1,
        TYPE_DOUBLE_OPERAND = 2,
        TYPE_DOUBLE_OPERAND_REG = 3,
        TYPE_SINGLE_OPERAND = 4,
        TYPE_CONDITIONAL_BRANCH = 5,
        // TODO: jmp, halt, wait, reset etc.
    };

    struct Instruction
    {
        InstructionType type;
        void* callback;
        std::string* name;
    };

    std::array<Instruction, VCPU_NUM_INSTRUCTIONS> instructions_;

    std::array<std::string, VCPU_MAX_INSTRUCTION_NAMES> instructionNames_;
    int numInstructionNames_;

    void addInstruction_(uint16_t begin, uint16_t end, std::string name, void* callback, InstructionType type);
    std::string getOperand_(uint16_t instr, int begin);
    std::string getRegister_(uint16_t instr, int begin);

};

#endif // VCPU_H
