#include "device.h"

void Device::set8(Bus::AddressRegion*, unsigned, uint8_t)
{
    abort();
}

uint8_t Device::get8(Bus::AddressRegion*, unsigned)
{
    abort();
}

void Device::set16(Bus::AddressRegion*, unsigned, uint16_t)
{
    abort();
}

uint16_t Device::get16(Bus::AddressRegion*, unsigned)
{
    abort();
}

void Device::pollInterrupts()
{
    abort();
}
