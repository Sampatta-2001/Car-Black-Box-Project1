/*
 * File:   main.c
 * Author: Sampatta Ghulanawar
 * Project Name : Car Black Box
 * Created on 3 September, 2025, 8:02 PM
 */

#include "main.h"
#pragma config WDTE = OFF

extern unsigned char curr_pos; // menu position set by menu_screen()

unsigned int once_down;

static void init_config(void)
{
    // initialize CLCD
    init_clcd();
    // initialize I2C
    init_i2c(100000);
    // initialize RTC
    init_ds1307();
    // initialize ADC
    init_adc();
    // initialize keypad
    init_digital_keypad();
    // initialize timer2
    init_timer2();
    // initialize UART
    init_uart(9600);

    GIE = 1;
    PEIE = 1;
}

void main(void)
{
    unsigned char control_flag = DASHBOARD_SCREEN;
    unsigned char key, key1, prev_key = ALL_RELEASED;
    unsigned char reset_flag = RESET_NOTHING;
    unsigned char menu_pos;
    unsigned char event[3] = "ON"; // 2 chars + '\0'
    unsigned char speed = 0;
    const char *gear[] = {"GN","GR","G1","G2","G3","G4"};
    unsigned char gr = 0; // gear index 0 - 5
    static unsigned char once = 0;

    // temp result holders for state machines
    unsigned char login_res, menu_res, view_res, clr_res, download_res, set_time_res, change_password_res;

    init_config();

    // log ON at startup
    log_event(event, speed);

    // store default password (first 4 bytes only)
    write_ext_eeprom(0x00, '1');
    write_ext_eeprom(0x01, '0');
    write_ext_eeprom(0x02, '1');
    write_ext_eeprom(0x03, '0');

    while (1)
    {
        speed = read_adc() / 10.3; // //0 to 1023 <-> 0 to 99
        key1 = read_digital_keypad(LEVEL);
        key = read_digital_keypad(STATE);
        for (unsigned int i = 300; i > 0; i--); // debounce

        // ---- Dashboard events ----
        if (key == SW1)
        {
            strcpy((char *)event, "CO");
            log_event(event, speed);
        }
        else if (key == SW2 && gr < 6) // gear up
        {
            gr++;
            strcpy(event, gear[gr]);
            log_event(event, speed);
        }
        else if (key == SW3 && gr > 0) // gear down
        {
            gr--;
            strcpy((char *)event, gear[gr]);
            log_event(event, speed);
        }
        else if ((key == SW4 || key == SW5) && control_flag == DASHBOARD_SCREEN)
        {
            control_flag = LOGIN_SCREEN;
            clear_screen();
            clcd_print("Enter Password", LINE1(1));
            clcd_write(LINE2(4), INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
            reset_flag = RESET_PASSWORD;
            TMR2 = 0;   // reset timer2 counter
            TMR2ON = 1; // start inactivity timer
        }

        // ---- Main menu long press actions ----
        else if (control_flag == MAIN_MENU_SCREEN)
        {
            unsigned char result = menu_screen(key1, reset_flag);

            if (result == LONG_PRESS_SW4)
            {
                switch (curr_pos)
                {
                    case 0: // View log
                        clear_screen();
                        __delay_ms(500);
                        reset_flag = RESET_VIEW;
                        control_flag = VIEW_LOG_SCREEN;
                        if (!once)
                        {
                            log_event("VL", speed);
                            once = 1;
                        }
                        continue;

                    case 1: // Clear log
                        clear_screen();
                        reset_flag = RESET_CLEAR;
                        control_flag = CLEAR_LOG_SCREEN;
                        continue;

                    case 2: // Download log
                        clear_screen();
                        reset_flag = RESET_DOWN;
                        control_flag = DOWNLOAD_LOG_SCREEN;
                        once_down = 0;
                        puts("LOGS\n\r");
                        log_event("DL", speed);
                        continue;

                    case 3: // Set time
                        clear_screen();
                        reset_flag = RESET_TIME;
                        control_flag = SET_TIME_SCREEN;
                        continue;

                    case 4: // Change password
                        clear_screen();
                        clcd_print("Enter New Password", LINE1(1));
                        reset_flag = NEW_PASS;
                        TMR2 = 0;
                        TMR2ON = 1;
                        control_flag = CHANGE_PASSWORD_SCREEN;
                        continue;

                    default:
                        reset_flag = RESET_NOTHING;
                        break;
                }
            }
            else if (result == LONG_PRESS_SW5)
            {
                clear_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                display_dashboard(event, speed);
                control_flag = DASHBOARD_SCREEN; // instead of break
                continue;
            }
        }

        // ---- Screen state machine ----
        switch (control_flag)
        {
            case DASHBOARD_SCREEN:
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                display_dashboard(event, speed);
                break;

            case LOGIN_SCREEN:
                login_res = login(key, reset_flag);
                switch (login_res)
                {
                    case RETURN_BACK:
                        control_flag = DASHBOARD_SCREEN;
                        clear_screen();
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        TMR2ON = 0;
                        break;

                    case LOGIN_SUCCESS:
                        clear_screen();
                        control_flag = MAIN_MENU_SCREEN;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        reset_flag = RESET_MENU;
                        TMR2 = 0;
                        TMR2ON = 1;
                        continue;
                }
                break;

            case MAIN_MENU_SCREEN:
                menu_res = menu_screen(key1, reset_flag);
                if (menu_res == RETURN_BACK)
                {
                    control_flag = DASHBOARD_SCREEN;
                    clear_screen();
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2ON = 0;
                    break;
                }
                break;

            case VIEW_LOG_SCREEN:
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                once = 0;
                view_res = view_log_screen(key1, reset_flag);
                switch (view_res)
                {
                    case RETURN_BACK:
                        control_flag = DASHBOARD_SCREEN;
                        clear_screen();
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        TMR2ON = 0;
                        break;

                    case LOGIN_SUCCESS:
                        clear_screen();
                        control_flag = MAIN_MENU_SCREEN;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        reset_flag = RESET_MENU;
                        TMR2 = 0;
                        TMR2ON = 1;
                        continue;
                }
                break;

            case CLEAR_LOG_SCREEN:
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clr_res = clear_log_screen(reset_flag, speed);
                if (clr_res == RETURN_BACK)
                {
                    control_flag = DASHBOARD_SCREEN;
                    clear_screen();
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2ON = 0;
                    break;
                }
                if (clr_res == LOGIN_SUCCESS)
                {
                    clear_screen();
                    control_flag = MAIN_MENU_SCREEN;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2 = 0;
                    TMR2ON = 1;
                    continue;
                }
                break;

            case DOWNLOAD_LOG_SCREEN:
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                download_res = download_log_screen(key, reset_flag);
                if (download_res == RETURN_BACK)
                {
                    control_flag = DASHBOARD_SCREEN;
                    clear_screen();
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2ON = 0;
                    break;
                }
                if (download_res == LOGIN_SUCCESS)
                {
                    clear_screen();
                    control_flag = MAIN_MENU_SCREEN;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2 = 0;
                    TMR2ON = 1;
                    continue;
                }
                break;

            case SET_TIME_SCREEN:
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                set_time_res = set_time_log_screen(key1, reset_flag, speed);
                if (set_time_res == RETURN_BACK)
                {
                    control_flag = DASHBOARD_SCREEN;
                    clear_screen();
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2ON = 0;
                    break;
                }
                if(set_time_res == LOGIN_SUCCESS)
                {
                    clear_screen();
                    control_flag = MAIN_MENU_SCREEN;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    reset_flag = RESET_MENU;
                    TMR2 = 0;
                    TMR2ON = 1;
                    continue;
                }
                break;

            case CHANGE_PASSWORD_SCREEN:
                change_password_res = change_password_screen(key, reset_flag, speed);
                if (change_password_res == RETURN_BACK)
                {
                    control_flag = DASHBOARD_SCREEN;
                    clear_screen();
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2ON = 0;
                    break;
                }
                break;
        }

        reset_flag = RESET_NOTHING;
    }
}
