/*
 * search.cpp - Chess Position Search and Evaluation System
 * 
 * This file implements the chess engine's main search functionality:
 * - Alpha-beta search with various enhancements
 * - Quiescence search for tactical stability
 * - Move ordering (PV moves, captures, killer moves, history moves)
 * - Time management and search control
 * - Principal Variation (PV) tracking
 * - Late Move Reduction (LMR) and other search optimizations
 * 
 * The search system is the core of the chess engine, responsible for
 * finding the best moves in any given position.
 */

#include "search.h"
#include "utils.h"
#include "attacks.h"
#include "eval.h"
#include "movegen.h"
#include "visual.h"






int score_move(int move){

    
    if(score_pv){
        if(pv_table[0][ply] == move){

            score_pv = 0;
            // pv score to the top
            return 20000;
        }
    }
    
    if (get_move_capture(move)){

        int target_piece = P;

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
            if (get_bit(bitboards[bb_piece], get_move_target(move))){

                target_piece = bb_piece;
                break;
            }
        
    }
        return mvv_lva[get_move_piece(move)][target_piece] + 10000;
}

    else{

        // killer move 1
        if (killer_moves[0][ply] == move){
            return 9000;
        }

        // killer move 2

        else if (killer_moves[1][ply] == move){
            return 8000;
        }

        // history move

        else{
            return history_moves[get_move_piece(move)][get_move_target(move)];
        }
        
    }

    return 0;
}


// sorting using built in cpp lib (O(n(log(n))))
struct MoveWithScore {
    int move;
    int score;
};

// helping our alpha beta pruning by feeding it top scoring moves first
void sort_moves(moves* move_list) {
    if (move_list->count > 256) {
        return;
    }

    MoveWithScore scored_moves[256];

    for (int i = 0; i < move_list->count; ++i) {
        scored_moves[i].move = move_list->moves[i];
        scored_moves[i].score = score_move(move_list->moves[i]);
    }

    std::sort(scored_moves, scored_moves + move_list->count, [](const MoveWithScore& a, const MoveWithScore& b) -> bool {
        return a.score > b.score; 
    });

    // Update the original move_list with the sorted moves
    for (int i = 0; i < move_list->count; ++i) {
        move_list->moves[i] = scored_moves[i].move;
    }
}


// Quiescence search
static inline int quiescence(int alpha, int beta){

    // every 2047 nodes listen for GUI/user input
    if ((nodes & 2047) == 0){
        communicate();
    }
    nodes++;

    if (ply > max_ply - 1)
        // evaluate position
        return evaluate();

    // get evaluation score
    int evaluation = evaluate();
    

    if (evaluation >= beta){
        return beta;
    }
    
    if (evaluation > alpha){
        alpha = evaluation;
    }
    
    moves move_list[1];
    
    // generate moves
    generate_moves(move_list);

    sort_moves(move_list);
    
    // loop over moves 
    for (int count = 0; count < move_list->count; count++){
        
        
        copy_board();
        
        ply++;

        repetition_index++;
        repetition_table[repetition_index] = hash_key;
        
        // make sure to make only legal moves, this time only taking captures
        if (make_move(move_list->moves[count], only_captures) == 0){
            
            ply--;

            repetition_index--;
            
            continue;
        }

        // current move score, flips each time
        int score = -quiescence(-beta, -alpha);
        
        ply--;

        repetition_index--;

        // reset
        take_back();
        

        if(is_stopped == 1){
            return 0;
        }
        
        
        
        // update if better score found

        if (score > alpha){
            alpha = score;

            if (score >= beta){
            return beta;
            }
            
        }
    }
    
    return alpha;
}




