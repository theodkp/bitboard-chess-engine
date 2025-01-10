// Headers

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unordered_map>



// FEN DEBUG POS - format / ranks - turn - castling - enpassant
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
#define en_passant_test "r3k2r/8/8/2pPN3/Pp6/8/PPPBBPpp/R3K2R b KQkq a3 0 1 "

// MACROS ETC
using U64 = unsigned long long;

// board pos map
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,no_sq
};

// piece colours
enum {white,black,both};

enum {rook, bishop};

// 4 bit binary string, representing if we can castle kingside/queenside
enum {wk = 1 , wq = 2 , bk = 4,bq = 8};

// Character rep
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// string version board pos map
const char *square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

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


// ASCII pieces

char ascii_pieces[13] = "PNBRQKpnbrqk";

// Unicode pieces

const char* unicode_pieces[12] = {
    "♙", "♘", "♗", "♖",
    "♕", "♔", "♟", "♞",
    "♝", "♜", "♛", "♚"
};

// convert ascii characters to constants
int char_pieces[256];

// Initialize everything to some default value (e.g., -1)
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

char promoted_pieces[12] =
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



// Randomising functions
unsigned int random_state = 1804289383;

// XORSHIFT32 algorithm
unsigned int get_random_number_U32(){
    unsigned int x = random_state;

    x^= x << 13;
    x^= x >> 17;
    x^= x << 5;


    random_state = x;

    return x;
}

U64 get_random_number_U64(){
    
    U64 n1,n2,n3,n4;

    n1 = ((U64)(get_random_number_U32()) & 0xFFFF);
    n2 = ((U64)(get_random_number_U32()) & 0xFFFF);
    n3 = ((U64)(get_random_number_U32()) & 0xFFFF);
    n4 = ((U64)(get_random_number_U32()) & 0xFFFF);

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);

}


// generate magic number
U64 gen_magic_number(){
    return get_random_number_U64() &  get_random_number_U64() & get_random_number_U64();
}




// non-zero binary string if bit set otherwise zero
inline bool get_bit(U64 bitboard, int square) {
    return (bitboard) & (1ULL << square);
}


// bitwise OR will set bit no matter if previously set or unset
inline void set_bit(U64& bitboard, int square) { 
    (bitboard) |= (1ULL << square); 
}

// unset bit, will not flip
inline void unset_bit(U64& bitboard, int square){
    (bitboard) &= ~(1ULL << square);

}

// Bit MANIPULATIONS


// Count bits
static inline int count_bits(U64 bitboard){

    return __builtin_popcountll(bitboard);
}


// Get the index of our least significant set bit
static inline int get_lsf_bit_index(U64 bitboard){
    if (bitboard){
        return count_bits((bitboard & -bitboard)-1);
    }
    else{
        return -1;
    }
    
}

// Print bitboard visulisation
void print_bitboard(U64 bitboard){

std::cout << "\n";
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
std::cout << "     Bitboard: " << bitboard;

}

// print unicode board

void print_board(){

    for (int rank = 0; rank < 8; rank++){
        for (int file = 0; file < 8; file++){

            int square = rank* 8 + file;

            if (!file){
            std::cout << 8 - rank << "  ";
        }

            int piece = -1;

            // loop over piece bitboards
            for (int bb_piece = P; bb_piece <= k; bb_piece++){
                if ( get_bit(bitboards[bb_piece],square) ){
                    piece = bb_piece;
                }
            }

            std::cout << ((piece == -1) ? "." : unicode_pieces[piece]) << " ";

        }

        std::cout << "\n";
    }
    std::cout << "\n"<<"   a b c d e f g h" << "\n""\n";


    std::cout << "side: " << ((!side && (side != -1)) ? "white" : "black") << "\n";


    std::cout << "En Passant: " <<( (en_passant != no_sq) ? square_to_coordinates[en_passant] : "no") << "\n";

    std::cout << "Castling rights: "
          << ((castle & wk) ? 'K' : '-')
          << ((castle & wq) ? 'Q' : '-')
          << ((castle & bk) ? 'k' : '-')
          << ((castle & bq) ? 'q' : '-') 
          << std::endl;



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

}


// ATTACKS***************


// lookup table will return binary rep of our possible pawn attacks for certain square
U64 pawn_attacks[2][64];

// knight attack table
U64 knight_attacks[64];

// king attack table
U64 king_attacks[64];

// bishop attack masks
U64 bishop_masks[64];

// rook attack masks
U64 rook_masks[64];

// bishop attacks table,  512 no of bytes
U64 bishop_attacks[64][512];

