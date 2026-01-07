/*
 *File: main.h
 */

#ifndef MAIN_H
#define MAIN_H
#define _XTAL_FREQ 20000000

#include <xc.h>
#include "adc.h"
#include "clcd.h"
#include "digital_keypad.h"
#include "ds1307.h"
#include "i2c.h"
#include "car_black_box.h"
#include "uart.h"
#include <string.h>
#include "external_eeprom.h"
#include "timers.h"

#define DASHBOARD_SCREEN 0x01
#define LOGIN_SCREEN     0x02
#define MAIN_MENU_SCREEN 0x03

#define RESET_PASSWORD   0x11
#define RESET_NOTHING    0x22
#define RETURN_BACK      0x33
#define LOGIN_SUCCESS    0x44
#define RESET_MENU       0x55
#define RESET_VIEW       0x66
#define RESET_CLEAR      0x77
#define RESET_DOWN       0x88
#define RESET_TIME       0x99
#define NEW_PASS         0x17
#define RE_PASS          0x18
#define RESET_CHANGE     0x21



#define VIEW_LOG_SCREEN         0x12
#define CLEAR_LOG_SCREEN        0x13
#define DOWNLOAD_LOG_SCREEN     0x14
#define SET_TIME_SCREEN         0x15
#define CHANGE_PASSWORD_SCREEN  0x16
#define LONG_PRESS_SW4          0x19
#define LONG_PRESS_SW5          0x20
#endif