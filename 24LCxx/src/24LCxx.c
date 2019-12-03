/*
 *
*/

#include "24LCxx.h"
#include "i2c.h"
//http://www.t-es-t.hu/download/microchip/an690a.pdf


extern void Delay_ms(uint32_t pWait);

/*
 * PRIVATE PROTOTYPES
*/
uint32_t __EE_WrBuf (uint16_t addr, uint8_t *buf, uint32_t cnt);
uint32_t __EE_RdBuf (uint16_t addr, uint8_t *buf, uint32_t cnt);
void __EE_Wr(uint16_t pAddr, uint8_t pVal);
uint8_t __EE_Rd(uint16_t pAddr);
uint16_t __EEP_detSize(uint8_t pType);
uint8_t __EE_CheckAddress(uint32_t addr);
void print_EE_all(void);



/*
 * PRIVATE VARIABLES
*/
uint8_t _pri_index_memory = 0;  //USED TO STORE THE INDEX OF VARIABLE MEMORIES
uint8_t _pri_EEP_ADDRESS = 0xA0;
uint8_t _pri_EEP_PAGE_SIZE = 64;
uint32_t _pri_EEP_MEM_SIZE = 256;

typedef struct{
    uint16_t mem_size;
    uint8_t page_size;
} t_eeprom_detect;



/*
 *      EE_Read:  Read cnt bytes from specified address into buf
 *
 *  Parameter:  addr - Byte address
 *              buf  - Data buffer
 *              cnt  - Number of bytes to read
 *
 *  Return:     0 on success, nonzero on error
*/
uint32_t EE_Read (uint32_t addr, uint8_t *buf, uint32_t cnt) {

	if (addr < (_pri_EEP_MEM_SIZE*1024)) {
		__EE_RdBuf ((uint16_t)addr, buf, cnt);
		return 0;
		/*if (__EE_RdBuf ((uint16_t)addr, buf, cnt) == 0) {
      return (0);
    }*/
	}
	return (1);
}

/*
 *      EE_Write:  Write cnt bytes from buf to the specified address
 *
 *  Parameter:  addr - Byte address
 *              buf  - Data buffer
 *              cnt  - Number of bytes to write
 *
 *  Return:     0 on success, nonzero on error
*/
uint32_t EE_Write (uint32_t addr, uint8_t *buf, uint32_t cnt) {
//	uint32_t num;

	if (addr < (_pri_EEP_MEM_SIZE*1024)) {
		__EE_WrBuf ((uint16_t)addr, buf, cnt);
    		/*while (cnt) {
      if (cnt > _pri_EEP_PAGE_SIZE) {
        num  = _pri_EEP_PAGE_SIZE;
        cnt -= _pri_EEP_PAGE_SIZE;
      }
      else {
        num = cnt;
        cnt = 0;
      }
      if (__EE_WrBuf ((uint16_t)addr, buf, num)) {
        return (1);
      }
    }*/
    		return (0);
	}
	return (1);
}

/*
 * Run a routine to try detect the size of memory
 *
 * Return:
 *  0 - detect
 *  1 - not detect
*/
uint8_t EE_SizeDetect(void){
    uint8_t i = 0;
    uint32_t __addr = 0;

    for(i=0; i<__EEP_detSize(2); i++){
        _pri_index_memory = i;
        __addr = ((__EEP_detSize(1)*128));
//        PRINTSTR("\nindex "); PRINTVALUE(_pri_index_memory);
//        PRINTSTR("\nSize k: "); PRINTVALUE(__EEP_detSize(1));
//        PRINTSTR("\nMem: "); PRINTVALUE(__addr);

        if(__EE_CheckAddress(__addr)==0){
//        	PRINTSTR("\nDetectado");
            _pri_EEP_MEM_SIZE = __EEP_detSize(1);
            _pri_EEP_PAGE_SIZE = __EEP_detSize(0);
//            __EE_Wr(__addr, 0xAC);
//            print_EE_all();
//            __EE_Wr(__addr, 0xFF);
            return 0;
        }
    }

//    print_EE_all();

    return 1;
}

/*
 * Returns the size of memory
 *
*/
uint32_t EE_Size(void){
    return __EEP_detSize(1)*1024;
}

/*
 * Returns the page size of memory
 *
*/
uint16_t EE_PageSize(void){
    return __EEP_detSize(0);
}

/*
 * Set the addres of current EEPROM chip
 *
*/
void EE_Chip_Address(uint8_t pAddr){
    _pri_EEP_ADDRESS = pAddr;
}

void EE_Set_Memory(uint32_t pMemSize, uint8_t pPageSize){
	_pri_EEP_PAGE_SIZE = pPageSize;
	_pri_EEP_MEM_SIZE = pMemSize;
}








