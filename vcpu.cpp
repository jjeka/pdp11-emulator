#include "vcpu.h"

Vcpu::Vcpu(std::string romFile, std::function<void()> executionStoppedCallback) :
    bus_(this),
    registerNames_ { "r0", "r1", "r2", "r3", "r4", "r5", "sp", "pc" },
    rom_(VCPU_ROM_OFFSET, VCPU_ROM_SIZE, BUS_ADDRESSREGION_READ, &bus_, "ROM"),
    ram_(VCPU_RAM_OFFSET, VCPU_RAM_SIZE, BUS_ADDRESSREGION_READ | BUS_ADDRESSREGION_WRITE, &bus_, "RAM"),
    fb_(VCPU_FB_OFFSET, VCPU_FB_SIZE, BUS_ADDRESSREGION_READ | BUS_ADDRESSREGION_WRITE, &bus_, "FB"),
    keyboard_(this),
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
    assert(callback || type == VCPU_INSTR_TYPE_NOT_IMPLEMENTED || type == VCPU_INSTR_TYPE_INVALID_OPCODE);
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
    return registers_[n];
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

uint16_t& Vcpu::getPSW()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return psw_;
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
    return fb_.getData();
}

unsigned Vcpu::getMemSize()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return VCPU_MEM_SIZE;
}

void Vcpu::toOctal_(uint16_t n, char* str)
{
    if (int16_t(n) > 0)
        sprintf(str, "0%o", unsigned(n));
    else if (int16_t(n) == 0)
        sprintf(str, "0");
    else
        sprintf(str, "-0%o", unsigned(-int16_t(n)));
}

