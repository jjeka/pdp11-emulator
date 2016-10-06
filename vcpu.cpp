#include "vcpu.h"

Vcpu::Vcpu(std::string romFile, std::function<void()> executionStoppedCallback) :
    registerNames_ { "R1", "R2", "R3", "R4", "R5", "R6", "SP", "PC" },
    thread_(std::bind(&Vcpu::threadFunc_, this))
{
    assert(VCPU_RAM_SIZE > 0);

    threadState_ = VCPU_THREAD_STATE_IDLE;
    threadRunning_ = false;
    executionStoppedCallback_ = executionStoppedCallback;
    status_ = VCPU_STATUS_OK;

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

    for (int i = 0; i < VCPU_NUM_INSTRUCTIONS; i++)
    {
        if (instructions_[i].type == VCPU_INSTR_TYPE_NOT_INITIALIZED)
            abort();
    }

    reset(romFile);
}

Vcpu::~Vcpu()
{
    threadState_ = VCPU_THREAD_STATE_DESTROY;
    thread_.join();
}

VcpuStatus Vcpu::getStatus()
{
    return status_;
}

void Vcpu::addInstruction_(uint16_t begin, uint16_t end, std::string name, void* callback, InstructionType type)
{
    //assert(callback || type == VCPU_INSTR_TYPE_NOT_IMPLEMENTED); // TODO: fix
    assert(type != VCPU_INSTR_TYPE_NOT_INITIALIZED);
    assert(begin <= end);

    std::string* nameAddr;
    if (type != VCPU_INSTR_TYPE_INVALID_OPCODE)
    {
        for (std::string& instrName : instructionNames_)
            assert(name != instrName);

        instructionNames_[numInstructionNames_] = name;
        nameAddr = &instructionNames_[numInstructionNames_];

        numInstructionNames_++;
    }
    else
        nameAddr = NULL;

    for (int i = begin; i <= end; i++)
    {
        assert(instructions_[i].type == VCPU_INSTR_TYPE_NOT_INITIALIZED);

        instructions_[i].type = type;
        instructions_[i].callback = callback;
        instructions_[i].name = nameAddr;
    }
}

std::string Vcpu::getRegisterName(unsigned n)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
	assert(n < VCPU_NUM_REGISTERS);
	return registerNames_[n];
}

uint16_t& Vcpu::getRegister(unsigned n)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
	assert(n < VCPU_NUM_REGISTERS);
    return (uint16_t&) memory_[VCPU_MEM_SIZE + n * sizeof (uint16_t)];
}

uint16_t& Vcpu::getPC()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return getRegister(VCPU_PC_REGISTER);
}

uint16_t& Vcpu::getSP()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return getRegister(VCPU_SP_REGISTER);
}

uint8_t& Vcpu::getPSW()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return (uint8_t&) memory_[VCPU_FLAG_REGISTER_OFFSET];
}

unsigned Vcpu::getNRegisters()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
	return VCPU_NUM_REGISTERS;
}

unsigned Vcpu::getDisplayWidth()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
	return VCPU_DISPLAY_WIDTH;
}

unsigned Vcpu::getDisplayHeight()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
	return VCPU_DISPLAY_HEIGHT;
}

void* Vcpu::getFramebuffer()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
	return (void*) &memory_[VCPU_FB_OFFSET];
}

unsigned Vcpu::getMemSize()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return VCPU_MEM_SIZE;
}

