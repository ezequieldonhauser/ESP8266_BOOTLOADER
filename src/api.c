#include "c_types.h"
#include "api.h"
#include "helper.h"
#include "config.h"

#define CHECKSUM_INIT	0xEF
#define MAGIC_NUMBER	0xAA55
#define RTC_SYS_ADDR	0x60001100
#define RTC_USER_ADDR	0x60001140
#define HEADER_MAGIC	0xEA
#define SECTION_MAGIC	0xE9
#define CHECKSUM_INIT	0xEF

//<editor-fold defaultstate="collapsed" desc="void USED rtc_set_device_mode(rtc_info_st *info)">
void USED rtc_set_device_mode(rtc_info_st *info)
{
	//set the RTC bootloader flag to the default APP2
	
	uint32 temp, i;
	uint8 crc, *ptr, *addr;

	//write magic and len
	addr = ((uint8*)RTC_USER_ADDR);
	temp = (uint32)(sizeof(rtc_info_st)<<16 | MAGIC_NUMBER);
	ets_memcpy((uint8*)addr, (uint8*)&temp, sizeof(uint32));
	
	//write RTC user info
	addr = ((uint8*)(RTC_USER_ADDR + 4));
	ets_memcpy((uint8*)addr, (uint8*)info, sizeof(rtc_info_st));
	
	//generate CRC to save into the RTC mem
	crc = CHECKSUM_INIT;
	ptr = (uint8*)info;
	for(i=0; i<sizeof(rtc_info_st); i++)
		crc ^= *ptr++;
	temp = (uint32)crc;
	
	//write CRC into RTC mem
	addr = ((uint8*)(RTC_USER_ADDR + 4 + sizeof(rtc_info_st)));
	ets_memcpy((uint8*)addr, (uint8*)&temp, sizeof(uint32));
}
//</editor-fold>
//<editor-fold defaultstate="collapsed" desc="void USED rtc_read_configs(rtc_info_st *info)">
void USED rtc_read_configs(rtc_info_st *info)
{
	uint32 temp, i;
	uint32 magic, len;
	uint8 crc, *ptr, *addr;
	rst_info rst;
	
	//read RTC reset data
	addr = ((uint8*)RTC_SYS_ADDR);
	ets_memcpy((uint8*)&rst, (uint8*)addr, sizeof(rst_info));
	if(rst.reason > 6)//at the first time is invalid data
		ets_memset((uint8*)&rst, 0, sizeof(rst_info));
	
	//check magic and len RTC user data
	addr = ((uint8*)RTC_USER_ADDR);
	ets_memcpy((uint8*)&temp, (uint8*)addr, sizeof(uint32));
	
	len = (uint32)((temp>>16)&0xFFFF);
	magic = (uint32)(temp&0xFFFF);
	
	if((magic != MAGIC_NUMBER)||(len != sizeof(rtc_info_st)))
		goto load_default;
	
	//read user data from RTC mem
	addr = ((uint8*)(RTC_USER_ADDR + 4));
	ets_memcpy((uint8*)info, (uint8*)addr, sizeof(rtc_info_st));
	
	//generate CRC to save into the RTC mem
	crc = CHECKSUM_INIT;
	ptr = (uint8*)info;
	for(i=0; i<sizeof(rtc_info_st); i++)
		crc ^= *ptr++;

	//read CRC from RTC mem
	addr = ((uint8*)(RTC_USER_ADDR + 4 + sizeof(rtc_info_st)));
	ets_memcpy((uint8*)&temp, (uint8*)addr, sizeof(uint32));
	
	//compare calculated CRC with readed CRC
	if(temp != (uint32)crc)
		goto load_default;

	//copy the reset info into user info
	info->reason	= rst.reason;
    info->exccause	= rst.exccause;
    info->epc1		= rst.epc1;
    info->epc2		= rst.epc2;
    info->epc3		= rst.epc3;
    info->excvaddr	= rst.excvaddr;
    info->depc		= rst.depc;

	//increment the spesific rst reason
	if(rst.reason == REASON_WDT_RST)
		info->hard_wdt_rst_cnt++;
	else if(rst.reason == REASON_EXCEPTION_RST)
		info->exception_rst_cnt++;
	else if(rst.reason == REASON_SOFT_WDT_RST)
		info->soft_wdt_rst_cnt++;
	else if(rst.reason == REASON_SOFT_RESTART)
		info->system_rst_cnt++;
	else if(rst.reason == REASON_DEEP_SLEEP_AWAKE)
		info->awake_rst_cnt++;
	else if(rst.reason == REASON_EXT_SYS_RST)
		info->extern_rst_cnt++;
	
	//success
	goto end;

//if arraives here, then copy default configuration
load_default:
	ets_memset((uint8*)info, 0, sizeof(rtc_info_st));
	info->device_mode = DEVICE_MODE_APP;
	info->reason	= rst.reason;
    info->exccause	= rst.exccause;
    info->epc1		= rst.epc1;
    info->epc2		= rst.epc2;
    info->epc3		= rst.epc3;
    info->excvaddr	= rst.excvaddr;
    info->depc		= rst.depc;
	goto end;

//save the new RTC user info
end:
	//write magic and len
	addr = ((uint8*)RTC_USER_ADDR);
	temp = (uint32)(sizeof(rtc_info_st)<<16 | MAGIC_NUMBER);
	ets_memcpy((uint8*)addr, (uint8*)&temp, sizeof(uint32));
	
	//write RTC user info
	addr = ((uint8*)(RTC_USER_ADDR + 4));
	ets_memcpy((uint8*)addr, (uint8*)info, sizeof(rtc_info_st));
	
	//generate CRC to save into the RTC mem
	crc = CHECKSUM_INIT;
	ptr = (uint8*)info;
	for(i=0; i<sizeof(rtc_info_st); i++)
		crc ^= *ptr++;
	temp = (uint32)crc;
	
	//write CRC into RTC mem
	addr = ((uint8*)(RTC_USER_ADDR + 4 + sizeof(rtc_info_st)));
	ets_memcpy((uint8*)addr, (uint8*)&temp, sizeof(uint32));
	return;
}
//</editor-fold>
//<editor-fold defaultstate="collapsed" desc="uint32 USED check_load_binary(uint32 flash_addr)">
uint32 USED check_load_binary(uint32 flash_addr)
{
	//check and load one of the two applications APP1 or APP2
	//then return the address to that application entry point
	
	uint32 i, j;
	uint32 remaining;
	uint32 readlen;
	uint32 load_addr;
	binary_header header;
	section_header section;
	uint8 buffer[256];
	uint8 checksum;
	
	//read the first header 
	if(SPIRead(flash_addr, &header, sizeof(binary_header)))
		return 0;
	
	//check the header magic and number of sections
	if((header.magic!=HEADER_MAGIC)||(header.count==0))
		return 0;
	
    flash_addr += sizeof(binary_header);

	//ignore the section ROM0 jump to the end of this section
    if(SPIRead(flash_addr, &section, sizeof(section_header)))
		return 0;
	
    flash_addr += sizeof(section_header);
    flash_addr += section.length;
	
    //read the secound header
    if(SPIRead(flash_addr, &header, sizeof(binary_header)))
		return 0;
	
	//check the header magic and number of sections
	if((header.magic!=SECTION_MAGIC)||(header.count==0))
		return 0;
	
    flash_addr += sizeof(binary_header);
	checksum = CHECKSUM_INIT;

	//calculate the checksum of all sections
    for(i=0; i<header.count; i++)
    {
        //read section header
        if(SPIRead(flash_addr, &section, sizeof(section_header)))
			return 0;
		
        flash_addr += sizeof(section_header);
		load_addr = section.address;
        remaining = section.length;

		//calculate the checksum
		while (remaining > 0)
		{
			//read the len of the buffer
			readlen = (remaining < sizeof(buffer)) ? remaining:sizeof(buffer);
			
			//read to the local buffer
			if(SPIRead(flash_addr, buffer, readlen))
				return 0;

			//load into the espesific address
			ets_memcpy((uint32*)load_addr, (uint32*)buffer, readlen);

			//add to chksum
			for(j=0; j<readlen; j++)
				checksum ^= buffer[j];
			
			//update the controls
			flash_addr += readlen;
			load_addr += readlen;
			remaining -= readlen;
		}
    }

	//read the last byte of the binary
	//that contains the checksum byte
	flash_addr = (flash_addr | 0xF) - 3;
	if(SPIRead(flash_addr, buffer, 4) != 0)
		return 0;

	if(buffer[3] != checksum)
		return 0;

	//everything checked
	return header.entry;
}
//</editor-fold>
//<editor-fold defaultstate="collapsed" desc="void USED hard_wdt_config(void)">
void USED hard_wdt_config(void)
{
	//start the HARD WDT to restart if something goes wrong
	WDT_CTRL &= 0x7e;	//Disable WDT
	INTC_EDGE_EN |= 1;	//0x3ff00004 |= 1
	WDT_REG1 = 0xb;		//WDT timeout
	WDT_REG2 = 0xd;
	WDT_CTRL |= 0x38;
	WDT_CTRL &= 0x79;
	WDT_CTRL |= 1;		//Enable WDT
}
//</editor-fold>
//<editor-fold defaultstate="collapsed" desc="void USED blink_led(void)">
void USED blink_led(void)
{
	//when the bootloader get an exception or can't load APP1 or APP2
	//it will stay on this function until power down
	
	uint32_t main_ree=0;
	uint32_t main_contador=1000;
	
	PIN_FUNC_SELECT(MUX_LED, FUN_LED);
	while(1)
    {
        if(main_contador)main_contador--;
        else
        {
            main_contador=2000000;
            if(main_ree)
            {
                GPIO_OUTPUT_SET(PIN_LED, 1);
                main_ree=0;
            }
            else
            {
                GPIO_OUTPUT_SET(PIN_LED, 0);
                main_ree=1;
            }
        }
    }
}
//</editor-fold>
//<editor-fold defaultstate="collapsed" desc="uint32 USED get_button(void)">
uint32 USED get_button(void)
{
	//get status of the button
	//return 1 - button pressed pin=down
	
	PIN_FUNC_SELECT(MUX_BTN, FUN_BTN);
	PIN_PULLUP_EN(MUX_BTN);//pullup
	gpio_output_set(0, 0, 0, 1<<PIN_BTN);
	return (GPIO_INPUT_GET(PIN_BTN)==0);
}
//</editor-fold>
