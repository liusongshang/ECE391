/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

#define FOUR	4
#define FIFTEEN	15
unsigned stored_packet[3];
static unsigned char cur_button;
unsigned long flags;
spinlock_t the_lock = SPIN_LOCK_UNLOCKED;
static int has_ack;
static unsigned long led_status;
// bitmask for the controller display
const static unsigned char seven_seg_info[16] = {0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD,0xED,0x86, 0xEF, 0xAF, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};

int tux_init_func(struct tty_struct* tty);
int tux_setLED_func(struct tty_struct* tty,unsigned long arg);
int tux_buttons_func(struct tty_struct* tty,unsigned long arg);

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned char bit_mask = FIFTEEN;
    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    printk("packet : %x %x %x\n", a, b, c); 
	switch(a){
		case MTCP_RESET: //re-initialize the controller to the same state it was in before the reset
			printk("into MTCP_RESET \n");
			tux_init_func(tty);	//is this enough?
			if(has_ack){
				tux_setLED_func(tty,led_status);
			}
			break;
		case MTCP_ACK:	//when the MTC successfully completes a command.
			printk("into MTCP_ACK	\n");
			has_ack = 1;	
			break;
		case MTCP_BIOC_EVENT:
//Generated when the Button Interrupt-on-change mode is enabled and a button is either pressed or released.
			stored_packet[0] = a;
			stored_packet[1] = b;
			stored_packet[2] = c;
			spin_lock_irqsave(&the_lock,flags);
			b &=  bit_mask;
			c &= bit_mask;
			spin_unlock_irqrestore(&(the_lock),flags);
			cur_button = b|c;	// shrike to one byte button
			spin_lock_irqsave(&(the_lock),flags);
			break;
	}
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
/* 
 * tuxctl_ioctl
 *   DESCRIPTION: the origin ioctl
 *   INPUTS: tty--the device	cmd--which ioctl to implement	
 *   OUTPUTS: none
 *   RETURN VALUE: always return 0 
 *   SIDE EFFECTS:            
 */
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		return tux_init_func(tty);
	case TUX_BUTTONS:
		return tux_buttons_func(tty,arg);
	case TUX_SET_LED:
		return tux_setLED_func(tty,arg);
	case TUX_LED_ACK:
	case TUX_LED_REQUEST:
	case TUX_READ_LED:
	default:
	    return -EINVAL;
    }
}
/* 
 * tux_init_func
 *   DESCRIPTION: Initializes any variables associated with the driver and returns 0
 * 		Assume that any user-level code that interacts with your device will call this ioctl before any others.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: always return 0 
 *   SIDE EFFECTS:            
 */
int tux_init_func(struct tty_struct* tty){
	unsigned char buf[1];
	printk("into tux init function \n");
	has_ack = 0;	//ready for cmd
	buf[0] = MTCP_BIOC_ON;	// enable button interrupt-on-change
	tuxctl_ldisc_put(tty,&buf[0],1);
	buf[0] = MTCP_LED_USR;	// Put the LED display into user-mode.
	tuxctl_ldisc_put(tty,&buf[0],1);
	spin_lock_irqsave(&(the_lock),flags);
	led_status = 0;
	spin_unlock_irqrestore(&(the_lock),flags);
	return 0;
}

/* 
 *   tux_setLED_func
 *   DESCRIPTION: Display the LED message contained by arg
 *   INPUTS: arg-- 32-bit integer. The low 16-bits specify a number whose
 *	 hexadecimal value is to be displayed on the 7-segment displays. 
 *	 The low 4 bits of the third byte specifies which LED’s should be turned on. 
 *	 The low 4 bits of the highest byte (bits 27:24) specify
 *	 whether the corresponding decimal points should be turned on. This ioctl should return 0.
 *   OUTPUTS: none
 *   RETURN VALUE: always return 0 
 *   SIDE EFFECTS:            
 */
int tux_setLED_func(struct tty_struct* tty,unsigned long arg){
//	printk("into setLED function");
	int i;	// counter
	unsigned char led_info[4];
	unsigned char buf[6];
	unsigned long bitmask = 0x000f;	// low 4 bits mask
	int led_turn_on = (arg>>4*FOUR) & bitmask;	// shift to get the led status
	int dp_turn_on = (arg>>6*FOUR) & bitmask;	// get the decimal point status
	if(has_ack==0){
		return 0;
	}
	spin_lock_irqsave(&(the_lock),flags);
	has_ack =0;
	spin_unlock_irqrestore(&(the_lock),flags);
	// low 16 bits, every four bits is a LED message
	led_info[0] =  arg & bitmask;
	led_info[1] = (arg & (bitmask<<FOUR))>>FOUR;
	led_info[2] = (arg&(bitmask<<2*FOUR))>>2*FOUR;
	led_info[3] = (arg&(bitmask<<3*FOUR))>>3*FOUR;
//	printk("arg is %x",arg);
//	buf[0] = MTCP_LED_USR;
//	tuxctl_ldisc_put(tty,&buf[0],1);
	buf[0] = MTCP_LED_SET;
	buf[1] = FIFTEEN;
	bitmask = 0x01;
	for(i=0;i<4;i++){
		if(led_turn_on & (bitmask<<i)){
			buf[i+2] = seven_seg_info[led_info[i]];	//from the 2nd position
			if(dp_turn_on&(bitmask<<i)){
				buf[i+2] = 0x10 | buf[i+2];	// add the digit at fifth 
			}
		}
		else{
			buf[2+i] = 0;
		}
	}

	led_status = arg;	// first save the led status
	//ssprintk("led_status is %x",led_status);
	tuxctl_ldisc_put(tty,buf,6);	// 6 bytes for 2 sets and 4 LEDS
	return 0;
}
/* 
 *  tux_buttons_func
 *   DESCRIPTION: store the button status in arg	low 8 bits
 * 		sets the bits of the low byte corresponding to the currently pressed buttons,
 *   INPUTS: arg-- 32-bit integer(a pointer)
 *   OUTPUTS: none
 *   RETURN VALUE: always return 0 
 *   SIDE EFFECTS:            
 */
int tux_buttons_func(struct tty_struct* tty,unsigned long arg){
	int ret;
	if(arg==0){
		return -EINVAL;
	}
//	unsigned long flags;	// 

//	unsigned char * ptr_button;
//	ptr_button = &(cur_button);
	spin_lock_irqsave(&the_lock,flags);
	ret=copy_to_user((void *)arg,&(cur_button),sizeof(char));
	spin_unlock_irqrestore(&(the_lock),flags);
	if(ret >0 ){
		return -EINVAL;
	}
	else{
		return 0;
	}
}
