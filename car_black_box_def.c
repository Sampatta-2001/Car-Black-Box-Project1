/*
 * File:   car_black_box_def.c
 * Author: Sampatta Ghulanawar
 * Project Name : Car Black Box
 * Created on 3 September, 2025, 8:02 PM
 */
/*
  Car black box definations
  Display dashboard function
 */

#include "main.h"

// #include "car_black_box.h"
// display dashboard functions
unsigned char clock_reg[3];
unsigned char time[7]; // HH:MM:SS //to store events in memory also so define it as global
                       // avoid storing : in array
unsigned char log[11]; // hh:mm:ss ev sp
char log_pos = 0, sec;
unsigned char return_time;
char curr_log_pos = 0;
char *menu[] = {"View Log", "Clear Log", "Download Log", "Set Time", "Change Password"};
unsigned char menu_pos = 0;
unsigned char curr_pos;
unsigned char prev_key = ALL_RELEASED;
extern unsigned int once_down;
static void get_time()
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD
    clock_reg[1] = read_ds1307(MIN_ADDR);  // MM -> BCD
    clock_reg[2] = read_ds1307(SEC_ADDR);  // SS -> BCD

    // BCD Format
    // clock_reg[0] = HH
    // clock_reg[1] = MM
    // clock_reg[2] = SS

    /* To store the time in HH:MM:SS format */
    // HH -> BCD to ascii
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';

    // time[2] = ':'; here no need colon
    //  MM
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';

    // time[5] = ':';
    //  SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
}

