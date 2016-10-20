#ifndef MEMORY_H
#define MEMORY_H

#include "device.h"
#include "bus.h"

class Memory : public Device
{
    uint8_t* data_;

public:
    Memory(uint16_t start, uint16_t size, unsigned flags, Bus* bus);
    uint8_t* getData();

    virtual void set8(Bus::AddressRegion* region, uint16_t address, uint8_t data);
    virtual uint8_t get8(Bus::AddressRegion* region, uint16_t address);
    virtual void set16(Bus::AddressRegion* region, uint16_t address, uint16_t data);
    virtual uint16_t get16(Bus::AddressRegion* region, uint16_t address);

};

#endif // MEMORY_H
