

#include "magics.h"
#include "attacks.h"
#include "utils.h"
#include <stdio.h>
#include <cstdio>
#include <cstring>

// generate magic number
U64 gen_magic_number(){
    return get_random_number_U64() &  get_random_number_U64() & get_random_number_U64();
}

U64 find_magic_number(int square, int relevant_bits, int bishop){
    U64 occupancies[4096];  

    U64 attacks[4096];

    U64 used_attacks[4096];


    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
    
    int occupancy_indices = 1 << relevant_bits;

    for (int i = 0 ; i < occupancy_indices; i++){
        occupancies[i] = set_occupancy(i,relevant_bits,attack_mask);

        attacks[i] = bishop ? gen_bishop_attacks(square,occupancies[i]) : gen_rook_attacks(square,occupancies[i]);


    } 

    for (int rc = 0 ; rc < 100000000; rc++ ){
        U64 magic_number = gen_magic_number();

        // skip bad nums
        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

        memset(used_attacks,0ULL,sizeof(used_attacks));

        int i,fail;
        // test magic index loop
        for (i = 0, fail = 0; !fail && i < occupancy_indices; i++){
            // init magic index
            int magic_index =(int)(( occupancies[i] * magic_number) >> (64 - relevant_bits));
            // if magic index is candidate
            if (used_attacks[magic_index] == 0ULL){
                // check if used attack
                used_attacks[magic_index] = attacks[i];
            }

            else if (used_attacks[magic_index] != attacks[i] ){
                fail = 1;
            }
        }
        if (!fail){
            return magic_number;

        }

       
    }

    printf("magic number doesnt work");
    return 0ULL;
};

// MAGIC *******************




// init magic numbers

void init_magic_numbers(){
    for (int square = 0; square < 64 ; square++){
        printf("0x%llxULL,\n", find_magic_number(square,rook_relevant_bits[square],rook));
    }

    printf("\n""");

    for (int square = 0; square < 64 ; square++){
        printf("0x%llxULL,\n", find_magic_number(square,bishop_relevant_bits[square],bishop));
    }
}