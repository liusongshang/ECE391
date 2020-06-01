#include "idt.h"
#include "systemcall_handler.h"
void init_idt(){
    int i=0;
    /*load the IDTR*/
    lidt(idt_desc_ptr);
    for(i=0;i<NUM_VEC;i++){
        idt[i].seg_selector=KERNEL_CS;
        /*for interrupt gate, the four reserved number is 01100*/
        idt[i].reserved4=0;
		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
        /*size of gate is 32 so we set size =1*/
		idt[i].size = 1;			
		idt[i].reserved0 = 0;
			
		idt[i].present = 1;	
        /*for system call handler should set DPL to 3 to make it accessible from user space*/	
		if (i==SYSTEMCALL_ENTRY) {
			idt[i].dpl=3;
		} else {
        /*for hardware interrupt amd exception handle,set DPL to 0 to prevent user level application from calling*/
			idt[i].dpl=0;
		}
        /*the numebr of exception, we use trap gate , the numebr is 01110*/
        if (i==SYSTEMCALL_ENTRY) {
			idt[i].reserved3 = 1;
		} else {
			idt[i].reserved3 = 0;
		}

    }
		/*initialize exception in IDT*/
        SET_IDT_ENTRY(idt[0], divide_error_exception);
	    SET_IDT_ENTRY(idt[1],  debug_exception);
	    SET_IDT_ENTRY(idt[2],  NMI_interrupt);
	    SET_IDT_ENTRY(idt[3],  breakpoint_exception);
	    SET_IDT_ENTRY(idt[4], overflow_exception);
	    SET_IDT_ENTRY(idt[5], bound_range_exceed_exception);
	    SET_IDT_ENTRY(idt[6],invalid_opcode_exception);
	    SET_IDT_ENTRY(idt[7], device_not_available_exception);
	    SET_IDT_ENTRY(idt[8], double_fault_exception);
	    SET_IDT_ENTRY(idt[9],  coprocessor_segment_overrun);
	    SET_IDT_ENTRY(idt[10], invalid_TSS_exception);
	    SET_IDT_ENTRY(idt[11], segment_not_present);
	    SET_IDT_ENTRY(idt[12], stack_fault_exception);
	    SET_IDT_ENTRY(idt[13], general_protection_exception);
	    SET_IDT_ENTRY(idt[14], page_fault_exception);
	    SET_IDT_ENTRY(idt[15], spurious_interrupt);
	    SET_IDT_ENTRY(idt[16], x87_FPU_floation_point_error);
	    SET_IDT_ENTRY(idt[17], alignment_check_exception);
	    SET_IDT_ENTRY(idt[18], machine_check_exception);
	    SET_IDT_ENTRY(idt[19], SIMD_floating_point_exception);
		/*initialize interrupt for pit,rtc and keyboard in IDT*/
		SET_IDT_ENTRY(idt[PIT_ENTRY], pit_interrupt);
        SET_IDT_ENTRY(idt[KEYBOARD_ENTRY], keyboard_interrupt);
	    SET_IDT_ENTRY(idt[RTC_ENTRY],rtc_handler);
		SET_IDT_ENTRY(idt[SYSTEMCALL_ENTRY],systemcall_handler);

}

