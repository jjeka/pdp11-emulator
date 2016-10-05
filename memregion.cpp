#include "memregion.h"
#include "vcpu.h"

MemRegion::MemRegion(uint16_t* mem, Vcpu* cpu) :
    mem_(mem),
    cpu_(cpu),
    readonly_((((uint8_t*) mem) - cpu->memory_) >= VCPU_RAM_OFFSET && (((uint8_t*) mem) - cpu->memory_) < VCPU_RAM_OFFSET + VCPU_RAM_SIZE)
{
}

void MemRegion::operator = (uint16_t val)
{
    set16(val);
}

MemRegion::operator uint16_t()
{
    return *mem_;
}

void MemRegion::set8(uint8_t val)
{
    if (readonly_)
        cpu_->status_ = VCPU_STATUS_WRITE_FROM_READONLY;
    else
        (*((uint8_t*) mem_)) = val;
}

void MemRegion::set16(uint16_t val)
{
    if (readonly_)
        cpu_->status_ = VCPU_STATUS_WRITE_FROM_READONLY;
    else
        (*mem_) = val;
}
