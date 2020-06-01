.global systemcall_handler

/*systemcall_handler
description: when system call happen, use systemcall_handler to save all register, push parameter into stack and call corresponding function in systemcall.c
input: call number is placed in eax, first arguments in ebx, then ecx and edx
return value: a value of -1 indicates an error, while others indicate some form of success*/

systemcall_handler:
    pushl %ebp      #save registers
    pushl %esi
    pushl %edi

    pushl %edx    #push three arguments in order
    pushl %ecx
    pushl %ebx  
    
    cmpl $1,%eax    #judge if number in eax is less than 1, lead to error 
    jl error
    cmpl $10,%eax      #judge if number in eax is bigger than 10, lead to error 
    jg error

    subl $1,%eax     #make it start from 0 to use for the jumptable
    call *jumptable(,%eax,4)   #call the corresponding function
    jmp finish

error:
    movl $-1,%eax    #a value of -1 indicate an error

finish:
    popl %ebx
    popl %ecx
    popl %edx      #pop arguments to restore stack
    
    popl %edi   
    popl %esi       #restore registers
    popl %ebp

    iret

#jumptable to specific systemcall function
jumptable:
    .long halt,execute,read,write,open,close,getargs,vidmap,set_handler,sigreturn
