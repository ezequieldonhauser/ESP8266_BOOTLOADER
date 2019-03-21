/**********************************************
 * Source Code BOOTLOADER for ESP8266
 * by Bintechnology
**********************************************/

#include "helper.h"
#include "c_types.h"
#include "eagle_soc.h"
#include "api.h"
#include "config.h"

//binaries addresses
#define BINARY_APP1	0x00001000
#define BINARY_APP2	0x00081000

//the bootloader entry point
void USED bootloader_main(void)
{
	//this application don't use .data section
	//almost every data will be in stack, when we
	//return from bootloader_task() the stack will be
	//where the internal bootloader left us.
	__asm volatile
	(
		"l32r a2, _vec_base\n"		//load the vector base to a2
		"wsr.vecbase a2\n"			//set vecbase register with that value
		"mov a15, a0\n"				//store return addr
		"call0 bootloader_task\n"	//call bootloader_task() to check and load APP
		"mov a0, a15\n"				//restore return addr
		"jx a2\n"					//jump into the APP entry point
	);
}

//the bootloader task
//do all the stuff, check and load the application
//and return the address of the entry point, if get some error
//stay blinking the system led forever.
uint32 NOINLINE USED bootloader_task(void)
{
	uint32 entry_point;
	rtc_info_st info;

	//start the hardware watchdog
	hard_wdt_config();
	
	//-------------------------------------------------------------
	//CPU_PLL=80MHz (CRYSTAL=26MHz) 
	//rom_i2c_writeReg(103,4,1,136);
	//rom_i2c_writeReg(103,4,2,145);
	//ets_update_cpu_frequency(80);
	//uart_div_modify(0, UART_START_CLOCK/115200);
	//-------------------------------------------------------------
	
	ets_set_user_start(0);
	ets_delay_us(100);
	ets_printf("\r\n\r\n");
	ets_printf("BINTECHNOLOGY BOOTLOADER "BOOTLOADER_VERSION"\r\n");
	
	//get the data from RTC mem
	rtc_read_configs(&info);

	//check if button is pressed or the UPDATER MODE flag is true
	//then we sould load the UPDATER APPLICATION
	if((get_button())||(info.device_mode==DEVICE_MODE_UPDATER))
	{
		//check and load the binary APP1
		entry_point = check_load_binary(BINARY_APP1);

		if(entry_point != 0)
		{
			ets_printf("LOADING APP1...\r\n");
			ets_delay_us(100);
			return entry_point;
		}
		
		//could'nt load APP1 (UPDATER)
		ets_printf("ERROR LOADING APP1\r\n");
	}

	//check and load the APP2, the default APP 
	entry_point = check_load_binary(BINARY_APP2);

	if(entry_point != 0)
	{
		ets_printf("LOADING APP2...\r\n");
		ets_delay_us(100);
		return entry_point;
	}

	//could'nt load APP2 (APPLICATION)
	ets_printf("ERROR LOADING APP2\r\n");

	//check and load the binary APP1
	entry_point = check_load_binary(BINARY_APP1);

	if(entry_point != 0)
	{
		//set the default APP to the RTC
		info.device_mode = DEVICE_MODE_UPDATER;
		rtc_set_device_mode(&info);
		
		ets_printf("LOADING APP1\r\n");
		ets_delay_us(100);
		return entry_point;
	}

	//error loading
	ets_printf("FATAL ERROR\r\n");
	ets_printf("HARD WDT RST COUNT: %lu\r\n", info.hard_wdt_rst_cnt);
	ets_printf("EXCEPTION RST COUNT: %lu\r\n", info.exception_rst_cnt);
	ets_printf("SOFT WDT RST COUNT: %lu\r\n", info.soft_wdt_rst_cnt);
	ets_printf("SYSTEM RST COUNT: %lu\r\n", info.system_rst_cnt);
	ets_printf("AWAKE RST COUNT: %lu\r\n", info.awake_rst_cnt);
	ets_printf("EXTERN RST COUNT: %lu\r\n", info.extern_rst_cnt);

	//call the blink led forever
	blink_led();
	
	return 0;//will not arrive here
}
