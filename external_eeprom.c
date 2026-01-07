/*
 * File:   external_eeprom.c
 * Author: sampatta ghulanawar
 *
 * Created on 5 September, 2025, 7:53 PM
 */


#include <xc.h>
#include "i2c.h"
#include "external_eeprom.h"

/*void init_at24c04(void)
{
    unsigned char dummy;
   
    dummy = read_ds1307(SEC_ADDR);
    dummy = dummy & 0x7F;
    write_ds1307(SEC_ADDR, dummy);// ch = 0
}*/


unsigned char read_ext_eeprom(unsigned char addr)
{
    unsigned char data;
    
    i2c_start();
    i2c_write(SLAVE_WRITE_EXT);
    i2c_write(addr);
    i2c_rep_start();
    i2c_write(SLAVE_READ_EXT);
    data = i2c_read(0);
    i2c_stop();
    
    return data;
}

void write_ext_eeprom(unsigned char addr, unsigned char data) // SEc_ADDR, data
{
    i2c_start();
    i2c_write(SLAVE_WRITE_EXT);
    i2c_write(addr);
    i2c_write(data);
    i2c_stop();
}

void ext_eeprom_24C02_str_write(unsigned char addr , char *data)
{
    while(*data != 0)
    {
        write_ext_eeprom(addr,*data);
        data++;
        addr++;
    }
 
}
