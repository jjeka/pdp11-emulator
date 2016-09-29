#include "vcpu.h"

// TODO: state for instruction fails

Vcpu::Vcpu(std::string romFile, std::function<void()> executionStoppedCallback) :
    registerNames_ { "R1", "R2", "R3", "R4", "R5", "R6", "SP", "PC" }
{
    assert(VCPU_RAM_SIZE > 0);

    executionStoppedCallback_ = executionStoppedCallback;
    status_ = VCPU_STATUS_OK;

    FILE* file = fopen(romFile.c_str(), "r");
    if (!file)
    {
        status_ = VCPU_STATUS_FAIL_OPEN_ROM;
        return;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size != VCPU_ROM_SIZE)
    {
        status_ = VCPU_STATUS_WRONG_ROM_SIZE;
        fclose(file);
        return;
    }
    fseek(file, 0, SEEK_SET);
    fread(&memory_[VCPU_RAM_OFFSET], 1, VCPU_RAM_SIZE, file);
    fclose(file);

    for (Instruction& instr : instructions_)
    {
        instr.callback = NULL;
        instr.name = NULL;
        instr.type = VCPU_INSTR_TYPE_NOT_INITIALIZED;
    }
    numInstructionNames_ = 0;

    for (int i = 0; i < VCPU_NUM_INSTRUCTIONS_IN_TABLE; i++)
        addInstruction_(VCPU_INSTRUCTIONS[i].begin,
                        VCPU_INSTRUCTIONS[i].end,
                        VCPU_INSTRUCTIONS[i].name,
                        VCPU_INSTRUCTIONS[i].callback,
                        VCPU_INSTRUCTIONS[i].type);

    // TODO: add check initialized

    memset(memory_, 0, VCPU_BUFFER_SIZE * sizeof (uint8_t));
    // ^ it's resets all registers

    srand(time(0));
    for (int i = 0; i < VCPU_MEM_SIZE; i++)
        memory_[i] = rand() % 256; // TODO: remove
}

Vcpu::~Vcpu()
{
}

 void Vcpu::addInstruction_(uint16_t begin, uint16_t end, std::string name, void* callback, InstructionType type)
 {
     //assert(callback); // TODO: !
     assert(type != VCPU_INSTR_TYPE_NOT_INITIALIZED);
     assert(begin < end);

     for (std::string& instrName : instructionNames_)
     {
         assert(name != instrName);
     }
     instructionNames_[numInstructionNames_] = name;
     std::string* nameAddr = &instructionNames_[numInstructionNames_];
     numInstructionNames_++;

     for (int i = begin; i < end; i++)
     {
         assert(instructions_[i].type == VCPU_INSTR_TYPE_NOT_INITIALIZED);

         instructions_[i].type = type;
         instructions_[i].callback = callback;
         instructions_[i].name = nameAddr;
     }
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
    return getRegister(VCPU_PC_REGISTER);
}

uint16_t& Vcpu::getSP()
{
    return getRegister(VCPU_SP_REGISTER);
}

