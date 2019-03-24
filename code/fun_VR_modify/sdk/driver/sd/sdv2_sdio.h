/**
 * @file
 * this is sdio driver header file, include this file before use
 * @author CJ
 */

#ifndef __SDV2_SDIO_H__
#define __SDV2_SDIO_H__

// IO_CURRENT_STATE R5 response
#define STATE_DIS				0x0
#define STATE_CMD				0x1
#define STATE_TRN				0x2
#define STATE_RFU				0x3

#define CISTPL_VERS_1			0x15
#define CISTPL_ALTSTR			0x16
#define CISTPL_MANFID			0x20
#define CISTPL_FUNCID			0x21
#define CISTPL_FUNCE			0x22

#define MS_SPI_MODE							1
#define MS_SD_MODE							2


typedef enum
{
    SNX_SDIO_1B_BLOCK    =  1,
    SNX_SDIO_2B_BLOCK    =  2,
    SNX_SDIO_4B_BLOCK    =  4,
    SNX_SDIO_8B_BLOCK    =  8,
    SNX_SDIO_16B_BLOCK   =  16,
    SNX_SDIO_32B_BLOCK   =  32,
    SNX_SDIO_64B_BLOCK   =  64,
    SNX_SDIO_128B_BLOCK  =  128,
    SNX_SDIO_256B_BLOCK  =  256,
    SNX_SDIO_512B_BLOCK  =  512,
    SNX_SDIO_1024B_BLOCK = 1024,
    SNX_SDIO_2048B_BLOCK = 2048
} snx_sdio_block_size_t;

typedef enum
{
	SNX_SDIO_BYTE_MODE  = 0,
  SNX_SDIO_BLOCK_MODE = 1,
} snx_sdio_transfer_mode_t;

//=========================================================================
// MS R/W mode
//=========================================================================
#define SD_READ_MODE 				0x1
#define SD_WRITE_MODE 				0x0

/**
 * @brief sdio command structure
 */
struct sdio_command
{
	uint8_t		function;		/**< function number */
	uint8_t		block;			/**< block mode */
	uint8_t		write;			/**< read/write mode */
	uint32_t	addr;			/**< register address */
	uint32_t	blk_size;		/**< block size */
	uint32_t	value;			/**< Data Value */
	uint32_t	dma_addr;			/**< register address */
	uint32_t	data_len;			/**< register address */
};

/**
 * @brief sdio card tuple table structure
 */
typedef struct tuple_tbl
{
	uint8_t code;				/**< tuple code */
	uint8_t body_size;			/**< tuple body size */
	uint8_t body[64];			/**< tuple body */
}tuple_t;

//=========================================================================
// Return message
//=========================================================================
#define	SDIO_RTN_ERR_FN						-1		/**< define return message*/
#define	SDIO_RTN_ERR_STATE					-2		/**< define return message*/
#define	SDIO_RTN_ERR_FLAG					-3		/**< define return message*/
#define	SDIO_RTN_ERR_CIS_POINTER			-4		/**< define return message*/
#define	SDIO_RTN_ERR_CARD_DETECT			-5		/**< define return message*/
#define	SDIO_RTN_ERR_RSP_CMD7				-7		/**< define return message*/

#define	SDIO_RTN_PASS						0		/**< define return message*/

// =========================================================================
// SDIO Function
// =========================================================================
int sdio_init(uint8_t mode);
void sdio_enable_irq(void);
void sd_sdio_enable(uint32_t isEnable);
void sd_sdio_reset(void);
int sd_sdio_fn_select(uint32_t fun_sel);
int sd_sdio_r5_check(uint8_t state, struct sd_m2_command *cmd_info);
int sd_sdio_cccr_read(void);
int sd_sdio_cis_read(void);
int sd_sdio_sd_identify(void);
int sd_sdio_cmd52(struct sdio_command *sdio_cmd_info, struct sd_m2_command *cmd_info);
int sd_sdio_cmd53(struct sdio_command *sdio_cmd_info, struct sd_m2_command *cmd_info);

#endif

