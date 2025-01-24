#include "iostream"
#include "search.h"
#include "attacks.h"

// print move
void print_move(int move){
    std::cout << square_to_coordinates[get_move_source(move)] <<  square_to_coordinates[get_move_target(move)]  << promoted_pieces[get_move_promoted(move)];
}

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
std::cout << bitboard;

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


    std::cout << "Hash Key: " << hash_key << "\n";

}


void print_move_scores(moves *move_list)
{
    for (int count = 0; count < move_list->count; count++){
        std::cout<<"     move:";
        print_move(move_list->moves[count]);
        std::cout << "   ";
        std::cout<<"score: "<< score_move(move_list->moves[count]) << "\n";
    }

    std::cout<< "\n";
}


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



// print move list
void print_move_list(moves *move_list){


    if (!move_list->count){
        return;
    }

    std::cout << "\nmove promoted piece capture double enpassant castling \n";

    for (int move_count = 0; move_count < move_list->count; move_count++){
        int move = move_list->moves[move_count];
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