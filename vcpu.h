#ifndef VCPU_H
#define VCPU_H

#include <string>
#include <cassert>
#include <array>

#define VCPU_SP_REGISTER 6
#define VCPU_PC_REGISTER 7
#define VCPU_NUM_REGISTERS 8
#define VCPU_MEM_SIZE (1 << 16)
#define VCPU_BUFFER_SIZE (VCPU_MEM_SIZE + VCPU_NUM_REGISTERS * sizeof (uint16_t))
#define VCPU_FLAG_REGISTER_OFFSET (VCPU_MEM_SIZE - sizeof (uint16_t))

#define VCPU_DISPLAY_WIDTH 300
#define VCPU_DISPLAY_HEIGHT 300
#define VCPU_FB_OFFSET 8000

class Vcpu
{
public:
    Vcpu(std::string romFile);
    ~Vcpu();

    std::string getRegisterName(unsigned n);
    uint16_t& getRegister(unsigned n);
    uint16_t& getPC();
    uint16_t& getSP();
    unsigned getNRegisters();
    unsigned getDisplayWidth();
    unsigned getDisplayHeight();
    void* getFramebuffer();
    std::string instrAtAddress(uint16_t address);
    void start();
    void pause();
    void step();
    void addBreakpoint(uint16_t address);


private:

    std::array<std::string, VCPU_NUM_REGISTERS> registerNames_;
	char memory_[VCPU_BUFFER_SIZE];

};

#endif // VCPU_H
