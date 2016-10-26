#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "device.h"

class Vcpu;

#define VCPU_KEY_MODIFIER_SHIFT (1 << 16)
#define VCPU_KEY_MODIFIER_CONTROL (1 << 17)

class Keyboard : Device
{
    Vcpu* vcpu_;
    uint16_t data_;
    bool handled_;

public:
    Keyboard(Vcpu* vcpu_);

    virtual void set8(Bus::AddressRegion* region, unsigned address, uint8_t data);
    virtual uint8_t get8(Bus::AddressRegion* region, unsigned address);
    virtual void set16(Bus::AddressRegion* region, unsigned address, uint16_t data);
    virtual uint16_t get16(Bus::AddressRegion* region, unsigned address);
    virtual void pollInterrupts();

    void keyPressed(uint8_t key, bool control, bool shift);

};

#endif // KEYBOARD_H
