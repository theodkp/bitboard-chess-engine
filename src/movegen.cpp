

#include "attacks.h"
#include "global.h"
#include "hash.h"



void add_move(moves *move_list, int move){

    //store move
    move_list->moves[move_list->count] = move;

    // increment move count
    move_list->count++;

}


int make_move(int move, int  move_flag){

    // quiet moves
    if (move_flag == all_moves){

        copy_board();

        // parse move
        int source = get_move_source(move);
        int target = get_move_target(move);
        int piece = get_move_piece(move);
        int promoted = get_move_promoted(move);
        int capture = get_move_capture(move);
        int double_move = get_move_double(move);
        int en_pass = get_move_enpassant(move);
        int castling = get_move_castling(move);

        
        // move piece
        unset_bit(bitboards[piece],source);
        set_bit(bitboards[piece],target);


        hash_key ^= piece_keys[piece][source]; 
        hash_key ^= piece_keys[piece][target];

        // capture moves
        if (capture){

            // depending on side we loop through opposite bitboards
            int start,end;

            if (side == white){
                start = p;
                end = k;
            }
            else{
                start = P;
                end = K;
            }
            // if piece on target square (capture) we remove it from bitboard
            for (int bb_piece = start; bb_piece <= end; bb_piece++){
                if (get_bit(bitboards[bb_piece], target)){
                    unset_bit(bitboards[bb_piece], target);

                    hash_key ^= piece_keys[bb_piece][target];
                    break;
                }



            }
        }


        // pawn promotions
        if (promoted){
            if (side == white){
                unset_bit(bitboards[P],target);
                hash_key ^= piece_keys[P][target];
                }
            
            else{
                unset_bit(bitboards[p],target);
                hash_key ^= piece_keys[p][target];

                }

            set_bit(bitboards[promoted],target);

            hash_key ^= piece_keys[promoted][target];

       
        }
        // en passant
        if (en_pass){
            // remove pawn taken in en passant

            if (side == white){
                unset_bit(bitboards[p], target + 8);
                hash_key ^= piece_keys[p][target + 8];
            }
            else{
                unset_bit(bitboards[P], target - 8);
                hash_key ^= piece_keys[P][target - 8];
            }
        }

        if (en_passant != no_sq) hash_key ^= enpassant_keys[en_passant];

        en_passant = no_sq;


        // Setting enpassant on double push, we set the en passant square to 1 square behind (depending on colour)
        if (double_move){
            if (side == white){
                en_passant = target + 8;
                // update hash key
                hash_key ^= enpassant_keys[target + 8];
            }
            else{
                en_passant = target - 8;
                hash_key ^= enpassant_keys[target - 8];
            }
        }

        // CASTLING
        if (castling){

            switch(target){
                // white king castle
                case (g1):
                    unset_bit(bitboards[R],h1);
                    set_bit(bitboards[R],f1);

                    // update hash key
                    hash_key ^= piece_keys[R][h1];
                    hash_key ^= piece_keys[R][f1];
                    break;
                // white queen castle
                case (c1):
                    unset_bit(bitboards[R],a1);
                    set_bit(bitboards[R],d1);

                    hash_key ^= piece_keys[R][a1];
                    hash_key ^= piece_keys[R][d1];
                    break;
                // black king castle
                case (g8):
                    unset_bit(bitboards[r],h8);
                    set_bit(bitboards[r],f8);

                    hash_key ^= piece_keys[r][h8];
                    hash_key ^= piece_keys[r][f8];
                    break;
                // black queen castle
                case (c8):
                    unset_bit(bitboards[r],a8);
                    set_bit(bitboards[r],d8);
                    
                    
                    hash_key ^= piece_keys[r][a8];
                    hash_key ^= piece_keys[r][d8];
                    break;
            }

        }
        // update castling rights hash key
        hash_key ^= castle_keys[castle];

        // updating castling rights (bitmask)
        castle &= castling_rights[source];
        castle &= castling_rights[target];

        hash_key ^= castle_keys[castle];

        // updating shared occupancy bitboards (white, black, both) after making moves
        memset(occupancies,0ULL,24);

        for (int i = P; i <= K; i++){
            occupancies[white] |= bitboards[i];
        }

        for (int i = p; i <= k; i++){
            occupancies[black] |= bitboards[i];
        }

        occupancies[both] |= occupancies[white];
        occupancies[both] |= occupancies[black];
        
        // change turns
        side ^= 1;
        // update hash key for side to move
        hash_key ^= side_key;

        // king avoiding checks
        
        if (is_square_attacked((side == white) ? get_lsf_bit_index(bitboards[k]): get_lsf_bit_index(bitboards[K]),side)){

            take_back();

            return 0;

        }
        else{
            return 1;
        }


    }
    // capture  moves
    else{
            if (get_move_capture(move)){
                return make_move(move,all_moves);

                
            }

            else{
                return 0 ;
            }
    }


}

