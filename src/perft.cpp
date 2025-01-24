#include "perft.h"

#include <cstdint>

#include "bitboard.h"
#include "time.h"
#include "movegen.h"
#include "search.h"
#include "visual.h"

void perft_driver(int depth){

    // base case

    if (depth == 0){
        nodes++;
        return;
    }

    moves move_list[1];

    generate_moves(move_list);

    for (int i = 0; i < move_list->count; i++){
        int move = move_list->moves[i];

        copy_board();

        if (!make_move(move,all_moves)){
            continue;
        }

        perft_driver(depth-1);



        take_back();

    }

}


// perft 
void perft_test(int depth){

    int start = get_time_ms();
    std::cout<< "PERFT TEST BEGIN" << "\n";

    


    moves move_list[1];

    generate_moves(move_list);

    for (int i = 0; i < move_list->count; i++){
        int move = move_list->moves[i];

        copy_board();

        if (!make_move(move,all_moves)){
            continue;
        }

        long cumulative_nodes = nodes;

        

        perft_driver(depth-1);

        long old_nodes = nodes- cumulative_nodes;



        take_back();

        


        // print move 
        print_move(move_list->moves[i]);
        std::cout << "  " << "cur_nodes: " << old_nodes << "\n";

    }
    std::cout <<"elapsed time: " <<  get_time_ms() - start << "\n";

    std::cout << "depth: " << depth << "\n";
    std::cout << "nodes: " << nodes;

}