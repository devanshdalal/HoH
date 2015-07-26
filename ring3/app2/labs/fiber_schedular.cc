#include "labs/fiber_schedular.h"



//
// stackptrs:      Type: addr_t[stackptrs_size].  array of stack pointers (generalizing: main_stack and f_stack)
// stackptrs_size: number of elements in 'stacks'.
//
// arrays:      Type: uint8_t [arrays_size]. array of memory region for stacks (generalizing: f_array)
// arrays_size: size of 'arrays'. equal to stackptrs_size*STACK_SIZE.
//
// Tip: divide arrays into stackptrs_size parts.
// Tip: you may implement a circular buffer using arrays inside shellstate_t
//      if you choose linked lists, create linked linked using arrays in
//      shellstate_t. (use array indexes as next pointers)
// Note: malloc/new is not available at this point.
//

/// EULAR PHI FUNCTION  LINK: http://en.wikipedia.org/wiki/Euler's_totient_function
void phi(addr_t* pmain_stack, addr_t* pf_stack, int* pret, bool* pdone,int *pn,preempt_t* preempt){		//// FIRST LONG COMPUTAION TASK
	addr_t& main_stack = *pmain_stack; // boilerplate: to ease the transition from existing code
	addr_t& f_stack    = *pf_stack;
	int& ret           = *pret;
	bool& done         = *pdone;
	int & n            = *pn;

	ret=0;
	for(int i=1;i<n;i++) {
        int j=i;
        for (; j>1 ; --j)
        {
        	if(i%j==0 and n%j==0)break;
        	//done=false;
        	//stack_saverestore(f_stack, main_stack);
        }
        if(j==1) ret++;
        done=false;
		preempt->saved_stack=0;
        stack_saverestore(f_stack, main_stack);
    }
	for(;;){
		preempt->saved_stack=0;
		done=true;
		stack_saverestore(f_stack,main_stack);
	}
}

void kthComposite(addr_t* pmain_stack, addr_t* pf_stack, int* pret, bool* pdone,int *pk,preempt_t* preempt){		/// SECOND LONG COMPUTAION TASK
	addr_t& main_stack = *pmain_stack; // boilerplate: to ease the transition from existing code
	addr_t& f_stack    = *pf_stack;
	int& ret           = *pret;
	bool& done         = *pdone;
	int & k            = *pk;

	int n=1;

	for (; k>0 ;n++){
        bool prime=1;
        for (int i = 2; i*i<=n ; ++i){
            if (n%i==0){
                prime=0;
                break;
            }
            ret=n;	done=false;
            // Not yielding on its will.
        	// stack_saverestore(f_stack,main_stack);
        }
        if (!prime){
            k--;
            if(k==0)break;
        }
        // ret=n;	done=false;
        // Not yielding on its will.        
        // stack_saverestore(f_stack,main_stack);
    }

	preempt->saved_stack=0;
	ret=n;  done=true; 
	stack_saverestore(f_stack,main_stack);

	// for(;;){
	// 	ret=n; done=true; 
	// 	stack_saverestore(f_stack,main_stack);
	// }
}


void shell_step_fiber_schedular(shellstate_t& state, addr_t& main_stack , preempt_t& preempt , addr_t stackptrs[], size_t stackptrs_size, addr_t arrays, size_t arrays_size , dev_lapic_t& lapic ){

	// lapic.reset_timer_count(0);
    //insert your code here
    if (state.fsc_c){													/// INITIALIZING THINGS FOR CALLED FIBERS
		for (int i = 1; i < state.lmt; ++i){
		    if (state.fs_done[i]){
		    	state.fs_done[i]=0;
		    	state.fs_call[i]=2;
		    	state.p2s[i]=state.currfs;
		    	state.currfs=-1;
		    	state.fs_inp[i]=state.fsc_i;
		    	hoh_debug("VAL " << state.fsc_c << " "  << (state.fsc_c==2?"kthComposite":"phi"));
		    	// stack_init6(stackptrs[i],arrays+i*arrays_size/stackptrs_size,arrays_size/stackptrs_size,
		    	//  (state.fsc_c==2?&kthComposite:&phi) , &stackptrs[0] , &stackptrs[i], &state.fs_ret[i] , &state.fs_done[i] , &state.fs_inp[i] , preempt );
		    	stack_init6(stackptrs[i],arrays+i*arrays_size/stackptrs_size,arrays_size/stackptrs_size,
		    	 (state.fsc_c==2?&kthComposite:&phi) , &main_stack , &stackptrs[i], &state.fs_ret[i] , &state.fs_done[i] , &state.fs_inp[i] , &preempt );
		    	break;
		    }
		}
		state.fsc_c=0;
    }
	for (int &i = state.turn,j=1; j < state.lmt; ++j)					/// ROUND ROBIN SCHEDULING OF FIBBERS TO GET RESOURCES (NOTE:i is reference variable)
	{
		i=i%(state.lmt-1)+1;
		if (!state.fs_done[i])
		{
			preempt.saved_stack = &stackptrs[i]; // INITIALIZING SAVED STACK VARIABLE
			lapic.reset_timer_count(100000);
			// stack_saverestore(stackptrs[0] ,stackptrs[i]); 
			stack_saverestore( main_stack ,stackptrs[i]);
			lapic.reset_timer_count(0);
			break;
		}
	}

}
