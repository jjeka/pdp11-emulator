#include "instructions.h"

InstructionInfo VCPU_INSTRUCTIONS[] = {
    { 0010000, 0017777, "mov"   , (void*) instr_mov , VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0110000, 0117777, "movb"  , (void*) instr_movb, VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0020000, 0027777, "cmp"   , (void*) instr_cmp , VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0120000, 0127777, "cmpb"  , (void*) instr_cmpb, VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0030000, 0037777, "bit"   , (void*) instr_bit , VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0130000, 0137777, "bitb"  , (void*) instr_bitb, VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0040000, 0047777, "bic"   , (void*) instr_bic , VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0140000, 0147777, "bicb"  , (void*) instr_bicb, VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0050000, 0057777, "bis"   , (void*) instr_bis , VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0150000, 0157777, "bisb"  , (void*) instr_bisb, VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0060000, 0067777, "add"   , (void*) instr_add , VCPU_INSTR_TYPE_DOUBLE_OPERAND },
    { 0160000, 0167777, "sub"   , (void*) instr_sub , VCPU_INSTR_TYPE_DOUBLE_OPERAND },

    /*{ 0070000, 0070777, "mul"   , NULL, VCPU_INSTR_TYPE_OPERAND_REGISTER },
    { 0071000, 0071777, "div"   , NULL, VCPU_INSTR_TYPE_OPERAND_REGISTER },
    { 0072000, 0072777, "ash"   , NULL, VCPU_INSTR_TYPE_OPERAND_REGISTER },
    { 0073000, 0073777, "ashc"  , NULL, VCPU_INSTR_TYPE_OPERAND_REGISTER },
    { 0074000, 0074777, "xor"   , NULL, VCPU_INSTR_TYPE_OPERAND_REGISTER },
    { 0075000, 0075777, "floating-point operations", NULL, VCPU_INSTR_TYPE_NOT_IMPLEMENTED },
    { 0076000, 0076777, "system instructions", NULL, VCPU_INSTR_TYPE_NOT_IMPLEMENTED },
    { 0077000, 0077777, "sob"   , NULL, VCPU_INSTR_TYPE_OPERAND_REGISTER },*/

    /*{ 0000300, 0000377, "swab", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0004000, 0004777, "jump to subroutine", NULL, VCPU_INSTR_TYPE_NOT_IMPLEMENTED },
    { 0104000, 0104777, "emulator trap", NULL, VCPU_INSTR_TYPE_NOT_IMPLEMENTED },
    { 0005000, 0005077, "clr", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0105000, 0105077, "clrb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0005100, 0005177, "com", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0105100, 0105177, "comb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0005200, 0005277, "inc", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0105200, 0105277, "incb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0005300, 0005377, "dec", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0105300, 0105377, "decb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0005400, 0005477, "neg", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0105400, 0105477, "negb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0005500, 0005577, "adc", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0105500, 0105577, "adcb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0005600, 0005677, "sbc", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0105600, 0105677, "sbcb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0005700, 0005777, "tst", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0105700, 0105777, "tstb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0006000, 0006077, "ror", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0106000, 0106077, "rorb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0006100, 0006177, "rol", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0106100, 0106177, "rolb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0006200, 0006277, "asr", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0106200, 0106277, "asrb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0006300, 0006377, "asl", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0106300, 0106377, "aslb", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0006400, 0006477, "mark", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0106400, 0106477, "mtps", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0006500, 0006577, "mfpi", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0106500, 0106577, "mfpd", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0006600, 0006677, "mtpi", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0106600, 0106677, "mtpd", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0006700, 0006777, "sxt", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },
    { 0106700, 0106777, "mfps", NULL, VCPU_INSTR_TYPE_SINGLE_OPERAND },

    { 0000000, 0000000 + 192, "system instruction", NULL, VCPU_INSTR_TYPE_NOT_IMPLEMENTED },
    { 0000400, 0000400 + 256, "br", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0001000, 0001000 + 256, "bne", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0001400, 0001400 + 256, "beq", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0002000, 0002000 + 256, "bge", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0002400, 0002400 + 256, "blt", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0003000, 0003000 + 256, "bgt", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0003400, 0003400 + 256, "ble", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0100000, 0100000 + 256, "bpl", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0100400, 0100400 + 256, "bmi", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0101000, 0101000 + 256, "bhi", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0101400, 0101400 + 256, "blos", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0102000, 0102000 + 256, "bvc", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0102400, 0102400 + 256, "bvs", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0103000, 0103000 + 256, "bcc or bhis", NULL, VCPU_INSTR_TYPE_BRANCH },
    { 0103400, 0103400 + 256, "bcs or blo", NULL, VCPU_INSTR_TYPE_BRANCH }*/

    // TODO: jmp, halt, wait, reset etc. + wtf last 2 instructions, check + system instructions collisions?
};

int VCPU_NUM_INSTRUCTIONS_IN_TABLE = sizeof (VCPU_INSTRUCTIONS) / sizeof(InstructionInfo);
