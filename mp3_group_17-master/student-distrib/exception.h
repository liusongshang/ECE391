#ifndef EXCEPTION_H
#define EXCEPTION_H
#include "systemcall.h"
/*to distinguish exception from user call's halt or exception*/
#define exception_signal 23
/*several exception functions*/
void divide_error_exception();
void debug_exception();
void NMI_interrupt();
void breakpoint_exception();
void overflow_exception();
void bound_range_exceed_exception();
void invalid_opcode_exception();
void device_not_available_exception();
void double_fault_exception();
void coprocessor_segment_overrun();
void invalid_TSS_exception();
void segment_not_present();
void stack_fault_exception();
void general_protection_exception();
void page_fault_exception();
void spurious_interrupt();
void x87_FPU_floation_point_error();
void alignment_check_exception();
void machine_check_exception();
void SIMD_floating_point_exception();

#endif



