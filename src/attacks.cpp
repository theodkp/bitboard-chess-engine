/*
 * attacks.cpp - Chess piece attack generation and lookup system
 * 
 * This file implements attack pattern generation for all chess pieces using bitboards.
 * It includes:
 * - Pre-computed attack tables for "leaper" pieces (pawns, knights, kings)
 * - Magic bitboard implementation for "slider" pieces (bishops, rooks, queens)
 * - Attack mask generation and lookup functions for all piece types
 * - Square attack checking for determining if a square is under attack
 * 
 * The magic bitboard technique is used to efficiently compute sliding piece attacks
 * by using pre-calculated lookup tables and magic number hashing.
 */

#include <attacks.h>
#include <bitboard.h>

U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];
U64 bishop_masks[64];
U64 rook_masks[64];
U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];


// Count of bits for each position in a bishop attack ray
const int bishop_relevant_bits[64] = {
6,5,5,5,5,5,5,6,
5,5,5,5,5,5,5,5,
5,5,7,7,7,7,5,5,
5,5,7,9,9,7,5,5,
5,5,7,9,9,7,5,5,
5,5,7,7,7,7,5,5,
5,5,5,5,5,5,5,5,
6,5,5,5,5,5,5,6,
};


// Count of bits for each position in a rook attack ray
const int rook_relevant_bits[64] = {
12,11,11,11,11,11,11,12,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
12,11,11,11,11,11,11,12,
};

// PAWN ATTACKS
U64 mask_pawn_attacks(int side, int square){

    // result board
    U64 attacks = 0ULL;

    // pawn bitboard
    U64 bitboard = 0ULL;


    set_bit(bitboard,square);


    if (!side)
    {
        // generate pawn attacks
        if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    }
    
    // black pawns
    else
    {
        // generate pawn attacks
        if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
        if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);    
    }


    return attacks;

    

}

// KNIGHT ATTACKS
U64 mask_knight_attacks(int square){
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    set_bit(bitboard,square);


    if ((bitboard >> 17) & not_h_file) attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & not_a_file) attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & not_hg_file) attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & not_ab_file) attacks |= (bitboard >> 6);
    if ((bitboard << 17) & not_a_file) attacks |= (bitboard << 17);
    if ((bitboard << 15) & not_h_file) attacks |= (bitboard << 15);
    if ((bitboard << 10) & not_ab_file) attacks |= (bitboard << 10);
    if ((bitboard << 6) & not_hg_file) attacks |= (bitboard << 6);


    return attacks;
}

// KING ATTACKS

U64 mask_king_attacks(int square){
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    set_bit(bitboard,square);


    if (bitboard >> 8) attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);
    if (bitboard << 8) attacks |= (bitboard << 8);
    if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
    if ((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);


    return attacks;
}



// Bishop attacks
U64 mask_bishop_attacks(int square){

    U64 attacks = 0ULL;

    int r,f;

    int tr = square / 8;
    int tf = square % 8;

    for (r = tr+ 1 , f = tf+1; r <= 6 && f <= 6; r++,f++) attacks|= (1ULL << (r * 8 + f));
    for (r = tr - 1 , f = tf+1; r >=1 && f <= 6; r--,f++) attacks|= (1ULL << (r * 8 + f));
    for (r = tr+ 1 , f = tf-1; r <= 6 && f >= 1; r++,f--) attacks|= (1ULL << (r * 8 + f));
    for (r = tr- 1 , f = tf-1; r >= 1 && f >= 1; r--,f--) attacks|= (1ULL << (r * 8 + f));

    return attacks;

}

// Bishop attacks
U64 gen_bishop_attacks(int square, U64 block){
 // result attacks bitboard
    U64 attacks = 0ULL;
    
    // init ranks & files
    int r, f;
    
    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;
    
    // generate bishop atacks
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    // return attack map
    return attacks;
}

U64 mask_rook_attacks(int square){

    U64 attacks = 0ULL;

    int r,f;

    int tr = square / 8;
    int tf = square % 8;

    // down
    for ( r = tr +1; r <= 6; r++) attacks|= (1ULL<< (r*8+tf));
    // up 
    for ( r = tr -1; r >=1; r--) attacks|= (1ULL<< (r*8+tf));
    // right
    for ( f = tf + 1; f <= 6; f++) attacks|= (1ULL<< (tr*8+f));
    // left
    for ( f = tf - 1; f >= 1; f--) attacks|= (1ULL<< (tr*8+f));

    return attacks;

}

