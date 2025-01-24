#include "attacks.h"
#include "search.h"
#include "uci.h"
#include "eval.h"


 // MAIN **************

int main()
{
    init_leaper_attacks();
    initCharPieces();


    // slider piece attacks
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);    

    init_random_keys();
    init_evaluation_masks();
    uci_loop();
    clear_hash_table();


    return 0;
}