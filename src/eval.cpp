

#include "eval.h"
#include "attacks.h"


U64 file_masks[64];
U64 rank_masks[64];
U64 isolated_masks[64];
U64 passed_masks[64];
U64 white_passed_masks[64];
U64 black_passed_masks[64];

U64 set_file_rank_mask(int file_number, int rank_number)
{
    // file or rank mask
    U64 mask = 0ULL;
    
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            
            if (file_number != -1)
            {
                if (file == file_number)
                    mask |= set_bit(mask, square);
            }
            
            else if (rank_number != -1)
            {
                if (rank == rank_number)
                    mask |= set_bit(mask, square);
            }
        }
    }
    
    return mask;
}

void init_evaluation_masks()
{
    /******** Init file masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            file_masks[square] |= set_file_rank_mask(file, -1);
        }
    }
    
    /******** Init rank masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            rank_masks[square] |= set_file_rank_mask(-1, rank);
        }
    }
    
    /******** Init isolated masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
            isolated_masks[square] |= set_file_rank_mask(file + 1, -1);
        }
    }
    
    /******** White passed masks ********/
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            white_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
            white_passed_masks[square] |= set_file_rank_mask(file, -1);
            white_passed_masks[square] |= set_file_rank_mask(file + 1, -1);
            
            // loop over redudant ranks
            for (int i = 0; i < (8 - rank); i++)
                // reset redudant bits 
                white_passed_masks[square] &= ~rank_masks[(7 - i) * 8 + file];
        }
    }
    
    
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;
            
            // init file mask for a current square
            black_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
            black_passed_masks[square] |= set_file_rank_mask(file, -1);
            black_passed_masks[square] |= set_file_rank_mask(file + 1, -1);
            
            for (int i = 0; i < rank + 1; i++)
                black_passed_masks[square] &= ~rank_masks[i * 8 + file];
            
        }
    }
}




// cumulative score of current board state, 0 would be even, we switch the sign by returning double negative for black
int evaluate(){
    int score = 0;

    U64 bitboard;
    int piece,square;

    int double_pawns = 0;

    // loop through all chess pieces
    for (int bb_piece = P; bb_piece <= k; bb_piece++){

        // each piece bitboard
        bitboard = bitboards[bb_piece];

        // while piece on board
        while(bitboard){

            piece = bb_piece;
            // get square of each piece
            square = get_lsf_bit_index(bitboard);
            // add to our cumulative score
            score+= material_score[piece];

            // scores for positional values based on our piece square tables added to our cumulative sum
            switch (piece){
                case Q:
                    score += count_bits(get_queen_attacks(square, occupancies[both]));
                    break;
                case P:
                    score += pawn_score[square];
                    
                    double_pawns = count_bits(bitboards[P] & file_masks[square]);
                    
                    if (double_pawns > 1)
                        score += double_pawns * double_pawn_penalty;
                    
                    if ((bitboards[P] & isolated_masks[square]) == 0)
                        score += isolated_pawn_penalty;
                    
                    if ((white_passed_masks[square] & bitboards[p]) == 0)
                        score += passed_pawn_bonus[get_rank[square]];

                    break;
                case N: score+= knight_score[square]; break;
                case R:
                    // positional score
                    score += rook_score[square];
                    
                    // semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += semi_open_file_score;
                    
                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += open_file_score;
                    
                    break;
                case B: score+= bishop_score[square]; break;
                case K:
                    // posirional score
                    score += king_score[square];
                    
                    // semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= semi_open_file_score;
                    
                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= open_file_score;
                    
                    score += count_bits(king_attacks[square] & occupancies[white]) * king_shield_bonus;
                    break;
                // black
                case q:
                    // mobility
                    score -= count_bits(get_queen_attacks(square, occupancies[both]));
                    break;

                case p:
                    score -= pawn_score[mirror_score[square]];

                    double_pawns = count_bits(bitboards[p] & file_masks[square]);
                    
                    if (double_pawns > 1)
                        score -= double_pawns * double_pawn_penalty;
                    
                    if ((bitboards[p] & isolated_masks[square]) == 0)
                        score -= isolated_pawn_penalty;
                    
                    if ((black_passed_masks[square] & bitboards[P]) == 0)
                        score -= passed_pawn_bonus[get_rank[mirror_score[square]]];

                    break;
                case n: score-= knight_score[mirror_score[square]]; break;
                case r:
                    // positional score
                    score -= rook_score[mirror_score[square]];
                    
                    // semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= semi_open_file_score;
                    
                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= open_file_score;
                    
                    break;
                case b: score-= bishop_score[mirror_score[square]]; break;
                case k:
                    // positional score
                    score -= king_score[mirror_score[square]];
                    
                    // semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += semi_open_file_score;
                    
                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += open_file_score;
                    
                    score -= count_bits(king_attacks[square] & occupancies[black]) * king_shield_bonus;
                    break;
            }

            // unset bit on current piece and move to next if available
            unset_bit(bitboard,square);
        }
    }

    // return final score based on side
    return (side == white) ? score : -score;


}



