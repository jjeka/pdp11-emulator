#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <cstdint>
#include <vector>

#include "instructions.h"
#include "memregion.h"
//we also need instruction structure/object to pass from vcpu to conveyor
struct instr_model
{
    int ticks_per_phase[5]; //how much ticks consumes each phase; may be set to 0 if respective conveyor phase is not applicable
    uint64_t instr_num = 0; //counter to track instruction order
    InstructionType type;   //number of operands and how we refer to them
    std::vector<unsigned> dependecies_in, dependencies_out; //memory dependency of instruction; is used to determine if delayed advance
                                                            //will be in place
    int curr_phase_advance; //when conveyor decides to advance, it seeks smallest ticks_per_phase - curr_phase_advance value; it is
                            //point of it's jump (if no delayed progress is in place)
};

class conveyor
{
public:
    conveyor();
    uint64_t add_instruction(instr_model* instr); //is called from vcpu; takes instruction object with full information about
                                                  //instruction and includes it in conveyor model
    uint64_t get_ticks();       //returns current number of elapsed ticks; is used to display progress in UI
private:
    uint64_t cur_ticks;
    uint64_t instr_counter;
    uint64_t lane_count;
    void advance();             //searches instruction to advance it on conveyor and modifies conveyor model accordingly
    std::vector<instr_model*> conv_model[5];                 //place where instruction models are stored, sorted by their conveyor phase
};

#endif // CONVEYOR_H
