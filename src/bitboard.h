#pragma once

#include <cstdint>

#include <array>     
  
#include <string>   


enum { P, N, B, R, Q, K, p, n, b, r, q, k };

enum {wk = 1 , wq = 2 , bk = 4,bq = 8};

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

static const char* unicode_pieces[12] = {
    "♙", "♘", "♗", "♖",
    "♕", "♔", "♟", "♞",
    "♝", "♜", "♛", "♚"
};


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


inline constexpr std::array<int,64> castling_rights = {{
    7, 15,15,15, 3,15,15,11,
   15,15,15,15,15,15,15,15,
   15,15,15,15,15,15,15,15,
   15,15,15,15,15,15,15,15,
   15,15,15,15,15,15,15,15,
   15,15,15,15,15,15,15,15,
   15,15,15,15,15,15,15,15,
   13,15,15,15,12,15,15,14
}};

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


class Board {
public:
    Board();
    explicit Board(const char *fen);

    void parse_fen(const char *fen);
    void initCharPieces();
    int is_repetition();

    U64 bitboard(int piece) const    { return bitboards[piece]; }
    U64 occupancy(int side)  const   { return occupancies[side]; }


private:

    // piece bitboards
    std::array<U64, 12> bitboards;
    // occupancy bitboards
    std::array<U64, 3>  occupancies;

    enum { white, black, both };
    int   side;               
    int   en_passant;          
    int   castle;     
    U64   hash_key;
    std::array<U64, 1000> repetitionTable;
    int   repetitionIndex;
    int   ply;

    // lookup table for chars
    std::array<int, 256> charPieces;

    static U64        generateHashKey(const Board&);

    void clear();
    void updateOccupancies();

};