static inline int negamax(int alpha, int beta, int depth){

    int score;

    int hash_flag = hash_flag_alpha;

    if (ply && is_repetition())
        return 0;
    
    int pv_node = beta - alpha > 1;

    


    if (ply && (score = read_hash_entry(alpha, beta, depth)) != no_hash_entry && pv_node == 0){
        return score;
     }
        


    if ((nodes & 2047) == 0){
        communicate();
    }
    
    // tscp chess engine - tom kerrigan


    pv_length[ply] = ply;


    // Base case
    if (depth == 0){
        return quiescence(alpha,beta);
    }


    if (ply > max_ply - 1)
        return evaluate();   
        
    
    nodes++;

    // make sure not in check
    int in_check = is_square_attacked((side == white) ? get_lsf_bit_index(bitboards[K]): get_lsf_bit_index(bitboards[k]), side ^1);



    if (in_check){
        depth++;
    }

    int legal_moves = 0;

     if (depth >= 3 && in_check == 0 && ply){
        copy_board();

        ply++;

        repetition_index++;
        repetition_table[repetition_index] = hash_key;

        if (en_passant != no_sq){
            hash_key ^= enpassant_keys[en_passant];
        }
        
        side ^= 1;

        hash_key ^= side_key; 
        
        en_passant = no_sq;
        
        int score = -negamax(-beta, -beta + 1, depth - 1 - 2);


        ply--;

        repetition_index--;
        
        take_back();

        // if time is up stop calculating
        if(is_stopped == 1 ){
            return 0;
        }
        
        if (score >= beta){
            return beta;
        }
            
    }
    
    
    
    moves move_list[1];
    
    // generate moves for current board state
    generate_moves(move_list);

    // if following pv route
    if(follow_pv){

        enable_pv_scoring(move_list);

    }


    sort_moves(move_list);

    int moves_searched = 0;


    
    
    // Loop through all generated moves
    for (int count = 0; count < move_list->count; count++){
        
        copy_board();
        
        ply++;


        repetition_index++;
        repetition_table[repetition_index] = hash_key;
        
        
        // skip if illegal move
        if (make_move(move_list->moves[count], all_moves) == 0){
            ply--;


            repetition_index--;
            
            continue;
        }

        legal_moves++;
        
        
        if (moves_searched == 0){
            // normal alpha beta search
            score = -negamax(-beta, -alpha, depth - 1);
        }
          
            
        // late move reduction    
        else{
            if(
                moves_searched >= full_depth_moves &&
                depth >= reduction_limit &&
                in_check == 0 && 
                get_move_capture(move_list->moves[count]) == 0 &&
                get_move_promoted(move_list->moves[count]) == 0
                )
                score = -negamax(-alpha - 1, -alpha, depth - 2);
            
            else score = alpha + 1;
            
            if(score > alpha){
                score = -negamax(-alpha - 1, -alpha, depth-1);
            
                if((score > alpha) && (score < beta))
                    score = -negamax(-beta, -alpha, depth-1);
            }
        }
        
        ply--;

        repetition_index--;

        // restore board after recursion breaks out
        take_back();
        
        if(is_stopped  == 1){
            return 0;
        }

        moves_searched++;

        // beta cutoff
        
        
        // update if better score found
        if (score > alpha){

            hash_flag = hash_flag_exact;
            if (get_move_capture(move_list->moves[count]) == 0){
            history_moves[get_move_piece(move_list->moves[count])][get_move_target(move_list->moves[count])] += depth;
            }
            
            alpha = score;

            // enable pound_pv flag

            pv_table[ply][ply] = move_list->moves[count];

            for (int next_ply = ply+1; next_ply < pv_length[ply + 1]; next_ply++){
                pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
            }
            
            // adjust PV length
            pv_length[ply] = pv_length[ply + 1];

            if (score >= beta){
                write_hash_entry(score, depth, hash_flag_beta);
                // killer moves
                if (get_move_capture(move_list->moves[count]) == 0){
                    killer_moves[1][ply] =killer_moves[0][ply];
                    killer_moves[0][ply] = move_list->moves[count];

                }
            
                return beta;
            }          
        }
    }
    // no legal moves
    if (legal_moves == 0){

        // checkmate

        if (in_check){
            // make sure checkmate is in as few moves as possible (+ply) 
            return -mate_value + ply;
        }

        //stalemate
        else{
            // draw
            return 0;
        }
    }
    
    write_hash_entry(alpha, depth, hash_flag);
    return alpha;
}

// searches postions for best move
void search_position(int depth){

    int score = 0;


    // reset nodes count on each search
    nodes = 0;
    
    is_stopped  = 0;
    // reset follow pv flag
    follow_pv = 0;
    score_pv = 0;

   

    // clear previous memory
    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_moves, 0, sizeof(history_moves));
    memset(pv_table, 0, sizeof(pv_table));
    memset(pv_length, 0, sizeof(pv_length));


    int alpha = -infinity;
    int beta = infinity;
    

    // iterative deepening
    for (int cur_depth = 1; cur_depth <= depth; cur_depth++){
        
        // return best move so far if out of time
        if (is_stopped  == 1){
            break;
        }


        follow_pv = 1;


        int score = negamax(alpha, beta, cur_depth);


        if ((score <= alpha) || (score >= beta)) {
            alpha = -infinity;    
            beta = infinity;      
            continue;
        }

        alpha = score - 50;
        beta = score + 50;


        if (score > -mate_value && score < -mate_score){
                std::cout<< " info score mate " << -(score + mate_value) / 2 - 1 <<  " depth " << cur_depth << " nodes " << nodes << " pv ";

        }
        
        else if (score > mate_score && score < mate_value){
            std::cout<< "info score mate " << (score - mate_value) / 2 + 1 <<  " depth " << cur_depth << " nodes " << nodes << " pv ";

        }
        
        else{
            std::cout<< "info score cp " << score << " depth " << cur_depth << " nodes " << nodes << " pv ";
        }
        
        // loop over the moves within a PV line
        for (int count = 0; count < pv_length[0]; count++)
        {
            // print PV move
            print_move(pv_table[0][count]);
            std::cout <<" ";
        }

        std::cout << std::endl;
        
    }
    
    

    std::cout << "bestmove ";
    print_move(pv_table[0][0]);
    std::cout << std::endl;
}



