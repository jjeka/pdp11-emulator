#ifndef VCPU_H
#define VCPU_H

#include <string>
#include <cassert>
#include <array>
#include <vector>
#include <cstring>
#include <set>
#include <functional>
#include <thread>

#include "instructions.h"
#include "keycodes.h"
#include "utils.h"
#include "bus.h"
#include "memory.h"
#include "keyboard.h"

#define VCPU_SP_REGISTER            6
#define VCPU_PC_REGISTER            7
#define VCPU_NUM_REGISTERS          8
#define VCPU_MEM_SIZE               (1 << 16)
#define VCPU_BUFFER_SIZE            (VCPU_MEM_SIZE + VCPU_NUM_REGISTERS * sizeof (uint16_t))
#define VCPU_FLAG_REGISTER_OFFSET   (VCPU_MEM_SIZE - sizeof (uint16_t))

#define VCPU_DISPLAY_WIDTH          100
#define VCPU_DISPLAY_HEIGHT         100

#define VCPU_ROM_OFFSET             0
#define VCPU_ROM_SIZE               (1 << 15)
#define VCPU_FB_OFFSET              (VCPU_ROM_OFFSET + VCPU_ROM_SIZE)
#define VCPU_FB_SIZE                (VCPU_DISPLAY_WIDTH * VCPU_DISPLAY_HEIGHT)
#define VCPU_RAM_OFFSET             (VCPU_FB_OFFSET + VCPU_FB_SIZE)
#define VCPU_RAM_SIZE               (0160000 + 01000 - VCPU_RAM_OFFSET)
// ||--- rom ---|--- fb ---|--- ram (including interrupt vectors) ---|--- devices ---| psw ||
#define VCPU_IV_OFFSET              0160000
#define VCPU_IV_KEYBOARD_OFFSET     VCPU_IV_OFFSET

#define VCPU_MEM_IO_OFFSET          0161000
#define VCPU_KEYBOARD_STATUS_OFFSET VCPU_MEM_IO_OFFSET

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

#define VCPU_GET_REG(instr,begin) ((instr >> begin) & 7)
#define VCPU_GET_ADDR_MODE(instr,begin) ((instr >> (begin + 3)) & 7)

#define VCPU_KEYBOARD_INTERRUPT_PRIORITY        4

enum VcpuStatus
{
    VCPU_STATUS_OK,
    VCPU_STATUS_FAIL_OPEN_ROM,
    VCPU_STATUS_WRONG_ROM_SIZE,
    VCPU_STATUS_WRITE_FROM_READONLY,
    VCPU_STATUS_NOT_IMPLEMENTED_INSTRUCTION,
    VCPU_STATUS_INVALID_INSTRUCTION,
    VCPU_STATUS_INVALID_OPCODE,
    VCPU_STATUS_INVALID_MEMORY_ACCESS,
    VCPU_STATUS_INVALID_PC
};

class Vcpu
{
public:
    Vcpu(std::string romFile, std::function<void()> executionStoppedCallback);
    ~Vcpu();

    VcpuStatus getStatus();

    uint16_t getWordAtAddress(uint16_t addr);
    std::string getRegisterName(unsigned n);
    uint16_t& getRegister(unsigned n);
    uint16_t& getPC();
    uint16_t& getSP();
    uint16_t& getPSW();
    unsigned getNRegisters();
    unsigned getDisplayWidth();
    unsigned getDisplayHeight();
    void* getFramebuffer();
	unsigned getMemSize();
    Bus& getBus();
    Keyboard& getKeyboard();
    std::string instrAtAddress(uint16_t address);
    void start();
    void reset();
    void reset(std::string romFile);
    void pause();
    void step();

    void addBreakpoint(uint16_t address);
    void removeBreakpoint(uint16_t address);
    bool breakpointExists(uint16_t address);
    bool breakpointHit();
    bool haltHit();

    bool getNegativeFlag();
    void setNegativeFlag(bool flag);
    bool getZeroFlag();
    void setZeroFlag(bool flag);
    bool getOverflowFlag();
    void setOverflowFlag(bool flag);
    bool getCarryFlag();
    void setCarryFlag(bool flag);

    bool interrupt(unsigned priority, uint16_t vectorAddress);

private:
    Bus bus_;
    VcpuStatus status_;
    std::array<std::string, VCPU_NUM_REGISTERS> registerNames_;
    uint16_t registers_[VCPU_NUM_REGISTERS];
    uint16_t psw_;
    Memory rom_;
    Memory ram_;
    Memory fb_;
    Keyboard keyboard_;
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
    std::thread thread_;

    enum VcpuThreadState
    {
        VCPU_THREAD_STATE_IDLE,
        VCPU_THREAD_STATE_RUNNING,
        VCPU_THREAD_STATE_SINGLE_INSTRUCTION,
        VCPU_THREAD_STATE_DESTROY
    };

    VcpuThreadState threadState_;
    bool threadRunning_;
    bool breakpointHit_;
    bool haltHit_;

    void threadFunc_();
    void addInstruction_(uint16_t begin, uint16_t end, std::string name, void* callback, InstructionType type);
    std::string getOperand_(uint16_t pc, uint16_t instr, int begin, uint16_t data, bool* dataNeeded, bool secondParam);
    std::string getRegisterByInstr_(uint16_t instr, int begin);
    void executeInstruction_();
    bool isByteInstruction_(uint16_t instr);
    unsigned getAddrByAddrMode_(int r, int mode, uint16_t incrementSize);
    void toOctal_(uint16_t n, char* str);

    void onHalt_();
    bool onJmp_(int r, int mode);

    friend class MemRegion;
    friend bool instr_halt(uint16_t instr, Vcpu& cpu);
    friend bool instr_jmp(uint16_t instr, Vcpu& cpu);
    friend bool instr_jsr(uint16_t instr, MemRegion16& reg, MemRegion16& dst, Vcpu& cpu);
};

#endif // VCPU_H
