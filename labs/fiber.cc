#include "labs/fiber.h"


void f(addr_t* pmain_stack, addr_t* pf_stack, int* pret, bool* pdone,int * kth){				/// kTH PRIME
	addr_t& main_stack = *pmain_stack; // boilerplate: to ease the transition from existing code
	addr_t& f_stack    = *pf_stack;
	int& ret           = *pret;
	bool& done         = *pdone;
	int& k             = *kth;

	int n;

	for ( n=2; k>0 ;n++){
	    bool prime=1;
	    for (int i = 2; i*i<=n ; ++i){
	        if (n%i==0){
	            prime=0;
	            break;
	        }
	    	ret=n; done=false; stack_saverestore(f_stack,main_stack);
	    }
	    if (prime){
	    	k--;
	    	if(k==0)break;
	    }
	    // ret=n; done=false; stack_saverestore(f_stack,main_stack);
	}
	for(;;){
		ret=n;done=true; stack_saverestore(f_stack,main_stack);
	}
}


void shell_step_fiber(shellstate_t& state, addr_t& main_stack, preempt_t& preempt , addr_t& f_stack, addr_t f_array, uint32_t f_arraysize , dev_lapic_t& lapic ){

    ////  INITIALIZING ON FIBERCALL 
    if (state.fibr_call==1)
	{
		state.fibr_call=2;
    	stack_init5(f_stack,f_array,f_arraysize, &f , &main_stack , &f_stack , &state.fibr_ret , &state.fibr_done , &state.fibr_inp);
	}
	///// JUMPING TO F
	if (!state.fibr_done) 
	{
		stack_saverestore(main_stack,f_stack);
	}

}

