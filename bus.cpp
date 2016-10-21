#include "bus.h"
#include "vcpu.h"
#include "device.h"

Bus::Bus(Vcpu* vcpu) :
    vcpu_(vcpu)
{

}

void Bus::addAddressRegion(Device *owner, uint16_t start, uint16_t size, unsigned flags)
{
    AddressRegion newRegion = {};
    newRegion.owner = owner;
    newRegion.start = start;
    newRegion.size = size;
    newRegion.flags = flags;

    for (auto& region : regions_)
    {
        assert(!(region.start >= newRegion.start && region.start < newRegion.start + newRegion.size));
        assert(!(newRegion.start >= region.start && newRegion.start < region.start + region.size));
    }

    regions_.push_back(newRegion);
}

Bus::AddressRegion* Bus::resolveRegion_(uint16_t start, uint16_t size, bool write)
{
    for (auto& region : regions_)
    {
        if (start >= region.start && start < region.start + region.size)
        {
            if (start + size > region.start + region.size)
            {
                fprintf(stderr, "WARNING: write to bad address region\n");
                return NULL;
            }
            if (write && !(region.flags & BUS_ADDRESSREGION_WRITE))
            {
                fprintf(stderr, "WARNING: writing this region is not permitted\n");
                return NULL;
            }
            if (!write && !(region.flags & BUS_ADDRESSREGION_READ))
            {
                fprintf(stderr, "WARNING: reading this region is not permitted\n");
                return NULL;
            }
            if (size != region.size && (region.flags & BUS_ADDRESSREGION_REGISTER))
            {
                fprintf(stderr, "WARNING: addressing device register not as whole\n");
                return NULL;
            }
        }
    }

    return NULL;
}

void Bus::set8(uint16_t address, uint8_t data)
{
    AddressRegion* region = resolveRegion_(address, 1, true);
    if (region)
        region->owner->set8(region, address, data);
}

uint8_t Bus::get8(uint16_t address)
{
    AddressRegion* region = resolveRegion_(address, 1, false);
    if (region)
        return region->owner->get8(region, address);
    return 0;
}

void Bus::set16(uint16_t address, uint16_t data)
{
    AddressRegion* region = resolveRegion_(address, 2, true);
    if (region)
        region->owner->set8(region, address, data);
}

uint16_t Bus::get16(uint16_t address)
{
    AddressRegion* region = resolveRegion_(address, 2, false);
    if (region)
        return region->owner->get16(region, address);
    return 0;
}
