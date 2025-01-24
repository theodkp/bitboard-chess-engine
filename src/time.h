#pragma once

#include <cstdio>

extern int is_quit;
extern int moves_to_go;
extern long move_time;
extern long base_time;
extern long increment;
extern long start_time;
extern long stop_time;
extern int is_time_set;
extern int is_stopped;

long get_time_ms();
int input_waiting();
void read_input();
void reset_time_control();

static void communicate()
{
	if (is_time_set == 1 && get_time_ms() > stop_time)
	{
		is_stopped = 1;
	}

	read_input();
}