void display_time()
{
    get_time();

    // HH:MM:SS
    clcd_putch(time[0], LINE2(2)); // HH:MM:SS
    clcd_putch(time[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(time[2], LINE2(5)); // HH:MM:SS
    clcd_putch(time[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(time[4], LINE2(8)); // HH:MM:SS
    clcd_putch(time[5], LINE2(9));
}

void display_dashboard(unsigned char event[], unsigned char speed)
{
    // display details in first line
    clcd_print("TIME     E  SP", LINE1(2));

    // display time
    display_time();

    // display event
    clcd_print(event, LINE2(11));

    // display speed
    // int to ascii-->it is two digit value
    clcd_putch((speed / 10) + '0', LINE2(14));
    clcd_putch((speed % 10) + '0', LINE2(15));
}
void store_event()
{
    unsigned char addr = 0x05;
    if (log_pos == 10) // 1 to 9
    {
        curr_log_pos = 10;
        log_pos = 0;
    }
    addr = 0x05 + log_pos * 10; // 5 15 25 35....
    ext_eeprom_24C02_str_write(addr, log);
    if (curr_log_pos < 9)
    {
        curr_log_pos++;
    }
    log_pos++; // 1 to 9
}

void log_event(unsigned char event[], unsigned char speed)
{
    get_time();
    strncpy(log, time, 6);      // hhmmss  0th index to 5th index
    strncpy(&log[6], event, 2); // 6th index for event

    // speed 98
    log[8] = speed / 10 + '0'; // speed  1st digit
    log[9] = speed % 10 + '0'; // speed  2nd digit
    log[10] = '\0';
    store_event();
}

unsigned char login(unsigned char key, unsigned char reset_flag)
{
    static char user_password[4];
    static unsigned char i;
    static unsigned char attempt_left = 3;

    if (reset_flag == RESET_PASSWORD)
    {
        i = 0;
        attempt_left = 3;
        memset(user_password, '\0', 4);
        key = ALL_RELEASED;
        return_time = 5;

        clear_screen();
        clcd_print("Enter Password", LINE1(1));
        clcd_write(LINE2(4), INST_MODE);
        clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
        __delay_us(100);

        return CHANGE_PASSWORD_SCREEN;
    }

    if (return_time == 0)
    {
        return RETURN_BACK; // timeout -> return
    }

    if (i < 4)
    {
        if (key == SW4)
        {
            clcd_putch('*', LINE2(4 + i));
            user_password[i++] = '1';
            return_time = 5;
        }
        else if (key == SW5)
        {
            clcd_putch('*', LINE2(4 + i));
            user_password[i++] = '0';
            return_time = 5;
        }
    }

    if (i == 4) // password entry complete
    {
        char s_password[4];
        for (int j = 0; j < 4; j++)
        {
            s_password[j] = read_ext_eeprom(j);
        }

        if (strncmp(user_password, s_password, 4) == 0)
        {
            clear_screen();
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            clcd_print("Login Success", LINE1(1));
            __delay_ms(2000);
            return LOGIN_SUCCESS;
        }
        else
        {
            attempt_left--;

            if (attempt_left == 0)
            {
                clear_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("You are Blocked", LINE1(0));
                clcd_print("Wait for", LINE2(0));
                clcd_print("secs", LINE2(13));
                sec = 60;
                while (sec)
                {
                    clcd_putch((sec / 10) + '0', LINE2(9));
                    clcd_putch((sec % 10) + '0', LINE2(10));
                    // countdown should be updated in ISR
                }
                attempt_left = 3;
            }
            else
            {
                clear_screen();
                clcd_print("Wrong Password", LINE1(0));
                clcd_print("Attempts left:", LINE2(0));
                clcd_putch(attempt_left + '0', LINE2(15));
                __delay_ms(2000);
            }

            clear_screen();
            clcd_print("Enter Password", LINE1(1));
            clcd_write(LINE2(4), INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);

            i = 0;
            return_time = 5;
        }
    }

    return CHANGE_PASSWORD_SCREEN;
}

// Display menu screen
unsigned char menu_screen(unsigned char key, unsigned char reset_flag)
{
    static unsigned char press_counter = 0;
    static unsigned char is_long_press = 0;

    if (reset_flag == RESET_MENU)
    {
        return_time = 5;
        menu_pos = 0;
        prev_key = ALL_RELEASED;
        press_counter = 0;
        is_long_press = 0;
    }

    if (return_time == 0)
    {
        return RETURN_BACK;
    }

    if (key == SW4 || key == SW5)
    {
        if (prev_key != key)
        {
            press_counter = 0;
            is_long_press = 0;
        }
        prev_key = key;
        press_counter++;

        if (press_counter > 20)
        {
            is_long_press = 1;
            press_counter = 0;
            return_time = 5;
            curr_pos = menu_pos;
            return (key == SW4) ? LONG_PRESS_SW4 : LONG_PRESS_SW5;
        }
    }
    //for short press
    else if (key == ALL_RELEASED)
    {
        if (press_counter > 0 && !is_long_press)
        {
            if (prev_key == SW4 && menu_pos > 0)
            {
                clear_screen();
                menu_pos--;
                return_time = 5;
            }
            else if (prev_key == SW5 && menu_pos < 4)
            {
                clear_screen();
                menu_pos++;
                return_time = 5;
            }
           // return_time = 5;
        }
        press_counter = 0;
        is_long_press = 0;
    }

    // Display menu
    if (menu_pos == 4) 
    {
        clcd_putch('*', LINE2(0));
        clcd_print(menu[menu_pos], LINE2(2));
        clcd_print(menu[menu_pos - 1], LINE1(2));
    } else 
    {
        clcd_putch('*', LINE1(0));
        clcd_print(menu[menu_pos], LINE1(2));
        clcd_print(menu[menu_pos + 1], LINE2(2));
    }
}

// View the log screen

unsigned char view_log_screen(unsigned char key, unsigned char reset_flag)
{
    clcd_print("#  TIME     E  SP", LINE1(0));

    char addr;
    static unsigned int view_pos;
    static unsigned int press_counter = 0;
    static unsigned char is_long_press = 0;

    // initialize view_pos with 0

    if (reset_flag == RESET_VIEW)
    {
        view_pos = 0;
        return_time = 5;
        prev_key = ALL_RELEASED;
        press_counter = 0;
        is_long_press = 0;
    }

    if (return_time == 0)
    {
        return RETURN_BACK;
    }
    // check for long press or short press of SW4 or SW5
    if (key == SW4 || key == SW5)
    {
        return_time = 5;
        if (prev_key != key)
        {
            press_counter = 0;
            is_long_press = 0;
        }

        prev_key = key;
        press_counter++;

        // check for threshold
        if (press_counter > 20)
        {
            press_counter = 0;
            is_long_press = 1;
            return (key == SW4) ? LOGIN_SUCCESS : RETURN_BACK;
        }
    }
    else if (key == ALL_RELEASED)
    {
        if (!is_long_press && press_counter > 0)
        {
            if (prev_key == SW4)
            {
                if (view_pos < curr_log_pos - 1)
                {
                    view_pos++;
                }
                else
                {
                    view_pos = 0;
                }
            }
            else if (prev_key == SW5)
            {
                if (view_pos > 0)
                {
                    view_pos--;
                }
            }
            return_time = 5;
        }
        press_counter = 0;
        is_long_press = 0;
    }

    // read the data from external eeprom
    for (int i = 0; i < 10; i++)
    {
        addr = 5 + view_pos * 10;
        log[i] = read_ext_eeprom(addr + i);
    }

    // print the logs in clcd
    clcd_putch(view_pos + '0', LINE2(0));
    clcd_putch(log[0], LINE2(2));
    clcd_putch(log[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(log[2], LINE2(5));
    clcd_putch(log[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(log[4], LINE2(8));
    clcd_putch(log[5], LINE2(9));
    clcd_putch(log[6], LINE2(11));
    clcd_putch(log[7], LINE2(12));
    clcd_putch(log[8], LINE2(14));
    clcd_putch(log[9], LINE2(15));

    return VIEW_LOG_SCREEN;
}

// To clear the stored logs
unsigned char clear_log_screen(unsigned char reset_flag, unsigned char speed)
{
    char addr;
    if(reset_flag==RESET_CLEAR)
    {
        return_time=5;
        log_pos=0;
        curr_log_pos=0;
        log_event("CL",speed);
        clear_screen();
        clcd_print("Cleared the log",LINE1(0));
        __delay_ms(1000);
        return LOGIN_SUCCESS;
        
    }
    if(return_time==0)
    {
        return RETURN_BACK;
    }
}

// to download the stored logs and display on the terminal

unsigned char download_log_screen(unsigned char key, unsigned char reset_flag)
{
    unsigned char addr;
    static unsigned int down_pos;

    if (reset_flag == RESET_DOWN)
    {
        return_time = 5;
        down_pos = 0;
        once_down = 0;
    }
    if (return_time == 0)
    {
        return RETURN_BACK;
    }

    if (log_pos == 0)
    {
        puts("No logs available\n\r");
    }
    if (!once_down)
    {
        once_down = 1;
        while (down_pos < curr_log_pos)
        {
            for (int i = 0; i < 10; i++)
            {
                addr = 5 + down_pos * 10;
                log[i] = read_ext_eeprom(addr + i);
            }
            putchar(down_pos + '0');
            puts("\t");
            putchar(log[0]);
            putchar(log[1]);
            puts(":");
            putchar(log[2]);
            putchar(log[3]);
            puts(":");
            putchar(log[4]);
            putchar(log[5]);
            puts("\t");
            putchar(log[6]);
            putchar(log[7]);
            puts("\t");
            putchar(log[8]);
            putchar(log[9]);
            puts("\n\r");

            down_pos++;
            if(down_pos>=curr_log_pos)
            {
                break;
            }
        }
       // once_down = 1;
    }
    clear_screen();
    clcd_print("Downloaded the", LINE1(0));
    clcd_print("Logs", LINE2(4));
    __delay_ms(1000);

    return LOGIN_SUCCESS;
}

unsigned char set_time_log_screen(unsigned char key, unsigned char reset_flag, unsigned char speed)
{
    static unsigned char field_flag = 0; // 0 = SS ,1 = MM, 2=HH
    static unsigned char flag = 1;       // for blinking
    static unsigned char wait = 0;
    static unsigned int press_timer = 0;
    static unsigned char is_long_press = 0;

    clcd_print("TIME <HH:MM:SS>", LINE1(0));
    if (reset_flag == RESET_TIME)
    {
        display_time();
        field_flag = 0;
        return_time = 5;
    }

    // Use SW5 to change the fields
    // to toggle the fields
    if (wait++ == 5)
    {
        wait = 0;
        flag =! flag;
        if (field_flag == 0)
        {
            if (flag)
            {
                clcd_putch(time[4], LINE2(8));
                clcd_putch(time[5], LINE2(9));
            }
            else
            {
                clcd_putch(' ', LINE2(8));
                clcd_putch(' ', LINE2(9));
            }
        }
        else if(field_flag == 1)
        {
            if (flag)
            {
                clcd_putch(time[2], LINE2(5));
                clcd_putch(time[3], LINE2(6));
            }
            else
            {
                clcd_putch(' ', LINE2(5));
                clcd_putch(' ', LINE2(6));
            }
        }
        else if(field_flag == 2)
        {
            if (flag)
            {
                clcd_putch(time[0], LINE2(2));
                clcd_putch(time[1], LINE2(3));
            }
            else
            {
                clcd_putch(' ', LINE2(2));
                clcd_putch(' ', LINE2(3));
            }
        }
    }

    // check long press or short press if long press save else increment or change the fields
    if (key == SW4 || key == SW5)
    {
        if (prev_key != key)
        {
            press_timer = 0;
            is_long_press = 0;
            return_time = 5;
        }

        prev_key = key;
        press_timer++;

        if (press_timer > 20)
        {
            is_long_press = 1;
            press_timer = 0;

            // change the value to BCD
            unsigned char hour_val = ((time[0] - '0') * 10) + (time[1] - '0');
            unsigned char hour_bcd = ((hour_val / 10) << 4) | (hour_val % 10);
            unsigned char min_val = ((time[2] - '0') * 10) + (time[3] - '0');
            unsigned char min_bcd = ((min_val / 10) << 4) | (min_val % 10);
            unsigned char sec_val = ((time[4] - '0') * 10) + (time[5] - '0');
            unsigned char sec_bcd = ((sec_val / 10) << 4) | (sec_val % 10);
            write_ds1307(HOUR_ADDR, hour_bcd);
            write_ds1307(MIN_ADDR, min_bcd);
            write_ds1307(SEC_ADDR, sec_bcd);

            // return RETURN_back
            clear_screen();
            log_event("SL", speed);
            clcd_print("Time changed", LINE1(0));
            clcd_print("Successfully", LINE2(5));

            __delay_ms(1000);
            clear_screen();
            
            return LOGIN_SUCCESS;
        }
    }

    else if (key == ALL_RELEASED)
    {
        if (!is_long_press && press_timer > 0)
        {
            if (prev_key == SW5)
            {
                if (field_flag < 2)
                    field_flag++;
                else
                    field_flag = 0;
            }
            // Depends on the field increment the value
            switch (field_flag)
            {
            // for sec field
            case 0:
                if (prev_key == SW4)
                {
                    if (time[5] < '9')
                    {
                        time[5]++;
                    }
                    else
                    {
                        time[5] = '0';
                        if (time[4] < '5')
                        {
                            time[4]++;
                        }
                        else
                        {
                            time[4] = '0';
                        }
                    }
                }

                break;
                // for min field
            case 1:
                if (prev_key == SW4)
                {
                    if (time[3] < '9')
                    {
                        time[3]++;
                    }
                    else
                    {
                        time[3] = '0';
                        if (time[2] < '5')
                        {
                            time[2]++;
                        }
                        else
                        {
                            time[2] = '0';
                        }
                    }
                }
                break;

                // for hour field
            case 2:
                if (prev_key == SW4)
                {
                    if (time[0] != '2' || time[1] < '3')
                    {
                        if (time[1] < '9')
                        {
                            time[1]++;
                        }
                        else
                        {
                            time[1] = '0';
                            if (time[0] < '2')
                            {
                                time[0]++;
                            }
                        }
                    }
                    else
                    {
                        time[0] = '0';
                        time[1] = '0';
                    }
                }
                break;
            }

            // HH:MM:SS
            clcd_putch(time[0], LINE2(2)); // HH:MM:SS
            clcd_putch(time[1], LINE2(3));
            clcd_putch(':', LINE2(4));
            clcd_putch(time[2], LINE2(5)); // HH:MM:SS
            clcd_putch(time[3], LINE2(6));
            clcd_putch(':', LINE2(7));
            clcd_putch(time[4], LINE2(8)); // HH:MM:SS
            clcd_putch(time[5], LINE2(9));
        }
        press_timer = 0;
        is_long_press = 0;
    }
    return SET_TIME_SCREEN;
}

unsigned char change_password_screen(unsigned char key, unsigned char reset_flag, unsigned char speed)
{
    static char new_pass[4];
    static char re_pass[4];
    static unsigned char i;
    static unsigned char j;
    static unsigned char stage;

    if (reset_flag == NEW_PASS)
    {
        i = 0;
        j = 0;
        stage = 0;
        return_time = 5;
        clear_screen();
        clcd_print("Enter New Pass", LINE1(0));
        clcd_write(LINE2(4), INST_MODE);
        clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
        __delay_us(100);
    }

    if (return_time == 0)
    {
        return RETURN_BACK; // timeout-->go back to menu
    }

    // inputs
    if ((key == SW4 || key == SW5) && i < 4)
    {
        clcd_putch('*', LINE2(4 + i));
        if (stage == 0)
        {
            new_pass[i] = (key == SW4) ? '1' : '0';
        }
        else
        {
            re_pass[i] = (key == SW4) ? '1' : '0';
        }
        i++;
        return_time = 5;
    }
    if (i == 4) // after 4 digit entered
    {
        if (stage == 0)
        {
            // move to re-enter stage
            stage = 1;
            i = 0;
            clear_screen();
            clcd_print("Re-enter Pass", LINE1(0));
            clcd_write(LINE2(4), INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
        }
        else
        {
            // comparing new_pass and re_pass
            if (strncmp(new_pass, re_pass, 4) == 0)
            {
                for (int j = 0; j < 4; j++)
                {
                    write_ext_eeprom(j, new_pass[j]);
                }
                clear_screen();
                clcd_print("Password Changed", LINE1(0));
                clcd_print("Successfully", LINE2(0));
                __delay_ms(2000);
                return LOGIN_SUCCESS; // back to menu
            }
            else
            {
                clear_screen();
                clcd_print("Password Change", LINE1(0));
                clcd_print("Failed", LINE2(5));
                __delay_ms(2000);
                return RETURN_BACK; // failed ->go back to menu
            }
        }
    }

    return CHANGE_PASSWORD_SCREEN; // stay on screen
}

void clear_screen()
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE); // clear the previous screen
    __delay_us(500);
}