// rook attacks table 
U64 rook_attacks[64][4096];

// returns a U64 bitboard with all bits set except given file/s

const U64 not_a_file = 18374403900871474942ULL;

const U64 not_h_file = 9187201950435737471ULL;

const U64 not_hg_file = 4557430888798830399ULL;

const U64 not_ab_file = 18229723555195321596ULL;

// Count of bits for each position in a bishop attack ray
const int bishop_relevant_bits[64] = {
6,5,5,5,5,5,5,6,
5,5,5,5,5,5,5,5,
5,5,7,7,7,7,5,5,
5,5,7,9,9,7,5,5,
5,5,7,9,9,7,5,5,
5,5,7,7,7,7,5,5,
5,5,5,5,5,5,5,5,
6,5,5,5,5,5,5,6,
};


// Count of bits for each position in a rook attack ray
const int rook_relevant_bits[64] = {
12,11,11,11,11,11,11,12,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
11,10,10,10,10,10,10,11,
12,11,11,11,11,11,11,12,
};


// rook magic numbers

U64 rook_magic_numbers[64] = {
0x8a80104000800020ULL,
0x140002000100040ULL,
0x2801880a0017001ULL,
0x100081001000420ULL,
0x200020010080420ULL,
0x3001c0002010008ULL,
0x8480008002000100ULL,
0x2080088004402900ULL,
0x800098204000ULL,
0x2024401000200040ULL,
0x100802000801000ULL,
0x120800800801000ULL,
0x208808088000400ULL,
0x2802200800400ULL,
0x2200800100020080ULL,
0x801000060821100ULL,
0x80044006422000ULL,
0x100808020004000ULL,
0x12108a0010204200ULL,
0x140848010000802ULL,
0x481828014002800ULL,
0x8094004002004100ULL,
0x4010040010010802ULL,
0x20008806104ULL,
0x100400080208000ULL,
0x2040002120081000ULL,
0x21200680100081ULL,
0x20100080080080ULL,
0x2000a00200410ULL,
0x20080800400ULL,
0x80088400100102ULL,
0x80004600042881ULL,
0x4040008040800020ULL,
0x440003000200801ULL,
0x4200011004500ULL,
0x188020010100100ULL,
0x14800401802800ULL,
0x2080040080800200ULL,
0x124080204001001ULL,
0x200046502000484ULL,
0x480400080088020ULL,
0x1000422010034000ULL,
0x30200100110040ULL,
0x100021010009ULL,
0x2002080100110004ULL,
0x202008004008002ULL,
0x20020004010100ULL,
0x2048440040820001ULL,
0x101002200408200ULL,
0x40802000401080ULL,
0x4008142004410100ULL,
0x2060820c0120200ULL,
0x1001004080100ULL,
0x20c020080040080ULL,
0x2935610830022400ULL,
0x44440041009200ULL,
0x280001040802101ULL,
0x2100190040002085ULL,
0x80c0084100102001ULL,
0x4024081001000421ULL,
0x20030a0244872ULL,
0x12001008414402ULL,
0x2006104900a0804ULL,
0x1004081002402ULL,
};

// bishop magic numbers
U64 bishop_magic_numbers[64] = { 
0x40040844404084ULL,
0x2004208a004208ULL,
0x10190041080202ULL,
0x108060845042010ULL,
0x581104180800210ULL,
0x2112080446200010ULL,
0x1080820820060210ULL,
0x3c0808410220200ULL,
0x4050404440404ULL,
0x21001420088ULL,
0x24d0080801082102ULL,
0x1020a0a020400ULL,
0x40308200402ULL,
0x4011002100800ULL,
0x401484104104005ULL,
0x801010402020200ULL,
0x400210c3880100ULL,
0x404022024108200ULL,
0x810018200204102ULL,
0x4002801a02003ULL,
0x85040820080400ULL,
0x810102c808880400ULL,
0xe900410884800ULL,
0x8002020480840102ULL,
0x220200865090201ULL,
0x2010100a02021202ULL,
0x152048408022401ULL,
0x20080002081110ULL,
0x4001001021004000ULL,
0x800040400a011002ULL,
0xe4004081011002ULL,
0x1c004001012080ULL,
0x8004200962a00220ULL,
0x8422100208500202ULL,
0x2000402200300c08ULL,
0x8646020080080080ULL,
0x80020a0200100808ULL,
0x2010004880111000ULL,
0x623000a080011400ULL,
0x42008c0340209202ULL,
0x209188240001000ULL,
0x400408a884001800ULL,
0x110400a6080400ULL,
0x1840060a44020800ULL,
0x90080104000041ULL,
0x201011000808101ULL,
0x1a2208080504f080ULL,
0x8012020600211212ULL,
0x500861011240000ULL,
0x180806108200800ULL,
0x4000020e01040044ULL,
0x300000261044000aULL,
0x802241102020002ULL,
0x20906061210001ULL,
0x5a84841004010310ULL,
0x4010801011c04ULL,
0xa010109502200ULL,
0x4a02012000ULL,
0x500201010098b028ULL,
0x8040002811040900ULL,
0x28000010020204ULL,
0x6000020202d0240ULL,
0x8918844842082200ULL,
0x4010011029020020ULL,
};
 