std::string Vcpu::instrAtAddress(uint16_t address)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);

    uint16_t instr = getMemoryWord_(address);
    uint16_t data1 = getMemoryWord_(address + sizeof (uint16_t));
    uint16_t data2 = getMemoryWord_(address + 2 * sizeof (uint16_t));

    char name[VCPU_INSTRUCTION_NAME_SIZE] = "";

    switch (instructions_[instr].type)
    {
    case VCPU_INSTR_TYPE_NOT_IMPLEMENTED:
        sprintf(name, "[not implemented: 0x%X]", instr);
        break;

    case VCPU_INSTR_TYPE_INVALID_OPCODE:
        sprintf(name, "[invalid opcode: 0x%X]", instr);
        break;

    case VCPU_INSTR_TYPE_DOUBLE_OPERAND:
    {
        std::string r1 = getOperand_(instr, 0, data2);
        std::string r2 = getOperand_(instr, 6, data1);
        sprintf(name, "%s %s %s", instructions_[instr].name->c_str(), r2.c_str(), r1.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_OPERAND_REGISTER:
    {
        std::string r1 = getOperand_(instr, 0, data1);
        std::string r2 = getRegisterByInstr_(instr, 6);
        sprintf(name, "%s %s %s", instructions_[instr].name->c_str(), r1.c_str(), r2.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_SINGLE_OPERAND:
    {
        std::string r = getOperand_(instr, 0, data1);
        sprintf(name, "%s %s", instructions_[instr].name->c_str(), r.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_REGISTER:
    {
        std::string r = getRegisterByInstr_(instr, 0);
        sprintf(name, "%s %s", instructions_[instr].name->c_str(), r.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_BRANCH:
        sprintf(name, "%s %X", instructions_[instr].name->c_str(), instr & 256);
        break;

    case VCPU_INSTR_TYPE_WITHOUT_PARAMETERS:
        sprintf(name, "%s", instructions_[instr].name->c_str());
        break;

    case VCPU_INSTR_TYPE_NOT_INITIALIZED:
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
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return GET_BIT(getPSW(), VCPU_NEGATIVE_FLAG_BIT);
}

void Vcpu::setNegativeFlag(bool flag)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    SET_BIT(getPSW(), VCPU_NEGATIVE_FLAG_BIT, flag);
}

bool Vcpu::getZeroFlag()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return GET_BIT(getPSW(), VCPU_ZERO_FLAG_BIT);
}

void Vcpu::setZeroFlag(bool flag)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    SET_BIT(getPSW(), VCPU_ZERO_FLAG_BIT, flag);
}

bool Vcpu::getOverflowFlag()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return GET_BIT(getPSW(), VCPU_OVERFLOW_FLAG_BIT);
}

void Vcpu::setOverflowFlag(bool flag)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    SET_BIT(getPSW(), VCPU_OVERFLOW_FLAG_BIT, flag);
}

bool Vcpu::getCarryFlag()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return GET_BIT(getPSW(), VCPU_CARRY_FLAG_BIT);
}

void Vcpu::setCarryFlag(bool flag)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    SET_BIT(getPSW(), VCPU_CARRY_FLAG_BIT, flag);
}

void Vcpu::start()
{
    assert(status_ == VCPU_STATUS_OK);

    breakpointHit_ = false;
    if (breakpointExists(getPC()))
        breakpointHit_ = true;

    if (threadState_ == VCPU_THREAD_STATE_IDLE)
        threadState_ = VCPU_THREAD_STATE_RUNNING;
}

void Vcpu::threadFunc_()
{
    while (threadState_ != VCPU_THREAD_STATE_DESTROY)
    {
        while (threadState_ == VCPU_THREAD_STATE_IDLE)
            usleep(1000);

        threadRunning_ = true;
        if (threadState_ == VCPU_THREAD_STATE_RUNNING)
        {
            while (threadState_ == VCPU_THREAD_STATE_RUNNING)
            {
                if (breakpointExists(getPC()))
                {
                    if (breakpointHit_) // we started at breakpoint so continue executing
                        breakpointHit_ = false;
                    else
                    {
                        breakpointHit_ = true;
                        executionStoppedCallback_();
                        threadState_ = VCPU_THREAD_STATE_IDLE;
                    }
                }

                executeInstruction_();

                if (status_ != VCPU_STATUS_OK)
                {
                    threadState_ = VCPU_THREAD_STATE_IDLE;
                    executionStoppedCallback_();
                }
            }
        }
        else if (threadState_ == VCPU_THREAD_STATE_SINGLE_INSTRUCTION)
        {
            executeInstruction_();
            threadState_ = VCPU_THREAD_STATE_IDLE;
        }
        threadRunning_ = false;
    }
}

void Vcpu::reset(std::string romFile)
{
    reset();

    FILE* file = fopen(romFile.c_str(), "rb");
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
    fread(&memory_[VCPU_ROM_OFFSET], 1, VCPU_ROM_SIZE, file);
    fclose(file);
}

void Vcpu::reset()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);

    if (threadState_ == VCPU_THREAD_STATE_RUNNING || threadState_ == VCPU_THREAD_STATE_SINGLE_INSTRUCTION)
        threadState_ = VCPU_THREAD_STATE_IDLE;

    while (threadRunning_)
        usleep(1000);

    for (int i = 0; i < (int) VCPU_BUFFER_SIZE; i++)
    {
        if (i < VCPU_ROM_OFFSET || i >= VCPU_ROM_OFFSET + VCPU_ROM_SIZE)
            memory_[i] = 0;
    }
    // ^ it's resets all registers

    getPC() = VCPU_ROM_OFFSET;
    breakpointHit_ = false;
    breakpoints_.clear();

    status_ = VCPU_STATUS_OK;
}

void Vcpu::pause()
{
    assert(status_ == VCPU_STATUS_OK);

    if (threadState_ == VCPU_THREAD_STATE_RUNNING || threadState_ == VCPU_THREAD_STATE_SINGLE_INSTRUCTION)
        threadState_ = VCPU_THREAD_STATE_IDLE;

    while (threadRunning_)
        usleep(1000);
}

void Vcpu::step()
{
    assert(status_ == VCPU_STATUS_OK);

    if (threadState_ == VCPU_THREAD_STATE_IDLE)
    {
        threadRunning_ = true;
        threadState_ = VCPU_THREAD_STATE_SINGLE_INSTRUCTION;

        while (threadState_ == VCPU_THREAD_STATE_SINGLE_INSTRUCTION)
            usleep(1000);

        threadState_ = VCPU_THREAD_STATE_IDLE;
    }
}

