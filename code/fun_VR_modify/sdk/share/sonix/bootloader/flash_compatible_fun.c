
/*_____ I N C L U D E S _____________________________________________________*/
#include "snc_spifc.h"
#include "flash_compatible_fun.h"

/*_____ D E F I N I T I O N S _______________________________________________*/

typedef enum{
	MX25L128,
	MX25L64,
	MX25L32,
	GD25Q32,
	UNKNOW,
}flash_model;


/*_____ V A R I A B L E S___________________________________________________*/

static flash_model cur_model 		= UNKNOW;	/* Record the current flash model */
static uint32_t GD25Q32_sub_model 	= 0;		/* Two types of GD25Q32 have the same id*/


/*
* When there's different bootloader address map,
* add the flash address distribution table with the 
* fllowing format below, and don't forget to increse 
* the "FLASH_DISTRIBUTE_TYPE_NUM".
*/
#define FLASH_DISTRIBUTE_TYPE_NUM   2

/*
* Add the flash model with the fllowing format below.
* and don't forget to increse the "FLASH_TABLE_NUM".
*/
#define FLASH_TABLE_NUM 	4

stFlashDetail_t stFlashDetail[FLASH_TABLE_NUM] = {

    /* MXIC - MX25L128 */
    {
        0xC22018,			/* ID 						*/
        1 << 0x18,			/* FlashSize 				*/	
        9,					/* Max PROTECT level 		*/
        0,					/* PROTECT mode				*/
        protect_MX25L128,	/* PROTECT funciotn point 	*/
    },

    /* MXIC - MX25L64 */
    {
        0xC22017,			/* ID 						*/
        1 << 0x17,			/* FlashSize 				*/	
        7,					/* Max PROTECT level 		*/
        0,					/* PROTECT mode				*/
        protect_MX25L64,		/* PROTECT funciotn point 	*/
    },


    /* MXIC - KH25L3206 */
    {
        0xC22016,			/* ID 						*/
        1 << 0x16,			/* FlashSize 				*/	
        7,					/* Max PROTECT level 		*/
        0,					/* PROTECT mode				*/
        protect_KH25L,		/* PROTECT funciotn point 	*/
    },


    /* Elm-tech - GD25Q32 */
    {
        0xC84016,			/* ID 						*/
        1 << 0x16,			/* FlashSize 				*/	
        7,					/* Max PROTECT level 		*/
        0,					/* PROTECT mode				*/
        protect_GD25Q32,		/* PROTECT funciotn point 	*/
    }
};

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: flash_model_select
* Description	: Find the input in the @stFlashDetail variable.
* Input			: Flash id
* Output		: None
* Return		: 0 - Unrecognized flash id.
				  else - success, return the pointer of the flash structure
* Note			: Return pointer of the stFlashDetail_t correspond to input 
*				  flash id, it refers the variable @stFlashDetail which 
*				  predefined above.
*****************************************************************************/
stFlashDetail_t* flash_model_select(uint32_t id)
{
    for(int i = 0; i < FLASH_TABLE_NUM; i++){
        if(id == stFlashDetail[i].id.data){
            cur_model = (flash_model)i;
            
            /* Exception : Two types of GD25Q32 have the same id*/
            if(cur_model == GD25Q32){
//                GD25Q32_sub_model = (SPIFC_Customer_RD_Mode(0x5a,1,0x00,2) == 0x5300) ? 1 : 0;
            }
            
            return &stFlashDetail[i];
        }
    }
    return 0;
}


/*****************************************************************************
* Function		: flash_protect_adapter
* Description	: The uni-entrance of prtection functions, user must call
*				  function "flash_model_select" to find the corresponding 
*				  flash before.
* Input			: PROTECT level
* Output		: None
* Return		: None
* Note			: It may contant level-to-size transfor algorithm if needed.
*****************************************************************************/
void flash_protect_adapter(uint8_t level)
{
	stFlashDetail[cur_model].protect_func(level);
}


void protect_KH25L(uint8_t level)	/* level 0~7 */
{
    while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);

    switch(level){
        case 0:
            SPIFC_RWStatus(SPIFC_WRITE_STATUS & 0x0);
            break;
        case 1:
            SPIFC_RWStatus(SPIFC_WRITE_STATUS & 0x24);
            break;
        case 2:
            SPIFC_RWStatus(SPIFC_WRITE_STATUS & 0x28);
            break;
        case 3:
            SPIFC_RWStatus(SPIFC_WRITE_STATUS & 0x20);
            break;
    }

    while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);
}


