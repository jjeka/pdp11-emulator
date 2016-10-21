#include "memory.h"
#include "utils.h"

Memory::Memory(unsigned start, unsigned size, unsigned flags, Bus* bus, const std::string& name) :
    data_(new uint8_t[size])
{
    bus->addAddressRegion(this, start, size, flags, name);
}

void Memory::set8(Bus::AddressRegion* region, unsigned address, uint8_t data)
{
    data_[address - region->start] = data;
}

uint8_t Memory::get8(Bus::AddressRegion* region, unsigned address)
{
    return data_[address - region->start];
}

void Memory::set16(Bus::AddressRegion* region, unsigned address, uint16_t data)
{
    *((uint16_t*) (&data_[address - region->start])) = leconvert(data);
}

uint16_t Memory::get16(Bus::AddressRegion* region, unsigned address)
{
    return leconvert(*((uint16_t*) (&data_[address - region->start])));
}

uint8_t* Memory::getData()
{
    return data_;
}