/*
 *      EE_WrBuf:
 *
 * Parameters: addr - EEPROM location address
 *             buf  - data buffer
 *             cnt  - number of bytes to read
 *
 * Return:     0 on success, nonzero on error
 *
 * ERRATA:
 * 	- Ulysses - 04/2018 - adicionado o tratamento para gravar dados pulando de pagina automaticamente.
*/
uint32_t __EE_WrBuf (uint16_t addr, uint8_t *buf, uint32_t cnt) {
  uint32_t c=0;
  uint32_t err=0;

  while(c < cnt){
	  I2CStart(LPC_I2C, _pri_EEP_ADDRESS);
	  I2CWrite(LPC_I2C, (addr >> 8));
	  I2CWrite(LPC_I2C, (addr & 0xFF));

	  while (c < cnt) {
//		  I2CWrite(LPC_I2C, *buf++);
		  I2CWrite(LPC_I2C, buf[c++]);
		  if(!((c+addr)%_pri_EEP_PAGE_SIZE)){break;}
	  }
	  I2CStop(LPC_I2C);

	  Delay_ms(5);
  }

  return (err);
}

/*
 *      EE_RdBuf:
 *
 * Parameters: addr - EEPROM location address
 *             buf  - data buffer
 *             cnt  - number of bytes to read
 *
 * Return:     0 on success, nonzero on error
*/
uint32_t __EE_RdBuf (uint16_t addr, uint8_t *buf, uint32_t cnt) {


	I2CStart(LPC_I2C, _pri_EEP_ADDRESS);
	I2CWrite(LPC_I2C, (addr >> 8));
	I2CWrite(LPC_I2C, (addr & 0xFF));
	I2CStop(LPC_I2C);
	I2CStart(LPC_I2C, _pri_EEP_ADDRESS|0x01);

	while (cnt--) {
		if (cnt) *buf = I2CRead(LPC_I2C,ACK);
		else *buf = I2CRead(LPC_I2C,NACK);
		buf++;
	}

	I2CStop(LPC_I2C);

	return 0;
}

void __EE_Wr(uint16_t pAddr, uint8_t pVal){
	I2CStart(LPC_I2C, _pri_EEP_ADDRESS);
	I2CWrite(LPC_I2C, (pAddr >> 8));
	I2CWrite(LPC_I2C, (pAddr & 0xFF));
	I2CWrite(LPC_I2C, pVal);
	I2CStop(LPC_I2C);

	Delay_ms(5);
}

uint8_t __EE_Rd(uint16_t pAddr){
	uint8_t __buf = 0x00;
	I2CStart(LPC_I2C, _pri_EEP_ADDRESS);
	I2CWrite(LPC_I2C, (pAddr >> 8));
	I2CWrite(LPC_I2C, (pAddr & 0xFF));
	I2CStop(LPC_I2C);
	I2CStart(LPC_I2C, _pri_EEP_ADDRESS|0x01);
	__buf = I2CRead(LPC_I2C,NACK);
	I2CStop(LPC_I2C);

	return __buf;
}

void print_EE_all(void){
	uint16_t i = 0;
	uint8_t j = 0;
	uint8_t buf;

//	PRINTSTR("\r\n");
//	PRINTSTR("\r\n");
//	PRINTSTR("ADDR ");PRINTVALUE_HEX(0);PRINTSTR("-");

	for(i=0 ; i<(64*128); i++){
		if(j==64){
			j = 0;
//			PRINTSTR("\r\n");
//			PRINTSTR("ADDR ");PRINTVALUE_HEX(i);PRINTSTR("-");
		}
		buf = __EE_Rd(i);
//		PRINTSTR("0x");PRINTVALUE_HEX(buf); PRINTSTR(" ");
		j++;
	}
//	PRINTSTR("\r\n");
//	PRINTSTR("\r\n");
}

/*
 * Write the value 0xAA at the specific address and return the value of ACKNOWLEDGE
 *
 * Parameter:
 *  uint8_t addr - address to write
 *
 * Return:
 *		0 - success
 *		1 - error
*/
uint8_t __EE_CheckAddress(uint32_t addr){
	uint8_t ret = 1;
    uint8_t last_value = 0x00;
    uint8_t last_value_0 = 0x00;
    uint8_t value = 0xAA;
    uint8_t buf = 0x00;

    last_value = __EE_Rd(addr);
    last_value_0 = __EE_Rd(0x00);

    if(last_value == value){value = 0xBB;}
    __EE_Wr(addr, value);

    buf = __EE_Rd(0x00);
    if(last_value_0 != buf){
    	ret = 0;
    }

    __EE_Wr(0x00, last_value_0);
    __EE_Wr(addr, last_value);

    return ret;
}

/*
 * Returns the size specification of memory detect, or internaly used to return the new value to read.
 *
 * Parameter:
 *  uint8_t pType - type of value returned
 *      0 - return page size in kilobits
 *      1 - return memory size in bytes
 *      2 - max memories available for analysis
 *
 * Return: size of type
*/
uint16_t __EEP_detSize(uint8_t pType){

    t_eeprom_detect memories[] ={
        {2, 8},     //24LC02
        {4, 16},    //24LC04
        {8, 16},    //24LC08
        {16, 16},   //24LC16
        {32, 32},   //24LC32
        {64, 32},   //24LC64
        {128, 64},  //24LC128
        {256, 64},  //24LC256
        {512, 128}, //24LC512
//        {1024, 128},//24LC1024
    };

    switch(pType){
    case 0: return memories[_pri_index_memory].page_size; break;
    case 1: return memories[_pri_index_memory].mem_size; break;
    case 2: return (sizeof(memories)-1); break;
    }

    return 0;
}

