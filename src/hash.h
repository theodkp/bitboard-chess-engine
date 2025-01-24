#pragma once


#include "random.h"
#include "bitboard.h"


// random piece keys [piece][square]
extern U64 piece_keys[12][64];

// random enpassant keys [square]
extern U64 enpassant_keys[64];

extern U64 castle_keys[16];

extern U64 side_key;


// transposition tables ****************************************

// hash table size
#define hash_size 800000
#define no_hash_entry 100000


// hash flag
#define hash_flag_exact 0
#define hash_flag_alpha 1
#define hash_flag_beta 2

typedef struct {
    U64 hash_key;   // hash key
    int depth;      // search depth
    int flag;        
    int score;      
} tt;              

extern tt hash_table[hash_size];


void init_random_keys(void);
U64 generate_hash_key(void);
void clear_hash_table(void);
void enable_pv_scoring(moves *move_list);
int read_hash_entry(int alpha, int beta, int depth);
void write_hash_entry(int score, int depth, int hash_flag);