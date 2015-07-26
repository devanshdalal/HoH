#include "labs/coroutine.h"

void g_kthprime(coroutine_t* pf_coro,f_t* pf_locals,int* pret,bool* pdone){             /// MY KTH PRIME 
	coroutine_t& f_coro = *pf_coro; // boilerplate: to ease the transition from existing code
	int& ret            = *pret;
	bool& done          = *pdone;
	int& k              = pf_locals->k;
	int& n              = pf_locals->n;
	int& i              = pf_locals->i;

	h_begin(f_coro);

	for ( n=2; k>0 ;n++){
        bool prime=1;
        for (i = 2; i*i<=n ; ++i){
            if (n%i==0){
                prime=0;
                break;
            }
        	done=false; h_yield(f_coro);
        }
        if (prime){
        	k--;
        	if(k==0)break;
        }
        ret=n;
        // ret=n; done=false; h_yield(f_coro);
    }
    ret=ret+1; done=true; h_end(f_coro);
}

void shell_step_coroutine(shellstate_t& shell, coroutine_t& f_coro, f_t& f_locals){
	////  INITIALIZING ON COROUTINECALL 
    if (shell.f_call==1)
	{
		shell.f_call=2;
		f_locals.i=f_locals.n=2;
		coroutine_reset(f_coro);
		f_locals.k=shell.f_inp;
	}

    ///// JUMPING TO F
	if(!shell.f_done){
	    g_kthprime(&f_coro,&f_locals,&shell.f_ret,&shell.f_done);
	}
}

