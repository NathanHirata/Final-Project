#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>
#include <usart.h>
#include <string.h>

#include "I2C.h"
#include "I2C_Support.h"
#include "Interrupt.h"
#include "Fan_Support.h"
#include "Main.h"
#include "ST7735_TFT.h"
#include "utils.h"

#pragma config OSC = INTIO67
#pragma config BOREN =OFF
#pragma config WDT=OFF
#pragma config LVP=OFF
#pragma config CCP2MX = PORTBE

void Test_Alarm();
char second = 0x00;
char minute = 0x00;
char hour = 0x00;
char dow = 0x00;
char day = 0x00;
char month = 0x00;
char year = 0x00;

char found;
char tempSecond = 0xff; 
signed int DS1621_tempC, DS1621_tempF;
char setup_second, setup_minute, setup_hour, setup_day, setup_month, setup_year;
char alarm_second, alarm_minute, alarm_hour, alarm_date;
char setup_alarm_second, setup_alarm_minute, setup_alarm_hour;
unsigned char heater_set_temp = 75;
unsigned char Nec_state = 0;
float volt;
char INT1_flag, INT2_flag;

short nec_ok = 0;
char Nec_code1;
char HEATER;
char duty_cycle;
int rps;
int rpm;
int ALARMEN;
int alarm_mode, MATCHED,color;

char buffer[31]     = " ECE3301L F'21 Final\0";                 // text storage for Header
char *nbr;
char *txt;
char tempC[]        = "+25";                                    // text storage for Temperature in Celcius on Main Screen
char tempF[]        = "+77";                                    // text storage for Temperature in Farenheit on Main Screen
char time[]         = "00:00:00";                               // text storage for Time on Main Screen
char date[]         = "00/00/00";                               // text storage for Date on Main Screen
char alarm_time[]   = "00:00:00";                               // text storage for Alarm Time on Main Screen
char Alarm_SW_Txt[] = "OFF";                                    // text storage for Alarm ON/OFF Switch on Main Screen
char Heater_Set_Temp_Txt[] = "075F";                            // text storage for Heater Temperature on Main Screen
char Heater_SW_Txt[]   = "OFF";                                 // text storage for Heater Mode on Main Screen
char array1[21]={0xa2,0x62,0xe2,0x22,0x02,0xc2,0xe0,            // array for the IR Remote Buttons
                 0xa8,0x90,0x68,0x98,0xb0,0x30,0x18,
                 0x7a,0x10,0x38,0x5a,0x42,0x4a,0x52};
                                                                
char DC_Txt[]       = "000";                                    // text storage for Duty Cycle value on Main Screen
char Volt_Txt[]     = "0.00V";                                  // text storage for Voltage on Main Screen
char RTC_ALARM_Txt[]= "0";                                      // text storage for RM on Main Screen
char RPM_Txt[]      = "0000";                                   // text storage for RPM on Main Screen

char setup_time[]       = "00:00:00";                           // text storage for Setup Time on Setup Time Screen
char setup_date[]       = "01/01/00";                           // text storage for Setup Date on Setup Time Screen
char setup_alarm_time[] = "00:00:00";                           // text storage for Setup Alarm Time on Setup Alarm Screen
char setup_heater_set_text[]   = "075F";                        // text storage for Setup Heater on Set Heater Screen

void putch (char c)
{   
    while (!TRMT);                                              
    TXREG = c;                                                  
}

void init_UART()
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 25);
    OSCCON = 0x70;
}

int c = 0;

void Do_Init()                      // Initialize the ports 
{ 
    init_UART();                    // Initialize the uart
    Init_ADC();                     // Initialize the ADCON0, 1, and 2
    OSCCON=0x70;                    // Set oscillator to 8 MHz 
    
    TRISA = 0x01;                   // Port A as outputs, pin 0 as input
    TRISB = 0x17;                   // Port B as outputs, pins 3 and 5 as inputs 
    TRISC = 0x01;                   // Port C as outputs, pin 0 as input
    TRISD = 0x00;                   // Port D as outputs
    TRISE = 0x00;                   // Port E as outputs
    PORTE = 0x00;                   // All Port E pins low
    
    ALARMEN=0;                      // Set alarm enable to 0
    alarm_mode=0;                   // Set alarm mode to OFF state, 0
    DS3231_Write_Alarm_Time();      // Call write alarm time function
    
    HEATER = 0;                     // Set heater to OFF state
    RBPU=0;                         // 

    TMR3L = 0x00;                   // 
    T3CON = 0x03;                   // 
    I2C_Init(100000);               // Initialize the I2C with 100000

    DS1621_Init();                  // Initialize the DS1612
    Init_Interrupt();               // Initialize the Interrupt
    Turn_Off_Fan();                 // Call function to turn off fan
    heater_set_temp = 75;           // Set base heater temperature at 75
}

