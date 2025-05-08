
#include <cstdio>
#include <cstring>
#include "search.h"
#include "utils.h"

Board::Board() {
    clear();
    initCharPieces();
}

Board::Board(const char *fen) {
    clear();
    initCharPieces();
    parse_fen(fen);
}


void Board::initCharPieces() {
    for (int i = 0; i < 256; i++)
        charPieces[i] = -1;
    
    // Now do specific assignments
    charPieces['P'] = P;
    charPieces['N'] = N;
    charPieces['B'] = B;
    charPieces['R'] = R;
    charPieces['Q'] = Q;
    charPieces['K'] = K;
    charPieces['p'] = p;
    charPieces['n'] = n;
    charPieces['b'] = b;
    charPieces['r'] = r;
    charPieces['q'] = q;
    charPieces['k'] = k;
}


void Board::clear() {
    bitboards        .fill(0);
    occupancies      .fill(0);
    repetitionTable  .fill(0);
    side             = white;
    en_passant        = no_sq;
    castle     = 0;
    hash_key          = 0ULL;
    repetitionIndex  = 0;
    ply              = 0;
}



// Input Fen String Output board position
void Board::parse_fen(const char *fen){
    clear();
    // loop through board
    for (int rank = 0; rank < 8; rank++){
        for (int file = 0 ; file < 8; file++){

            int square = rank*8 + file;

            // if current char in fen string is a piece 
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z') ){
                
                // convert piece from fen to char piece, ie 'p' - > P
                int piece = charPieces[*fen];

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


int Board::is_repetition()
{
    for (int index = 0; index < repetitionIndex; index++)
        if (repetitionTable[index] == hash_key)
            return 1;
    
    return 0;
}