uint8_t& Vcpu::getPSW()
{
    return (uint8_t&) memory_[VCPU_FLAG_REGISTER_OFFSET];
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

unsigned Vcpu::getMemSize()
{
    return VCPU_MEM_SIZE;
}

std::string Vcpu::instrAtAddress(uint16_t address)
{
    uint16_t instr = getMemoryWord_(address);
    uint16_t data1 = getMemoryWord_(address + sizeof (uint16_t));
    uint16_t data2 = getMemoryWord_(address + 2 * sizeof (uint16_t));

    char name[VCPU_INSTRUCTION_NAME_SIZE] = "";

    switch (instructions_[instr].type)
    {
    case VCPU_INSTR_TYPE_NOT_INITIALIZED:
        sprintf(name, "[not initialized: 0x%X]", instr);
        break;

    case VCPU_INSTR_TYPE_NOT_IMPLEMENTED:
        sprintf(name, "[not implemented: 0x%X]", instr);
        break;

    case VCPU_INSTR_TYPE_DOUBLE_OPERAND:
    {
        std::string r1 = getOperand_(instr, 0, data2);
        std::string r2 = getOperand_(instr, 6, data1);
        sprintf(name, "%s %s %s", instructions_[instr].name->c_str(), r1.c_str(), r2.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_OPERAND_REGISTER:
    {
        std::string r1 = getOperand_(instr, 0, data1);
        std::string r2 = getRegisterByInstr_(instr, 6);
        sprintf(name, "%s %s %s", instructions_[instr].name->c_str(), r2.c_str(), r1.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_SINGLE_OPERAND:
    {
        std::string r = getOperand_(instr, 0, data1);
        sprintf(name, "%s %s", instructions_[instr].name->c_str(), r.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_BRANCH:
        sprintf(name, "%s %X", instructions_[instr].name->c_str(), instr & 256);
        break;

    default:
        abort();
        break;
    }
    return name;
}

std::string Vcpu::getRegisterByInstr_(uint16_t instr, int begin)
{
    int r = (instr >> begin) & 7;
    return registerNames_[r];
}

std::string Vcpu::getOperand_(uint16_t instr, int begin, uint16_t data)
{
    int r = (instr >> begin) & 7;
    int mode = (instr >> (begin + 3)) & 7;

    char result[VCPU_INSTRUCTION_NAME_SIZE] = "";
    if (r != VCPU_PC_REGISTER || (r == VCPU_PC_REGISTER &&
                                  (mode != VCPU_ADDR_MODE_AUTOINCREMENT &&
                                   mode != VCPU_ADDR_MODE_AUTOINCREMENT_DEFFERED &&
                                   mode != VCPU_ADDR_MODE_INDEX &&
                                   mode != VCPU_ADDR_MODE_INDEX_DEFERRED)))
    {
        const char* registerName = registerNames_[r].c_str();

        switch (mode)
        {
        case VCPU_ADDR_MODE_REGISTER:
            sprintf(result, "%s", registerName);
            break;

        case VCPU_ADDR_MODE_REGISTER_DEFFERED:
            sprintf(result, "(%s)", registerName);
            break;

        case VCPU_ADDR_MODE_AUTOINCREMENT:
            sprintf(result, "(%s)+", registerName);
            break;

        case VCPU_ADDR_MODE_AUTOINCREMENT_DEFFERED:
            sprintf(result, "@(%s)+", registerName);
            break;

        case VCPU_ADDR_MODE_AUTODECREMENT:
            sprintf(result, "-(%s)", registerName);
            break;

        case VCPU_ADDR_MODE_AUTODECREMENT_DEFFERED:
            sprintf(result, "@-(%s)", registerName);
            break;

        case VCPU_ADDR_MODE_INDEX:
            sprintf(result, "%o(%s)", data, registerName);
            break;

        case VCPU_ADDR_MODE_INDEX_DEFERRED:
            sprintf(result, "@%o(%s)", data, registerName);
            break;

        default:
            abort();
            break;
        }
    }
    else
    {
        switch (mode)
        {
        case VCPU_ADDR_MODE_AUTOINCREMENT: // immediate
            sprintf(result, "#%o", data);
            break;

        case VCPU_ADDR_MODE_AUTOINCREMENT_DEFFERED: // absolute
            sprintf(result, "@#%o", data);
            break;

        case VCPU_ADDR_MODE_INDEX: // relative
            sprintf(result, "A (PC+%o)", data);
            break;

        case VCPU_ADDR_MODE_INDEX_DEFERRED: // relative deferred
            sprintf(result, "@A (PC+%o)", data);
            break;

        default:
            abort();
            break;
        }
    }
    return result;
}

bool Vcpu::getNegativeFlag()
{
    return VCPU_GET_BIT(getPSW(), VCPU_NEGATIVE_FLAG_BIT);
}

void Vcpu::setNegativeFlag(bool flag)
{
    VCPU_SET_BIT(getPSW(), VCPU_NEGATIVE_FLAG_BIT, flag);
}

bool Vcpu::getZeroFlag()
{
    return VCPU_GET_BIT(getPSW(), VCPU_ZERO_FLAG_BIT);
}

void Vcpu::setZeroFlag(bool flag)
{
    VCPU_SET_BIT(getPSW(), VCPU_ZERO_FLAG_BIT, flag);
}

bool Vcpu::getOverflowFlag()
{
    return VCPU_GET_BIT(getPSW(), VCPU_OVERFLOW_FLAG_BIT);
}

void Vcpu::setOverflowFlag(bool flag)
{
    VCPU_SET_BIT(getPSW(), VCPU_OVERFLOW_FLAG_BIT, flag);
}

bool Vcpu::getCarryFlag()
{
    return VCPU_GET_BIT(getPSW(), VCPU_CARRY_FLAG_BIT);
}

void Vcpu::setCarryFlag(bool flag)
{
    VCPU_SET_BIT(getPSW(), VCPU_CARRY_FLAG_BIT, flag);
}

void Vcpu::start()
{
}

void Vcpu::reset()
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
    breakpoints_.insert(address);
}

void Vcpu::removeBreakpoint(uint16_t address)
{
    breakpoints_.erase(address);
}

bool Vcpu::breakpointExists(uint16_t address)
{
    return breakpoints_.find(address) != breakpoints_.end();
}

bool breakpointHit()
{
    return false;
}

bool Vcpu::readonlyAddr_(uint16_t addr)
{
    return addr >= VCPU_RAM_OFFSET && addr < VCPU_RAM_OFFSET + VCPU_RAM_SIZE;
}

void Vcpu::executeInstruction_()
{
    uint16_t instr = getMemoryWord_(getPC());

    assert(instructions_[instr].type != VCPU_INSTR_TYPE_NOT_INITIALIZED);
    assert(instructions_[instr].type != VCPU_INSTR_TYPE_NOT_IMPLEMENTED);
    assert(instructions_[instr].callback);

    getPC() += sizeof (uint16_t);

    switch (instructions_[instr].type)
    {
    case VCPU_INSTR_TYPE_DOUBLE_OPERAND:
    {
        uint16_t incrementSize = (instr & (1 << 15)) ? 1 : 2;

        uint16_t& src = getAddrByAddrMode_(VCPU_GET_REG(instr, 6), VCPU_GET_ADDR_MODE(instr, 6), incrementSize);
        uint16_t& dst = getAddrByAddrMode_(VCPU_GET_REG(instr, 0), VCPU_GET_ADDR_MODE(instr, 0), incrementSize);

        VcpuPSW psw = { getNegativeFlag(), getZeroFlag(), getOverflowFlag(), getCarryFlag() };

        ((vcpu_instr_double_operand_callback*) instructions_[instr].callback)(dst, src, psw);

        setNegativeFlag(psw.n);
        setZeroFlag(psw.z);
        setOverflowFlag(psw.o);
        setCarryFlag(psw.c);
    }
        break;

    default:
        //abort(); TODO:
        break;
    }
}

uint16_t& Vcpu::getMemoryWord_(uint16_t addr)
{
    return *((uint16_t*) &memory_[addr]);
}

uint16_t& Vcpu::getAddrByAddrMode_(int r, int mode, uint16_t incrementSize)
{
    uint16_t* addr = NULL;

    switch (mode)
    {
    case VCPU_ADDR_MODE_REGISTER:
        addr = &getRegister(r);
        break;

    case VCPU_ADDR_MODE_REGISTER_DEFFERED:
        addr = &getMemoryWord_(getRegister(r));
        break;

    case VCPU_ADDR_MODE_AUTOINCREMENT:
        addr = &getMemoryWord_(getRegister(r));
        getRegister(r) += incrementSize;
        break;

    case VCPU_ADDR_MODE_AUTOINCREMENT_DEFFERED:
        addr = &getMemoryWord_(getRegister(r));
        getRegister(r) += sizeof (uint16_t); // it's correct even for byte instructions
        break;

    case VCPU_ADDR_MODE_AUTODECREMENT:
        addr = &getMemoryWord_(getRegister(r));
        getRegister(r) -= incrementSize;
        break;

    case VCPU_ADDR_MODE_AUTODECREMENT_DEFFERED:
        addr = &getMemoryWord_(getMemoryWord_(getRegister(r)));
        getRegister(r) -= sizeof (uint16_t); // it's correct even for byte instructions
        break;

    case VCPU_ADDR_MODE_INDEX:
        addr = &getMemoryWord_(getRegister(r) + *((uint16_t*) &getMemoryWord_(getPC())));
        getPC() += sizeof (uint16_t);
        break;

    case VCPU_ADDR_MODE_INDEX_DEFERRED:
        addr = &getMemoryWord_(getMemoryWord_(getRegister(r) + *((uint16_t*) &getMemoryWord_(getPC()))));
        getPC() += sizeof (uint16_t);
        break;

    default:
        abort();
        break;
    }

    return *addr;
}