// PAWN ATTACKS
U64 mask_pawn_attacks(int side, int square){

    // result board
    U64 attacks = 0ULL;

    // pawn bitboard
    U64 bitboard = 0ULL;


    set_bit(bitboard,square);


    if (!side){

        /// pawn attacks white
       if (bitboard & not_h_file) attacks |= bitboard >>7;
       if (bitboard & not_a_file) attacks |= bitboard >>9;

    }
    else{
        /// pawn attacks black
        if (bitboard & not_a_file) attacks |= bitboard <<7;
        if (bitboard & not_h_file) attacks |= bitboard <<9;

    }


    return attacks;

    

}

// KNIGHT ATTACKS
U64 mask_knight_attacks(int square){
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    set_bit(bitboard,square);


    if (bitboard & not_a_file) attacks |= bitboard >>17;
    if (bitboard & not_h_file) attacks |= bitboard >>15;
    if (bitboard & not_ab_file) attacks |= bitboard >>10;
    if (bitboard & not_hg_file) attacks |= bitboard >>6;


    if (bitboard & not_h_file) attacks |= bitboard <<17;
    if (bitboard & not_a_file) attacks |= bitboard <<15;
    if (bitboard & not_hg_file) attacks |= bitboard <<10;
    if (bitboard & not_ab_file) attacks |= bitboard <<6;


    return attacks;
}

// KING ATTACKS

U64 mask_king_attacks(int square){
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    set_bit(bitboard,square);


    if (bitboard >> 8) attacks |= (bitboard >>8);
    if (bitboard & not_a_file) attacks |= bitboard >>9;
    if (bitboard & not_h_file) attacks |= bitboard >>7;
    if (bitboard & not_a_file) attacks |= bitboard >>1;

    if (bitboard << 8) attacks |= (bitboard <<8);
    if (bitboard & not_h_file) attacks |= bitboard <<9;
    if (bitboard & not_a_file) attacks |= bitboard <<7;
    if (bitboard & not_h_file) attacks |= bitboard <<1;


    return attacks;
}



// Bishop attacks
U64 mask_bishop_attacks(int square){

    U64 attacks = 0ULL;

    int r,f;

    int tr = square / 8;
    int tf = square % 8;

    for (r = tr+ 1 , f = tf+1; r <= 6 && f <= 6; r++,f++) attacks|= (1ULL << (r * 8 + f));
    for (r = tr - 1 , f = tf+1; r >=1 && f <= 6; r--,f++) attacks|= (1ULL << (r * 8 + f));
    for (r = tr+ 1 , f = tf-1; r <= 6 && f >= 1; r++,f--) attacks|= (1ULL << (r * 8 + f));
    for (r = tr- 1 , f = tf-1; r >= 1 && f >= 1; r--,f--) attacks|= (1ULL << (r * 8 + f));

    return attacks;

}

// Bishop attacks
U64 gen_bishop_attacks(int square, U64 block){
 // result attacks bitboard
    U64 attacks = 0ULL;
    
    // init ranks & files
    int r, f;
    
    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;
    
    // generate bishop atacks
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    // return attack map
    return attacks;
}

U64 mask_rook_attacks(int square){

    U64 attacks = 0ULL;

    int r,f;

    int tr = square / 8;
    int tf = square % 8;

    // down
    for ( r = tr +1; r <= 6; r++) attacks|= (1ULL<< (r*8+tf));
    // up 
    for ( r = tr -1; r >=1; r--) attacks|= (1ULL<< (r*8+tf));
    // right
    for ( f = tf + 1; f <= 6; f++) attacks|= (1ULL<< (tr*8+f));
    // left
    for ( f = tf - 1; f >= 1; f--) attacks|= (1ULL<< (tr*8+f));

    return attacks;

}

