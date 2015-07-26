#pragma once
#include "util/config.h"

struct shellstate_t{
	char TAB[80];																/// FOR FAST CONVERSION OF SCANKEY TO ASCII
	int SIZ;
	bool changed;
	int kcount,option;															/// KEY COUNT
	char inp[70],out[70];														/// NORMAL I/O
	int i1,calc;																
	int cstate;																	/// FOR STATE MACHINES
	int f_inp,f_ret,f_call;														/// VARIABLES FOR MANAGING COROUTINES 
	bool f_done;																/// COROUTINE DONE 
	int fibr_call,fibr_ret,fibr_inp;											/// VARIABLES FOR MANAGING FIBERS 
	bool fibr_done;																/// FIBER DONE
	int fs_call[10],fs_ret[10],fs_inp[10],fs_done[10],lmt;						/// ARRAYS FOR FIBER SCHEDULER
	int fsc_c,fsc_i;															/// INPUT OF FIBER SCHEDULER
	int pcount;																	/// TOTAL RUNNING PROCESSES
	
	/// for fiber scheduling.
	char fs_chari[6][30],fs_charo[6][30];										/// FIBER SCHEDULER I/O
	int currfs,fs_len[6];														/// CURR FREE SLOT
	bool slotfr[6];																/// IS ith SLOT FREE
	int p2s[10];																/// FIBER TO SLOT MAP
	int turn;																	/// NEXT FIBER TO RUM 
};

struct renderstate_t{
	// char TAB[300];
	// int SIZ;
	int kcount,option,id;
	char inp[70],out[70];
	int i1;
	int cstate;
	bool changed;
	// int f_inp,f_ret,f_call;
	// bool f_done;
	// int fibr_call,fibr_ret,fibr_inp;
	// bool fibr_done;
	// int fs_call[10],fs_ret[10],fs_inp[10],fs_done[10],lmt;
	// int fsc_c,fsc_i;
	int pcount;

	int fs_call[10];

	/// for fiber scheduling.
	int p2s[10];
	char fs_chari[6][30],fs_charo[6][30];
	int currfs,fs_len[6];
	bool slotfr[6];
};

void shell_init(shellstate_t& state);
void shell_update(uint8_t scankey, shellstate_t& stateinout);
void shell_step(shellstate_t& stateinout);
void shell_render(const shellstate_t& shell, renderstate_t& render);

bool render_eq(const renderstate_t& a, const renderstate_t& b);
void render(const renderstate_t& state, int w, int h, addr_t display_base);

