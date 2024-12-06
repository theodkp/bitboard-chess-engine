/// Headers

#include <iostream>
#include <stdio.h>

/// MACROS ETC
using U64 = unsigned long long;

inline bool get_bit(U64 bitboard, int square) {
    return bitboard & (1ULL << square);
}
///// Bit MANIPULATIONS


void print_bitboard(U64 bitboard){

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
 
}

//// MAIN 

int main(){

    U64 bitboard = 1ULL;
    print_bitboard(bitboard);

    return 0;
}