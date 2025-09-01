
#include "random.h"
#include "global.h"

// XORSHIFT32 algorithm
unsigned int GenRandom::get_random_number_U32(){
    unsigned int x = random_state;

    x^= x << 13;
    x^= x >> 17;
    x^= x << 5;


    random_state = x;

    return x;
}

U64 GenRandom::get_random_number_U64(){
    
    U64 n1,n2,n3,n4;

    n1 = ((U64)(get_random_number_U32()) & 0xFFFF);
    n2 = ((U64)(get_random_number_U32()) & 0xFFFF);
    n3 = ((U64)(get_random_number_U32()) & 0xFFFF);
    n4 = ((U64)(get_random_number_U32()) & 0xFFFF);

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);

}