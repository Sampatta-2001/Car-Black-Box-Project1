#include <xc.h>
#include "i2c.h"
#include "ds1307.h"

void init_ds1307(void)       //communicating with RTC to initialize
{
    unsigned char dummy;
   
    dummy = read_ds1307(SEC_ADDR);  //master nedds to read the data from the address to clear the 7th bit
    dummy = dummy & 0x7F;     //to clear  the 7th bit so that ch=0
    write_ds1307(SEC_ADDR, dummy);// ch = 0
}

unsigned char read_ds1307(unsigned char addr)
{
    unsigned char data;
    
    i2c_start();
    i2c_write(SLAVE_WRITE);
    i2c_write(addr);        //Transmit the address only then we can read the data
    i2c_rep_start();
    i2c_write(SLAVE_READ);
    data = i2c_read(0);   //here 0 is acknowldge
    i2c_stop();
    
    return data;
}

void write_ds1307(unsigned char addr, unsigned char data) // SEc_ADDR, data
{
    i2c_start();
    i2c_write(SLAVE_WRITE);
    i2c_write(addr);
    i2c_write(data);
    i2c_stop();
}

void write_ds1307_str_write(unsigned char addr , char *data)
{
    while(*data != 0)
    {
        write_ds1307(addr,*data);
        data++;
        addr++;
    }
 
}