/*****************************************************************************
* Flash Type: MX25L64
* PROTECT Level: 0~7
* Size: 8MB
* register 	Area
* 	0		0x0
* 	1		0x20000-0x800000
* 	2		0x40000-0x800000
* 	3		0x80000-0x800000
* 	4		0x100000-0x800000
* 	5		0x200000-0x800000
* 	6		0x400000-0x800000
* 	7		0-0x800000
*****************************************************************************/
void protect_MX25L64(uint8_t level)	/* level 0~7 */
{
    uint32_t status_register = SPIFC_RWStatus(SPIFC_READ_STATUS);

    /* Level dimention transfor */
    switch(level){
        case PROTECT_AREA_NONE:				/* Unlock All */
        case PROTECT_AREA_CRITICAL:			/* 0x200000 - 2MB */
        case PROTECT_AREA_FW: 				/* 0x300000 - 3MB */
        case PROTECT_AREA_UPDATE:			/* 0x3C0000 - 3.75MB */
            level = 0;		
        break;

        case PROTECT_AREA_ALL:				/* PROTECT All Flash */
            level = 0x1C;
        break;
    }	
    status_register &= ~(0xF << 2 );
    status_register |= (level << 2);

    SPIFC_RWStatus(SPIFC_WRITE_STATUS & status_register);
    while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);
}



/*****************************************************************************
* Flash Type: MX25L12835F
* PROTECT Level: 0~9
* Size: 16MB
* register	Area
* 	0		0x0
* 	1		0-0x10000
* 	2		0-0x20000
* 	3		0-0x40000
* 	4		0-0x80000
* 	5		0-0x100000
* 	6		0-0x200000
* 	7		0-0x400000
* 	8		0-0x800000
* 	9		0-0x1000000
*****************************************************************************/
void protect_MX25L128(uint8_t level)	/* level 0~9 */
{
    uint32_t status_register = SPIFC_RWStatus(SPIFC_READ_STATUS);

    /* Level dimention transfor */
    switch(level){
        case PROTECT_AREA_NONE:				/* Unlock All */
            level = 0;		
        break;
        
        case PROTECT_AREA_CRITICAL:			/* 0x200000 - 2MB */
        case PROTECT_AREA_FW: 				/* 0x300000 - 3MB */
        case PROTECT_AREA_UPDATE:			/* 0x3C0000 - 3.75MB */
            level = 6;
        break;
        
        case PROTECT_AREA_ALL:				/* PROTECT All Flash */
            level = 9;
        break;
    }	

    status_register = (1 <<(11)) |	/* Configuration reg - Top/Button select bit*/
                      (level << 2); /* Status reg - level of PROTECTed block*/

//    SPIFC_Customer_WR_Mode(0x01, 0, 0, 2, status_register);

    while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);
}


/*****************************************************************************
* Flash Type: GD25Q32
* PROTECT Level: 0~6
* Size: 4MB
* register 	Area
* 	10		0x0
* 	11		0-0x10000
* 	12		0-0x20000
* 	13		0-0x40000
* 	14		0-0x80000
* 	15		0-0x100000
* 	6		0-0x200000
* 	7		0-0x400000
* 	8		0-0x800000
* 	9		0-0x1000000
*****************************************************************************/
void protect_GD25Q32(uint8_t level)	/* level 0~7 */
{
    while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);

    if(GD25Q32_sub_model == 0)
    {		
        switch(level){
        case 0:
//            SPIFC_Customer_WR_Mode(0x01,0,0,2,0x0000);
            break;
        case 1:
//            SPIFC_Customer_WR_Mode(0x01,0,0,2,0x4018);
            break;
        case 2:
//            SPIFC_Customer_WR_Mode(0x01,0,0,2,0x4014);
            break;
        case 3:
//            SPIFC_Customer_WR_Mode(0x01,0,0,2,0x4000);
            break;
                
        }
    }
    else
    {	
        switch(level){
        case 0:
//            SPIFC_Customer_WR_Mode(0x01,0,0,1,0x00);
            while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);
//            SPIFC_Customer_WR_Mode(0x31,0,0,1,0x00);
            break;
        case 1:
//            SPIFC_Customer_WR_Mode(0x01,0,0,1,0x18);
            while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);
//            SPIFC_Customer_WR_Mode(0x31,0,0,1,0x40);
            break;
        case 2:
//            SPIFC_Customer_WR_Mode(0x01,0,0,1,0x14);
            while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);
//            SPIFC_Customer_WR_Mode(0x31,0,0,1,0x40);
            break;
        case 3:
//            SPIFC_Customer_WR_Mode(0x01,0,0,1,0x00);
            while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);
//            SPIFC_Customer_WR_Mode(0x31,0,0,1,0x40);
            break;
            
        }
    }
    while(SPIFC_RWStatus(SPIFC_READ_STATUS) & 1);
}


