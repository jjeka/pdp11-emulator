#ifndef MEMORY_H
#define MEMORY_H

#include "device.h"
#include "bus.h"

class Memory : public Device
{
    uint8_t* data_;

public:
    Memory(unsigned start, unsigned size, unsigned flags, Bus* bus, const std::string& name);
    uint8_t* getData();

    virtual void set8(Bus::AddressRegion* region, unsigned address, uint8_t data);
    virtual uint8_t get8(Bus::AddressRegion* region, unsigned address);
    virtual void set16(Bus::AddressRegion* region, unsigned address, uint16_t data);
    virtual uint16_t get16(Bus::AddressRegion* region, unsigned address);

};

#endif // MEMORY_H
