/*
 * hash.cpp - Zobrist Hashing and Transposition Table Implementation
 * 
 * This file implements the chess engine's position hashing system, including:
 * - Zobrist hash key generation for board positions
 * - Transposition table for storing previously evaluated positions
 * - Hash key management for pieces, castling rights, en-passant, and side to move
 * - PV (Principal Variation) table handling for best move sequences
 * 
 * The hashing system is crucial for move ordering and avoiding redundant position evaluations.
 */

#include "hash.h"
#include "search.h"
U64 piece_keys[12][64];

// random enpassant keys [square]
U64 enpassant_keys[64];

U64 castle_keys[16];

U64 side_key;

tt hash_table[hash_size];

void init_random_keys(){
    random_state = 1804289383;

    for (int piece = P; piece <= k; piece++){
        for (int square = 0; square < 64; square++){
            piece_keys[piece][square] = get_random_number_U64();

        }
            
    }
    
    for (int square = 0; square < 64; square++){
        enpassant_keys[square] = get_random_number_U64();
    }
        
    
    for (int index = 0; index < 16; index++){
        castle_keys[index] = get_random_number_U64();
    }
        
        
    side_key = get_random_number_U64();
}

// generate hash key
U64 generate_hash_key()
{
    U64 final_key = 0ULL;
    
    U64 bitboard;
    
    for (int piece = P; piece <= k; piece++){
        bitboard = bitboards[piece];
        
        while (bitboard){
            int square = get_lsf_bit_index(bitboard);
            
            final_key ^= piece_keys[piece][square];
            
            unset_bit(bitboard, square);
        }
    }
    
    if (en_passant != no_sq){
        final_key ^= enpassant_keys[en_passant];

    }
    
    final_key ^= castle_keys[castle];
    
    if (side == black) final_key ^= side_key;
    
    return final_key;
}

// clear transposition table
void clear_hash_table()
{
    for (int index = 0; index < hash_size; index++)
    {
        hash_table[index].hash_key = 0;
        hash_table[index].depth = 0;
        hash_table[index].flag = 0;
        hash_table[index].score = 0;
    }
}

 // pv move scoring
void enable_pv_scoring(moves *move_list)
{
    follow_pv = 0;
    
    for (int count = 0; count < move_list->count; count++){
        if (pv_table[0][ply] == move_list->moves[count]){
            score_pv = 1;
            
            follow_pv = 1;
        }
    }
}

// read hash
int read_hash_entry(int alpha, int beta, int depth)
{
    tt *hash_entry = &hash_table[hash_key % hash_size];
    
    if (hash_entry->hash_key == hash_key){
        if (hash_entry->depth >= depth){
            int score = hash_entry->score;
            
            if (score < -mate_score) score += ply;
            if (score > mate_score) score -= ply;
        
            if (hash_entry->flag == hash_flag_exact){
                return score;
            }
            
            if ((hash_entry->flag == hash_flag_alpha) && (score <= alpha)){
                return alpha;
            }
            
            if ((hash_entry->flag == hash_flag_beta) && (score >= beta)){
                return beta;
            }
        }
    }
    
    // if hash entry doesn't exist
    return no_hash_entry;
}

// write hash 
void write_hash_entry(int score, int depth, int hash_flag)
{
    tt *hash_entry = &hash_table[hash_key % hash_size];

    if (score < -mate_score) score -= ply;
    if (score > mate_score) score += ply;

    hash_entry->hash_key = hash_key;
    hash_entry->score = score;
    hash_entry->flag = hash_flag;
    hash_entry->depth = depth;
}