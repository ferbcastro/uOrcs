#include "btb.hpp"

const uint16_t kNumEntries = 1024;
const uint8_t kNumSlots = 12;

// =============================================================================
btb_t::btb_t() : was_branch_cond(0) {
    sets = new set_t[kNumEntries];
}

// =============================================================================
btb_t::~btb_t() {
    for (uint16_t it = 0; it < kNumEntries; it++) {
        if (sets[it].slots != nullptr)
            delete sets[it].slots;
    }

    delete[] sets;
}

// =============================================================================
bool btb_t::has_last_cond_branch_jumped(uint64_t current_instruction_address) {
    uint64_t difference;

    difference = current_instruction_address - last_branch_address;
    
    return (difference > last_branch_opcode_size);
}

// =============================================================================
uint16_t btb_t::return_entry(uint64_t instruction_address) {
    return static_cast<uint16_t>(instruction_address & 0x3ff);
} 

// =============================================================================
bool btb_t::is_branch_in_btb(uint64_t current_cycle, uint64_t op_address) {
    uint16_t entry = return_entry(op_address);

    if (sets[entry].slots == nullptr) return false;

    // If entry is in btb, variable last_access_cycle is updated
    for (uint8_t it = 0; it < kNumSlots; it++) {
        if (sets[entry].slots->opcode_address == op_address) {
            sets[entry].slots->last_access_cycle = current_cycle;
            return true;
        }
    }

    return false;
}

// =============================================================================
// Be sure that the brach is not already in btb
void btb_t::insert_in_btb(uint64_t current_cycle, uint64_t op_address) {
    uint64_t temp;
    uint8_t it, least_used_slot;
    uint16_t entry = return_entry(op_address);
    set_t& selected_set = sets[entry];

    if (selected_set.slots == nullptr) {
        selected_set.allocate_slots();
        it = 0;
    } else {
        least_used_slot = 0;
        for (it = 0; it < kNumSlots; it++) {
            if (selected_set.slots[it].is_empty == true) break;
            temp = selected_set.slots[least_used_slot].last_access_cycle;
            if (selected_set.slots[it].last_access_cycle < temp) 
                least_used_slot = it;
        }
        if (it == kNumSlots) it = least_used_slot;
    }
    
    selected_set.slots[it].is_empty = false;
    selected_set.slots[it].last_access_cycle = current_cycle;
    selected_set.slots[it].opcode_address = op_address; 
}

// =============================================================================
set_t::set_t() : slots(nullptr) {}

// =============================================================================
void set_t::allocate_slots() {
    slots = new slot_t[kNumSlots];
}

// =============================================================================
slot_t::slot_t() : is_empty(true) {};