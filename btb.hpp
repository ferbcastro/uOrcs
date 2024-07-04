#ifndef BTB_
#define BTB_

#include <cstdint>  // uint

class slot_t {
   public:
    bool is_empty;
    uint64_t last_access_cycle;
    uint64_t opcode_address;

    // =========================================================================
    /// Methods.
    // =========================================================================
    slot_t();
};

class set_t {
   public:
    slot_t *slots;

    // =========================================================================
    /// Methods.
    // =========================================================================
    set_t();
    void allocate_slots();
};

class btb_t {
   public:
    bool was_last_branch_hit;
    bool was_branch_cond;
    uint64_t last_branch_address;
    uint32_t last_branch_opcode_size;

    // =========================================================================
    /// Methods.
    // =========================================================================
    btb_t();
    ~btb_t();
    bool has_last_cond_branch_jumped(uint64_t current_instruction_address);
    bool is_branch_in_btb(uint64_t current_cycle, uint64_t op_address);
    void insert_in_btb(uint64_t current_cycle, uint64_t op_address);

   private:
    // Maps branch address to btb entry
    uint16_t return_entry(uint64_t instruction_address);
    set_t *sets; 
};

#endif