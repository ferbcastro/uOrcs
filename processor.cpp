#include "processor.hpp"  // allocate, clock, processor_t, statistics

// If you use clangd, it'll give you a warning on this include, but it's used by
// ORCS_PRINTF. Try removing it to see what happens.
// ~ Gabriel
#include <cstdio>

#include "opcode_package.hpp"  // opcode_package_t
#include "orcs_engine.hpp"     // orcs_engine_t
#include "simulator.hpp"       // ORCS_PRINTF
#include "trace_reader.hpp"

const uint16_t kUncondBranchPenalty = 11;
const uint16_t kCondBranchPenalty = 511;

// =============================================================================
processor_t::processor_t() : cycle(0), total_branches(0), num_wrong_predictions(0) {};

// =============================================================================
void processor_t::allocate(){};

// =============================================================================
void processor_t::clock() {
    uint64_t op_address;
    bool gshare_c, bimodal_c, has_jumped;

    // Get the next instruction from the trace.
    opcode_package_t new_instruction;
    if (!orcs_engine.trace_reader->trace_fetch(&new_instruction)) {
        // If EOF.
        orcs_engine.simulator_alive = false;
        return;
    }

    if (btb.was_branch_cond == true) {
        op_address = new_instruction.opcode_address;
        has_jumped = btb.has_last_cond_branch_jumped(op_address);
        gshare_c = !(has_jumped ^ predictor.result_gshare);
        bimodal_c = !(has_jumped ^ predictor.result_bimodal);
        
        // Btb hit and wrong prediction -> += 512 cycles
        if (btb.was_last_branch_hit == true) {
            switch (predictor.used_predictor) {
              case GSHARE:
                if (!gshare_c) {
                    cycle += kCondBranchPenalty;
                    num_wrong_predictions++;
                }
                break;
              case BIMODAL:
                if (!bimodal_c) {
                    cycle += kCondBranchPenalty;
                    num_wrong_predictions++;
                }
                break;
              default:
                break;
            }
        // Btb miss and conditional branch jumped -> += 512 cycles
        } else if (has_jumped) {
            cycle += kCondBranchPenalty; 
        }

        op_address = btb.last_branch_address;
        if (has_jumped)
            predictor.increment_predictor(predictor.used_predictor, op_address);
        else   
            predictor.decrement_predictor(predictor.used_predictor, op_address);

        // Selector favors gshare          
        if (gshare_c && !bimodal_c) 
            predictor.increment_selector(op_address);
        // Selector favors bimodal
        if (!gshare_c && bimodal_c)
            predictor.decrement_selector(op_address);
        
        predictor.update_gbhr(has_jumped == true ? 1 : 0);
    }

    if (new_instruction.opcode_operation == INSTRUCTION_OPERATION_BRANCH) {
        total_branches++;
        op_address = new_instruction.opcode_address;
        if (btb.is_branch_in_btb(cycle, op_address) == true) {
            btb.was_last_branch_hit = 1; 
        } else {
            btb.insert_in_btb (cycle, op_address);
            btb.was_last_branch_hit = 0;
            // Btb miss and unconditional jump -> += 12 cycles
            if (new_instruction.branch_type == BRANCH_UNCOND) {
                cycle += kUncondBranchPenalty;
            }
        }

        if (new_instruction.branch_type == BRANCH_COND) {
            predictor.choose_predictor(op_address);
            predictor.make_prediction(op_address);
            btb.was_branch_cond = true;
            btb.last_branch_address = new_instruction.opcode_address;
            btb.last_branch_opcode_size = new_instruction.opcode_size;
        } else {
            btb.was_branch_cond = false;
        }
    } else {
        btb.was_branch_cond = false;
    }

    cycle++;
};

// =============================================================================
void processor_t::statistics() {
    ORCS_PRINTF("######################################################\n");
    ORCS_PRINTF("cycles:%lu\n", cycle);
    ORCS_PRINTF("total of branches:%lu\n", total_branches);
    ORCS_PRINTF("total of wrong predictions:%lu\n", num_wrong_predictions);
    ORCS_PRINTF("processor_t\n");
    ORCS_PRINTF("######################################################\n");
};