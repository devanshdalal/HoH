#pragma once

#include "util/config.h"


//
// preempt_t
//
//
//

#define _ring3_preempt(_name, _f) \
         __asm(                                   \
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
         "  #Insertion starts here           \n\t"\
         "                                   \n\t"\
         "  pushl %eax                       \n\t"\
         "  movl %gs:28,%eax                 \n\t"\
         "  movl %edi,1024(%eax)             \n\t"\
         "  movl %esi,1028(%eax)             \n\t"\
         "  movl %ebp,1032(%eax)             \n\t"\
         "  movl %ebx,1040(%eax)             \n\t"\
         "  movl %edx,1044(%eax)             \n\t"\
         "  movl %ecx,1048(%eax)             \n\t"\
         "  popl %ebx                        \n\t"\
         "  movl %ebx,1052(%eax)             \n\t"\
         "                                   \n\t"\
         "  #saving fpu                      \n\t"\
         "  fxsave (%eax)                    \n\t"\
         "                                   \n\t"\
         "  addl $8,%esp                     \n\t"\
         "  popl 1056(%eax)                  \n\t"\
         "  addl $4,%esp                     \n\t"\
         "  popl 1060(%eax)                  \n\t"\
         "  popl 1036(%eax)                  \n\t"\
         "  addl $4,%esp                     \n\t"\
         "                                   \n\t"\
         "  #loading cr3                     \n\t"\
         "  movl %gs:32,%ebx                 \n\t"\
         "  movl %ebx,%cr3                   \n\t"\
         "                                   \n\t"\
         "  sti                              \n\t"\
         "                                   \n\t"\
         "                                   \n\t"\
         "  movl $0, %ebp                    \n\t"\
         "  pushl %gs:0                      \n\t"\
         "  pushl %gs:4                      \n\t"\
         "  call core_loop                   \n\t"\
         "                                   \n\t"\
         )                                        \



