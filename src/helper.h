#ifndef ROM_FUNCTIONS_H
#define ROM_FUNCTIONS_H

#include "c_types.h"
#include "eagle_soc.h"

//------------------------------------------------------------------------------
// ROM FUNCTIONS
//------------------------------------------------------------------------------
extern void ets_set_user_start(void (*user_start_fn)());
extern void ets_update_cpu_frequency(uint8);
extern void ets_printf(char*, ...);
extern void ets_delay_us(int);
extern void ets_memset(void*, uint8, uint32);
extern void ets_memcpy(void*, const void*, uint32);
extern void software_reset(void);
extern void gpio_output_set(uint32_t,uint32_t,uint32_t,uint32_t);
extern uint32 gpio_input_get(void);
extern void rom_i2c_writeReg(uint32, uint32, uint32, uint32);
extern void uart_div_modify(uint32_t, uint32_t);
extern uint32 SPIRead(uint32 addr, void *outptr, uint32 len);
extern uint32 SPIWrite(uint32 addr, void *inptr, uint32 len);

struct MD5Context
{
    uint32_t buf[4];
    uint32_t bits[2];
    uint8_t in[64];
};

extern void MD5Init(struct MD5Context *ctx);
extern void MD5Update(struct MD5Context *ctx, void *buf, uint32_t len);
extern void MD5Final(uint8_t digest[16], struct MD5Context *ctx);



#define UART_START_CLOCK	(80000000)



#define GPIO_OUTPUT_SET(gpio_no, bit_value)			\
	gpio_output_set((bit_value)<<gpio_no,			\
	((~(bit_value))&0x01)<<gpio_no, 1<<gpio_no,0)	\


//------------------------------------------------------------------------------
// REGISTERS
//------------------------------------------------------------------------------

//DPORT REGISTERS
#define INTC_EDGE_EN	*(volatile uint32_t *)0x3FF00004

//RTC REGISTERS
#define WDT_CTRL		*(volatile uint32_t *)0x60000900
#define WDT_REG1		*(volatile uint32_t *)0x60000904
#define WDT_REG2		*(volatile uint32_t *)0x60000908
#define WDT_FEED		*(volatile uint32_t *)0x60000914
#define WDT_FEED_MAGIC	0x73

#endif