U64 gen_rook_attacks(int square,U64 block){

    // result attacks bitboard
    U64 attacks = 0ULL;
    
    // init ranks & files
    int r, f;
    
    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;
    
    // generate rook attacks
    for (r = tr + 1; r <= 7; r++)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    
    for (r = tr - 1; r >= 0; r--)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    
    for (f = tf + 1; f <= 7; f++)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    
    for (f = tf - 1; f >= 0; f--)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    
    // return attack map
    return attacks;
}


// Initialize pre computed attack arrays
void init_leaper_attacks(){

    for (int square = 0; square <64; square++){
        pawn_attacks[white][square] = mask_pawn_attacks(white,square);
        pawn_attacks[black][square] = mask_pawn_attacks(black,square);

        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
    }

}

// magic bitboard
U64 set_occupancy(int index, int bits_in_mask , U64 attack_mask){

    U64 occupancy = 0ULL;

    for (int count = 0; count < bits_in_mask; count++){
        
        int square = get_lsf_bit_index(attack_mask);

        unset_bit(attack_mask,square);


        if (index & (1 << count))
            occupancy |= (1ULL<< square);
        
    }

    return occupancy;

}


// MAGIC *******************

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

// slider piece attack tables

void init_sliders_attacks(int bishop){

    for (int square = 0 ; square < 64; square++){
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];


        // init relevant bit count 
        int relevant_bits_count = count_bits(attack_mask);

        // occupancy indices
        int occupancy_indices = 1 << relevant_bits_count;


        // loop over occupancy indices

        for (int i = 0; i < occupancy_indices; i++){
            if (bishop){
                U64 occupancy = set_occupancy(i,relevant_bits_count,attack_mask);

                int magic_index =( occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);


                // bishop attacks

                bishop_attacks[square][magic_index] = gen_bishop_attacks(square,occupancy);
            }
            else{

                U64 occupancy = set_occupancy(i,relevant_bits_count,attack_mask);

                int magic_index =( occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);


                // bishop attacks

                rook_attacks[square][magic_index] = gen_rook_attacks(square,occupancy);
                    
            }

        }
    }


}

// get bishop attacks 

static inline U64 get_bishop_attacks(int square, U64 occupancy){

    // get bishop attacks for current board layout
    occupancy &= bishop_masks[square];
    occupancy*= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];


    return bishop_attacks[square][occupancy];

}

// get rook attacks 

static inline U64 get_rook_attacks(int square, U64 occupancy){

    // get rook attacks for current board layout
    occupancy &= rook_masks[square];
    occupancy*= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];

    return rook_attacks[square][occupancy];

}



// get queen attacks

static inline U64 get_queen_attacks(int square, U64 occupancy){

    // combine bishop and rook attacks, to get queen attacks

    U64 result = 0ULL;

    U64 bishop_occupancy = occupancy;

    U64 rook_occupancy= occupancy;


    bishop_occupancy &= bishop_masks[square];
    bishop_occupancy*= bishop_magic_numbers[square];
    bishop_occupancy >>= 64 - bishop_relevant_bits[square];

    result = bishop_attacks[square][bishop_occupancy];

    // get rook attacks for current board layout
    rook_occupancy &= rook_masks[square];
    rook_occupancy*= rook_magic_numbers[square];
    rook_occupancy >>= 64 - rook_relevant_bits[square];

    result |= rook_attacks[square][rook_occupancy];

    return result;

}


// Init all prerequisite functions 

void init_all(){
    init_leaper_attacks();
    initCharPieces();


    // slider piece attacks
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);

    std::cout << "init all" << "\n";
    

    

}

// is square attacked?

static inline int is_square_attacked(int square, int side){

    // LEAPER
    // attacked by pawns
    if ((side == white) && (pawn_attacks[black][square]) & bitboards[P] )return 1;
    if ((side == black) && (pawn_attacks[white][square]) & bitboards[p] )return 1;

    // attacked by knight
    if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;
    
    // attacked by king
    if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

    // SLIDER - have to make sure no blocking pieces
    // attacked by bishop

    if (get_bishop_attacks(square,occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b]) ) return 1;

    // attacked by rook

    if (get_rook_attacks(square,occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r]) ) return 1;

    // attacked by queen

    if (get_queen_attacks(square,occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q]) ) return 1; 
        
    return 0;
}



// MOVE GENERATION ****************************

// encode move into hexidecimal

#define encode_move(source,target,piece,promoted,capture,double, enpassant, castling) \
    ( ((source) & 0x3F)               | \
      (((target) & 0x3F) << 6)        | \
      (((piece) & 0x0F) << 12)        | \
      (((promoted) & 0x0F) << 16)     | \
      ((capture) << 20)               | \
      ((double) << 21)           | \
      ((enpassant) << 22)            | \
      ((castling) << 23) )

