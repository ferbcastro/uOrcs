#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "btb.hpp"  // 
#include "branch_predictor.hpp" // 

class processor_t {
   public:
    btb_t *btb;
    combined_predictor_t *predictor;
    uint64_t cycle, total_branches, num_wrong_predictions, num_misses;

    // =========================================================================
    // Methods.
    // =========================================================================
    processor_t();
    void allocate();
    void clock();
    void statistics();
};

#endif  // PROCESSOR_HPP
