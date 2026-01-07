#include <xc.h>

void init_adc()
{
    ADCS2 = 0;
    ADCS1 = 1;  //ADC clock frequency (0.625MHz)
    ADCS0 = 0;
    
    CHS2 = 0;
    CHS1 = 0; // selected channel 0(AN0)
    CHS0 = 0;
    
    ADON = 1; //ADC on
    ADFM = 1; //right justified result
    
    PCFG3 = 1;
    PCFG2 = 1; // make AN0 as analog
    PCFG1 = 1;
    PCFG0 = 0;
}

unsigned short read_adc()
{
    // initiate the conversion
    GO = 1;
    // wait till the conversion complete
    while(GO);
    //fetch value from ADC reg
      return ADRESL |  ( (ADRESH & 0x03) << 8) ;
}