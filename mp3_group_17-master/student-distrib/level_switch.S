# level_switch contain the assmble code of context switch
# vim:ts=4 noexpandtab

#define ASM     1
#include "level_switch.h"

.data
    # data for user and kernal information from x86_desc.h
    USER_DS = 0x2B
    USER_CS = 0x0023
    STACK_BOTTOM_USER=0x83FFFFC #128mB + 4mB -4B

.global context_switch

# context_switch
# DISCRIPTION: the assembly code for context switch
# INPUT: entry bits[24:27]
# OUTPUT: NONE
# RETURN VALUE: NONE
# SIDE EFFECTS: NONE
context_switch:
    cli
    pushl %ebp
    movl %esp, %ebp     # set ebp

    movl 8(%ebp),%edx  #store entry
    #movl %ebx,%edx
    pushl $USER_DS  #push ds
    pushl $STACK_BOTTOM_USER # push esp

    pushfl      #get eflag
    popl  %eax
    orl   $0x200, %eax #enables interrupt bit in eflags   
    
    pushl %eax #puah flag

    pushl $USER_CS

    pushl %edx  #push entry

    iret




