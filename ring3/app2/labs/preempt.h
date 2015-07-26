#pragma once

#include "util/config.h"


//
// preempt_t : State for your timer/preemption handler
//
// Note:
//  We've one instance of core_t per each core.
//  To access this instance, you need to use %gs:0
//  (The entire kernel doesn't have any global/static variables)
//
// %gs:core_offset_preempt will point to start of preempt_t instance
//
// for example: 
// %gs:0 will return pointer to core_t
// %gs:core_offset_mainstack will return core_t::main_stack
// %gs:core_offset_preempt+0 will return core_t::saved_stack
// %gs:core_offset_preempt+4 will return core_t::saved_proc
// %gs:core_offset_preempt+8 will return core_t::saved_cr3
//
// etc.
//
// See Definition of core_t in x86/main.h
//

struct preempt_t{
  // your data structure, if any
  addr_t* saved_stack; //feel free to change it - provided as an example
  addr_t saved_proc;
  addr_t saved_cr3;
};

//
// 
// This macro is being called from x86/except.cc
//
//
// _name: label name
// _f   : C function to be called 
//        ex: we may have to do send EOI to LAPIC or PIC etc.
//
#  define  _ring0_preempt(_name,_f)            \
  __asm(                                       \
      "  .text                            \n\t"\
      " " STR(_name) ":                   \n\t"\
      "  pushl %edx                       \n\t"\
      "  pushl %ecx                       \n\t"\
      "  pushl %eax                       \n\t"\
      "  call " STR(_f) "                 \n\t"\
      "  popl  %eax                       \n\t"\
      "  popl  %ecx                       \n\t"\
      "  popl  %edx                       \n\t"\
      "                                   \n\t"\
      "  #Data race avoidance             \n\t"\
      "  cmpl $0,%gs:" STR(core_offset_preempt) "  \n\t"\
      "  je iret_toring0                  \n\t"\
      "                                   \n\t"\
      "                                   \n\t"\
      "                                   \n\t"\
      "  #saving General purpose regs     \n\t"\
      "  pushl %eax                       \n\t"\
      "  pushl %ebx                       \n\t"\
      "  pushl %ecx                       \n\t"\
      "  pushl %edx                       \n\t"\
      "  pushl %edi                       \n\t"\
      "  pushl %esi                       \n\t"\
      "  pushl %ebp                       \n\t"\
      "                                   \n\t"\
      "  #Saving FPU                      \n\t"\
      "  subl $512, %esp                  \n\t"\
      "  movl %esp, %eax                  \n\t"\
      "  andl $0xf, %eax                  \n\t"\
      "  movl %esp, %ebx                  \n\t"\
      "  subl %eax, %ebx                  \n\t"\
      "  subl $16, %esp                   \n\t"\
      "  fxsave (%ebx)                    \n\t"\
      "                                   \n\t"\
      "  pushl $1f                        \n\t"\
      "                                   \n\t"\
      "                                   \n\t"\
      "  movl %gs:" STR(core_offset_preempt) ",%eax   \n\t"\
      "  movl %eax,(%eax)                          \n\t"\
      "  movl %esp,(%eax)                          \n\t"\
      "                                             \n\t"\
      "  movl %gs:" STR(core_offset_mainstack) ",%eax   \n\t"\
      "  movl %eax,%esp                           \n\t"\
      "  sti # to enable intrupts on returns \n\t"\
      "  ret                                 \n\t"\
      "                                   \n\t"\
      "                                   \n\t"\
      "1:                                 \n\t"\
      "                                   \n\t"\
      "  #restoring FPU                   \n\t"\
      "  addl $16, %esp                  \n\t"\
      "  movl %esp, %eax                  \n\t"\
      "  andl $0xf, %eax                  \n\t"\
      "  movl %esp, %ebx                  \n\t"\
      "  subl %eax, %ebx                  \n\t"\
      "  addl $512, %esp                  \n\t"\
      "  fxrstor (%ebx)                   \n\t"\
      "                                   \n\t"\
      "  #Restoring General purpose regs  \n\t"\
      "  popl %ebp                        \n\t"\
      "  popl %esi                        \n\t"\
      "  popl %edi                        \n\t"\
      "  popl %edx                        \n\t"\
      "  popl %ecx                        \n\t"\
      "  popl %ebx                        \n\t"\
      "  popl %eax                        \n\t"\
      "                                   \n\t"\
      "  jmp iret_toring0                 \n\t"\
      )                                        \

