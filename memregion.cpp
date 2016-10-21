#include "memregion.h"
#include "vcpu.h"

MemRegion8::MemRegion8(unsigned addr, Vcpu* cpu) :
    addr_(addr),
    vcpu_(cpu)
{
}

void MemRegion8::operator =(uint8_t val)
{
    if (addr_ >= (1 << 16))
    {
        unsigned reg = addr_ & 0xffff;
        assert(reg < VCPU_NUM_REGISTERS);
        vcpu_->getRegister(reg) = leconvert((leconvert(vcpu_->getRegister(reg)) & 0xff00) | uint16_t(val));
    }
    else if (addr_ == VCPU_FLAG_REGISTER_OFFSET)
        vcpu_->getPSW() = val;
    else if (addr_ == VCPU_FLAG_REGISTER_OFFSET + 1)
        fprintf(stderr, "WARING: attempt to write into PSW hi");
    else
        vcpu_->getBus().set8(addr_, val);
}

void MemRegion8::operator =(const MemRegion8& val)
{
    *this = uint8_t(val);
}

MemRegion8::operator uint8_t() const
{
    if (addr_ >= (1 << 16))
    {
        unsigned reg = addr_ & 0xffff;
        assert(reg < VCPU_NUM_REGISTERS);
        return uint8_t(leconvert(vcpu_->getRegister(reg)));
    }
    else if (addr_ == VCPU_FLAG_REGISTER_OFFSET)
        return vcpu_->getPSW();
    else if (addr_ == VCPU_FLAG_REGISTER_OFFSET + 1)
    {
        fprintf(stderr, "WARING: attempt to read PSW hi");
        return 0;
    }
    else
        return vcpu_->getBus().get8(addr_);
}

MemRegion16::MemRegion16(unsigned addr, Vcpu* cpu) :
    addr_(addr),
    vcpu_(cpu)
{
}

void MemRegion16::operator =(uint16_t val)
{
    if (addr_ >= (1 << 16))
    {
        unsigned reg = addr_ & 0xffff;
        assert(reg < VCPU_NUM_REGISTERS);
        vcpu_->getRegister(reg) = val;
    }
    else if (addr_ == VCPU_FLAG_REGISTER_OFFSET)
        vcpu_->getPSW() = uint8_t(val);
    else if (addr_ == VCPU_FLAG_REGISTER_OFFSET + 1 || addr_ == VCPU_FLAG_REGISTER_OFFSET - 1)
    {
        fprintf(stderr, "ERROR: Unaligned partial PSW write");
        assert(false);
    }
    else
        vcpu_->getBus().set16(addr_, val);
}

MemRegion16::operator uint16_t() const
{
    if (addr_ >= (1 << 16))
    {
        unsigned reg = addr_ & 0xffff;
        assert(reg < VCPU_NUM_REGISTERS);
        return vcpu_->getRegister(reg);
    }
    else if (addr_ == VCPU_FLAG_REGISTER_OFFSET)
        return vcpu_->getPSW();
    else if (addr_ == VCPU_FLAG_REGISTER_OFFSET + 1 || addr_ == VCPU_FLAG_REGISTER_OFFSET - 1)
    {
        fprintf(stderr, "ERROR: Unaligned partial PSW read");
        assert(false);
    }
    else
        return vcpu_->getBus().get16(addr_);
}

void MemRegion16::operator =(const MemRegion16& val)
{
    *this = uint16_t(val);
}
