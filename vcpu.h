#ifndef VCPU_H
#define VCPU_H

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

};

#endif // VCPU_H
