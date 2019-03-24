/**
 * @file
 * this is sd control header file, include this file before use
 * @author CJ
 */

#ifndef __SD_CTRL_H__
#define __SD_CTRL_H__

#ifdef USE_SONIX_CONFIG
#include "sonix_config.h"
#endif

#if defined (CONFIG_OS_FREERTOS)
#define OS_ON					1
#endif

#if(OS_ON == 1)
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#endif

#define SD_BUS_WIDTH_1BIT								0x0
#define SD_BUS_WIDTH_4BIT								0x2

#define	SD_MEMORY_IO										0x00
#define	SD_MEMORY_CARD									0x01
#define	SD_MEMORY_UNKNOWN								0x02

#define	SD_TAKE_SEMAPHORE_WAIT_TIME			1000
#define	MAX_SRAM_BUF_SIZE								(64*1024)

//=========================================================================
// MS time counter
//=========================================================================
#define TIME_CNT												0x20000000

#define SD_DETECT_NONE									0					/**< define return message*/
#define SD_DETECT_CD_PIN								1					/**< define return message*/

#define SD_RTN_ERR_CARD_REMOVED					(-700)		/**< define return message*/

#define SD_RTN_SD_CARD_WRITE_PROTECT		(-801)		/**< define return message*/
#define SD_RTN_SD_CARD_NO_WRITE_PROTECT	(-800)		/**< define return message*/

/**
 * @brief sd info structure
 */
struct sd_info{
	uint8_t ms_mode;					/**< ms mode is SD or SPI */
	uint8_t	sd_ver;						/**< sd card version */
	uint8_t	bus_width;				/**< sd bus width is 1bit or 4bit */
	uint8_t write_protect;		/**< sd card write protect infomation */

#if(OS_ON == 1)	
	xSemaphoreHandle xSEM_SD_CardDetect;
	xSemaphoreHandle xSEM_SD_CardDetect_USBD;
#endif
	
	// ----- Identification mode parameter ----- //
	uint32_t sd_cid[5];				/**< sd card identification */
	uint32_t sd_csd[5];				/**< sd card specific data */

	// ----- card specific data information ----- //
	uint16_t sd_rd_blk_len;		/**< sd card block length */
	uint32_t sd_capacity;			/**< sd card capacity */
};

//=========================================================================
// function
//=========================================================================
int 			sd_init(uint8_t mode);
int 			sd_uninit(void);
int 			sd_identify(void);
void 			sd_erase(uint32_t erase_start_addr, uint32_t erase_end_addr, uint32_t mode);
int 			sd_write(uint32_t dst_addr, uint32_t src_addr, uint32_t size);
int 			sd_read(uint32_t dst_addr, uint32_t src_addr, uint32_t size);
uint32_t 	sd_get_capacity(void);
int 			sd_write_protect_detect(void);
int 			sd_card_detect(void);
struct 		sd_info* sd_get_info(void);
int 			get_sd_error(void);

//void sd_test(void);
int 			snx_sd_initial_for_remove (void);
int				sd_get_memory_present(void);

#endif /* __SD_CTRL_H__  */
