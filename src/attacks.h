#pragma once


#include <stdint.h>
#include "global.h"


extern U64 pawn_attacks[2][64];
extern U64 knight_attacks[64];
extern U64 king_attacks[64];
extern U64 bishop_masks[64];
extern U64 rook_masks[64];
extern U64 bishop_attacks[64][512];
extern U64 rook_attacks[64][4096];

const U64 not_a_file = 18374403900871474942ULL;

const U64 not_h_file = 9187201950435737471ULL;

const U64 not_hg_file = 4557430888798830399ULL;

const U64 not_ab_file = 18229723555195321596ULL;

// Count of bits for each position in a bishop attack ray
extern const int bishop_relevant_bits[64];


// Count of bits for each position in a rook attack ray
extern const int rook_relevant_bits[64];

extern U64 rook_magic_numbers[64];
extern U64 bishop_magic_numbers[64];

// Function prototypes
U64 mask_pawn_attacks(int side, int square);
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);
U64 mask_bishop_attacks(int square);
U64 gen_bishop_attacks(int square, U64 block);
U64 mask_rook_attacks(int square);
U64 gen_rook_attacks(int square, U64 block);
void init_leaper_attacks();
void init_sliders_attacks(int bishop);
U64 get_bishop_attacks(int square, U64 occupancy);
U64 get_rook_attacks(int square, U64 occupancy);
U64 get_queen_attacks(int square, U64 occupancy);
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);

int is_square_attacked(int square, int side);



