#include "memregion.h"
#include "vcpu.h"

MemRegion::MemRegion(uint16_t* mem, Vcpu* cpu) :
    mem_(mem),
    cpu_(cpu),
    readonly_((((uint8_t*) mem) - cpu->memory_) >= VCPU_ROM_OFFSET && (((uint8_t*) mem) - cpu->memory_) < VCPU_ROM_OFFSET + VCPU_ROM_SIZE),
    wordReadError_((((uint8_t*) mem) - cpu->memory_) == VCPU_MEM_SIZE - 1)
{
}

void MemRegion::operator = (uint16_t val)
{
    set16(val);
}

void MemRegion::operator = (MemRegion& val)
{
    set16(uint16_t(val));
}

MemRegion::operator uint16_t()
{
    if (wordReadError_)
    {
        cpu_->status_ = VCPU_STATUS_INVALID_MEMORY_ACCESS;
        return *((uint8_t*) mem_);
    }
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
    else if (wordReadError_)
        cpu_->status_ = VCPU_STATUS_INVALID_MEMORY_ACCESS;
    else
        (*mem_) = val;
}
