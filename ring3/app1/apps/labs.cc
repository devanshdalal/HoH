#include "apps/labs.h"

#include "x86/except.h"

uint32_t f2(uint8_t x, uint8_t y, uint8_t z)
{
  return *(int *)((2u<<30) + (((uint32_t)x)*256u*256u + ((uint32_t)y)*256u + ((uint32_t)z))*4u);
}

uint32_t sum_nbrs1(uint8_t x, uint8_t y, uint8_t z)
{
  uint32_t sum = 0;
  int d = (1<<6),add;
  for (int i = -d; i < d; ++i)
  {
    for (int j = -d; j < d; ++j)
    {
      for (int k = -d; k < d; ++k)
      {
	    add=f2(x+i,y+j,z+k);
        sum += add;
      }
    }
  } 
  return sum;
}

uint32_t sum_nbrs2(uint8_t x, uint8_t y, uint8_t z)
{
  uint32_t sum = 0;
  int d = (1<<6),add;
  for (int i = d-1; i >= -d; --i)
  {
    for (int j = -d; j < d; ++j)
    {
      for (int k = -d; k < d; ++k)
      {
	    add=f2(x+i,y+j,z+k);
        sum += add;
      }
    }
  } 
  return sum;
}

void for_each()
{
  int sum=0;
  for (int i = 0; i <= 255; i+=32)
  {
    for (int j = 0; j <= 255; j+=32)
    {
      for (int k = 0; k <= 255; k+=32)
      {
        sum+=sum_nbrs1(i,j,k);
        // sum+=sum_nbrs2(i,j,k+32);
      }
    }
  }
  hoh_debug("SUM="<<(uint32_t)sum);
}
//
// app step
//
static void apps_loop_step(int rank, addr_t& main_stack, apps_t& apps, uint32_t* systemcallmmio){
  hoh_debug("apps_loop_step");
  for_each();
  uint32_t ret1;
  uint32_t ret2;
  uint32_t ret3;
  xsyscall(systemcallmmio, 0x1, 0,0,0, ret1,ret2,ret3);
}



//
// reset
//
extern "C" void apps_reset(int rank, apps_t& apps, bitpool_t& pool4k, uint32_t* systemcallmmio){


  asm volatile ("nop ");


}


//
// main loop
//
extern "C" void apps_loop(int rank, addr_t* pmain_stack, apps_t* papps, uint32_t* systemcallmmio){
  addr_t& main_stack = *pmain_stack;
  apps_t& apps       = *papps;
  uint32_t esp;
  asm volatile ("mov %%esp,%0":"=r"(esp)::);
  for(;;){
    apps_loop_step(rank, main_stack, apps, systemcallmmio);
  }

}
