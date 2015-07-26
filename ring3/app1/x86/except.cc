#include "x86/except.h"
#include "x86/main.h"
#include "util/util.h"






extern "C" void handle_user_fault(usertrapframe_t& x){

  hoh_debug("user fault: rank="<<x.rank<<","<<x.masterro<<","<<x.masterrw<<","<<x.sharedrw
                               <<",num="<<x.num<<",ec="<<x.errorcode<<",esp="<<x.old_esp<<",eip="<<x.old_eip
                               <<",cr2="<<x.cr2);

  core_t* pcore=(core_t*) x.masterrw;
  core_t& core = *pcore;

  uint32_t new_page = (uint32_t)prevalign((addr_t)x.cr2,4<<20);
  uint32_t next_page = new_page + (4<<20);
  uint32_t address = new_page;
  
  uint32_t ret1;
  uint32_t ret2;
  uint32_t ret3;

  if(pcore->apps.count==4){
    pcore->apps.head++;
    pcore->apps.head%=4;
    xsyscall(pcore->syscallmmio, 0x6, new_page, pcore->apps.fifo[pcore->apps.head],0, ret1,ret2,ret3);
    pcore->apps.fifo[pcore->apps.head]=new_page;
  }
  else{
    pcore->apps.head++;
    pcore->apps.head%=4;
    xsyscall(pcore->syscallmmio, 0x9, 0, 0, 0, ret1,ret2,ret3);
    hoh_debug("ret2 " << ret2);
    xsyscall(pcore->syscallmmio, 0x6, new_page, ret2,0, ret1,ret2,ret3);
    pcore->apps.fifo[pcore->apps.head]=new_page;
    pcore->apps.count++;
  }

  uint32_t x_n = (new_page - (2u<<30))/(256u*256u*4u);
  uint32_t lim = (next_page - (2u<<30))/(256u*256u*4u);
 
  for (int i = x_n; i < lim; ++i)
  {
    for (int j = 0; j < 256; ++j)
    {
      for (int k = 0; k < 256; ++k)
      {
        *(int *)((2<<30)+(i*256u*256u+j*256u+k)*4u) = i+j+k;
      }
    }
  }
  pcore->apps.pf_count++;
  hoh_debug("Count = "<<pcore->apps.pf_count);
}