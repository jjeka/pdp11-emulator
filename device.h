#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include "bus.h"

class Device
{
public:
    virtual void set8(Bus::AddressRegion* region, unsigned address, uint8_t data) = 0;
    virtual uint8_t get8(Bus::AddressRegion* region, unsigned address) = 0;
    virtual void set16(Bus::AddressRegion* region, unsigned address, uint16_t data) = 0;
    virtual uint16_t get16(Bus::AddressRegion* region, unsigned address) = 0;
};

#endif // DEVICE_H
