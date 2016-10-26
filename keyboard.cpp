#include "keyboard.h"
#include "vcpu.h"

Keyboard::Keyboard(Vcpu *vcpu) :
    vcpu_(vcpu),
    data_(0),
    handled_(true)
{
    vcpu->getBus().addAddressRegion(this, VCPU_KEYBOARD_STATUS_OFFSET, sizeof (uint16_t),
                                    BUS_ADDRESSREGION_READ | BUS_ADDRESSREGION_REGISTER, "KB_DATA");
}

void Keyboard::keyPressed(uint8_t key, bool control, bool shift)
{
    data_ = key | (control ? VCPU_KEY_MODIFIER_CONTROL : 0) | (shift ? VCPU_KEY_MODIFIER_SHIFT : 0);
    handled_ = false;
}

void Keyboard::set8(Bus::AddressRegion*, unsigned, uint8_t)
{
    abort();
}

uint8_t Keyboard::get8(Bus::AddressRegion*, unsigned)
{
    abort();
}

void Keyboard::set16(Bus::AddressRegion*, unsigned, uint16_t)
{
    abort();
}

uint16_t Keyboard::get16(Bus::AddressRegion*, unsigned)
{
    return data_;
}

void Keyboard::pollInterrupts()
{
    if (!handled_ && vcpu_->interrupt(VCPU_KEYBOARD_INTERRUPT_PRIORITY, VCPU_IV_KEYBOARD_OFFSET))
        handled_ = true;
}
