#pragma once

#include <global.h>

#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))


// non-zero binary string if bit set otherwise zero
inline bool get_bit(U64 bitboard, int square) {
    return (bitboard) & (1ULL << square);
}


// unset bit, will not flip
inline void unset_bit(U64& bitboard, int square){
    (bitboard) &= ~(1ULL << square);
}

// Bit MANIPULATIONS


// Count bits
static inline int count_bits(U64 bitboard){

    return __builtin_popcountll(bitboard);
}


// Get the index of our least significant set bit
static inline int get_lsf_bit_index(U64 bitboard){
    if (bitboard){
        return count_bits((bitboard & -bitboard)-1);
    }
    else{
        return -1;
    }
    
}




