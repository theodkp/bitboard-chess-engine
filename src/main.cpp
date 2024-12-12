/// Headers

#include <iostream>
#include <stdio.h>

/// MACROS ETC
using U64 = unsigned long long;



// board pos map
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,
};

enum {white,black};


const char *square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

// non-zero binary string if bit set otherwise zero
inline bool get_bit(U64 bitboard, int square) {
    return bitboard & (1ULL << square);
}


// bitwise OR will set bit no matter if previously set or unset
inline void set_bit(U64& bitboard, int square) { 
    bitboard |= (1ULL << square); 
}

inline void unset_bit(U64& bitboard, int square){
    bitboard &= ~(1ULL << square);

}

///// Bit MANIPULATIONS


// Count bits
static inline int count_bits(U64 bitboard){

    return __builtin_popcountll(bitboard);
}


static inline int get_lsf_bit_index(U64 bitboard){
    if (bitboard){
        return count_bits((bitboard & -bitboard)-1);
    }
    else{
        return -1;
    }
    
}

/// Print bitboard visulisation
void print_bitboard(U64 bitboard){

std::cout << "\n";
for (int rank = 0; rank< 8; rank++){
    for (int file = 0; file < 8; file++){
        int square = (rank*8) + file;

        if (!file){
            std::cout << 8 - rank << "  ";
        }
        

        std::cout <<  ( get_bit(bitboard,square) ? 1 : 0) << " ";

        
    }
    std::cout << "\n";
}

std::cout << "\n"<<"   a b c d e f g h" << "\n""\n";
std::cout << "     Bitboard: " << bitboard;

}
/// ATTACKS


/// lookup table will return binary rep of our possible pawn attacks for certain square
U64 pawn_attacks[2][64];

/// knight attack table
U64 knight_attacks[64];

/// king attack table
U64 king_attacks[64];

/// returns a U64 bitboard with all bits set except given file/s

const U64 not_a_file = 18374403900871474942ULL;

const U64 not_h_file = 9187201950435737471ULL;

const U64 not_hg_file = 4557430888798830399ULL;

const U64 not_ab_file = 18229723555195321596ULL;


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
 

/// PAWN ATTACKS
U64 mask_pawn_attacks(int side, int square){

    // result board
    U64 attacks = 0ULL;

    // pawn bitboard
    U64 bitboard = 0ULL;


    set_bit(bitboard,square);


    if (!side){

        /// pawn attacks white
       if (bitboard & not_h_file) attacks |= bitboard >>7;
       if (bitboard & not_a_file) attacks |= bitboard >>9;

    }
    else{
        /// pawn attacks black
        if (bitboard & not_a_file) attacks |= bitboard <<7;
        if (bitboard & not_h_file) attacks |= bitboard <<9;

    }


    return attacks;

    

}

/// KNIGHT ATTACKS
U64 mask_knight_attacks(int square){
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    set_bit(bitboard,square);


    if (bitboard & not_a_file) attacks |= bitboard >>17;
    if (bitboard & not_h_file) attacks |= bitboard >>15;
    if (bitboard & not_ab_file) attacks |= bitboard >>10;
    if (bitboard & not_hg_file) attacks |= bitboard >>6;


    if (bitboard & not_h_file) attacks |= bitboard <<17;
    if (bitboard & not_a_file) attacks |= bitboard <<15;
    if (bitboard & not_hg_file) attacks |= bitboard <<10;
    if (bitboard & not_ab_file) attacks |= bitboard <<6;


    return attacks;
}

/// KING ATTACKS

U64 mask_king_attacks(int square){
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    set_bit(bitboard,square);


    if (bitboard >> 8) attacks |= (bitboard >>8);
    if (bitboard & not_a_file) attacks |= bitboard >>9;
    if (bitboard & not_h_file) attacks |= bitboard >>7;
    if (bitboard & not_a_file) attacks |= bitboard >>1;

    if (bitboard << 8) attacks |= (bitboard <<8);
    if (bitboard & not_h_file) attacks |= bitboard <<9;
    if (bitboard & not_a_file) attacks |= bitboard <<7;
    if (bitboard & not_h_file) attacks |= bitboard <<1;


    return attacks;
}



/// Bishop attacks
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

/// Bishop attacks
U64 gen_bishop_attacks(int square, U64 block){

    U64 attacks = 0ULL;

    int r,f;

    int tr = square / 8;
    int tf = square % 8;

    for (r = tr+ 1 , f = tf+1; r <= 7 && f <= 7; r++,f++){
        attacks|= (1ULL << (r * 8 + f));
        // If diag is being blocked by piece
        if ((1ULL << ( r*8 + f )) & block) break;
    } 
    for (r = tr - 1 , f = tf+1; r >=0 && f <= 7; r--,f++) {
        attacks|= (1ULL << (r * 8 + f));
        if ((1ULL << ( r*8 + f )) & block) break;
    } 
    for (r = tr+ 1 , f = tf-1; r <= 7 && f >= 0; r++,f--){
        attacks|= (1ULL << (r * 8 + f));
        if ((1ULL << ( r*8 + f )) & block) break;
    } 
    for (r = tr- 1 , f = tf-1; r >= 0 && f >= 0; r--,f--) {
        attacks|= (1ULL << (r * 8 + f));
        if ((1ULL << ( r*8 + f )) & block) break;
    }

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

    U64 attacks = 0ULL;

    int r,f;

    int tr = square / 8;
    int tf = square % 8;

    // down
    for ( r = tr +1; r <= 6; r++){
        attacks|= (1ULL<< (r*8+tf));
        if ((1ULL<< (r*8+tf)) & block) break;
    } 
    // up 
    for ( r = tr -1; r >=1; r--){
        attacks|= (1ULL<< (r*8+tf));
        if ((1ULL<< (r*8+tf)) & block) break;
    } 
    // right
    for ( f = tf + 1; f <= 6; f++){
        attacks|= (1ULL<< (tr*8+f));
        if ((1ULL<< (tr*8+f)) & block) break;

    } 
    // left
    for ( f = tf - 1; f >= 1; f--){
        attacks|= (1ULL<< (tr*8+f));
        if ((1ULL<< (tr*8+f)) & block) break;
    }

    return attacks;

}

void init_leaper_attacks(){

    for (int square = 0; square <64; square++){
        pawn_attacks[white][square] = mask_pawn_attacks(white,square);
        pawn_attacks[black][square] = mask_pawn_attacks(black,square);

        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
    }

}

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

 //// MAIN 

int main(){

    init_leaper_attacks();

    for (int rank = 0; rank < 8; rank++){
        for (int file = 0; file < 8;file++){
            int square = rank*8+file;

            std::cout << count_bits(mask_rook_attacks(square)) << ",";
        }

        std::cout << "\n";
    }

   
    return 0;
}