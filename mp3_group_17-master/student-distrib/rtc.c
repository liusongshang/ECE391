#include "rtc.h"
#include "keyboard.h"
#include "lib.h"
#include "pit.h"
terminal_type* term[3];
/*some code cite from https://wiki.osdev.org/RTC*/
/* 
 *  change_rate(int rate)
 *   DESCRIPTION: change the frequence of the RTC
 *   INPUTS: rate
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if unsuccess, 0 if success
 *   SIDE EFFECTS: change the frequence of the RTC to the rate we set
 */
int change_rate(int rate){
    if((rate < 6) || (rate > MAX_RATE))     
        return -1;                          // the rate less than 6 and bigger than 15 is not available
    cli();                                  // disable interrupts
    outb(NMI_REG_A,RTC_PORT);               // select register A, and disable NMI
    char prev = inb(RTC_VALUE);             // read the current value of register A
    outb(NMI_REG_A,RTC_PORT);               // set the index again (a read will reset the index to register D)
    outb((prev&FIVE_TO_EIGHT) | rate, RTC_VALUE);    //write only our rate to A. Note, rate is the bottom 4 bits.
    sti();                                  // enable interrupts
    return 0;
}

/* 
 *  rtc_init
 *   DESCRIPTION: init the RTC  
 *   INPUTS: nome
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: init the RTC, open the irq8, set the RTC frequence to default 1024 hz
 */
void rtc_init(){
    int i;
    rtc_count = 0;
    //disable_irq(IRQ_8);
    //cli();                             // disable interrupts
    outb(NMI_REG_B,RTC_PORT);           // select register B, and disable NMI
    char prev = inb(RTC_VALUE);         // read the current value of register B
    outb(NMI_REG_B,RTC_PORT);           // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, RTC_VALUE);       // write the previous value ORed with 0x40. This turns on bit 6 of register B
    enable_irq(IRQ_8);                  // rtc IRQ number is 8
    //sti();                              // enable interrupts
    //change_rate(MAX_RATE);             // set the rate to be 2 HZ (default rate)
    for (i=0;i<3;i++){
        term[i] = get_terminal(i);
        term[i]->rtc_freq = 256;
        rtc_flag[i]=-17;    //just some unique number, used for further checking
    }
    
}


/* 
 *  rtc_interrupt
 *   DESCRIPTION: receive the rtc interrupt
 *   INPUTS: nome
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handler the rtc interrupt
 */
void rtc_interrupt(){
    int i;
    cli();                                  //disable interrupts
    outb(NMI_REG_C,RTC_PORT);           	// select register C
    rtc_count = (rtc_count+1)%DEFAULT_FREQ;
    for(i=0;i<3;i++){
        if((rtc_count+1)%  (term[i]->rtc_freq) == 0 ){
            if(rtc_flag[i]!=-17 && (rtc_flag[i]<1024)){
                rtc_flag[i]++;
            }
        }
    }
    inb(RTC_VALUE);                         // just throw away contents
   //test_interrupts();                      // test the rtc interrupt                              
    send_eoi(IRQ_8);                        // send the end of interrupt of irq8(rtc) to pic
    send_eoi(IRQ_SLAVE);                    // send the end of interrupt of irq2(slave) to pic
    sti();
   
}

/* 
 *  rtc_read
 *   DESCRIPTION: do nothing
 *   INPUTS: nome
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: return until interrupt
 */
int rtc_read(int32_t fd, void* buf, int32_t nbytes){
    rtc_flag[scheduled_terminal]--;
    //sti();                          // wait for the interrupt
    while(rtc_flag[scheduled_terminal]<0);
    //cli();
    return 0;
}

/* 
 *  rtc_write
 *   DESCRIPTION: write the frequency to the rtc
 *   INPUTS: nome
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: write the frequency
 */
extern int rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    cli();
    int32_t freq,rate,temp;
    if ((buf == NULL) || (nbytes != 4))
        return -1;                                                                  //return -1 for error if buf has nothing
    freq = *((int32_t*) buf);
    temp = *((int32_t*) buf);
    if( freq < MIN_FREQ || freq > DEFAULT_FREQ)
        return -1;                                                                  //return -1 for error if the frequence is not availbale
    rate = 0;
    freq = TWO_POWER_16 / freq;
    while(freq != 1){
        if((freq % 2)!= 0)
            return -1;                                                              //if it's not power of 2, return -1 for error
        else
            freq /= 2;
            rate++;
    }  
    //change_rate(rate);
    term[scheduled_terminal]->rtc_freq=DEFAULT_FREQ/temp;
    sti();
    return 0;    
}

/* 
 *  rtc_open
 *   DESCRIPTION: intialize the frequency   
 *   INPUTS: nome
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:intialize the frequency
 */
extern int rtc_open(const uint8_t* filename){
    //change_rate(MAX_RATE);
    cli();
    term[scheduled_terminal]->rtc_freq = 1024;
    rtc_flag[scheduled_terminal] = 0;
    sti();
    return 0;
}

/* 
 *  rtc_close
 *   DESCRIPTION: DO NOTHING
 *   INPUTS: nome
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:do nothing
 */
extern int rtc_close(int32_t fd){
    return 0;
}

