#include "vcpu.h"

Vcpu::Vcpu(std::string romFile) :
    registerNames_ { "R1", "R2", "R3", "R4", "R5", "R6", "SP", "PC" }
{
    romFile;

    for (Instruction& instr : instructions_)
    {
        instr.callback = NULL;
        instr.name = NULL;
        instr.type = TYPE_NOT_INITIALIZED;
    }
    numInstructionNames_ = 0;

    addInstruction_(0010000, 0017777, "mov", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0110000, 0117777, "movb", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0020000, 0027777, "cmp", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0120000, 0127777, "cmpb", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0030000, 0037777, "bit", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0130000, 0137777, "bitb", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0040000, 0047777, "bic", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0140000, 0147777, "bicb", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0050000, 0057777, "bis", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0150000, 0157777, "bisb", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0060000, 0067777, "add", NULL, TYPE_DOUBLE_OPERAND);
    addInstruction_(0160000, 0167777, "sub", NULL, TYPE_DOUBLE_OPERAND);

    addInstruction_(0070000, 0070777, "mul", NULL, TYPE_DOUBLE_OPERAND_REG);
    addInstruction_(0071000, 0071777, "div", NULL, TYPE_DOUBLE_OPERAND_REG);
    addInstruction_(0072000, 0072777, "ash", NULL, TYPE_DOUBLE_OPERAND_REG);
    addInstruction_(0073000, 0073777, "ashc", NULL, TYPE_DOUBLE_OPERAND_REG);
    addInstruction_(0074000, 0074777, "xor", NULL, TYPE_DOUBLE_OPERAND_REG);
    addInstruction_(0075000, 0075777, "floating-point operations", NULL, TYPE_NOT_IMPLEMENTED);
    addInstruction_(0076000, 0076777, "system instructions", NULL, TYPE_NOT_IMPLEMENTED);
    addInstruction_(0077000, 0077777, "sob", NULL, TYPE_DOUBLE_OPERAND_REG);

    addInstruction_(0000300, 0000377, "swab", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0004000, 0004777, "jump to subroutine", NULL, TYPE_NOT_IMPLEMENTED);
    addInstruction_(0104000, 0104777, "emulator trap", NULL, TYPE_NOT_IMPLEMENTED);
    addInstruction_(0005000, 0005077, "clr", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0105000, 0105077, "clrb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0005100, 0005177, "com", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0105100, 0105177, "comb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0005200, 0005277, "inc", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0105200, 0105277, "incb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0005300, 0005377, "dec", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0105300, 0105377, "decb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0005400, 0005477, "neg", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0105400, 0105477, "negb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0005500, 0005577, "adc", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0105500, 0105577, "adcb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0005600, 0005677, "sbc", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0105600, 0105677, "sbcb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0005700, 0005777, "tst", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0105700, 0105777, "tstb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0006000, 0006077, "ror", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0106000, 0106077, "rorb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0006100, 0006177, "rol", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0106100, 0106177, "rolb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0006200, 0006277, "asr", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0106200, 0106277, "asrb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0006300, 0006377, "asl", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0106300, 0106377, "aslb", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0006400, 0006477, "mark", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0106400, 0106477, "mtps", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0006500, 0006577, "mfpi", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0106500, 0106577, "mfpd", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0006600, 0006677, "mtpi", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0106600, 0106677, "mtpd", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0006700, 0006777, "sxt", NULL, TYPE_SINGLE_OPERAND);
    addInstruction_(0106700, 0106777, "mfps", NULL, TYPE_SINGLE_OPERAND);

    addInstruction_(0000000, 0000000 + 192, "system instruction", NULL, TYPE_NOT_IMPLEMENTED); // TODO: wtf?
    addInstruction_(0000400, 0000400 + 256, "br", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0001000, 0001000 + 256, "bne", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0001400, 0001400 + 256, "beq", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0002000, 0002000 + 256, "bge", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0002400, 0002400 + 256, "blt", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0003000, 0003000 + 256, "bgt", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0003400, 0003400 + 256, "ble", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0100000, 0100000 + 256, "bpl", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0100400, 0100400 + 256, "bmi", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0101000, 0101000 + 256, "bhi", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0101400, 0101400 + 256, "blos", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0102000, 0102000 + 256, "bvc", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0102400, 0102400 + 256, "bvs", NULL, TYPE_CONDITIONAL_BRANCH);
    addInstruction_(0103000, 0103000 + 256, "bcc or bhis", NULL, TYPE_CONDITIONAL_BRANCH); // TODO: ??
    addInstruction_(0103400, 0103400 + 256, "bcs or blo", NULL, TYPE_CONDITIONAL_BRANCH); // TODO: ??

    // TODO: add check initialized

    memset(memory_, 0, VCPU_BUFFER_SIZE * sizeof (uint8_t));

    srand(time(0)); // TODO: remove
    for (int i = 0; i < VCPU_BUFFER_SIZE; i++)
    {
        memory_[i] = rand() % 256;
    }
}

Vcpu::~Vcpu()
{
}

 void Vcpu::addInstruction_(uint16_t begin, uint16_t end, std::string name, void* callback, InstructionType type)
 {
     //assert(callback); // TODO: !
     assert(type != TYPE_NOT_INITIALIZED);
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
         assert(instructions_[i].type == TYPE_NOT_INITIALIZED);

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
	return getRegister(VCPU_SP_REGISTER);
}

uint16_t& Vcpu::getSP()
{
	return getRegister(VCPU_PC_REGISTER);
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

unsigned getMemSize()
{
    return VCPU_MEM_SIZE;
}

std::string Vcpu::instrAtAddress(uint16_t address)
{
    uint16_t instr = *((uint16_t*) &memory_[address]);

    char name[VCPU_MAX_INSTRUCTION_NAME] = "";

    switch (instructions_[instr].type)
    {
    case TYPE_NOT_INITIALIZED:
        sprintf(name, "[not initialized: 0x%X]", instr);
        break;

    case TYPE_NOT_IMPLEMENTED:
        sprintf(name, "[not implemented: 0x%X]", instr);
        break;

    case TYPE_DOUBLE_OPERAND:
    {
        std::string r1 = getOperand_(instr, 0);
        std::string r2 = getOperand_(instr, 6);
        if (r1.empty() || r2.empty())
            sprintf(name, "[invalid %s instruction operand: 0x%X]", instructions_[instr].name->c_str(), instr);
        else
            sprintf(name, "%s %s %s", instructions_[instr].name->c_str(), r1.c_str(), r2.c_str());
    }
        break;

    case TYPE_DOUBLE_OPERAND_REG:
    {
        std::string r1 = getOperand_(instr, 0);
        std::string r2 = getRegister_(instr, 6);
        if (r1.empty() || r2.empty())
            sprintf(name, "[invalid %s instruction operand: 0x%X]", instructions_[instr].name->c_str(), instr);
        else
            sprintf(name, "%s %s %s", instructions_[instr].name->c_str(), r2.c_str(), r1.c_str());
    }
        break;

    case TYPE_SINGLE_OPERAND:
    {
        std::string r = getOperand_(instr, 0);
        if (r.empty())
            sprintf(name, "[invalid %s instruction operand: 0x%X]", instructions_[instr].name->c_str(), instr);
        else
            sprintf(name, "%s %s", instructions_[instr].name->c_str(), r.c_str());
    }
        break;

    case TYPE_CONDITIONAL_BRANCH:
    {
            sprintf(name, "%s %X", instructions_[instr].name->c_str(), instr & 256);
    }
        break;

    default:
        abort();
        break;
    }
    return name;
}

std::string Vcpu::getRegister_(uint16_t instr, int begin)
{
    int r = (instr >> begin) & 7;
    return registerNames_[r];
}

std::string Vcpu::getOperand_(uint16_t instr, int begin)
{
    int r = (instr >> begin) & 7;
    int mode = (instr >> (begin + 3)) & 7;

    char result[VCPU_MAX_INSTRUCTION_NAME] = "";
    if (r != VCPU_PC_REGISTER)
    {
        const char* registerName = registerNames_[r].c_str();

        switch (mode)
        {
        case 0: // TODO: constants
            sprintf(result, "%s", registerName);
            break;

        case 1:
            sprintf(result, "(%s)", registerName);
            break;

        case 2:
            sprintf(result, "(%s)+", registerName);
            break;

        case 3:
            sprintf(result, "@(%s)+", registerName);
            break;

        case 4:
            sprintf(result, "-(%s)", registerName);
            break;

        case 5:
            sprintf(result, "@-(%s)", registerName);
            break;

        case 6:
            sprintf(result, "X(%s)", registerName);
            break;

        case 7:
            sprintf(result, "@X(%s)", registerName);
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
        case 2:
            strcpy(result, "#n");
            break;

        case 3:
            strcpy(result, "@#n");
            break;

        case 6:
            strcpy(result, "a");
            break;

        case 7:
            strcpy(result, "@a");
            break;

        default:
            // else error (empty result)
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

void Vcpu::pause()
{
}

void Vcpu::step()
{
}

void Vcpu::addBreakpoint(uint16_t address)
{
    address;
}
