#include "conveyor.h"

conveyor::conveyor()
{
    ticks_without_conv_ = 0;
    cur_ticks_ = 0;
    instr_counter_ = 0;
    instruction_cap_ = 10;
    alu_num_ = 3;
}

uint64_t conveyor::add_instruction(instr_model *instr)
{
    /**free(instr);
    return 0;*/
    //print_state();
    instr->instr_num = instr_counter_;
    instr->conv_phase = 0;
    instr_counter_++;
    bool can_insert_instr = false; //we can insert new instruction if every instruction on conveyor has passed first phase; otherwise
                                   //we have to make advancements until no instruction is in first phase and we have less than cap number
                                   //of instructions
    for (int i = 0; i < 5; i++)
        ticks_without_conv_ += instr->ticks_per_phase[i];

    while (!can_insert_instr)
    {
        can_insert_instr = true;
        for (int i = 0; i < (int) conv_model_.size(); i++)
            if (conv_model_[i]->conv_phase == 0)
            {
                can_insert_instr = false;
                break;
            }
        if (can_insert_instr && (conv_model_.size() < instruction_cap_))
            break;
        advance();
    }
    conv_model_.push_back(instr);
    return instr_counter_;
}

void conveyor::advance()
{

    //print_state();
    if (conv_model_.empty())
        return;

    instr_model * hyp_instr = conv_model_[0]; //hypothetical instruction to advance; we try with instruction awaiting it's fetch

    int alu_occupied = 0;
    bool bus_occupied = false;

    for (int i = 0; i < (int) conv_model_.size(); i++)
        if (conv_model_[i]->conv_phase == 3 && conv_model_[i]->curr_phase_advance != 0)
            alu_occupied++;

    for (int i = 0; i < (int) conv_model_.size(); i++)
    {//looking for a closer jump point

        if ((conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase] - conv_model_[i]->curr_phase_advance) <
            (hyp_instr->ticks_per_phase[hyp_instr->conv_phase] - hyp_instr->curr_phase_advance))
        {//jump point is closer in time; gotta check memory collisions and that there is a free ALU if we want to start to calculate
            bool no_collision = true;

            if (conv_model_[i]->conv_phase == 3 && conv_model_[i]->curr_phase_advance == 0 && alu_occupied >= alu_num_)
                no_collision = false;

            if (!is_memory_collision(i) && no_collision)
            {//so, our jump point is closer and no collisions are present and there is free ALU if we need one
                hyp_instr = conv_model_[i];
            }
        }
    }
    //so, we found closest jump point - closest event in our conveyor model. It's time to advance our conveyor accordingly
    //our instruction is first to switch to the next phase; therefore, all other instruction will advance inside their phases
    //(considering bus and ALU occupation)

    int advance_ticks = hyp_instr->ticks_per_phase[hyp_instr->conv_phase] - hyp_instr->curr_phase_advance;
    //number of ticks by which conveyor advances

    cur_ticks_ += (uint64_t) advance_ticks; //tick advancement

    for (int i = 0; i < (int)conv_model_.size(); i++)
        conv_model_[i]->has_advanced = false; //no instruction has advanced yet


    for (int i = 0; i < (int)conv_model_.size(); i++)
    {//we look at our instructions and try to advance their execution by advance_ticks;  first we try unfinished instructions

        if (conv_model_[i]->curr_phase_advance == 0) //if execution of phase has not began, we advance it in next iteration
            continue;
        if (is_memory_collision(i))
        {
            conv_model_[i]->has_advanced = true;
            continue;
        }
        //first we occupy bus if necessary and if possible
        if (conv_model_[i]->conv_phase == 0 || conv_model_[i]->conv_phase == 2 || conv_model_[i]->conv_phase == 4)
        {
            if (!bus_occupied)
                bus_occupied = true; //if we have to use bus and its free - occupy it
            else
            {//if bus is occupied - we can't advance
                conv_model_[i]->has_advanced = true;
                continue;
            }
        }

        //occupy one ALU if necessary
        if (conv_model_[i]->conv_phase == 3)
        {
            if (alu_occupied < alu_num_)
                alu_occupied ++; //if we have to use ALU and one is free - occupy it
            else
            {//no free ALU - we can't advance
                conv_model_[i]->has_advanced = true;
                continue;
            }
        }

        conv_model_[i]->curr_phase_advance += advance_ticks;

        if (conv_model_[i]->curr_phase_advance >= conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase])
        {//if current phase is finished - transfer to the next one
            conv_model_[i]->conv_phase++;
            conv_model_[i]->curr_phase_advance = 0;
            while(conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase] == 0)
                conv_model_[i]->conv_phase++;

            if (conv_model_[i]->conv_phase > 4)
            {//we advanced past last phase - pop instruction from conveyor and free occupied by support structures memory
                free(conv_model_[i]);
                conv_model_.erase(conv_model_.begin() + i);
            }
        }
        conv_model_[i]->has_advanced = true;
    }

    for (int i = 0; i < (int)conv_model_.size(); i++)
    {

        if (conv_model_[i]->has_advanced)
            continue;
        if (is_memory_collision(i))
        {
            conv_model_[i]->has_advanced = true;
            continue;
        }
        if (conv_model_[i]->conv_phase == 0 || conv_model_[i]->conv_phase == 2 || conv_model_[i]->conv_phase == 4)
        {
            if (!bus_occupied)
                bus_occupied = true; //if we have to use bus and its free - occupy it
            else
            {//if bus is occupied - we can't advance
                conv_model_[i]->has_advanced = true;
                continue;
            }
        }
         //occupy one ALU if necessary
        if (conv_model_[i]->conv_phase == 3)
        {
            if (alu_occupied < alu_num_)
                alu_occupied ++; //if we have to use ALU and one is free - occupy it
            else
            {//no free ALU - we can't advance
                conv_model_[i]->has_advanced = true;
                continue;
            }
        }
        conv_model_[i]->curr_phase_advance += advance_ticks;

        if (conv_model_[i]->curr_phase_advance >= conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase])
        {
        //if current phase is finished - transfer to the next one
            conv_model_[i]->conv_phase++;
            conv_model_[i]->curr_phase_advance = 0;
            while(conv_model_[i]->ticks_per_phase[conv_model_[i]->conv_phase] == 0)
                conv_model_[i]->conv_phase++;



            if (conv_model_[i]->conv_phase > 4)
            {//we advanced past last phase - pop instruction from conveyor and free occupied by support structures memory
                free(conv_model_[i]);
                conv_model_.erase(conv_model_.begin() + i);
            }
        }
        conv_model_[i]->has_advanced = true;
    }
    //print_state();
}

