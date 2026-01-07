#include <xc.h>
#include "uart.h"

void init_uart(unsigned long baud)
{
    /* Setting RC6 and RC7 to work as Serial Port */
    SPEN = 1;
    
    TXEN=1;   //Transmit bit enabled
    
    /* Continuous Reception Enable Bit */
    CREN = 1;
    
    BRGH=1;  //high speed
    
    /* Baud Rate Setting Register */
    SPBRG = (FOSC / (16 * baud)) - 1;     //Baud rate=FOSC/16(x+1) where x is calculated
}

unsigned char getchar(void)
{
    /* Wait for the byte to be received */
    while (RCIF != 1)
    {
        continue;
    }
    
    /* Clear the interrupt flag */
    RCIF = 0;
    
    /* Return the data to the caller */
    return RCREG;
}                                                         // ^
                                                          // |
void putchar(unsigned char data)   //----------------------  |
{
    /* Transmit the data to the Serial Port */
    TXREG = data;
    
    /* Wait till the transmission is complete */
    do
    {
        continue;
    } while (TXIF != 1);
    
    /* Clear the interrupt flag */
    TXIF = 0;
}

void puts(const char *s)
{
    while (*s)
    {
        putchar(*s++);
    }
}