#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

#include <p18f4620.h>
#include "utils.h"

extern char found;
extern char Nec_code1;
extern short nec_ok;
extern char array1[21];
extern char duty_cycle;


char ALARM_COLOR[]={0x00,0x02,0x04,         // duty cycle array color values
                    0x06,0x10,0x12,
                    0x14,0x16};

char check_for_button_input()
{       
    if (nec_ok == 1)
    {
        nec_ok = 0;                         // clear nec IR flag


        INTCONbits.INT0IE = 1;              // Enable external interrupt
        INTCON2bits.INTEDG0 = 0;            // Edge programming for INT0 falling edge

        found = 0xff;                       
        for (int j=0; j< 21; j++)
        {
            if (Nec_code1 == array1[j])     // searches for button in array     
            {
                found = j;  
                j = 21;
            }
        }
            
        if (found == 0xff)                  // if button is not valid return zero 
        {                                   // print statement to tera term
            printf ("Cannot find button \r\n");
            return (0);
        }
        else
        {
            return (1);
        }
    }
}

char bcd_2_dec (char bcd)
{
    int dec;
    dec = ((bcd>> 4) * 10) + (bcd & 0x0f);
    return dec;
}

int dec_2_bcd (char dec)
{
    int bcd;
    bcd = ((dec / 10) << 4) + (dec % 10);
    return bcd;
}

void Do_Beep()                              // beep for half second
{
    Activate_Buzzer();
    Wait_One_Sec();
    Deactivate_Buzzer();
    Wait_One_Sec();
    do_update_pwm(duty_cycle);
}

void Do_Beep_Good()                         // beep for half second at 2KHz
{
    Activate_Buzzer_2KHz();
    Wait_One_Sec();
    Deactivate_Buzzer();
    Wait_One_Sec();
    do_update_pwm(duty_cycle);
}

void Do_Beep_Bad()                          // beep for half second at 500Hz
{
    Activate_Buzzer_500Hz();
    Wait_One_Sec();
    Deactivate_Buzzer();
    Wait_One_Sec();
    do_update_pwm(duty_cycle);
}

void Wait_One_Sec()
{
    for (int k=0;k<0xffff;k++);
}

void Activate_Buzzer()                      // normal buzzer 
{
    PR2 = 0b11111001 ;
    T2CON = 0b00000101 ;
    CCPR2L = 0b01001010 ;
    CCP2CON = 0b00111100 ;
}

void Activate_Buzzer_500Hz()                // low pitch buzzer
{
    PR2 = 0b11111001 ;
    T2CON = 0b00000111 ;
    CCPR2L = 0b01001010 ;
    CCP2CON = 0b00111100 ;
}

void Activate_Buzzer_2KHz()                 // high pitch buzzer
{
    PR2 = 0b11111001 ;
    T2CON = 0b00000101 ;
    CCPR2L = 0b01001010 ;
    CCP2CON = 0b00111100 ;
}

void Deactivate_Buzzer()                    // disables buzzer
{
    CCP2CON = 0x0;
	PORTBbits.RB3 = 0;
}

void do_update_pwm(char duty_cycle) 
{ 
float dc_f;
int dc_I;
	PR2 = 0b00000100 ;                      // Set the frequency for 25 Khz 
	T2CON = 0b00000111 ;                    // As given in website
	dc_f = ( 4.0 * duty_cycle / 20.0) ;     // calculate factor of duty cycle versus a 25 Khz signal
	dc_I = (int) dc_f;                      // Truncate integer
	if (dc_I > duty_cycle) dc_I++;          // Round up function
	CCP1CON = ((dc_I & 0x03) << 4) | 0b00001100;
	CCPR1L = (dc_I) >> 2;
}

void Set_RGB_Color(char color)
{
    if(color==0)PORTA=0x00;                 // if color is 0 turn off LED
    else PORTA=ALARM_COLOR[color];          // if color is not 0, cycle through RGB colors
}

float read_volt()
{
    float step = get_full_ADC();            // number of steps from ADC
    float voltPS = step * 4/1000;           // converts steps to Volts
    return voltPS;
}

unsigned int get_full_ADC()
{
    int result;
    ADCON0bits.GO=1;                        // Start Conversion
    while(ADCON0bits.DONE==1);              // wait for conversion to be completed
    result = (ADRESH * 0x100) + ADRESL;     // combine result of upper byte and
                                            // lower byte into result
    return result;                          // return the result.
}

void Init_ADC()
{
    ADCON0=0x01;
    ADCON1=0x0E;                            // selects pins AN0 through AN3 as analog signal, 
                                            // VDD-Vref+ as reference voltage
    ADCON2=0xA9;                            // right justify the result. Set the bit conversion time 
                                            // (TAD) and acquisition time
}


