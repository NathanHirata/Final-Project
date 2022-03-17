#include <xc.h>
#include <p18f4620.h>
#include "Interrupt.h"
#include "stdio.h"

unsigned char bit_count;
unsigned int Time_Elapsed;

extern unsigned char Nec_state;
extern short nec_ok;
unsigned long long Nec_code;

extern char Nec_code1;
extern char INT2_flag;

void Init_Interrupt(void)
{
                                                     
    INTCONbits.INT0IF = 0 ;                                 // clear int0 flag
    INTCON3bits.INT2IF = 0;                                 // clear int2 flag
    INTCONbits.INT0IE = 1;                                  // enable int0
    INTCON3bits.INT2IE = 1;                                 // enable int2
    INTCON2bits.INTEDG0 = 0;                                // set int0 to falling edge
    INTCON2bits.INTEDG2 = 0;                                // set int2 to falling edge
    TMR1H = 0;                                              // Reset Timer1
    TMR1L = 0;                                              // Timer1 value
    PIR1bits.TMR1IF = 0;                                    // Clear timer 1 interrupt flag
    PIE1bits.TMR1IE = 1;                                    // Enable Timer 1 interrupt
    INTCONbits.PEIE = 1;                                    // Enable Peripheral interrupt
    INTCONbits.GIE = 1;                                     // Enable global interrupts

}

void interrupt  high_priority chkisr() 
{    
    if (INTCONbits.INT0IF == 1) INT0_isr();                 // check if INT0 has occurred
    if (INTCON3bits.INT2IF == 1) INT2_isr();                // check if INT2 has occurred
    if (PIR1bits.TMR1IF == 1) TIMER1_isr();                 // check if INT2 has occurred

}

void TIMER1_isr(void)
{
    Nec_state = 0;                                          // Reset decoding process
    INTCON2bits.INTEDG0 = 0;                                // Edge programming for INT0 falling edge
    T1CONbits.TMR1ON = 0;                                   // Disable T1 Timer
    PIR1bits.TMR1IF = 0;                                    // Clear interrupt flag
}

void force_nec_state0()
{
    Nec_state=0;                                            // reset nec state
    T1CONbits.TMR1ON = 0;                                   // stop timer1
}

void INT0_isr() 
{    
 INTCONbits.INT0IF = 0;                                     // Clear external interrupt
    if (Nec_state != 0) 
    {
        Time_Elapsed = (TMR1H << 8) | TMR1L;                // Store Timer1 value
        TMR1H = 0;                                          // Reset Timer1
        TMR1L = 0;                                          // timer1 value
    }

    switch (Nec_state) 
    {
        case 0:
        {
                                                            // Clear Timer 1
            TMR1H = 0;                                      // Reset Timer1
            TMR1L = 0;
            PIR1bits.TMR1IF = 0;                            // Clear interrupt flag
            T1CON = 0x90;                                   // Program Timer1 mode with count = 1usec 
                                                            // using System clock running at 8Mhz
            T1CONbits.TMR1ON = 1;                           // Enable Timer 1
            bit_count = 0;                                  // Force bit count (bit_count) to 0
            Nec_code = 0;                                   // Set Nec_code = 0
            Nec_state = 1;                                  // Set Nec_State to state 1
            INTCON2bits.INTEDG0 = 1;                        // Change Edge interrupt of INT0 to Low to High            
            return;
        }

        case 1:
        {
            if (8500 < Time_Elapsed < 9500)                 // Check if Time Elapsed is between
            {                                               // 8500us and 9500us
                Nec_state = 2;                              // change to next state 2
            } else force_nec_state0();                      // else force nec_state to 0
            INTCON2bits.INTEDG0 = 0;                        // Change Edge interrupt of INT0 to high to low 
            return;
        }

        case 2:
        {
            if (4000 < Time_Elapsed < 5000)                 // Check if Time Elapsed is between
            {                                               // 4000us and 5000us
                Nec_state = 3;                              // change to next state 3
            } else force_nec_state0();                      // else force nec_state to 0
            INTCON2bits.INTEDG0 = 1;                        // Change Edge interrupt of INT0 to low to high  
            return;
        }

        case 3:
        {
            if (400 < Time_Elapsed < 700)                   // Check if Time Elapsed is between
            {                                               // 400us and 700us
                Nec_state = 4;                              // change to next state 4
            } else force_nec_state0();                      // else force nec_state to 0
            INTCON2bits.INTEDG0 = 0;                        // Change Edge interrupt of INT0 to high to low  
            return;
        }

        case 4:
        {
            if (400 < Time_Elapsed < 1800)                  // Check if Time Elapsed is between
            {                                               // 400us and 1800us
                Nec_code = Nec_code << 1;                   // shift nec code left by 1 bit    
                if (Time_Elapsed > 1000)                    // if Time Elapsed>1000us    
                    Nec_code += 1;                          // add 1 to nec code
                bit_count++;                                // increase bit count by 1
                if (bit_count > 31)                         // if bit count > 31
                {
                    nec_ok = 1;                             // set nec_ok = 1
                    Nec_code1 = (char) ((Nec_code >> 8));   // get button code from full nec code
                    INTCONbits.INT0IE = 0;                  // disable external interrupt
                    Nec_state = 0;                          // change to next state 0
                }
                Nec_state = 3;                              // change to next state 3
            } else force_nec_state0();                      // else force nec_state to 0
            INTCON2bits.INTEDG0 = 1;                        // Change Edge interrupt of INT0 to low to high  
            return;
        }
    }
}

void INT2_isr()
{
    INTCON3bits.INT2IF=0;                                   // Clear the interrupt flag
    INT2_flag=1;                                            // activate int2 software flag
}







