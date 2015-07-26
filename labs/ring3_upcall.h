#pragma once

#include "util/config.h"


//
// upcall
//
//
//

#define _ring3_upcall(_name, _f)                  \
         __asm(                                   \
         "  .text                            \n\t"\
         " " STR(_name) ":                   \n\t"\
         "                                   \n\t"\
         "                                   \n\t"\
         "                                   \n\t"\
         "  #insert your code here           \n\t"\
         "                                   \n\t"\
         "  pushl %eax                       \n\t"\
         "  pushl %ebx                       \n\t"\
         "  pushl %ecx                       \n\t"\
         "                                   \n\t"\
         "  movl %gs:28,%ebx                 \n\t"\
         "  movl %ebx,%eax                   \n\t"\
         "  addl $1084,%eax                  \n\t"\
         "  movl (%eax),%eax                 \n\t"\
         "                                   \n\t"\
         "  #moving cr2                      \n\t"\
         "  addl $4092,%eax                  \n\t"\
         "  movl %cr2,%ecx                   \n\t"\
         "  movl %ecx,(%eax)                 \n\t"\
         "                                   \n\t"\
         "  #moving oldeip                   \n\t"\
         "  movl 20(%esp),%ecx               \n\t"\
         "  movl %ecx,-4(%eax)                 \n\t"\
         "                                   \n\t"\
         "  #moving oldesp                   \n\t"\
         "  movl 32(%esp),%ecx               \n\t"\
         "  movl %ecx,-8(%eax)                 \n\t"\
         "                                   \n\t"\
         "  #moving errorcode                \n\t"\
         "  movl 16(%esp),%ecx               \n\t"\
         "  movl %ecx,-12(%eax)                 \n\t"\
         "                                   \n\t"\
         "  #moving num                      \n\t"\
         "  movl 12(%esp),%ecx               \n\t"\
         "  movl %ecx,-16(%eax)                 \n\t"\
         "                                   \n\t"\
         "  #moving sharedrw                 \n\t"\
         "  movl 1076(%ebx),%ecx             \n\t"\
         "  movl %ecx,-20(%eax)                 \n\t"\
         "                                   \n\t"\
         "  #moving masterrw                 \n\t"\
         "  movl 1072(%ebx),%ecx             \n\t"\
         "  movl %ecx,-24(%eax)                 \n\t"\
         "                                   \n\t"\
         "  #moving masterro                 \n\t"\
         "  movl 1068(%ebx),%ecx             \n\t"\
         "  movl %ecx,-28(%eax)               \n\t"\
         "                                   \n\t"\
         "  #moving rank                     \n\t"\
         "  movl 1064(%ebx),%ecx             \n\t"\
         "  movl %ecx,-32(%eax)               \n\t"\
         "                                   \n\t"\
         "  #editing stack                   \n\t"\
         "  movl 1080(%ebx),%ecx             \n\t"\
         "  addl $4,%ecx                     \n\t"\
         "  movl %ecx,20(%esp)               \n\t"\
         "  subl $32,%eax                    \n\t"\
         "  movl %eax,32(%esp)               \n\t"\
         "                                   \n\t"\
         "                                   \n\t"\
         "  popl %ecx                        \n\t"\
         "  popl %ebx                        \n\t"\
         "  popl %eax                        \n\t"\
         "                                   \n\t"\
         "                                   \n\t"\
         "                                   \n\t"\
         "  pushl %edx                       \n\t"\
         "  pushl %ecx                       \n\t"\
         "  pushl %eax                       \n\t"\
         "  call " STR(_f) "                 \n\t"\
         "  popl  %eax                       \n\t"\
         "  popl  %ecx                       \n\t"\
         "  popl  %edx                       \n\t"\
         "                                   \n\t"\
         "  jmp iret_toring3                 \n\t"\
         )                                        \