void main() 
{
    Do_Init();                                                                                  // Initialization  
    Initialize_Screen();                                                                        // Initialize the Screen
    DS3231_Turn_Off_Alarm();                                                                    // Turn off the alarm time
    DS3231_Read_Alarm_Time();                                                                   // Read alarm time for the first time

    tempSecond = 0xff;                                                                          // Temperature Second is loaded with 0xff
    while (1)
    {
        DS3231_Read_Time();                                                                     // Reads the time

        if(tempSecond != second)                                                                // if Temperature Second does not equal Second
        {
            tempSecond = second;                                                                // Temperature Second will be loaded with value of Second
            rpm = get_RPM();                                                                    // rpm will be load with the result of function get_RPM
            volt = read_volt();                                                                 // volt will be loaded with the result of function read_volt())
            if (volt>3) MATCHED=0;                                                              // if volt is greater than 3, MATCHED will be 0
            else if(volt<=3) MATCHED=1;                                                         // else if volt is less than or equal to 3, MATCHED will be 1
            DS1621_tempC = DS1621_Read_Temp();                                                  // DS1621_tempC will be loaded with the result of function DS1621_Read_Temp()
            if ((DS1621_tempC & 0x80) == 0x80) DS1621_tempC = - (0x80 - DS1621_tempC & 0x7f);   // DS1621_tempC and 0x80 equals 0x80 then DS1621_tempC will equal -(0x80 - DS1621_tempC & 0x7f)
            DS1621_tempF = (DS1621_tempC * 9 / 5) + 32;                                         // DS1621_tempF will equal (DS1621_tempC * 9 / 5) + 32

            printf ("%02x:%02x:%02x %02x/%02x/%02x",hour,minute,second,month,day,year);         // Display the time and data
            printf (" Temp = %d C = %d F ", DS1621_tempC, DS1621_tempF);                        // Display the Temperature for both Celcius and Farenheit
            printf ("alarm = %d Heater = %d ", RTC_ALARM_NOT, HEATER);                          // Display the Enable of the Alarm and Heater
            printf ("RPM = %d  dc = %d\r\n", rpm, duty_cycle);                                  // Display the RPM and Duty Cycle
            Set_RPM_RGB(rpm);                                                                   // Set the color of the RPM LED
            Monitor_Heater();                                                                   // Calls on function to read values of duty cycle and fan enable
            Test_Alarm();                                                                       // Initiates function for alarm control
            Update_Screen();                                                                    // Refresh screen with updated information
        }
        
        if (check_for_button_input() == 1)                                                      // Checks for IR button pressed
        {
            nec_ok = 0;
            switch (found)                                                                      // Switch statement for screen control
            {
                case Ch_Minus:                                                                  // Switch to Setup Time Screen
                    Do_Beep_Good();
                    Do_Setup_Time();
                    break;
                
                case Channel:                                                                   // Switch to Alarm Setup Time Screen
                    Do_Beep_Good();
                    Do_Setup_Alarm_Time();
                    break;
                    
                case Ch_Plus:                                                                   // Switch to Heater Temp Setup Screen
                    Do_Beep_Good();
                    Do_Setup_Heater_Temp();            
                    break;
                    
                case Play_Pause:                                                                // Toggles Heater to turn on/off
                    Do_Beep_Good();
                    Toggle_Heater();
                     break;           
        
                default:                                                                        // Any other button pressed will produce a low pitch beep
                    Do_Beep_Bad();
                    break;
            }
        }    
        
        if (INT2_flag == 1)                                                                     // if push button is pressed, interrupt is activated turns the Alarm Enable ON/OFF
        {
            INT2_flag = 0;
            ALARMEN=!ALARMEN;
        }
    }
}

void Test_Alarm()
{
    if(alarm_mode == 0 && ALARMEN == 0)                     // if Alarm mode and Alarm enable is off
    {
        MATCHED=0;                                          // MATCHED will equal 0
        DS3231_Turn_Off_Alarm();                            // Alarm is OFF
        alarm_mode=0;                                       // Alarm is OFF
        c=0;
        Set_RGB_Color(0);                                   // LED is OFF
        Deactivate_Buzzer();                                // Buzzer is OFF
    }
    if(alarm_mode == 0 && ALARMEN == 1)                     // if Alarm mode is OFF and Alarm Enable is ON
    {
        DS3231_Turn_On_Alarm();                             // Enables alarm
        alarm_mode=1;                                       // Alarm is loaded with 1
    }
    
    if(alarm_mode==1 && ALARMEN == 0)                       // If alarm_mode is 1 and alarmen is 0
    {
        MATCHED=0;                                          // MATCHED is loaded with 0
        DS3231_Turn_Off_Alarm();                            // Disables alarm
        alarm_mode=0;                                       // Alarm mode is loaded with 0
        c=0;            
        Set_RGB_Color(0);                                   // Turns LED OFF
        Deactivate_Buzzer();                                // Turns buzzer OFF
    }
    
    if(alarm_mode == 1 && ALARMEN == 1)                     // if alarm_mode and alarmen is both 1
    {
        if(RTC_ALARM_NOT == 0)                              // if RTC_ALARM_NOT equals 0
        {
            if (MATCHED==1)                                 // if MATCHED is 1
            {
                Activate_Buzzer();                          // Activates Buzzer
                Set_RGB_Color(c);                           // Sets the LED color for alarm
                c++;                                        // Increments number to change color
                if(c>7)c=0;                                 // if c is greater than 7, value resets to 0
                Wait_One_Sec();                             // Delay
            }
            else 
            {
                Set_RGB_Color(0);                           // Sets LED to OFF
                Deactivate_Buzzer();                        // Deactivates Buzzer
                c=0;                                        
                DS3231_Turn_Off_Alarm();                    // Disables Alarm
                DS3231_Turn_On_Alarm();                     // Enables Alarm
            }
        }
    }
}






