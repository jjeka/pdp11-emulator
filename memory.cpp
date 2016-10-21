#include "memory.h"
#include <arpa/inet.h>

Memory::Memory(uint16_t start, uint16_t size, unsigned flags, Bus* bus) :
    data_(new uint8_t[size])
{
    bus->addAddressRegion(this, start, size, flags);
}

void Memory::set8(Bus::AddressRegion* region, uint16_t address, uint8_t data)
{
    data_[address - region->start] = data;
}

uint8_t Memory::get8(Bus::AddressRegion* region, uint16_t address)
{
    return data_[address - region->start];
}

void Memory::set16(Bus::AddressRegion* region, uint16_t address, uint16_t data)
{
    *((uint16_t*) (&data_[address - region->start])) = htons(data);
}

uint16_t Memory::get16(Bus::AddressRegion* region, uint16_t address)
{
    return ntohs(*((uint16_t*) (&data_[address - region->start])));
}