bool conveyor::is_memory_collision(int instr_ind)
{
    bool no_collision = true;

    if (conv_model_[instr_ind]->conv_phase == 4)
    {// collision is possible only on write-back

        for (int j = 0; j < (int) conv_model_.size(); j++)
        {

            if (conv_model_[j]->instr_num >= conv_model_[instr_ind]->instr_num)
                continue; //instruction happens after our chosen; no collision
            //collision: we try to write into area from which previous instruction reads; lost causality

            for (int ii = 0; ii < (int) conv_model_[instr_ind]->dependencies_out_num; ii++)
            {

                for (int jj = 0; jj < (int) conv_model_[j]->dependencies_in_num; jj++)
                {

                    if (conv_model_[instr_ind]->dependencies_out[ii] == conv_model_[j]->dependencies_in[jj])
                    {// yup, that's a collision
                        no_collision = false;
                        break;
                    }
                }

                if (!no_collision)
                    break;
            }
            if (!no_collision)
                break;
        }
    }
    return !no_collision;
}

uint64_t conveyor::get_ticks_with_conv()
{
    return cur_ticks_;
}
uint64_t conveyor::get_ticks_without_conv()
{
    return ticks_without_conv_;
}

void conveyor::print_state()
{
    printf("State: %" PRId64" %" PRId64"\n", cur_ticks_, ticks_without_conv_);
    for (int i = 0; i < 5; i++)
    {
        printf("\t%d:", i+1);
        for (int j = 0; j < conv_model_.size(); j++)
            if (conv_model_[j]->conv_phase == i)
                printf(" %d(%d out of %d)", conv_model_[j]->instr_num, conv_model_[j]->curr_phase_advance, conv_model_[j]->ticks_per_phase[i]);
        printf("\n");
    }
}