std::string Vcpu::instrAtAddress(uint16_t address)
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);

    uint16_t instr = getWordAtAddress(address);
    uint16_t data1 = getWordAtAddress(address + sizeof (uint16_t));
    uint16_t data2 = getWordAtAddress(address + 2 * sizeof (uint16_t));

    char name[VCPU_INSTRUCTION_NAME_SIZE] = "";
    char temp[VCPU_INSTRUCTION_NAME_SIZE] = "";

    switch (instructions_[instr].type)
    {
    case VCPU_INSTR_TYPE_NOT_IMPLEMENTED:
        sprintf(name, "[not implemented: %s]", instructions_[instr].name->c_str());
        break;

    case VCPU_INSTR_TYPE_INVALID_OPCODE:
        sprintf(name, "[invalid opcode]");
        break;

    case VCPU_INSTR_TYPE_DOUBLE_OPERAND:
    {
        bool dataNeeded = false;

        std::string r2 = getOperand_(address, instr, 6, data1, &dataNeeded, false);
        std::string r1 = getOperand_(address, instr, 0, dataNeeded ? data2 : data1, NULL, dataNeeded);
        sprintf(name, "%s %s, %s", instructions_[instr].name->c_str(), r2.c_str(), r1.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_OPERAND_REGISTER:
    case VCPU_INSTR_TYPE_OPERAND_REGISTER_EX:
    {
        std::string r1 = getOperand_(address, instr, 0, data1, NULL, false);
        std::string r2 = getRegisterByInstr_(instr, 6);
        sprintf(name, "%s %s, %s", instructions_[instr].name->c_str(), r1.c_str(), r2.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_SINGLE_REGISTER:
    {
        std::string r = getRegisterByInstr_(instr, 0);
        sprintf(name, "%s %s", instructions_[instr].name->c_str(), r.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_SINGLE_OPERAND:
    case VCPU_INSTR_TYPE_SINGLE_OPERAND_EX:
    {
        std::string r = getOperand_(address, instr, 0, data1, NULL, false);
        sprintf(name, "%s %s", instructions_[instr].name->c_str(), r.c_str());
    }
        break;

    case VCPU_INSTR_TYPE_BRANCH:
        toOctal_(int16_t(int8_t(instr & 255)) * 2 + 2, temp);
        sprintf(name, "%s 0%o", instructions_[instr].name->c_str(), unsigned(uint16_t(address + int16_t(int8_t(instr & 255)) * 2 + 2)));
        break;

    case VCPU_INSTR_TYPE_NUMBER:
        sprintf(name, "%s 0%o", instructions_[instr].name->c_str(), unsigned(uint8_t(instr & 63)));
        break;

    case VCPU_INSTR_TYPE_REGISTER_NUMBER:
    {
        std::string r = getRegisterByInstr_(instr, 6);
        sprintf(name, "%s %s,0%o", instructions_[instr].name->c_str(), r.c_str(), unsigned(uint8_t(instr & 63)));
    }
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

std::string Vcpu::getOperand_(uint16_t pc, uint16_t instr, int begin, uint16_t data, bool* dataNeeded, bool secondParam)
{
    int r = (instr >> begin) & 7;
    int mode = (instr >> (begin + 3)) & 7;

    char result[VCPU_INSTRUCTION_NAME_SIZE] = "";
    char temp[VCPU_INSTRUCTION_NAME_SIZE] = "";
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
            toOctal_(data, temp);
            sprintf(result, "%s(%s)", temp, registerName);
            if (dataNeeded)
                *dataNeeded = true;
            break;

        case VCPU_ADDR_MODE_INDEX_DEFERRED:
            toOctal_(data, temp);
            sprintf(result, "@%s(%s)", temp, registerName);
            if (dataNeeded)
                *dataNeeded = true;
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
            toOctal_(data, temp);
            sprintf(result, "$%s", temp);
            if (dataNeeded)
                *dataNeeded = true;
            break;

        case VCPU_ADDR_MODE_AUTOINCREMENT_DEFFERED: // absolute
            toOctal_(data, temp);
            sprintf(result, "@$%s", temp);
            if (dataNeeded)
                *dataNeeded = true;
            break;

        case VCPU_ADDR_MODE_INDEX: // relative
            sprintf(result, "0%o", unsigned((uint16_t(pc + data +
                    (2 + (secondParam ? 1 : 0)) * sizeof (uint16_t))))); // unsigned octal needed
            if (dataNeeded)
                *dataNeeded = true;
            break;

        case VCPU_ADDR_MODE_INDEX_DEFERRED: // relative deferred
            sprintf(result, "@0%o", unsigned((uint16_t(pc + data +
                    (2 + (secondParam ? 1 : 0)) * sizeof (uint16_t))))); // unsigned octal needed
            if (dataNeeded)
                *dataNeeded = true;
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
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

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
                        break;
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
    fread(rom_.getData(), 1, VCPU_ROM_SIZE, file);
    fclose(file);
}

void Vcpu::reset()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);

    if (threadState_ == VCPU_THREAD_STATE_RUNNING || threadState_ == VCPU_THREAD_STATE_SINGLE_INSTRUCTION)
        threadState_ = VCPU_THREAD_STATE_IDLE;

    while (threadRunning_)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    memset(ram_.getData(), 0, VCPU_RAM_SIZE);
    memset(fb_.getData(), 0, VCPU_FB_SIZE);
    memset(registers_, 0, VCPU_NUM_REGISTERS * sizeof (uint16_t));
    psw_ = 0;

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
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void Vcpu::step()
{
    assert(status_ == VCPU_STATUS_OK);

    if (threadState_ == VCPU_THREAD_STATE_IDLE)
    {
        threadRunning_ = true;
        threadState_ = VCPU_THREAD_STATE_SINGLE_INSTRUCTION;

        while (threadState_ == VCPU_THREAD_STATE_SINGLE_INSTRUCTION)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        threadState_ = VCPU_THREAD_STATE_IDLE;
    }
}
\

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

bool Vcpu::haltHit()
{
    assert(status_ != VCPU_STATUS_FAIL_OPEN_ROM && status_ != VCPU_STATUS_WRONG_ROM_SIZE);
    return haltHit_;
}

void Vcpu::executeInstruction_()
{
    keyboard_.pollInterrupts();

    haltHit_ = false;

    uint16_t instr = getWordAtAddress(getPC());

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
        uint16_t incrementSize = isByteInstruction_(instr) ? 1 : 2;
        unsigned srcAddr = getAddrByAddrMode_(VCPU_GET_REG(instr, 6), VCPU_GET_ADDR_MODE(instr, 6), incrementSize);
        unsigned dstAddr = getAddrByAddrMode_(VCPU_GET_REG(instr, 0), VCPU_GET_ADDR_MODE(instr, 0), incrementSize);

        if (isByteInstruction_(instr))
        {
            MemRegion8 srcRegion(srcAddr, this);
            MemRegion8 dstRegion(dstAddr, this);

            if (!((vcpu_instr_double_operand_8_callback*) instructions_[instr].callback)(dstRegion, srcRegion, psw))
                status_ = VCPU_STATUS_INVALID_INSTRUCTION;
        }
        else
        {
            MemRegion16 srcRegion(srcAddr, this);
            MemRegion16 dstRegion(dstAddr, this);

            if (!((vcpu_instr_double_operand_16_callback*) instructions_[instr].callback)(dstRegion, srcRegion, psw))
                status_ = VCPU_STATUS_INVALID_INSTRUCTION;
        }

    }
        break;

    case VCPU_INSTR_TYPE_OPERAND_REGISTER:
    case VCPU_INSTR_TYPE_OPERAND_REGISTER_EX:
    {
        MemRegion16 srcRegion(getAddrByAddrMode_(VCPU_GET_REG(instr, 0), VCPU_GET_ADDR_MODE(instr, 0), 2), this);
        MemRegion16 regRegion(getAddrByAddrMode_(VCPU_GET_REG(instr, 6), VCPU_ADDR_MODE_REGISTER, 2), this);
        MemRegion16 reg2Region(getAddrByAddrMode_(
                                 (VCPU_GET_REG(instr, 6) % 2 == 0) ? (VCPU_GET_REG(instr, 6) + 1) : VCPU_GET_REG(instr, 6),
                                 VCPU_ADDR_MODE_REGISTER, 2), this);

        if (instructions_[instr].type == VCPU_INSTR_TYPE_OPERAND_REGISTER)
        {
            if (!((vcpu_instr_operand_register_callback*) instructions_[instr].callback)
                    ((VCPU_GET_REG(instr, 6) % 2 == 1), regRegion, reg2Region, srcRegion, psw))
                status_ = VCPU_STATUS_INVALID_INSTRUCTION;
        }
        else
        {
            if (!((vcpu_instr_operand_register_ex_callback*) instructions_[instr].callback)
                    (instr, regRegion, srcRegion, *this))
                status_ = VCPU_STATUS_INVALID_INSTRUCTION;
        }
    }
    break;

    case VCPU_INSTR_TYPE_SINGLE_REGISTER:
    {
        MemRegion16 regRegion(getAddrByAddrMode_(VCPU_GET_REG(instr, 0), VCPU_ADDR_MODE_REGISTER, 2), this);

        if (!((vcpu_instr_single_register_callback*) instructions_[instr].callback)
                (regRegion, *this))
            status_ = VCPU_STATUS_INVALID_INSTRUCTION;
    }
    break;

    case VCPU_INSTR_TYPE_SINGLE_OPERAND:
    {
        unsigned addr = getAddrByAddrMode_(VCPU_GET_REG(instr, 0), VCPU_GET_ADDR_MODE(instr, 0), 2);

        if (isByteInstruction_(instr))
        {
            MemRegion8 region(addr, this);
            if (!((vcpu_instr_single_operand_8_callback*) instructions_[instr].callback)(region, psw))
                status_ = VCPU_STATUS_INVALID_INSTRUCTION;
        }
        else
        {
            MemRegion16 region(addr, this);
            if (!((vcpu_instr_single_operand_16_callback*) instructions_[instr].callback)(region, psw))
                status_ = VCPU_STATUS_INVALID_INSTRUCTION;
        }
    }
    break;

    case VCPU_INSTR_TYPE_WITHOUT_PARAMETERS:
    case VCPU_INSTR_TYPE_SINGLE_OPERAND_EX:
    {
        if (!((vcpu_instr_without_parameters_and_ex_callback*) instructions_[instr].callback)(instr, *this))
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

    case VCPU_INSTR_TYPE_NUMBER:
    {
        uint8_t n = (instr & 63);

        if (!((vcpu_instr_number_callback*) instructions_[instr].callback)(n, *this))
            status_ = VCPU_STATUS_INVALID_INSTRUCTION;
    }
    break;

    case VCPU_INSTR_TYPE_REGISTER_NUMBER:
    {
        uint8_t n = (instr & 63);
        MemRegion16 regRegion(getAddrByAddrMode_(VCPU_GET_REG(instr, 6), VCPU_ADDR_MODE_REGISTER, 2), this);

        if (!((vcpu_instr_register_number_callback*) instructions_[instr].callback)(regRegion, n, *this))
            status_ = VCPU_STATUS_INVALID_INSTRUCTION;
    }
    break;

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

    if (getPC() % sizeof (uint16_t) != 0)
    {
        status_ = VCPU_STATUS_INVALID_PC;
        getPC() = prevPC;
    }
    else if (status_ != VCPU_STATUS_OK)
        getPC() = prevPC;
}

bool Vcpu::isByteInstruction_(uint16_t instr)
{
    /*single or double operand instruction*/
    return (instr & (1 << 15)) && !(instr >= 0160000 && instr  <= 0167777 /* sub instruction */);
}

uint16_t Vcpu::getWordAtAddress(uint16_t addr)
{
    return bus_.get16(addr);
}

unsigned Vcpu::getAddrByAddrMode_(int r, int mode, uint16_t incrementSize)
{
    unsigned addr = 0;

    switch (mode)
    {
    case VCPU_ADDR_MODE_REGISTER:
        addr = VCPU_MEM_SIZE + r;
        break;

    case VCPU_ADDR_MODE_REGISTER_DEFFERED:
        addr = getRegister(r);
        break;

    case VCPU_ADDR_MODE_AUTOINCREMENT:
        addr = getRegister(r);
        getRegister(r) += ((r == VCPU_PC_REGISTER) ? sizeof (uint16_t) : incrementSize);
        break;

    case VCPU_ADDR_MODE_AUTOINCREMENT_DEFFERED:
        addr = getWordAtAddress(getRegister(r));
        getRegister(r) += sizeof (uint16_t); // it's correct even for byte instructions
        break;

    case VCPU_ADDR_MODE_AUTODECREMENT:
        getRegister(r) -= ((r == VCPU_PC_REGISTER) ? sizeof (uint16_t) : incrementSize);
        addr = getRegister(r);
        break;

    case VCPU_ADDR_MODE_AUTODECREMENT_DEFFERED:
        getRegister(r) -= sizeof (uint16_t); // it's correct even for byte instructions
        addr = getWordAtAddress(getRegister(r));
        break;

    case VCPU_ADDR_MODE_INDEX:
    {
        uint16_t pcIteration = ((r == VCPU_PC_REGISTER) ? sizeof (uint16_t) : 0);
        addr = uint16_t(getRegister(r) + pcIteration + getWordAtAddress(getPC()));
        getPC() += sizeof (uint16_t);
    }
        break;

    case VCPU_ADDR_MODE_INDEX_DEFERRED:
    {
        uint16_t pcIteration = ((r == VCPU_PC_REGISTER) ? sizeof (uint16_t) : 0);
        addr = getWordAtAddress(getRegister(r) + pcIteration + getWordAtAddress(getPC()));
        getPC() += sizeof (uint16_t);
    }
        break;

    default:
        abort();
        break;
    }

    return addr;
}

void Vcpu::onHalt_()
{
    haltHit_ = true;
    VcpuThreadState prevThreadState = threadState_;
    threadState_ = VCPU_THREAD_STATE_IDLE;
    if (prevThreadState != VCPU_THREAD_STATE_SINGLE_INSTRUCTION)
        executionStoppedCallback_();
}

bool Vcpu::onJmp_(int r, int mode)
{
    switch (mode)
    {
    case VCPU_ADDR_MODE_REGISTER:
        return false;

    case VCPU_ADDR_MODE_REGISTER_DEFFERED:
        getPC() = getRegister(r);
        break;

    case VCPU_ADDR_MODE_AUTOINCREMENT:
        getPC() = getRegister(r);
        getRegister(r) += sizeof (uint16_t);
        break;

    case VCPU_ADDR_MODE_AUTOINCREMENT_DEFFERED:
        getPC() = getWordAtAddress(getRegister(r));
        getRegister(r) += sizeof (uint16_t);
        break;

    case VCPU_ADDR_MODE_AUTODECREMENT:
        getPC() = getRegister(r);
        getRegister(r) -= sizeof (uint16_t);
        break;

    case VCPU_ADDR_MODE_AUTODECREMENT_DEFFERED:
        getPC() = getWordAtAddress(getRegister(r));
        getRegister(r) -= sizeof (uint16_t);
        break;

    case VCPU_ADDR_MODE_INDEX:
        getPC() = getRegister(r) + getWordAtAddress(getPC()) + sizeof (uint16_t);
        break;

    case VCPU_ADDR_MODE_INDEX_DEFERRED:
        getPC() = getWordAtAddress(getRegister(r) + getWordAtAddress(getPC()) + sizeof (uint16_t));
        break;

    default:
        abort();
        break;
    }

    return true;
}


Bus& Vcpu::getBus()
{
    return bus_;
}

bool Vcpu::interrupt(unsigned priority, uint16_t vectorAddress)
{
    if (priority <= (getPSW() >> 5))
        return false;

    getSP() -= sizeof (uint16_t);
    bus_.set16(getSP(), getPSW());
    getSP() -= sizeof (uint16_t);
    bus_.set16(getSP(), getPC());

    fprintf(stderr, "0%o\n", vectorAddress);
    fprintf(stderr, "0%o\n", getWordAtAddress(vectorAddress));

    getPC() = getWordAtAddress(vectorAddress);
    getPSW() = getWordAtAddress(vectorAddress + sizeof (uint16_t));

    return true;
}

Keyboard& Vcpu::getKeyboard()
{
    return keyboard_;
}
