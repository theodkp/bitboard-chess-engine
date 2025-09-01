#pragma once

class GenRandom{

public:
    GenRandom() : random_state() {} 

    unsigned long long get_random_number_U64();


private:
    unsigned int get_random_number_U32();
    unsigned int random_state = 1804289383;


};