#ifndef BRANCH_PREDICTOR_
#define BRANCH_PREDICTOR_

#include <cstdint>

struct counter {
    char LSB;
    char MSB;
};

enum type_predictor_t {
    GSHARE = 0,
    BIMODAL
};

class combined_predictor_t {
   public:
    type_predictor_t used_predictor;
    bool result_gshare;
    bool result_bimodal;
    
    // =========================================================================
    /// Methods.
    // =========================================================================
    combined_predictor_t();
    ~combined_predictor_t();
    void choose_predictor(uint64_t op_address);
    void make_prediction(uint64_t op_address);
    void increment_predictor(type_predictor_t used_predictor, uint64_t op_address); // no increment if 3
    void decrement_predictor(type_predictor_t used_predictor, uint64_t op_address); // no decrement if 0
    void increment_selector(uint64_t op_address);
    void decrement_selector(uint64_t op_address);
    void update_gbhr(char has_jumped);

   private:
    char *glob_branch_hist_reg;
    counter *gshare_counter_table;
    counter *bimodal_counter_table;
    counter *selector_counter_table; 

    // =========================================================================
    /// Methods.
    // =========================================================================
    uint16_t return_entry(uint64_t op_address);
    bool gshare_preditor(uint16_t entry);
    bool bimodal_predictor(uint16_t entry);
};

#endif