void Vcpu::addBreakpoint(uint16_t address)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    breakpoints_.insert(address);
}

void Vcpu::removeBreakpoint(uint16_t address)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    breakpoints_.erase(address);
}

bool Vcpu::breakpointExists(uint16_t address)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return breakpoints_.find(address) != breakpoints_.end();
}

bool Vcpu::breakpointHit()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return breakpointHit_;
}

void Vcpu::executeInstruction_()
{
    uint16_t instr = getMemoryWord_(getPC());

    if (instructions_[instr].type == VCPU_INSTR_TYPE_NOT_IMPLEMENTED)
    {
        status_ = VCPU_STATUS_NOT_IMPLEMENTED_INSTRUCTION;
        threadState_ = VCPU_THREAD_STATE_IDLE;
        return;
    }
    if (instructions_[instr].type == VCPU_INSTR_TYPE_INVALID_OPCODE)
    {
        status_ = VCPU_STATUS_INVALID_OPCODE;
        threadState_ = VCPU_THREAD_STATE_IDLE;
        return;
    }

    assert(instructions_[instr].type != VCPU_INSTR_TYPE_NOT_INITIALIZED);
    assert(instructions_[instr].callback);

    uint16_t prevPC = getPC();
    getPC() += sizeof (uint16_t);

    VcpuPSW psw = { getNegativeFlag(), getZeroFlag(), getOverflowFlag(), getCarryFlag() };

    switch (instructions_[instr].type)
    {
    case VCPU_INSTR_TYPE_DOUBLE_OPERAND:
    {
        uint16_t incrementSize = (instr & (1 << 15)) ? 1 : 2;
        MemRegion dstRegion(&getAddrByAddrMode_(VCPU_GET_REG(instr, 6), VCPU_GET_ADDR_MODE(instr, 6), incrementSize), this);
        MemRegion srcRegion(&getAddrByAddrMode_(VCPU_GET_REG(instr, 0), VCPU_GET_ADDR_MODE(instr, 0), incrementSize), this);

        if (!((vcpu_instr_double_operand_callback*) instructions_[instr].callback)(dstRegion, srcRegion, psw))
            status_ = VCPU_STATUS_INVALID_INSTRUCTION;
    }
        break;

    case VCPU_INSTR_TYPE_OPERAND_REGISTER:
    {
        MemRegion srcRegion(&getAddrByAddrMode_(VCPU_GET_REG(instr, 0), VCPU_GET_ADDR_MODE(instr, 0), 2), this);
        MemRegion regRegion(&getAddrByAddrMode_(VCPU_GET_REG(instr, 6), VCPU_ADDR_MODE_REGISTER, 2), this);
        MemRegion reg2Region(&getAddrByAddrMode_(
                                 (VCPU_GET_REG(instr, 6) % 2 == 0) ? (VCPU_GET_REG(instr, 6) + 1) : VCPU_GET_REG(instr, 6),
                                 VCPU_ADDR_MODE_REGISTER, 2), this);

        if (!((vcpu_instr_operand_register_callback*) instructions_[instr].callback)
                ((VCPU_GET_REG(instr, 6) % 2 == 1), regRegion, reg2Region, srcRegion, psw))
        status_ = VCPU_STATUS_INVALID_INSTRUCTION;
    }
    break;

    case VCPU_INSTR_TYPE_SINGLE_OPERAND:
    {
        MemRegion region(&getAddrByAddrMode_(VCPU_GET_REG(instr, 0), VCPU_GET_ADDR_MODE(instr, 0), 2), this);

        if (!((vcpu_instr_single_operand_callback*) instructions_[instr].callback)(region, psw))
            status_ = VCPU_STATUS_INVALID_INSTRUCTION;
    }
    break;

    case VCPU_INSTR_TYPE_BRANCH:
    {
        int8_t offset = (instr & 255);
        if (!((vcpu_instr_branch_callback*) instructions_[instr].callback)(getPC(), offset, psw))
            status_ = VCPU_STATUS_INVALID_INSTRUCTION;
    }
    break;

    // TODO: case VCPU_INSTR_TYPE_REGISTER, case VCPU_INSTR_TYPE_WITHOUT_PARAMETERS

    case VCPU_INSTR_TYPE_NOT_INITIALIZED:
    case VCPU_INSTR_TYPE_NOT_IMPLEMENTED:
    case VCPU_INSTR_TYPE_INVALID_OPCODE:
    default:
        abort();
        break;
    }

    setNegativeFlag(psw.n);
    setZeroFlag(psw.z);
    setOverflowFlag(psw.v);
    setCarryFlag(psw.c);

    if (status_ != VCPU_STATUS_OK)
    {
        getPC() = prevPC;
        threadState_ = VCPU_THREAD_STATE_IDLE;
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
