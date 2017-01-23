#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <cstdint>
#include <vector>
#include <cassert>

#include "instructions.h"
#include "memregion.h"
//we also need instruction structure/object to pass from vcpu to conveyor
struct instr_model
{
    int ticks_per_phase[5]; //how much ticks consumes each phase; may be set to 0 if respective conveyor phase is not applicable
    uint64_t instr_num = 0; //counter to track instruction order
    InstructionType type;   //number of operands and how we refer to them
    uint16_t instr;         //instruction word; is used to determine how much ticks it consumes on ALU
    unsigned dependencies_in[8], dependencies_out[8]; //addresses of memory dependencies of instruction; is used to determine if
                                                            //delayed advance will be in place
    int dependencies_in_num = 0, dependencies_out_num = 0;
    int curr_phase_advance; //when conveyor decides to advance, it seeks smallest ticks_per_phase - curr_phase_advance value; it is
                            //point of it's jump (if no delayed progress is in place). Also it's modified on conveyor advance.
    int alu_occupation = -1;//which alu instruction occupies in phase of computation; -1 if it doesn't occupy any now
    int ticks_to_fetch;
    int ticks_to_writeback;
    int conv_phase;
    bool has_advanced;
};

class conveyor
{
public:
    conveyor();
    uint64_t add_instruction(instr_model* instr); //is called from vcpu; takes instruction object with full information about
                                                  //instruction and includes it in conveyor model
    uint64_t get_ticks_with_conv();       //returns current number of elapsed ticks; is used to display progress in UI
    uint64_t get_ticks_without_conv();
private:
    uint64_t cur_ticks_;
    uint64_t ticks_without_conv_;
    int alu_num_;           //number of alu in system
    uint64_t instr_counter_;
    uint64_t instruction_cap_;  //maximum amount of instructions stuck on conveyor at the same time. At the beginning conveyor grabs a
                                //lot of instructions until it reaches this number. Then it only grabs an instruction when it has completed
                                //one
    uint64_t instruction_num_;  //current number of instrucions stuck on the conveyor
    void advance();             //searches instruction to advance it on conveyor and modifies conveyor model accordingly
    std::vector<instr_model*> conv_model_; //place where instruction models are stored
    bool is_memory_collision(int instr_ind);
};

#endif // CONVEYOR_H
