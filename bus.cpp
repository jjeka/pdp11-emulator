#include "bus.h"
#include "vcpu.h"
#include "device.h"

Bus::Bus(Vcpu* vcpu) :
    vcpu_(vcpu)
{

}

void Bus::addAddressRegion(Device *owner, unsigned start, unsigned size, unsigned flags, const std::string& name)
{
    assert(start + size <= VCPU_MEM_SIZE - 2);

    AddressRegion newRegion = {};
    newRegion.owner = owner;
    newRegion.start = start;
    newRegion.size = size;
    newRegion.flags = flags;
    newRegion.name = name;

    for (auto& region : regions_)
    {
        assert(!(region.start >= newRegion.start && region.start < newRegion.start + newRegion.size));
        assert(!(newRegion.start >= region.start && newRegion.start < region.start + region.size));
    }

    regions_.push_back(newRegion);
}

Bus::AddressRegion* Bus::resolveRegion_(unsigned start, unsigned size, bool write)
{
    for (auto& region : regions_)
    {
        if (start >= region.start && start < region.start + region.size)
        {
            if (start + size > region.start + region.size)
            {
                fprintf(stderr, "WARNING: accessing memory region border: (0%o, %u)\n", start, size);
                return NULL;
            }
            if (write && !(region.flags & BUS_ADDRESSREGION_WRITE))
            {
                fprintf(stderr, "WARNING: writing this region is not permitted: (0%o, %u)\n", start, size);
                return NULL;
            }
            if (!write && !(region.flags & BUS_ADDRESSREGION_READ))
            {
                fprintf(stderr, "WARNING: reading this region is not permitted: (0%o, %u)\n", start, size);
                return NULL;
            }
            if (size != region.size && (region.flags & BUS_ADDRESSREGION_REGISTER))
            {
                fprintf(stderr, "WARNING: addressing device register not as whole: (0%o, %u)\n", start, size);
                return NULL;
            }

            return &region;
        }
    }

    //fprintf(stderr, "WARNING: unable to resolve address: (0%o, %u)\n", start, size);
    return NULL;
}

void Bus::set8(unsigned address, uint8_t data)
{
    AddressRegion* region = resolveRegion_(address, 1, true);
    if (region)
        region->owner->set8(region, address, data);
}

uint8_t Bus::get8(unsigned address)
{
    AddressRegion* region = resolveRegion_(address, 1, false);
    if (region)
        return region->owner->get8(region, address);
    return 0;
}

void Bus::set16(unsigned address, uint16_t data)
{
    AddressRegion* region = resolveRegion_(address, 2, true);
    if (region)
        region->owner->set16(region, address, data);
}

uint16_t Bus::get16(unsigned address)
{
    AddressRegion* region = resolveRegion_(address, 2, false);
    if (region)
        return region->owner->get16(region, address);
    return 0;
}
