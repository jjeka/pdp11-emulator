#include "vcpu.h"

Vcpu::Vcpu(std::string romFile) :
    registerNames_ { "r1", "r2", "r3", "r4", "r5", "r6", "r7 (sp)", "r8 (pc)" }
{
}

Vcpu::~Vcpu()
{
}

std::string Vcpu::getRegisterName(unsigned n)
{
	assert(n < VCPU_NUM_REGISTERS);
	return registerNames_[n];
}

uint16_t& Vcpu::getRegister(unsigned n)
{
	assert(n < VCPU_NUM_REGISTERS);
    return (uint16_t&) memory_[VCPU_MEM_SIZE + n * sizeof (uint16_t)];
}

uint16_t& Vcpu::getPC()
{
	return getRegister(VCPU_SP_REGISTER);
}

uint16_t& Vcpu::getSP()
{
	return getRegister(VCPU_PC_REGISTER);
}

unsigned Vcpu::getNRegisters()
{
	return VCPU_NUM_REGISTERS;
}

unsigned Vcpu::getDisplayWidth()
{
	return VCPU_DISPLAY_WIDTH;
}

unsigned Vcpu::getDisplayHeight()
{
	return VCPU_DISPLAY_HEIGHT;
}

void* Vcpu::getFramebuffer()
{
	return (void*) &memory_[VCPU_FB_OFFSET];
}

std::string Vcpu::instrAtAddress(uint16_t address)
{
	return "instruction";
}

void Vcpu::start()
{
}

void Vcpu::pause()
{
}

void Vcpu::step()
{
}

void Vcpu::addBreakpoint(uint16_t address)
{

}
