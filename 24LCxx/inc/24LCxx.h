/*
 *
*/

#include <stdint.h>

#ifndef __24LCXX_H__
#define __24LCXX_H__


void EE_Chip_Address(uint8_t pAddr);
void EE_Set_Memory(uint32_t pMemSize, uint8_t pPageSize);

uint32_t EE_Read   (uint32_t addr, uint8_t *buf, uint32_t cnt);
uint32_t EE_Write  (uint32_t addr, uint8_t *buf, uint32_t cnt);

uint8_t EE_SizeDetect(void);

uint32_t EE_Size(void);
uint16_t EE_PageSize(void);



#endif /* __EEPROM_H */
