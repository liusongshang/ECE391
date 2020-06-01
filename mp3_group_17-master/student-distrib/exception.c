#include "exception.h"

#include "lib.h"
/*exception handler for divide error*/
void divide_error_exception(){
    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 0: divide_error_exception\n");
    bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for debug exception*/
void debug_exception(){
  
    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 1: debug_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);

}
/*exception handler for nmi interrupt*/
void NMI_interrupt(){
 
    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 2: NMI_interrupt\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for breakpoint exception*/
void breakpoint_exception(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 3: breakpoint_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for overflow exception*/
void overflow_exception(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 4: overflow_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for exceed exception*/
void bound_range_exceed_exception(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 5: bound_range_exceed_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for opcode exception*/
void invalid_opcode_exception(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 6: invalid_opcode_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for device not available*/
void device_not_available_exception(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 7: device_not_available_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for double fault*/
void double_fault_exception(){
 
    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 8: double_fault_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for segment overrun*/
void coprocessor_segment_overrun(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 9:  coprocessor_segment_overrun\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for invalid tss*/
void invalid_TSS_exception(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 10: invalid_TSS_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for segment not present*/
void segment_not_present(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 11: segment_not_present\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for stack fault exception*/
void stack_fault_exception(){
 
    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 12: stack_fault_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for general protection exception*/
void general_protection_exception(){
  
    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 13: general_protection_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for page fault exception*/
void page_fault_exception(){
  
    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 14: page_fault_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for spurious interrupt*/
void spurious_interrupt(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception EXCEPTION_SIGNAL: spurious_interrupt\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for floating point error*/
void x87_FPU_floation_point_error(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 16: x87_FPU_floation_point_error\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for alignment check*/
void alignment_check_exception(){

    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 17: alignment_check_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for machine check*/
void machine_check_exception(){
    
    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 18: machine_check_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}
/*exception handler for simd floatiing point*/
void SIMD_floating_point_exception(){
   
    clear();
    printf("\n\n Your PC ran into a problem.\n");
    printf("\n\n exception 19: SIMD_floating_point_exception\n");
     bluescreen();
    halt(EXCEPTION_SIGNAL);
}


