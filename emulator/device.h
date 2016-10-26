#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include "bus.h"

class Device
{
public:
    virtual void set8(Bus::AddressRegion* region, unsigned address, uint8_t data);
    virtual uint8_t get8(Bus::AddressRegion* region, unsigned address);
    virtual void set16(Bus::AddressRegion* region, unsigned address, uint16_t data);
    virtual uint16_t get16(Bus::AddressRegion* region, unsigned address);
    virtual void pollInterrupts();


};

#endif // DEVICE_H
