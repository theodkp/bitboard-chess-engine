#pragma once

#include <utils.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <windows.h>
#include <algorithm>
#include <io.h>
#include <cerrno>
#include <fcntl.h>



// MACROS ETC

// board pos map
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};


// 4 bit binary string, representing if we can castle kingside/queenside
enum {wk = 1 , wq = 2 , bk = 4,bq = 8};

// Character rep
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// string version board pos map
static const char *square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

extern U64 bitboards[12];
extern U64 occupancies[3];
extern int side;
extern int en_passant;
extern int castle;
extern U64 hash_key;
extern U64 repetition_table[1000];
extern const int castling_rights[64];
extern int repetition_index;
extern int ply;


// ASCII pieces

// Unicode pieces

static const char* unicode_pieces[12] = {
    "♙", "♘", "♗", "♖",
    "♕", "♔", "♟", "♞",
    "♝", "♜", "♛", "♚"
};

// convert ascii characters to constants
extern int char_pieces[256];

// Initialize everything to some default value 


static char promoted_pieces[12] =
{
    '\0',
    'n',
    'b',
    'r',
    'q',
    '\0',
    '\0',
    'n',
    'b',
    'r',
    'q',
    '\0',
};


// move list structure
typedef struct {
    int moves[256];

    int count;

} moves;


#define encode_move(source, target, piece, promoted, capture, double, enpassant, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (double << 21) |    \
    (enpassant << 22) | \
    (castling << 23)    \

// extracting info from decoded move
#define get_move_source(move) (move & 0x3f)
#define get_move_target(move) ((move & 0xfc0) >> 6)
#define get_move_piece(move) ((move & 0xf000) >> 12)
#define get_move_promoted(move) ((move & 0xf0000) >> 16)
#define get_move_capture(move) ((move & 0x100000))
#define get_move_double(move) ((move & 0x200000))
#define get_move_enpassant(move) ((move & 0x400000))
#define get_move_castling(move) ((move & 0x800000))


#define copy_board()                                    \
    U64 bitboards_copy[12], occupancies_copy[3];       \
    int side_copy, en_passant_copy, castle_copy;       \
    memcpy(bitboards_copy, bitboards, 96);             \
    memcpy(occupancies_copy, occupancies, 24);         \
    side_copy = side;                                  \
    en_passant_copy = en_passant;                      \
    castle_copy = castle;                   \
    U64 hash_key_copy = hash_key;   

#define take_back()                                     \
    memcpy(bitboards, bitboards_copy, 96);             \
    memcpy(occupancies, occupancies_copy, 24);         \
    side = side_copy;                                  \
    en_passant = en_passant_copy;                      \
    castle = castle_copy;                   \
    hash_key = hash_key_copy;                             


void parse_fen(const char *fen);
void initCharPieces();
int is_repetition();