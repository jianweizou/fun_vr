#ifndef 	__FLASH_COMPATIBLE_FUN_H__
#define 	__FLASH_COMPATIBLE_FUN_H__

#include 	<stdint.h>

#define SIZE_OTA		(768 * 1024)	//764K
#define SIZE_SECTOR		4096	
#define SIZE_HEADER		4096
#define SIZE_FAC_FW		(768 * 1024)

#define	PROTECT_AREA_NONE			0	/* 0 */
#define	PROTECT_AREA_CRITICAL		1	/* 0x200000 - 2MB */
#define	PROTECT_AREA_FW				2	/* 0x300000 - 3MB */
#define	PROTECT_AREA_UPDATE			3	/* 0x3C0000 - 3.75MB */
#define	PROTECT_AREA_ALL			99	/* PROTECT All Flash */		

typedef struct{
    uint32_t boot_loader;
    uint32_t sn;
    uint32_t test_mode;
    uint32_t factory_fw;
    uint32_t voice;
    uint32_t execute_fw;
    uint32_t config_save;
    uint32_t debug_log;
    uint32_t update_buf;
    uint32_t boot_info;
    uint32_t private_config;
}stAddrMap_t;

typedef struct{
    union{
        uint32_t data;
        struct{
            uint8_t memory_desity;
            uint8_t memory_type;
            uint8_t manufacturer;
        }info;
    }id;

    uint32_t 	size;
    uint8_t 	max_PROTECT_lv;
    uint8_t		PROTECT_from_lsb;
    void 		(*protect_func)(uint8_t);

}stFlashDetail_t;

//void flash_model_setup(void);
stFlashDetail_t* flash_model_select(uint32_t id);
void flash_protect_adapter(uint8_t level);


void protect_KH25L(uint8_t level);
void protect_MX25L64(uint8_t level);
void protect_MX25L128(uint8_t level);
void protect_GD25Q32(uint8_t level);

#endif
