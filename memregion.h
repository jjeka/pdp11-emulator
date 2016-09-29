#ifndef MEMREGION_H
#define MEMREGION_H

#include <cinttypes>

class Vcpu;

class MemRegion
{
public:

    MemRegion(uint16_t* mem, Vcpu* cpu);
    void operator = (uint16_t val);
    operator uint16_t();

    void set8(uint8_t val);
    void set16(uint16_t val);

private:

    uint16_t* mem_;
    Vcpu* cpu_;
    bool readonly_;

};


#endif // MEMREGION_H
