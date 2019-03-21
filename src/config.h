#ifndef CONFIG_H
#define CONFIG_H

//bootloader version
#define BOOTLOADER_VERSION	"1.00"

//PIN 5  = GPIO14 = LED
#define PIN_LED		14			//GPIO_NUM_14
#define MUX_LED		0x6000080C	//PERIPHS_IO_MUX_MTMS_U
#define FUN_LED		3			//FUNC_GPIO14

//PIN 12 = GPIO0  = BUTTON
#define PIN_BTN		0			//GPIO_NUM_14
#define MUX_BTN		0x60000834	//PERIPHS_IO_MUX_GPIO0_U
#define FUN_BTN		0			//FUNC_GPIO0

#endif
