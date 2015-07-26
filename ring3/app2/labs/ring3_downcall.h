#pragma once


#include "util/ring3.h"
#include "devices/lapic.h"
#include "util/bitpool.h"

static inline void ring3_downcall(process_t& proc, dev_lapic_t& lapic, bitpool_t& pool4M){

  uint32_t* systemcall_mmio = reinterpret_cast<uint32_t*>(proc.masterrw);
  uint32_t fnum             = systemcall_mmio[1];  //read fnum first.

  if(fnum==0){ //make sure you check fnum.
    return;
  }

  uint32_t farg1=systemcall_mmio[2];
  uint32_t farg2=systemcall_mmio[3];
  uint32_t farg3=systemcall_mmio[4];


  hoh_debug("Systemcall: "<<fnum<<": "<<farg1<<","<<farg2<<","<<farg3);

  uint32_t fret1=0;
  uint32_t fret2=0;
  uint32_t fret3=0;

  switch(fnum){
  default:{
			hoh_debug("Invalid system call. Killing the application");
			//proc.state=PROC_DONE;
			//free resources
			proc.state=1;
			for(uint32_t i = 0; i < 1024 ; i++){
				if( proc.mmu.get_flags(i)&0x4 )
					free(pool4M,(addr_t)(0xfffff000&proc.mmu.get(i)));
			}
          }break;
  case 0: { //nop: don't do anything
            fret1=1;
          }break;
  case 1: { //done
            proc.state=1;
            for(uint32_t i = 0 ; i < 1024 ; i++){
                if( (proc.mmu.get_flags(i) & 0x4) !=0)
                  free(pool4M,(addr_t)(proc.mmu.get(i)&0xfffff000));
            }
            fret1=1;
          }break;
  case 2: {  //mmio_read
          	switch(farg1){
          		case 8:
                fret2=mmio::read8((addr_t)farg2,0);
          			fret1=1;
          			break;
          		case 16:
                fret2=mmio::read16((addr_t)farg2,0);
          			fret1=1;
          			break;
          		case 32:
                fret2=mmio::read32((addr_t)farg2,0);
                fret1=1;
                break;
              default:
                fret1=0;
                hoh_debug("WARNING: SYSCALL FAILED");
            }
          }break;
  case 3: {  //mmio_write
        switch(farg1){
              case 8:
                mmio::write8((addr_t)farg2,0,farg3);
                fret1=1;
                break;
              case 16:
                mmio::write16((addr_t)farg2,0,farg3);
                fret1=1;
                break;
              case 32:
                mmio::write32((addr_t)farg2,0,farg3);
          			fret1=1;
          			break;
          		default:
                fret1=0;
                hoh_debug("WARNING: SYSCALL FAILED");
          	}
          }break;
  case 4: {  //io_read
  			switch(farg1){
          		case 8:
                fret2=io::read8((uint32_t)farg2,0);
          			fret1=1;
          			break;
          		case 16:
                fret2=io::read16((uint32_t)farg2,0);
          			fret1=1;
          			break;
          		case 32:
					fret2=io::read32((uint32_t)farg2,0);
          			fret1=1;
          			break;
          		default:
                fret1=0;
                hoh_debug("WARNING: SYSCALL FAILED");
          	}
          }break;
  case 5: {  //io_write
  			switch(farg1){
          		case 8:
          			fret1=1;
          			io::write8((uint32_t)farg2,0,farg3);
          			break;
          		case 16:
          			fret1=1;
          			io::write16((uint32_t)farg2,0,farg3);
          			break;
          		case 32:
          			fret1=1;
          			io::write32((uint32_t)farg2,0,farg3);
          			break;
          		default:
                fret1=0;
                hoh_debug("WARNING: SYSCALL FAILED");
          	}
          }break;
  case 6: {  //mmu_swapva
             //align va1 and va2 using prev_align
             //swap using: proc.mmu.get/set()
            if ((2<<30)<=farg1 and farg1<(3<<30) and (2<<30)<=farg2 and farg2<(3<<30))
            {
            	proc.mmu.swap((uint32_t)prevalign(addr_t(farg1),4<<20)>>22,(uint32_t)prevalign(addr_t(farg2),4<<20)>>22);
            	fret1=1;
            }else{
            	fret1=0;
            }
          }break;
  case 7: {  //mmu_mapmmio
             //align va using prev_align
             //proc.mmu.map
			if ((farg1<(3<<30) and farg1>=(2<<30))) // invalid if it is in VA_RANGE 
			{
				fret1=0; // syscall failed
			}else{
				proc.mmu.map_large(prevalign(addr_t(farg1),4<<20),prevalign(addr_t(farg1),4<<20),proc.mmu.get_flags(farg1>>22)|4,1);
				fret1=1;  // syscall passed.
			}
          }break;
  case 8: {  //mmu_mappmio
  			proc.iopl=3; //and make sure in your ring3_step you respect this variable
          }break;
  case 9: {  //addr_t x=alloc(pool4M);
             //unused_page = 0;
             //unused_page = find a page in VA_RANGE. ie: [ (0x2<<30) , (0x3<<30) )
             //if you're able to find a page: 
             //   proc.mmu.map_large(unused_page,x,0x87);
			addr_t x;
			bool unused_page=0;
			fret1=0;
			for (uint32_t i = 2<<30; i < (3<<30); i+=(4<<20))
			{
				if ( (proc.mmu.m_page[i>>22]&4) == 0 )
				{
					if (!pool4M.canalloc())break;
					x = alloc(pool4M);
					// if (x==0)break;  // not enough resources
					unused_page=fret1=1;
					proc.mmu.map_large( (addr_t)i, x ,0x87,1);
					fret2=i;
					break;
				}
			}
			if (!unused_page)
			{
				hoh_debug("MEM ERROR: FREE PAGE IS NOT AVAILABLE");
			}
	   }break;
  }


  if(fnum!=0){
    // do not modify the arguments if fnum is zero.
    systemcall_mmio[2]=fret1;
    systemcall_mmio[3]=fret2;
    systemcall_mmio[4]=fret3;
    systemcall_mmio[1]=0; //modify this last. ZERO means systemcall done.
  }

  hoh_debug("Systemcall: "<<fnum<<": "<<farg1<<","<<farg2<<","<<farg3<<" RESULT: "<<fret1<<","<<fret2<<","<<fret3);

}

