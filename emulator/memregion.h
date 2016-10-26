#ifndef MEMREGION_H
#define MEMREGION_H

#include <cinttypes>

class Vcpu;

class MemRegion16
{
    unsigned addr_;
    Vcpu* vcpu_;

public:
    MemRegion16(unsigned addr, Vcpu* vcpu);
    void operator = (uint16_t val);
    void operator = (const MemRegion16& val);
    operator uint16_t() const;


};

class MemRegion8
{
    Vcpu* vcpu_;
    unsigned addr_;

public:
    MemRegion8(unsigned addr, Vcpu* vcpu);
    void operator = (uint8_t val);
    void operator = (const MemRegion8& val);
    operator uint8_t() const;

};

#endif // MEMREGION_H
