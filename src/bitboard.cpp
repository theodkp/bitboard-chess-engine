/*
 * bitboard.cpp
 * 
 * This file implements the core bitboard representation of the chess board.
 * Bitboards are 64-bit integers where each bit represents a square on the chess board.
 * This file handles:
 *   - Bitboard data structures for pieces and board state
 *   - FEN string parsing to set up the board position
 *   - Position tracking for repetition detection
 *   - Castling rights and en passant state
 * 
 * Bitboards provide an efficient way to represent and manipulate the chess position,
 * enabling fast move generation and position evaluation.
 */

#include <cstdio>
#include <cstring>
#include "search.h"



// define piece bitboards (ie black knights)
U64 bitboards[12];

// define occupancy bitboards (all white pieces, all black pieces, all pieces)
U64 occupancies[3];

// whos turn
int side;

// en passant 
int en_passant = no_sq;

// castling rights
int castle;

U64 hash_key;

U64 repetition_table[1000];

int repetition_index;

int ply;

const int castling_rights[64] = {
    7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};



int char_pieces[256];

void initCharPieces() {
    for (int i = 0; i < 256; i++)
        char_pieces[i] = -1;
    
    // Now do specific assignments
    char_pieces['P'] = P;
    char_pieces['N'] = N;
    char_pieces['B'] = B;
    char_pieces['R'] = R;
    char_pieces['Q'] = Q;
    char_pieces['K'] = K;
    char_pieces['p'] = p;
    char_pieces['n'] = n;
    char_pieces['b'] = b;
    char_pieces['r'] = r;
    char_pieces['q'] = q;
    char_pieces['k'] = k;
}


// Input Fen String Output board position
void parse_fen(const char *fen){
    // reset board to default positions
    // default boards
    memset(bitboards, 0ULL, sizeof(bitboards));
    // side occupancy boards (white, black , all)
    memset(occupancies,0ULL, sizeof(occupancies));
    // init side white
    side = 0;

    // en passant init
    en_passant = no_sq;
    // castling rights init
    castle = 0;

    // reset hash key
    hash_key = 0ULL;

    repetition_index = 0;

    memset(repetition_table, 0ULL, sizeof(repetition_table));
    // loop through board
    for (int rank = 0; rank < 8; rank++){
        for (int file = 0 ; file < 8; file++){

            int square = rank*8 + file;

            // if current char in fen string is a piece 
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z') ){
                
                // convert piece from fen to char piece, ie 'p' - > P
                int piece = char_pieces[*fen];

                // set bit (place piece) on piece bitboard at current square
                set_bit(bitboards[piece],square);



                // increment our pointer value pointing the next character along
                fen++;


            }

            if(*fen >= '0' && *fen <= '9'){

                // converts char version of int to an actual int
                int offset = *fen - '0';




                int piece = -1;
                
                for (int bb_piece = P; bb_piece <= k; bb_piece++)
                {
                    if (get_bit(bitboards[bb_piece], square))
                        piece = bb_piece;
                }
                
                if (piece == -1)
                    file--;

                // skip that many squares along the file
                file+= offset;

                // increment to next char in fen
                fen++;
            }

            // if current char in fen string is a seperator between ranks, skip over it
            if (*fen == '/'){
                fen++;
            }

        }
    }

    // shift pass white space
    fen++;

    // assign move to either black or white
    (*fen == 'w') ? (side = white) : (side = black);

    // avoiding more white space
    fen+=2;

    // setting up our castling rights
    while(*fen != ' '){
        switch(*fen){
            case 'K' : castle |= wk;break;
            case 'Q' : castle |= wq;break;
            case 'k' : castle |= bk;break;
            case 'q' : castle |= bq;break;
            case '-': break;
        }
        fen++;
    }

    // white space skip

    fen++;

    // setting up en passant rights

    if (*fen != '-'){
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');

        en_passant = rank * 8 + file;


    }

    else{
        en_passant = no_sq;
    }


    // loop through all white piece bitboards and add to our white board
    for (int piece=P; piece <= K; piece++){

        occupancies[white] |= bitboards[piece];

    }

    // loop through all black piece bitboards and add to our black board
    for (int piece=p; piece <= k;piece++){
        occupancies[black] |= bitboards[piece];
    }


    // Populate our both bitboards with all piece bitboards
    occupancies[both] |= occupancies[black];
    occupancies[both] |= occupancies[white];

    // generate hash key
    hash_key = generate_hash_key();
}


int is_repetition()
{
    for (int index = 0; index < repetition_index; index++)
        if (repetition_table[index] == hash_key)
            return 1;
    
    return 0;
}
