#include "util/util.h"
#include "x86/main.h"

#include "util/bitpool.h"

struct config_t{

};


void config_init(int rank, config_t& confg){

}


// down call
static inline void xsyscall(uint32_t* systemcallmmio, uint32_t infnum,  uint32_t inarg1, uint32_t inarg2, uint32_t inarg3){

  uint32_t fnum=infnum;
  uint32_t arg1=inarg1;
  uint32_t arg3=inarg3;
  uint32_t arg2=inarg2;

  systemcallmmio[2]=arg1;
  systemcallmmio[3]=arg2;
  systemcallmmio[4]=arg3;
  systemcallmmio[1]=fnum; //write this field at the end.

  hoh_debug("Shell Before making system call");
  asm volatile("int $0x48":::"memory");
  hoh_debug("Shell After making system call");

  hoh_assert(systemcallmmio[1]==0,"XXX");
  // uint32_t ret1=systemcallmmio[2];
  // uint32_t ret2=systemcallmmio[3];
  // uint32_t ret3=systemcallmmio[4];

  //..... print return values
}


struct core_t{
public:
  uint32_t    syscallmmio[16];
  core_t*     self;
  int         rank;
  addr_t      main_stack;
  addr_t      main_stackbegin;
  size_t      main_stacksize;
  addr_t      main_stackend_cached;

  size_t      magic;  //=0xface600d;
  bitpool_t   pool4K;
  apps_t      apps;
  config_t    config;

public:
  core_t(int trank,const bitpool_t& tpooltmp, addr_t tstack, size_t tstacksize, const config_t& tconfig) : 
    self(this),
    rank(trank),
    main_stackbegin(tstack), // XXX: reset the pool too
    main_stacksize(tstacksize),
    main_stackend_cached(tstack+tstacksize),
    pool4K(tpooltmp),
    config(tconfig),
    apps()
  {
    main_stack = addr_t(0xfacebaad); //to tell you that their value is random
    magic=0xface600d;

    hoh_debug("Ring3 core: "<<rank<<" "<<uintptr_t(self));
  }
};



extern "C" void core_mem_init(int rank, addr_t masterro, addr_t masterrw, addr_t sharedrw, addr_t& ret_stack, addr_t& ret_core){
  hoh_debug("Shell core_mem_init");
  enum { PAGE_SIZE=4<<10 };
  bitpool_t pool_tmp(uint32_t(PAGE_SIZE),masterrw);

  //addr_t from = masterrw         + PAGE_SIZE*2;
  //addr_t to   = masterrw+(4<<20) - PAGE_SIZE;

  //for(addr_t p=from; p<to; p+=pool_tmp.datasize()){
  //  pool_tmp.add_mem(p,p+pool_tmp.datasize());
  //}
  //hoh_debug("Shell Pool Size: "<<pool_tmp.remaining()<<" Pool="<<uint32_t(&pool_tmp));
  //hoh_assert(pool_tmp.remaining() > 10, "Not enough memory");

  //stack
  //hoh_assert(canalloc(pool_tmp),"XXX");
  //auto stack = alloc(pool_tmp);
  //auto stacksize=pool_tmp.datasize();

  ////core
  //hoh_debug("size of core="<<sizeof(core_t));
  //hoh_assert(sizeof(core_t)<pool_tmp.datasize(),"XXX" << std::hex << ": "<<sizeof(core_t));
  //hoh_assert(canalloc(pool_tmp),"XXX");

  config_t config;
  config_init(rank, config);

#if 0
  core_t* pcore = allocT<core_t>(pool_tmp, rank, pool_tmp, stack, stacksize, config);

  ret_core   = addr_t(pcore);
  ret_stack  = pcore->main_stackbegin+pcore->main_stacksize-8;
#else

  core_t* core =  (core_t*)masterrw;
  addr_t  stack=  masterrw + PAGE_SIZE * 500;
  size_t stacksize=PAGE_SIZE;
  new ((void*)core) core_t(rank, pool_tmp, stack, stacksize, config);

  hoh_debug("Hello, from ring3 shell!");
  ret_stack  = core -> main_stackbegin + core-> main_stacksize - 16;
  ret_core   = addr_t(core);

  hoh_debug(" Core: "<<uintptr_t(ret_core));
#endif
}


extern "C" void core_reset(int rank, core_t& core){
  hoh_debug(rank<<": ring3 core_reset begin: "<< uintptr_t(&core));
  hoh_assert(rank == core.rank, "XXX:"<<rank<<","<<core.rank);

  apps_reset(core.rank, core.apps, core.pool4K);
}


extern "C" void core_loop(int rank, core_t* pcore){
  core_t& core=*pcore;
  hoh_assert(rank == core.rank, "XXX");
  hoh_debug(rank<<": core_loop begin");


  
  hoh_debug("SYSTEM CALL TESTING STARTS HERE");
  // mapmmio
  xsyscall(pcore->syscallmmio,7,1<<30,0,0);
  int* addr = (int *)(1<<30);
  *(addr) = 4; // therefore mapped in user space. Check with 

  // ALLOCATING A NEW PAGE
  xsyscall(pcore->syscallmmio,9,0,0,0);
  int *v1=(int *)(pcore->syscallmmio[3]);
  xsyscall(pcore->syscallmmio,9,0,0,0);
  int *v2=(int *)(pcore->syscallmmio[3]);

  *v1 = 5;
  *v2 = 10;

  hoh_debug("[v1] "<<*v1);
  hoh_debug("[v2] "<<*v2);

  // mswap
  xsyscall(pcore->syscallmmio,6,(int)v1,(int)v2,0);

  hoh_debug("[v1] "<<*v1);
  hoh_debug("[v2] "<<*v2);

  // mmio_write
  xsyscall(pcore->syscallmmio,3,32,1<<29,11);

  // mmio_read
  xsyscall(pcore->syscallmmio,2,32,1<<29,0);
  int read = pcore->syscallmmio[3];
  hoh_debug("read  = "<<read);

  // To be done later
  // io_write
  // read = 99;
  // xsyscall(pcore->syscallmmio,4,32, 1 ,2);
  // io_read
  // xsyscall(pcore->syscallmmio,2,32, 1 ,0);
  // read = pcore->syscallmmio[3];
  // hoh_debug("read  = "<<read);

  // exit system call
  
  // xsyscall(pcore->syscallmmio,1,0,0,0);


  apps_loop(core.rank, &core.main_stack, &core.apps);
}


