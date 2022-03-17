#include <p18f4620.h>
#include "Main.h"
#include "Fan_Support.h"
#include "stdio.h"

extern char HEATER;
extern char duty_cycle;
extern char heater_set_temp;
extern signed int DS1621_tempF;

#define FAN_LED PORTEbits.RE1                           // FAN LED connection PORTE.1
#define D2_RD   PORTDbits.RD5                           // Green color connection on D2 LED
#define D2_GR   PORTDbits.RD6                           // Blue color connection on D2 LED

void Set_RPM_RGB(int rpm);

int get_duty_cycle(signed int temp, int set_temp)
{
    if (HEATER==0)                                      
    {
        duty_cycle=0;                                   // if heater is off, force duty cycle to 0
    }
    else
    {
        if(set_temp<temp) duty_cycle=0;                 // if temp is greater than heater temp, force duty cycle to 0
        else
        {
            duty_cycle=(set_temp - temp)*2;             // calculates duty cycle 
            if(duty_cycle>100) duty_cycle=100;          // limits duty cycle to 100
        }
    }
    return duty_cycle;
}

void Monitor_Heater()
{
    duty_cycle = get_duty_cycle(DS1621_tempF, heater_set_temp); 
    do_update_pwm(duty_cycle);                          
    if (HEATER == 1)                                    // if heater is on start fan
    {
            FAN_EN = 1;                                 // activates fan
            FAN_LED=1;                                  // turns off LED for fan  
    }
    else                                                // if heater is off disable fan
    { 
        FAN_EN = 0;                                     // disables fan
        FAN_LED = 0;                                    // turns off LED for fan  
    }
}

void Toggle_Heater()
{
    if(HEATER==0)Turn_On_Fan();                         // if heater is off then turn fan on
    else Turn_Off_Fan();                                // if heater is on then turn fan off

}

int get_RPM()
{
    int RPS = TMR3L / 2;                                // read the count. Since there are 2 pulses per rev
                                                        // then RPS = count /2
    TMR3L = 0;                                          // clear out the count
    return (RPS * 60);                                  // return RPM = 60 * RPS 
}

void Turn_Off_Fan()
{
    printf("Fan turned off\r\n");   
    HEATER=0;                                           // sets heater switch too off
    FAN_LED=0;                                          // turns off LED for fan  
}

void Turn_On_Fan()
{
    printf("Fan turned on\r\n");
    do_update_pwm(duty_cycle);                          // sets heater switch too on
    HEATER=1;                                           // sends a 1 to the FAN EN powering fan on
    
}

void Set_RPM_RGB(int rpm)
{
    if(rpm==0)                                          // if rpm=0 D2 is off
    {
        D2_GR=0;
        D2_RD=0;
    }
    else if(0<rpm && rpm<1800)                          // if rpm<1800 D2 is red 
    {
        D2_GR=0;
        D2_RD=1;
    }
    else if(1799<rpm && rpm<2700)                       // if rpm<2700 D2 is yellow
    {
        D2_GR=1;
        D2_RD=1;
    }
    else if(rpm>2699)                                   // if rpm>2700 D2 is green
    {
        D2_GR=1;
        D2_RD=0;
    }
}





