#ifndef EXT_EEPROM_H
#define	EXT_EEPROM_H

#define SLAVE_WRITE_EXT         0xA0 // 0xD0
#define SLAVE_READ_EXT          0xA1 // 0xD1

#define SEC_ADDR                0x00
#define MIN_ADDR                0x01
#define HOUR_ADDR               0x02


void init_at24c04(void);
unsigned char read_ext_eeprom(unsigned char addr);
void write_ext_eeprom(unsigned char addr, unsigned char data);
void ext_eeprom_24C02_str_write(unsigned char addr , char *data);

#endif	/* EXT_EEPROM_H */

