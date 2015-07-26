#pragma once

#include "util/elf.h"
#include "util/ring3.h"
#include "util/bitpool.h"
#include "devices/mmu32.h"
#include "devices/gdt32.h"
#include <stddef.h>
#include <algorithm>


static inline void elf_load(addr_t from, size_t fromsize, process_t& proc, bitpool_t& pool4M){
	enum {PAGE_SIZE=4<<20};
	hoh_assert(elf_canread_prog_header(from,fromsize), "XXX");
	const Elf32_Ehdr& header      = *(Elf32_Ehdr*)(from);
	uint32_t prog_offset          = header.e_phoff;
	const Elf32_Phdr* prog_header = (Elf32_Phdr*)(from+prog_offset);
	uint32_t    prog_num          = header.e_phnum;

	elf_dump(from, fromsize);

	size_t numpages = elf_numpages(prog_header,prog_num);
	hoh_assert(numpages == 1, "XXX");

	addr_t to = alloc(pool4M); 

	// 
	// insert your code here
	//
	hoh_assert( *(uint32_t *)from==0x464c457f ,"Magic number test failed");  // checked although 

	for(int i=0;i<prog_num; i++){
		if(prog_header[i].p_type != PT_LOAD){
		  continue;
		}

		// Check if elf is well formed or not
		hoh_assert( prog_header[i].p_offset>=0  ,"p_offset should be in valid range");

		hoh_assert( prog_header[i].p_filesz <= prog_header[i].p_memsz   ,"p_filesz should be less than memsz");
		hoh_assert( prog_header[i].p_filesz >=0   ," p_filesz should be positive ");
		hoh_assert( prog_header[i].p_offset + prog_header[i].p_filesz <= fromsize  ," p_filesz should be positive ");
		hoh_assert( prog_header[i].p_vaddr >=0   ," p_vaddr should be positive ");
		
		hoh_assert( prog_header[i].p_vaddr + prog_header[i].p_memsz <= PAGE_SIZE   ,"elf file should lie in one page only");
		hoh_assert( prog_header[i].p_vaddr % prog_header[i].p_align == prog_header[i].p_offset % prog_header[i].p_align , "Not properly alligned " );
		
		hoh_assert(prog_header[i].p_align==0 or  (prog_header[i].p_align && (!(prog_header[i].p_align&(prog_header[i].p_align-1)))) , "palign 0 or 2^x, x>0" );
		memset( to + prog_header[i].p_vaddr , 0 , prog_header[i].p_memsz  )	; 
		memcpy( to + prog_header[i].p_vaddr , from + prog_header[i].p_offset , prog_header[i].p_filesz  );
	}


	addr_t masterrw = alloc(pool4M);
	hoh_assert( header.e_entry < PAGE_SIZE ,"eip should never point outside page");
	proc.mmu.map_identity();
	proc.mmu.map_large(masterrw,masterrw,0x87,1);
	proc.mmu.map_large(to,to,0x85,1);

	proc.eflags = 0x3200 ;
	proc.masterrw = masterrw;
	proc.rank = 0;
	proc.masterro = 0;
	proc.sharedrw = 0;
	proc.iopl=0;
	proc.eip = (uint32_t)(to + header.e_entry);
	proc.startip = (to + header.e_entry);
	proc.stackend = (masterrw + PAGE_SIZE - 4*1024) ;

	*(addr_t *)(masterrw + PAGE_SIZE - 4 ) = proc.sharedrw;
	*(addr_t *)(masterrw + PAGE_SIZE - 8 ) = proc.masterrw;
	*(addr_t *)(masterrw + PAGE_SIZE -12 ) = proc.masterro;
	*(addr_t *)(masterrw + PAGE_SIZE -16 ) = 0;
	proc.esp = (uint32_t)(masterrw + PAGE_SIZE -16 ) ;
	memset(proc.masterrw,0,0x40);
	hoh_debug("Done loading elf file");
}




static inline void ring3_step(preempt_t& preempt, process_t& proc, dev_lapic_t& lapic){

	//
	//insert your code here
	//
	if (proc.state==1)
	{
		return;
	}
	proc.eflags = (proc.eflags & ~(3u<<12)) | (proc.iopl<<12);
	// proc.mmu.reset();
	asm volatile(														\
		"  pushl %%ebx                \n\t"                             \
		"  movl %%cr3,%%ebx           \n\t"                             \
		"  movl %%ebx,8(%%eax)        \n\t"                             \
		"  popl %%ebx                 \n\t"                             \
		"                             \n\t"                             \
		:                                                               \
		: "a" (&preempt)              									\
		:                                                               \
	);

	preempt.saved_proc =(addr_t) (&proc);
	// lapic.reset_timer_count(10000000);
	asm volatile(                                                         \
		"  cli                        \n\t"                               \
		"                             \n\t"                               \
		"  #trapframe                 \n\t"                               \
		"  pushl $0x23                \n\t"                               \
		"  pushl 1036(%%eax)          \n\t"                               \
		"  pushl 1060(%%eax)          \n\t"                               \
		"  pushl $0x1b                \n\t"                               \
		"  pushl 1056(%%eax)          \n\t"                               \
		"                             \n\t"                               \
		"                             \n\t"                               \
		"                             \n\t"                               \
		"  #loading cr3               \n\t"                               \
		"  movl %%ebx,%%cr3           \n\t"                               \
		"                             \n\t"                               \
		"                             \n\t"                               \
		"  # fxrstr                   \n\t"                               \
		"  fxrstor (%%eax)            \n\t"                               \
		"                             \n\t"                               \
		"  movl 1024(%%eax),%%edi     \n\t"                               \
		"  movl 1028(%%eax),%%esi     \n\t"                               \
		"  movl 1032(%%eax),%%ebp     \n\t"                               \
		"  movl 1040(%%eax),%%ebx     \n\t"                               \
		"  movl 1044(%%eax),%%edx     \n\t"                               \
		"  movl 1048(%%eax),%%ecx     \n\t"                               \
		"  movl 1052(%%eax),%%eax     \n\t"                               \
		"                             \n\t"                               \
		"                             \n\t"                               \
		"  iret                       \n\t"                               \
		"                             \n\t"                               \
		:                                                                 \
		:"a" (&proc) , "b" (&proc.mmu.m_page)                                                  \
		:                                                                 \
	);                                                                    \

}

static inline void ring3_step_done(process_t& proc, dev_lapic_t& lapic){
  
	//
	//insert your code here
	//

	lapic.reset_timer_count(0);

}



