.global keyboard_interrupt,rtc_handler,pit_interrupt

/* keyboard_interrupt: when keyboard interrupt happen, use keyboard_interrupt to save all register and call keyboard_handler in keyboard.c*/
keyboard_interrupt:
	pushl %eax				#save registers
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx			
    call keyboard_handler
	popl %ebx				#restore registers
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
    iret					#interrupt return



/* rtc_handler: when rtc interrupt happen, use rtc_handler to save all register and call rtc_handler in rtc.c*/
rtc_handler:
	pushl %eax				#save register
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx	
	call rtc_interrupt
	popl %ebx					#restore registers
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
    iret					#interrupt return
	
/* pit interrupt happen, use pit handler to save all register and call pit_handler in pit.c*/
pit_interrupt:
	pushl %eax				#save register
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx	
	call pit_handler
	popl %ebx					#restore registers
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
    iret					#interrupt return
	
	
	.end
