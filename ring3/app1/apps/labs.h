#pragma once

#include "util/debug.h"
#include "util/bitpool.h"

struct apps_t{
	int fifo[4]; 
	int head=0;
	int count=0;
	int pf_count=0;
};


extern "C" void apps_reset(int rank, apps_t& apps, bitpool_t& pool4k, uint32_t* systemcallmmio);
extern "C" void apps_loop(int rank, addr_t* main_stack, apps_t* apps, uint32_t* systemcallmmio);

void for_each();
uint32_t weightedsum_neighbours(uint8_t x, uint8_t y, uint8_t z);
uint32_t sum_neighbours(uint8_t x, uint8_t y, uint8_t z);
uint32_t w(uint8_t x, uint8_t y, uint8_t z);
uint32_t f2(uint8_t x, uint8_t y, uint8_t z);
uint32_t to_index(uint8_t x, uint8_t y, uint8_t z);