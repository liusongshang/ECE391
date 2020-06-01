/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
/* 
 *  i8258_init
 *   DESCRIPTION: Initialize the 8259 PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initialize the 8259 PIC
 */
/* Initialize the 8259 PIC */ // intialize pic

void i8259_init(void) {
    //initialize 
    master_mask = 0xff;
    slave_mask =0xff;
    //same as we saw in thw code of init_8259A(int auto)
    //mask all interrupts on pic
    outb(MASK_ALL_INTERUPT,MASTER_8259_PORT+1);
    outb(MASK_ALL_INTERUPT,SLAVE_8259_PORT+1);

    //initialize the master pic
    outb(ICW1, MASTER_8259_PORT);       
    //Thefirst word, ICW1, is delivered to the first PIC port—either 0x20 or 0xA0—and tells the PIC that it is being initialized
	outb(ICW2_MASTER, MASTER_8259_PORT+1);
    //The high bits of theinterrupt vector numbers are provided in ICW2
	outb(ICW3_MASTER, MASTER_8259_PORT+1);
    //The specific IR pin used in the master/slave relationship is specified by ICW3.
	//has a slave on ir2
    outb(ICW4, MASTER_8259_PORT+1);
    //ICW4 specifies the 8086 protocol, normal EOI signalling, and a couple ofother (unused) options that we have ignored here

    //same in the slave
    outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_PORT+1);
	outb(ICW3_SLAVE, SLAVE_8259_PORT+1);
    // IRQ2 in master  
	outb(ICW4, SLAVE_8259_PORT+1);

    //restore all the irq 
    outb(MASK_ALL_INTERUPT,MASTER_8259_PORT+1);
    outb(MASK_ALL_INTERUPT,SLAVE_8259_PORT+1);
    enable_irq(ICW3_SLAVE); // irq2 in master is where slave connect 
}
/* 
 *  enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: uint32_t irq_num
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enable (unmask) the specified IRQ
 */
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    //irq num is 0 to 15
    //master_mask =0;
    //slave_mask=0;
    uint8_t un_mask;
    if(irq_num<0 || irq_num>max_irq_num){
        return;
    }
    // irq in master pic
    if(irq_num >=0 && irq_num<=max_master_irq_num){      // magic number here
        //make irq unmask here
        //low level
        un_mask= ~(1<<irq_num);
        master_mask= master_mask & un_mask;
        outb(master_mask,MASTER_8259_PORT+1);   // unmask before here
        return;
    }    
    if(irq_num>max_master_irq_num){
        irq_num=irq_num-1-max_master_irq_num;//-8
        un_mask= ~(1<<irq_num);
        slave_mask = slave_mask & un_mask;      //unmask before here
        outb(slave_mask,SLAVE_8259_PORT+1);
        return;
    }
    return;
}
/* 
 *  enable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: uint32_t irq_num
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Disable (mask) the specified IRQ
 */
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint8_t mask;
    if(irq_num<0 || irq_num>max_irq_num){
        return;
    }
    // irq in master pic
    if(irq_num >=0 && irq_num<=max_master_irq_num){      // magic number here
        //make irq unmask here
        //low level
         mask= (1<<irq_num);
        master_mask= master_mask | mask;         //mask the irq here
        outb(master_mask,MASTER_8259_PORT+1);
        return;
    }    
    if(irq_num>max_master_irq_num){                 // remmember magic number
        irq_num=irq_num-1-max_master_irq_num;
         mask= (1<<irq_num);             //mask the irq here
        slave_mask = slave_mask | mask;
        outb(slave_mask,SLAVE_8259_PORT+1);
        return;
    }
    return;
}
/* 
 *  send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *   INPUTS: uint32_t irq_num
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Send end-of-interrupt signal for the specified IRQ
 */
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    /* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
    if(irq_num<0 || irq_num>max_irq_num){
        return;
    }
    if(irq_num > max_master_irq_num){ 
      irq_num=irq_num-1-max_master_irq_num; //irq_num-8
        outb(EOI | irq_num, SLAVE_8259_PORT);   // send eoi to slave
        outb(EOI | SLAVE_CONNECT_MASTER, MASTER_8259_PORT);   // remmber to send to master ICW3_SLAVE' value is 2 which is
    }
    else
    { 
        outb(EOI | irq_num, MASTER_8259_PORT);   // has problem here
    }
    
    

}
