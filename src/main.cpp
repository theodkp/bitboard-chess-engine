#include "attacks.h"
#include "search.h"
#include "uci.h"
#include "eval.h"
void init_all(){
    init_leaper_attacks();
    initCharPieces();


    // slider piece attacks
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);    

    init_random_keys();
    init_evaluation_masks();
    clear_hash_table();

    

}


 // MAIN **************

int main()
{
   init_all();
   uci_loop();

    return 0;
}