// extracting info from decoded move
#define get_move_source(move) (move & 0x3f)
#define get_move_target(move) ((move & 0xfc0) >> 6)
#define get_move_piece(move) ((move & 0xf000) >> 12)
#define get_move_promoted(move) ((move & 0xf0000) >> 16)
#define get_move_capture(move) ((move & 0x100000))
#define get_move_double(move) ((move & 0x200000))
#define get_move_enpassant(move) ((move & 0x400000))
#define get_move_castling(move) ((move & 0x800000))


// move list structure
typedef struct {
    int move[256];

    int count;

} moves;

// add move to move list

void add_move(moves *move_list, int move){

    //store move
    move_list->move[move_list->count] = move;

    // increment move count
    move_list->count++;

}



// print move
void print_move(int move){
    std::cout << square_to_coordinates[get_move_source(move)] <<  square_to_coordinates[get_move_target(move)]  << promoted_pieces[get_move_promoted(move)] ;
}

// print move list
void print_move_list(moves *move_list){


    if (!move_list->count){
        return;
    }

    std::cout << "\nmove promoted piece capture double enpassant castling \n";

    for (int move_count = 0; move_count < move_list->count; move_count++){
        int move = move_list->move[move_count];
        std::cout << square_to_coordinates[get_move_source(move)] 
        <<  square_to_coordinates[get_move_target(move)] << "     "  
        << (get_move_promoted(move) ? promoted_pieces[get_move_promoted(move)] : ' ') << "     "  
        << unicode_pieces[get_move_piece(move)] << "     "  
        << (get_move_capture(move) ? 1 : 0) << "        "  
        << (get_move_double(move) ? 1 : 0  )<< "       "  
        <<( get_move_enpassant(move) ? 1 : 0 ) << "       "  
        << (get_move_castling(move) ? 1 : 0 )  << "\n"  ;
    

    
    }
    std::cout<< "move count: " <<  move_list->count << "\n";
    
}


#define copy_board()                                    \
    U64 bitboards_copy[12], occupancies_copy[3];       \
    int side_copy, en_passant_copy, castle_copy;       \
    memcpy(bitboards_copy, bitboards, 96);             \
    memcpy(occupancies_copy, occupancies, 24);         \
    side_copy = side;                                  \
    en_passant_copy = en_passant;                      \
    castle_copy = castle;

#define take_back()                                     \
    memcpy(bitboards, bitboards_copy, 96);             \
    memcpy(occupancies, occupancies_copy, 24);         \
    side = side_copy;                                  \
    en_passant = en_passant_copy;                      \
    castle = castle_copy;



enum {all_moves, only_captures};


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
                    break;
                }



            }
        }


        // pawn promotions
        if (promoted){
            unset_bit(bitboards[(side == white) ? P : p ],target);
            set_bit(bitboards[promoted],target);
        }
       

        // en passant
        if (en_pass){
            // remove pawn being taken
            (side == white) ? unset_bit(bitboards[p], target + 8) :
                              unset_bit(bitboards[P], target - 8);
        }
    
    en_passant = no_sq;
    return 1;

    }
    // capture  moves
    else{
            if (get_move_capture(move)){
                //recursive call
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
                            add_move(move_list, encode_move(source,target,piece,r,1,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,q,1,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,n,1,0,0,0));
                            add_move(move_list, encode_move(source,target,piece,b,1,0,0,0));

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
                            add_move(move_list, encode_move(e8,f8,piece,0,0,0,0,1));

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

// print attacked squares

void print_attacked_squares(int side){

    // for each square
    for (int rank = 0; rank <8;rank++){
        for (int file = 0; file < 8; file++){

            int square = rank*8+file;


            if(!file){
                std::cout<< 8- rank << "  ";
            }
            // if current square is being attacked 1 else 0
            std::cout << (is_square_attacked(square,side) ? 1 : 0) << " ";
        }

        std::cout<< "\n";
    }

    std::cout << "\n"<<"   a b c d e f g h" << "\n""\n";
}





 // MAIN **************

int main(){

    init_all();

    parse_fen("8/8/8/8/2pPN3/8/8/8 b - d3 0 1 ");
    print_board();


    moves move_list[1];

    generate_moves(move_list);


    for (int i = 0; i < move_list->count; i++){
        int move = move_list->move[i];

        copy_board();

        make_move(move,all_moves);
        print_board();
        getchar();

        take_back();
        print_board();
        getchar();

    }


    return 0;
}