// generate all possible moves
void generate_moves(moves *move_list){


    // move count
    move_list->count = 0;


    int source,target;


    // bitboard copy
    U64 bitboard, attacks; 


    // loop over all piece bitboards

    for (int piece = P; piece<= k; piece++){

        // create piece bitboard copy
        bitboard = bitboards[piece];
        

        if (side == white){
            // GEN WHITE PAWN MOVES
            if (piece == P){
                
                // looping until all our bits are popped off 
                while(bitboard){

                    source = get_lsf_bit_index(bitboard);


                    target = source - 8;

                    // quiet pawn moves (do not alter material)

                    // make sure target square is not occupied
                    if (!(target < a8 ) && !get_bit(occupancies[both],target)){

                        // pawn promotion
                        if (source >= a7 && source <= h7){

                            // add each promotion move
                            add_move(move_list, encode_move(source,target,piece,R,0,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,Q,0,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,N,0,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,B,0,0,0,0));

                        }
                        // single and double pawn push
                        else{
                            // move one square ahead
                            add_move(move_list, encode_move(source,target,piece,0,0,0,0,0));
                            
                            // make sure square above target is clear (target - 8) and pawn is on second rank
                            if((source >= a2 && source <= h2) && !get_bit(occupancies[both],target - 8)){
                                add_move(move_list, encode_move(source,target-8,piece,0,0,1,0,0));

                            }


                        }
                    }

                    // pawn captures

                    // our pawn attack table for our source square, bitwise and with all black occupancies(making sure we only take when black pieces are present)
                    attacks = pawn_attacks[side][source] & occupancies[black];


                    while (attacks){
                        target = get_lsf_bit_index(attacks);

                        if (source >= a7 && source <= h7){

                            // capture and promote
                            add_move(move_list, encode_move(source,target,piece,R,1,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,Q,1,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,N,1,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,B,1,0,0,0));

                        }
                        // simple pawn capture
                        else{
                            add_move(move_list, encode_move(source,target,piece,0,1,0,0,0));
                        }


                        unset_bit(attacks,target);
                    }

                    // en passant capture
                    // checks if pawns normal attacks match up with any en passant squares set in fen
                    if (en_passant != no_sq){

                        U64 en_passant_attacks = pawn_attacks[side][source] & (1ULL << en_passant);

                        if (en_passant_attacks){
                            int target_en_passant = get_lsf_bit_index(en_passant_attacks);
                            add_move(move_list, encode_move(source,target_en_passant,piece,0,1,0,1,0));

                            
                        }
 
                    }




                    // clear current bit and move to  next piece in loop
                    unset_bit(bitboard,source);

                }


            }

            //WHITE KING CASTLING 
            if(piece == K){
                // kingside castle available
                if (castle & wk){

                    // make sure clear between king and rook
                    if (!get_bit(occupancies[both],f1) && !get_bit(occupancies[both],g1)){
                        // make sure king and f1 square not under attack
                        if (!is_square_attacked(e1,black) && !is_square_attacked(f1,black)){
                            add_move(move_list, encode_move(e1,g1,piece,0,0,0,0,1));

                        }

                    }


                }
                // queenside castling availablle
                if (castle & wq) {

                    // make sure clear between king and rook
                    if (!get_bit(occupancies[both],b1) && !get_bit(occupancies[both],c1) && !get_bit(occupancies[both],d1)){
                        // make sure king and d1 square not under attack
                        if (!is_square_attacked(e1,black) && !is_square_attacked(d1,black)){
                             add_move(move_list, encode_move(e1,c1,piece,0,0,0,0,1));

                        }

                    }

                }
            }

        }

        else {
            // GEN BLACK PAWN MOVES
            if (piece == p){
                
                // looping until all our bits are popped off 
                while(bitboard){

                    source = get_lsf_bit_index(bitboard);


                    target = source + 8;

                    // quiet pawn moves (do not alter material)

                    // make sure target square is not occupied
                    if (!(target > h1 ) && !get_bit(occupancies[both],target)){

                        // pawn promotion
                        if (source >= a2 && source <= h2){

                            //placeholder for move generator
                            add_move(move_list, encode_move(source,target,piece,r,0,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,q,0,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,n,0,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,b,0,0,0,0));

                        }
                        // single and double pawn push
                        else{
                            // move one square ahead
                            add_move(move_list, encode_move(source,target,piece,0,0,0,0,0));
                            
                            // make sure square above target is clear (target + 8) and pawn is on seventh rank(start square)
                            if((source >= a7 && source <= h7) && !get_bit(occupancies[both],target + 8)){
                                add_move(move_list, encode_move(source,target+8,piece,0,0,1,0,0));

                            }


                        }
                    }

                    // our pawn attack table for our source square, bitwise and with all white occupancies(making sure we only take when white pieces are present)
                    attacks = pawn_attacks[side][source] & occupancies[white];


                    while (attacks){
                        target = get_lsf_bit_index(attacks);

                        if (source >= a2 && source <= h2){

                            // capture and promote
                            add_move(move_list, encode_move(source,target,piece,r,1,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,q,1,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,n,1,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,b,1,0,0,0));

                        }
                        // simple pawn capture
                        else{
                            add_move(move_list, encode_move(source,target,piece,0,1,0,0,0));
                        }


                        unset_bit(attacks,target);
                    }

                    // en passant capture

                    if (en_passant != no_sq){

                        U64 en_passant_attacks = pawn_attacks[side][source] & (1ULL << en_passant);

                        if (en_passant_attacks){
                            int target_en_passant = get_lsf_bit_index(en_passant_attacks);
                            add_move(move_list, encode_move(source,target_en_passant,piece,0,1,0,1,0));

                            
                        }
 
                    }


                    // clear current bit and move to next in loop
                    unset_bit(bitboard,source);

                }


            }

            // GEN BLACK KING CASTLING
            if(piece == k){
                // kingside castle available
                if (castle & bk){

                    // make sure clear between king and rook
                    if (!get_bit(occupancies[both],f8) && !get_bit(occupancies[both],g8)){
                        // make sure king and f8 square not under attack
                        if (!is_square_attacked(e8,white) && !is_square_attacked(f8,white)){
                            add_move(move_list, encode_move(e8,g8,piece,0,0,0,0,1));

                        }

                    }


                }
                // queenside castling availablle
                if (castle & bq) {

                    // make sure clear between king and rook
                    if (!get_bit(occupancies[both],b8) && !get_bit(occupancies[both],c8) && !get_bit(occupancies[both],d8)){
                        // make sure king and d8 square not under attack
                        if (!is_square_attacked(e8,white) && !is_square_attacked(d8,white)){
                            add_move(move_list, encode_move(e8,c8,piece,0,0,0,0,1));

                        }

                    }

                }
            }

        }

        // GEN KNIGHT MOVES
        if ((side == white) ? piece == N : piece == n ){

            while (bitboard){
                source  = get_lsf_bit_index(bitboard);


                // by using the not operator we make sure that only empty or opposite side pieces are potential attacks
                attacks = knight_attacks[source] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);


                // go through our possible attack targets
                while (attacks){
                    
                    // each target
                    target = get_lsf_bit_index(attacks);

                    // quiet
                    //if target square is not occupied by opponent pieces, we can move
                    if (!get_bit( ((side == white) ? occupancies[black] : occupancies[white]),target) ){
                        add_move(move_list, encode_move(source,target,piece,0,0,0,0,0));


                    }
                    
                    // else target has to be an opponent piece, we can take
                    else{add_move(move_list, encode_move(source,target,piece,0,1,0,0,0));}


                    unset_bit(attacks,target);

                }
 

                unset_bit(bitboard,source);
            }
        }

        // GEN BISHOP MOVES

        if ((side == white) ? piece == B : piece == b ){

            while (bitboard){
                source  = get_lsf_bit_index(bitboard);


                // by using the not operator we make sure that only empty or opposite side pieces are potential attacks
                attacks = get_bishop_attacks(source,occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);


                // go through our possible attack targets
                while (attacks){
                    
                    // each target
                    target = get_lsf_bit_index(attacks);

                    // quiet
                    //if target square is not occupied by opponent pieces, we can move
                    if (!get_bit( ((side == white) ? occupancies[black] : occupancies[white]),target) ){
                        add_move(move_list, encode_move(source,target,piece,0,0,0,0,0));


                    }
                    
                    // else target has to be an opponent piece, we can take
                    else{add_move(move_list, encode_move(source,target,piece,0,1,0,0,0));}


                    unset_bit(attacks,target);

                }
 

                unset_bit(bitboard,source);
            }
        }

        // GEN ROOK MOVES
        if ((side == white) ? piece == R : piece == r ){

            while (bitboard){
                source  = get_lsf_bit_index(bitboard);


                // by using the not operator we make sure that only empty or opposite side pieces are potential attacks
                attacks = get_rook_attacks(source,occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);


                // go through our possible attack targets
                while (attacks){
                    
                    // each target
                    target = get_lsf_bit_index(attacks);

                    // quiet
                    //if target square is not occupied by opponent pieces, we can move
                    if (!get_bit( ((side == white) ? occupancies[black] : occupancies[white]),target) ){
                        add_move(move_list, encode_move(source,target,piece,0,0,0,0,0));


                    }
                    
                    // else target has to be an opponent piece, we can take
                    else{add_move(move_list, encode_move(source,target,piece,0,1,0,0,0));}



                    unset_bit(attacks,target);

                }
 

                unset_bit(bitboard,source);
            }
        }


        // GEN QUEEN MOVES
        if ((side == white) ? piece == Q : piece == q ){

            while (bitboard){
                source  = get_lsf_bit_index(bitboard);


                // by using the not operator we make sure that only empty or opposite side pieces are potential attacks
                attacks = get_queen_attacks(source,occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);


                // go through our possible attack targets
                while (attacks){
                    
                    // each target
                    target = get_lsf_bit_index(attacks);

                    // quiet
                    //if target square is not occupied by opponent pieces, we can move
                    if (!get_bit( ((side == white) ? occupancies[black] : occupancies[white]),target) ){
                        add_move(move_list, encode_move(source,target,piece,0,0,0,0,0));


                    }
                    
                    // else target has to be an opponent piece, we can take
                    else{add_move(move_list, encode_move(source,target,piece,0,1,0,0,0));}


                    unset_bit(attacks,target);

                }
 

                unset_bit(bitboard,source);
            }
        }


        // GEN KING MOVES
        if ((side == white) ? piece == K : piece == k ){

            while (bitboard){
                source  = get_lsf_bit_index(bitboard);


                // by using the not operator we make sure that only empty or opposite side pieces are potential attacks
                attacks = king_attacks[source] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);


                // go through our possible attack targets
                while (attacks){
                    
                    // each target
                    target = get_lsf_bit_index(attacks);

                    // quiet
                    //if target square is not occupied by opponent pieces, we can move
                    if (!get_bit( ((side == white) ? occupancies[black] : occupancies[white]),target) ){
                        add_move(move_list, encode_move(source,target,piece,0,0,0,0,0));


                    }
                    
                    // else target has to be an opponent piece, we can take
                    else{add_move(move_list, encode_move(source,target,piece,0,1,0,0,0));}



                    unset_bit(attacks,target);

                }
 

                unset_bit(bitboard,source);
            }
        }




    }


}