U64 gen_rook_attacks(int square,U64 block){

    // result attacks bitboard
    U64 attacks = 0ULL;
    
    // init ranks & files
    int r, f;
    
    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;
    
    // generate rook attacks
    for (r = tr + 1; r <= 7; r++)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    
    for (r = tr - 1; r >= 0; r--)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    
    for (f = tf + 1; f <= 7; f++)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    
    for (f = tf - 1; f >= 0; f--)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    
    // return attack map
    return attacks;
}


// Initialize pre computed attack arrays
void init_leaper_attacks(){

    for (int square = 0; square <64; square++){
        pawn_attacks[white][square] = mask_pawn_attacks(white,square);
        pawn_attacks[black][square] = mask_pawn_attacks(black,square);

        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
    }

}

// ATTACKS***************


// lookup table will return binary rep of our possible pawn attacks for certain square


// magic bitboard
U64 set_occupancy(int index, int bits_in_mask , U64 attack_mask){

    U64 occupancy = 0ULL;

    for (int count = 0; count < bits_in_mask; count++){
        
        int square = get_lsf_bit_index(attack_mask);

        unset_bit(attack_mask,square);


        if (index & (1 << count))
            occupancy |= (1ULL<< square);
        
    }

    return occupancy;

}




// slider piece attack tables

void init_sliders_attacks(int bishop){

    for (int square = 0 ; square < 64; square++){
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];


        // init relevant bit count 
        int relevant_bits_count = count_bits(attack_mask);

        // occupancy indices
        int occupancy_indices = 1 << relevant_bits_count;


        // loop over occupancy indices

        for (int i = 0; i < occupancy_indices; i++){
            if (bishop){
                U64 occupancy = set_occupancy(i,relevant_bits_count,attack_mask);

                int magic_index =( occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);


                // bishop attacks

                bishop_attacks[square][magic_index] = gen_bishop_attacks(square,occupancy);
            }
            else{

                U64 occupancy = set_occupancy(i,relevant_bits_count,attack_mask);

                int magic_index =( occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);


                // bishop attacks

                rook_attacks[square][magic_index] = gen_rook_attacks(square,occupancy);
                    
            }

        }
    }


}

// get bishop attacks 

U64 get_bishop_attacks(int square, U64 occupancy){

    // get bishop attacks for current board layout
    occupancy &= bishop_masks[square];
    occupancy*= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];


    return bishop_attacks[square][occupancy];

}

// get rook attacks 

U64 get_rook_attacks(int square, U64 occupancy){

    // get rook attacks for current board layout
    occupancy &= rook_masks[square];
    occupancy*= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];

    return rook_attacks[square][occupancy];

}



// get queen attacks

U64 get_queen_attacks(int square, U64 occupancy){

    // combine bishop and rook attacks, to get queen attacks

    U64 result = 0ULL;

    U64 bishop_occupancy = occupancy;

    U64 rook_occupancy= occupancy;


    bishop_occupancy &= bishop_masks[square];
    bishop_occupancy*= bishop_magic_numbers[square];
    bishop_occupancy >>= 64 - bishop_relevant_bits[square];

    result = bishop_attacks[square][bishop_occupancy];

    // get rook attacks for current board layout
    rook_occupancy &= rook_masks[square];
    rook_occupancy*= rook_magic_numbers[square];
    rook_occupancy >>= 64 - rook_relevant_bits[square];

    result |= rook_attacks[square][rook_occupancy];

    return result;

}

int is_square_attacked(int square, int side){

    // LEAPER
    // attacked by pawns
    if ((side == white) && (pawn_attacks[black][square]) & bitboards[P] )return 1;
    if ((side == black) && (pawn_attacks[white][square]) & bitboards[p] )return 1;

    // attacked by knight
    if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;
    
    // attacked by king
    if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

    // SLIDER - have to make sure no blocking pieces
    // attacked by bishop

    if (get_bishop_attacks(square,occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b]) ) return 1;

    // attacked by rook

    if (get_rook_attacks(square,occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r]) ) return 1;

    // attacked by queen

    if (get_queen_attacks(square,occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q]) ) return 1; 
        
    return 0;
}