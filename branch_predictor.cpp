#include "branch_predictor.hpp"

const uint8_t kNumBitsGbhr = 15;
const uint16_t kSizeArrayGShare = 32768;
const uint16_t kSizeArrayBimodal = 32768;
const uint16_t kSizeArraySelector = 32768;

// =============================================================================
combined_predictor_t::combined_predictor_t() {
    uint16_t it;

    glob_branch_hist_reg = new char[kNumBitsGbhr];
    gshare_counter_table = new counter[kSizeArrayGShare];
    bimodal_counter_table = new counter[kSizeArrayBimodal];
    selector_counter_table = new counter[kSizeArraySelector];

    for (it = 0; it < kNumBitsGbhr; it++)
        glob_branch_hist_reg[it] = 0;
    // Initializes branches as taken
    // Bimodal predictor is selected
    for (it = 0; it < kSizeArraySelector; it++) {
        bimodal_counter_table[it].LSB = 1;
        bimodal_counter_table[it].MSB = 1;
        gshare_counter_table[it].LSB = 1;
        gshare_counter_table[it].MSB = 1;
        selector_counter_table[it].LSB = 0;
        selector_counter_table[it].MSB = 0;
    }
}

// =============================================================================
combined_predictor_t::~combined_predictor_t() {
    delete[] glob_branch_hist_reg;
    delete[] gshare_counter_table;
    delete[] bimodal_counter_table;
    delete[] selector_counter_table;
}

// =============================================================================
void combined_predictor_t::choose_predictor(uint64_t op_address) {
    uint16_t entry = return_entry(op_address);

    switch (selector_counter_table[entry].MSB) {
      case 0:
        used_predictor = BIMODAL;
        break;
      case 1:
        used_predictor = GSHARE;
        break;
      default:
        break;
    }
}

// =============================================================================
void combined_predictor_t::make_prediction(uint64_t op_address) {
    uint16_t entry = return_entry(op_address);

    result_bimodal = bimodal_predictor(entry);
    result_gshare = gshare_preditor(entry);
}

// =============================================================================
bool combined_predictor_t::gshare_preditor(uint16_t entry) {
    uint16_t gbhr(0);

    // Transforms gbhr, stored in string form, to a 16-bit integer
    for (uint8_t it = 0; it < kNumBitsGbhr; it++) 
        gbhr |= (glob_branch_hist_reg[it] << it);
    entry ^= gbhr;

    return gshare_counter_table[entry].MSB;
}

// =============================================================================
bool combined_predictor_t::bimodal_predictor(uint16_t entry) {
    return gshare_counter_table[entry].MSB;
}

// =============================================================================
// 00 -> 01, 01 -> 10, 10 -> 11, 11 -> No action
void combined_predictor_t::increment_predictor(type_predictor_t used_predictor, 
                                               uint64_t op_address) {
    uint16_t gbhr(0);
    uint16_t entry = return_entry(op_address);
    
    switch (used_predictor) {
      case GSHARE:
        for (uint8_t it = 0; it < kNumBitsGbhr; it++) 
            gbhr |= (glob_branch_hist_reg[it] << it); 
        entry ^= gbhr;
        if (gshare_counter_table[entry].MSB == 0) {
            if (gshare_counter_table[entry].LSB == 0) {
                gshare_counter_table[entry].LSB = 1;
            } else {
                gshare_counter_table[entry].MSB = 1;
                gshare_counter_table[entry].LSB = 0;
            }
        } else {
            if (gshare_counter_table[entry].LSB == 0) {
                gshare_counter_table[entry].LSB = 1;
            }
        }
        break;
      case BIMODAL:
        if (bimodal_counter_table[entry].MSB == 0) {
            if (bimodal_counter_table[entry].LSB == 0) {
                bimodal_counter_table[entry].LSB = 1;
            } else {
                bimodal_counter_table[entry].MSB = 1;
                bimodal_counter_table[entry].LSB = 0;
            }
        } else {
            if (bimodal_counter_table[entry].LSB == 0) {
                bimodal_counter_table[entry].LSB = 1;
            }
        }
        break;
      default:
        break;
    }
}

// =============================================================================
// 11 -> 10, 10 -> 01, 01 -> 00, 00 -> No action
void combined_predictor_t::decrement_predictor(type_predictor_t used_predictor, 
                                               uint64_t op_address) {
    uint16_t gbhr(0);
    uint16_t entry = return_entry(op_address);

    switch (used_predictor) {
      case GSHARE:
        for (uint8_t it = 0; it < kNumBitsGbhr; it++) 
            gbhr |= (glob_branch_hist_reg[it] << it); 
        entry ^= gbhr;

        if (gshare_counter_table[entry].MSB == 1) {
            if (gshare_counter_table[entry].LSB == 0) {
                gshare_counter_table[entry].MSB = 0;
                gshare_counter_table[entry].LSB = 1;
            } else {
                gshare_counter_table[entry].LSB = 0;
            }
        } else {
            if (gshare_counter_table[entry].LSB == 1) {
                gshare_counter_table[entry].LSB = 0;
            }
        }
        break;
      case BIMODAL:
        if (bimodal_counter_table[entry].MSB == 1) {
            if (bimodal_counter_table[entry].LSB == 0) {
                bimodal_counter_table[entry].MSB = 0;
                bimodal_counter_table[entry].LSB = 1;
            } else {
                bimodal_counter_table[entry].LSB = 0;
            }
        } else {
            if (bimodal_counter_table[entry].LSB == 1) {
                bimodal_counter_table[entry].LSB = 0;
            }
        }
        break;
      default:
        break;
    }
}

// =============================================================================
// Same logic implemented in increment_predictor
void combined_predictor_t::increment_selector(uint64_t op_address) {
    uint16_t entry = return_entry(op_address);

    if (selector_counter_table[entry].MSB == 0) {
        if (selector_counter_table[entry].LSB == 0) {
            selector_counter_table[entry].LSB = 1;
        } else {
            selector_counter_table[entry].LSB = 0;
            selector_counter_table[entry].MSB = 1;
        }
    } else {
        if (selector_counter_table[entry].LSB == 0) {
            selector_counter_table[entry].LSB = 1;
        }
    }
}

// =============================================================================
// Same logic implemented in decrement_selector
void combined_predictor_t::decrement_selector(uint64_t op_address) {
    uint16_t entry = return_entry(op_address);

    if (selector_counter_table[entry].MSB == 1) {
        if (selector_counter_table[entry].LSB == 0) {
            selector_counter_table[entry].MSB = 0;
            selector_counter_table[entry].LSB = 1;
        } else {
            selector_counter_table[entry].LSB = 0;
        }
    } else {
        if (selector_counter_table[entry].LSB == 1) {
            selector_counter_table[entry].LSB = 0;
        }
    }
}

// =============================================================================
// gbhr is shifted to the left and lsb receives branch status
void combined_predictor_t::update_gbhr(char has_jumped) {
    for (uint8_t it = kNumBitsGbhr - 1; it > 0; it--) 
        glob_branch_hist_reg[it] = glob_branch_hist_reg[it - 1]; 
    
    glob_branch_hist_reg[0] = has_jumped;
}

// =============================================================================
// 15 least significant bits form the entry
uint16_t combined_predictor_t::return_entry(uint64_t op_address) {
    return static_cast<uint16_t>(op_address & 0x7fff);
}