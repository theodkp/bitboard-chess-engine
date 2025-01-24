#pragma once
#include <iostream>
#include <sstream>
#include <string>


#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "


int parse_move(const char* move_as_string);
void parse_position(const char* command);
void parse_go(char* command);
void uci_loop();