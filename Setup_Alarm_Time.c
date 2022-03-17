#include <xc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "I2C_Support.h"
#include "Setup_Alarm_Time.h"
#include "Setup_Time.h"
#include "Main_Screen.h"
#include "Main.h"
#include "ST7735_TFT.h"
#include "utils.h"

extern char setup_alarm_time[]; 
extern unsigned char alarm_second, alarm_minute, alarm_hour, alarm_date;
extern unsigned char setup_alarm_second, setup_alarm_minute, setup_alarm_hour;
extern char found;
extern char *txt;
extern short nec_ok;

char Select_Alarm_Field;  
char In_Alarm_Time_Setup;

void Do_Setup_Alarm_Time(void)
{
    Select_Alarm_Field = 0;
    In_Alarm_Time_Setup = 1;
    DS3231_Read_Alarm_Time();                                                   // Read alarm time
    alarm_second = bcd_2_dec(alarm_second);                                     // convert to dec for use of second
    alarm_minute = bcd_2_dec(alarm_minute);                                     // convert to dec for use of minute
    alarm_hour   = bcd_2_dec(alarm_hour);                                       // convert to dec for use of hour
    setup_alarm_second = alarm_second;                                          // Beginning setup alarm time as current set alarm time, second
    setup_alarm_minute = alarm_minute;                                          // alarm minute
    setup_alarm_hour = alarm_hour;                                              // alarm hour
    Initialize_Setup_Alarm_Time_Screen();                                       // Initialize the alarm setup screen
    Update_Setup_Alarm_Time_Screen();                                           // Update the alarm screen with latest info
    while (In_Alarm_Time_Setup == 1)
    {   
         if (check_for_button_input() == 1)
         {
             if (found >= Prev && found <= EQ)
             {
                Do_Beep_Good();                                                 // Performs high pitched beep
                if (found == Prev) Go_Prev_Alarm_Field();                       // if Previous button pressed, move left
                if (found == Next) Go_Next_Alarm_Field();                       // if Next button pressed, move right
                if (found == Play_Pause) Do_Save_New_Alarm_Time();              // if Play/Pause button pressed, save alarm set time and return
                if (found == Minus) Decrease_Alarm_Time();                      // if Minus button pressed, decrement number
                if (found == Plus) Increase_Alarm_Time();                       // if Plus button pressed, increment number
                if (found == EQ) Exit_Setup_Alarm_Time();                       // if Equal button pressed, exit setup alarm time screen
                found = 0xff;       
             }
             else
             {
                 Do_Beep_Bad();                                                 // Performs low pitch beep
             }
        }                   
    }
}

void Increase_Alarm_Time()
{
            switch (Select_Alarm_Field)                                         // Switch statement to increment fields
            {
                case 0:                                                         // Set limit for hour increment from 0-23
                    setup_alarm_hour++;
                    if (setup_alarm_hour == 24) setup_alarm_hour = 0; 
                    break;

                case 1:                                                         // Set limit for minute increment from 0-59
                    setup_alarm_minute++;
                    if (setup_alarm_minute == 60) setup_alarm_minute = 0; 
                    break;

                case 2:                                                         // Set limit for second increment from 0-59
                    setup_alarm_second++;
                    if (setup_alarm_second == 60) setup_alarm_second = 0; 
                    break;

                default:
                    break;
            }    
            Update_Setup_Alarm_Time_Screen();                                   // Update alarm screen with latest info
}

void Decrease_Alarm_Time()
{       
            switch (Select_Alarm_Field)                                         // Switch statement to decrement fields
            {
                case 0:
                    if (setup_alarm_hour == 0) setup_alarm_hour = 23;           // Set limit for hour decrement from 23-0
                    else --setup_alarm_hour;
                    break;

                case 1:
                    if (setup_alarm_minute == 0) setup_alarm_minute = 59;       // Set limit for minute decrement from 59-0
                    else --setup_alarm_minute;
                    break;

                case 2:
                    if (setup_alarm_second == 0) setup_alarm_second = 59;       // Set limit for second increment from 59-0
                    else --setup_alarm_second;
                    break;  

                default:
                break;
            }                
            Update_Setup_Alarm_Time_Screen();                                   // Update alarm screen with latest info
} 

void Go_Next_Alarm_Field()
{
     
    Select_Alarm_Field++;                                                       // Move to next field
    if (Select_Alarm_Field == 3) Select_Alarm_Field = 0;                        // Wrap around if necessary
    Update_Setup_Screen_Cursor_Forward(Select_Alarm_Field);                     // Update cursor
                                     
}  

void Go_Prev_Alarm_Field()
{
    if (Select_Alarm_Field == 0) Select_Alarm_Field = 2;
    else Select_Alarm_Field--;                                                  // Move to next field
    Update_Setup_Screen_Cursor_Backward(Select_Alarm_Field);                    // Update cursor
} 
          
void Exit_Setup_Alarm_Time()
{
    DS3231_Read_Time();                                                         // Read time
    Initialize_Screen();                                                        // Initialize the screen before returning  
    In_Alarm_Time_Setup = 0;
}

void Do_Save_New_Alarm_Time()
{
    DS3231_Write_Alarm_Time();                                                  // Write alarm time
    DS3231_Read_Alarm_Time();                                                   // Read alarm time
    DS3231_Read_Time();                                                         // Read current time
    Initialize_Screen();                                                        // Initialize main screen before returning
    In_Alarm_Time_Setup = 0;
}
     
void Initialize_Setup_Alarm_Time_Screen(void) 
{ 
    fillScreen(ST7735_BLACK);                                                   // Fills background of screen with color passed to it
 
    strcpy(txt, "ECE3301L F'21 Final");                                         // Text displayed 
    drawtext(start_x , start_y, txt, ST7735_WHITE  , ST7735_BLACK, TS_1);
                                                                                // X and Y coordinates of where the text is to be displayed

    strcpy(txt, "  Alarm");
    drawtext(start_x+5 , start_y+10, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2);
    strcpy(txt, "  Setup");                                                     // Text displayed 
    drawtext(start_x+5 , start_y+25, txt, ST7735_MAGENTA, ST7735_BLACK, TS_2); 
    
    strcpy(txt, "Time");
    drawtext(time_x  , time_y , txt, ST7735_BLUE   , ST7735_BLACK, TS_1);
    fillRect(data_time_x-1, data_time_y+16, 25,2,ST7735_CYAN);
}

void Update_Setup_Alarm_Time_Screen(void)
{
    printf ("%x:%x:%x\r\n", setup_alarm_hour,setup_alarm_minute,setup_alarm_second);            // Display for Alarm Setup Time
    setup_alarm_time[0]  = (setup_alarm_hour/10)  + '0';                                        // Text storage for alarm hour tens
    setup_alarm_time[1]  = (setup_alarm_hour%10)  + '0';                                        // Text storage for alarm hour ones
    setup_alarm_time[3]  = (setup_alarm_minute/10)  + '0';                                      // Text storage for alarm minute tens
    setup_alarm_time[4]  = (setup_alarm_minute%10)  + '0';                                      // Text storage for alarm minute ones
    setup_alarm_time[6]  = (setup_alarm_second/10)  + '0';                                      // Text storage for alarm second tens
    setup_alarm_time[7]  = (setup_alarm_second%10)  + '0';                                      // Text storage for alarm second ones
    drawtext(data_time_x, data_time_y, setup_alarm_time, ST7735_CYAN, ST7735_BLACK, TS_2);      // Draws text
}