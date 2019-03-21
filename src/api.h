#ifndef API_H
#define API_H

#include "c_types.h"

//------------------------------------------------------------------------------
//MEMORY MAP - BINTECHNOLOGY 1.00
//------------------------------------------------------------------------------
//0x00000		4KBytes		BOOTLOADER_BINARY	INIT
//0x10000		384KBytes	UPDATER_BINARY		APP1
//0x61000		4KBytes		RF_CALL				APP1
//0x62000		4KBytes		PHY_DATA			APP1
//0x63000		16KBytes	SYS_INIT_DATA		APP1
//0x67000		4KBytes		RF_CALL				APP2
//0x68000		4KBytes		PHY_DATA			APP2
//0x69000		16KBytes	SYS_INIT_DATA		APP2
//0x6D000		4KBytes		ALL_CONFIGS			CONF
//0x6E000		72KBytes	APP2_CONFIGS		APP2
//0x80000		4KBytes		RESERVED			NULL
//0x81000		508KBytes	APP2_BINARY			APP2
//------------------------------------------------------------------------------

typedef struct {
	uint8 magic;
	uint8 count;
	uint8 flags1;
	uint8 flags2;
	uint32 entry;
}binary_header;

typedef struct {
	uint32 address;
	uint32 length;
}section_header;

enum rst_reason {
	REASON_DEFAULT_RST		= 0,
	REASON_WDT_RST			= 1,
	REASON_EXCEPTION_RST	= 2,
	REASON_SOFT_WDT_RST   	= 3,
	REASON_SOFT_RESTART 	= 4,
	REASON_DEEP_SLEEP_AWAKE	= 5,
	REASON_EXT_SYS_RST      = 6
};

enum device_mode
{
	DEVICE_MODE_UPDATER = 0,
	DEVICE_MODE_APP = 1,
	DEVICE_MODE_CONFIG = 2
};

typedef struct {
    uint32 reason;
    uint32 exccause;
    uint32 epc1;
    uint32 epc2;
    uint32 epc3;
    uint32 excvaddr;
    uint32 depc;
}rst_info;

typedef struct {
	uint32 reason;
    uint32 exccause;
    uint32 epc1;
    uint32 epc2;
    uint32 epc3;
    uint32 excvaddr;
    uint32 depc;
	uint32 device_mode;
    uint32 hard_wdt_rst_cnt;
	uint32 exception_rst_cnt;
	uint32 soft_wdt_rst_cnt;
	uint32 system_rst_cnt;
	uint32 awake_rst_cnt;
	uint32 extern_rst_cnt;
}rtc_info_st;

//prototypes
void USED rtc_set_device_mode(rtc_info_st *info);
void USED rtc_read_configs(rtc_info_st *info);
uint32 USED check_load_binary(uint32 flash_addr);
void USED hard_wdt_config(void);
void USED blink_led(void);
uint32 USED get_button(void);

#endif
