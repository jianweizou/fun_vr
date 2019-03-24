/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file ppu.c
 *
 * @author Syuan Jhao
 * @version 0.1
 * @date 2018/02/23
 * @license
 * @description
 */

#include "snc_ppu.h"
#include "reg_util.h"
#include "register_7320.h"
#include "Rot_Scl_Table.h"
#define PPU_VERSION        0x73200000

//=============================================================================
//                  Constant Definition
//=============================================================================

/*********************************************************************************************
  |---------------------------------------------------------------------------------------|
  |                      PPU VRAM Address Map                                             |
  |---------------------------------------------------------------------------------------|
  |  Text1  |  Text2 |  Text1   |  Text2   |  Text1   |  Text2   |  Sprite   |  Sprite    |
  |  Attr   |  Attr  |  Palette |  Palette |  Palette |  Palette |  Palette  |  Parameter |
  |  Num    |  Num   |  1       |  1       |  2       |  2       | 16x512Byte|            |
  |         |        |          |          |          |          |           |            |
  |_______________________________________________________________________________________|
 0x4005   0x4005   0x4005     0x4005     0x4005     0x4005     0x4005      0x4005      0x4005
  1000     2800     4000       4200       4400       4600       4800        6800        7000  
***********************************************************************************************/
// PPU_CTRL
#define BIT_TX1_EN              0
#define BIT_TX2_EN              1
#define BIT_SPR_EN              2
#define BIT_TX3_EN              3
#define BIT_PPU_EN              4
#define BIT_PIXEL_LINE_SEL      7
#define BIT_PIXEL_LINE_SEL_HD   9

// PPU_TEXT_CTRL
#define BIT_BMP_EN              0
#define BIT_ATTRInREG           1
#define BIT_TEXT_WAP            2
#define BIT_HIGH_COLOR          3
#define BIT_VCMP_EN             6
#define BIT_BLD_EN              7
#define BIT_VSCRN_SIZE          8
#define BIT_TX1_VSRN_SIZE       8
#define BIT_TX2_VSRN_SIZE       8
#define BIT_TX1_VSRN_SIZE_MASK  0x300
#define BIT_TX2_VSRN_SIZE_MASK  0x300


// PPU_TEXT_ATTR
#define TEXT_COLOR_MASK         0x03
#define BIT_HFLIP               2
#define BIT_VFLIP               3
#define BIT_HSIZE               4
#define BIT_VSIZE               6
#define BIT_PALETTE             8
#define BIT_TEXT_DEPTH          12

// PPU DMA REQ
#define BIT_DMA_REQ_START       15
#define BIT_BURST_LENGTH        0

// Bright
#define BIT_BRIGHT_LEVEL_MASK   0x1F
#define BIT_BRIGHT_MODE         5
#define BIT_BRIGHT_OUT          6
#define BIT_BRIGHT_EN           7

// Line DMA
#define BIT_LINE_DMA_EN         0

// Scaling DMA
#define BIT_SCALING_DMA_EN      0
#define BIT_SCALING_START       1
#define BIT_SCALING_SEL         2

// PPU Text Start Address
#define SRAM_TX_STA_ADDRH       0x0018
#define DDR_TX_STA_ADDRH        0x0030

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================


//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

/* PPU */

/**
 *  \brief  PPU Initial
 *
 *  \param [in] Mem_Device      enum of ppu_load_memory_t
 *  \return                     0: ok, other: fail
 *
 *  \details
 */
int
PPU_Init(
    ppu_load_memory_t Mem_Device)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;

    // Enable ppu
    if(Mem_Device == PPU_SRAM)
    {
        reg_write_bits(&pDev->TX_LINE_STA_ADDRH, (SRAM_TX_STA_ADDRH<<16)|(SRAM_TX_STA_ADDRH));
    }
        
    else if(Mem_Device == PPU_DDR)
    {
        reg_write_bits(&pDev->TX_LINE_STA_ADDRH, (DDR_TX_STA_ADDRH<<16)|(DDR_TX_STA_ADDRH));
    }
    
    else if(Mem_Device == PPU_SRAM_DDR)
    {
        reg_write_bits(&pDev->TX_LINE_STA_ADDRH, (SRAM_TX_STA_ADDRH<<16)|(DDR_TX_STA_ADDRH));
    }
    
    else if(Mem_Device == PPU_DDR_SRAM)
    {
        reg_write_bits(&pDev->TX_LINE_STA_ADDRH, (DDR_TX_STA_ADDRH)|(SRAM_TX_STA_ADDRH<<16));
    }
    
    else
    {
        /* Wrong Mem_Device address */
        return -1;
    }

    reg_set_bit(&pDev->PPU_CTRL, BIT_PPU_EN);
    
    return rval;
}

/**
 *  \brief  PPU Deinitial
 *
 *  \param 
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int 
PPU_Deinit(void)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;

    // Disable ppu
    reg_clear_bit(&pDev->PPU_CTRL, BIT_PPU_EN);
    
    return rval;
}

/**
 *  \brief  Set PPU background color(RGB565)
 *
 *  \param [in] R           Red(5 bits)
 *  \param [in] G           Green(6 bits)
 *  \param [in] B           Blue(5 bits)
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
PPU_Background_Color(
    uint8_t R,
    uint8_t G,
    uint8_t B)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;

    // Set Red
    reg_write_mask_bits(&pDev->Backdrop_Color, (R&0x1F)<<11, (0x1F)<<11);
    // Set Green
    reg_write_mask_bits(&pDev->Backdrop_Color, (G&0x3F)<<5, (0x3F)<<5);
    // Set Blue
    reg_write_mask_bits(&pDev->Backdrop_Color, (B&0x1F), (0x1F));
    
    return rval;
}

/**
 *  \brief  Move table data from AHB_RAM to VRAM
 *
 *  \param [in] uiAHB_Addr           AHB_RAM Address
 *  \param [in] uiVRAM_Addr          VRAM Address
 *  \param [in] uiDataLength         Length of move data
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int 
PPU_Table_Move(
    uint32_t uiAHB_Addr,
    uint32_t uiVRAM_Addr,
    uint32_t uiDataLength)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    //Set target address
    reg_write_bits(&pDev->DMA_DES_ADDRL, uiVRAM_Addr);
    reg_write_bits(&pDev->DMA_DES_ADDRH, uiVRAM_Addr>>16);
    
    //Set resource address
    reg_write_bits(&pDev->DMA_SRC_ADDRL, uiAHB_Addr);
    reg_write_bits(&pDev->DMA_SRC_ADDRH, uiAHB_Addr>>16);
    
    reg_write_bits(&pDev->DMA_LEN, (uiDataLength>>1)-1);
    reg_set_bit(&pDev->DMA_REQ, BIT_DMA_REQ_START);
    
    while(reg_read_mask_bits(&pDev->DMA_REQ, 0x01<<BIT_DMA_REQ_START) != 0);
    return rval;
}

/**
 *  \brief  Write Data to PPU VRAM
 *
 *  \param [in] uiAddr               VRAM Address
 *  \param [in] usData               Data write to VRAM
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_VRAM_Write(
    uint32_t uiAddr, 
    uint16_t Data)
{
    int rval = 0;
    uint16_t *Addr;
    Addr = (uint16_t *)(uiAddr);
    *Addr = Data;
    
    return rval;
}

/**
 *  \brief  Read Data from PPU VRAM
 *
 *  \param [in] uiAddr               VRAM Address
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_VRAM_Read(
    uint32_t uiAddr)
{
    uint16_t *Addr;
    Addr = (uint16_t *)(uiAddr);
    return(*Addr);
}

/**
 *  \brief  Set Brightness Parameters
 *
 *  \param [in] BREn                 0/1 : Disable/Enable PPU Brightness Adjustment
 *  \param [in] BR_Lev               enum ppu_brightness_level
 *  \param [in] BROut                enum ppu_bright_out
 *  \param [in] BRM                  enum ppu_birghtness_mode
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Brightness_Setting(
    uint8_t BREn,
    ppu_brightness_level_t BR_Lev,
    ppu_bright_out_t BROut,
    ppu_birghtness_mode_t BRM)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(BREn == 1)
    {
        reg_write_mask_bits(&pDev->Bright_CTRL, (BR_Lev&0x1F), BIT_BRIGHT_LEVEL_MASK);
        reg_write_mask_bits(&pDev->Bright_CTRL, (BROut&0x01)<<BIT_BRIGHT_OUT, (0x01)<<BIT_BRIGHT_OUT);
        reg_write_mask_bits(&pDev->Bright_CTRL, (BRM&0x01)<<BIT_BRIGHT_MODE, (0x01)<<BIT_BRIGHT_MODE);
        reg_set_bit(&pDev->Bright_CTRL, BIT_BRIGHT_EN);
    }
    else
    {
        reg_clear_bit(&pDev->Bright_CTRL, BIT_BRIGHT_EN);
    }
    
    return rval;
}

/**
 *  \brief  Set PPU Brightness Adjustment Region Setting
 *
 *  \param [in] HStr                 Horizontal Start Value
 *  \param [in] HEnd                 Horizontal End Value
 *  \param [in] VStr                 Vertical Start Value
 *  \param [in] VEnd                 Vertical End Value
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Brightness_Region_Setting(
    uint16_t HStr,
    uint16_t HEnd,
    uint16_t VStr,
    uint16_t VEnd)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    reg_write_bits(&pDev->Brightness_STA_X, HStr);
    reg_write_bits(&pDev->Brightness_END_X, HEnd);
    
    reg_write_bits(&pDev->Brightness_STA_Y, VStr);
    reg_write_bits(&pDev->Brightness_END_Y, VEnd);
    
    return rval;
}

/**
 *  \brief  Use the API when panel size is not 320x240
 *
 *  \param [in] Status                 enum ppu_other_resolution_status_t
 *  \param [in] Size_X                 Panel width
 *  \param [in] Size_Y                 Panel height
 *  \return                            0: ok, other: fail
 *
 *  \details Maximum resolution support to 1280x720
 */
int
PPU_Other_Resolution_Initial(
    ppu_other_resolution_status_t Status,
    uint16_t Size_X,
    uint16_t Size_Y)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;

    if(Status == PPU_OTHER_RES_ENABLE)
    {
        reg_write_bits(&pDev->TX1_DISP_CTRL2, 0xFFFFFFFF);
        reg_write_bits(&pDev->TX2_DISP_CTRL2, 0xFFFFFFFF);
        reg_write_bits(&pDev->OTHER_RES_DISP_START_X, 0);
        reg_write_bits(&pDev->OTHER_RES_DISP_START_Y, 0);
        reg_write_bits(&pDev->OTHER_RES_DISP_SIZE_X, Size_X);
        reg_write_bits(&pDev->OTHER_RES_DISP_SIZE_Y, Size_Y);
        reg_set_bit(&pDev->PPU_CTRL, BIT_PIXEL_LINE_SEL);
        reg_set_bit(&pDev->PPU_CTRL, BIT_PIXEL_LINE_SEL_HD);

        if((Size_X <=1024 && Size_X>=512) || (Size_Y <=512 && Size_Y>=256))
        {
            reg_write_mask_bits(&pDev->TX1_CTRL, 0x01<<BIT_TX1_VSRN_SIZE, BIT_TX1_VSRN_SIZE_MASK);
            reg_write_mask_bits(&pDev->TX2_CTRL, 0x01<<BIT_TX2_VSRN_SIZE, BIT_TX2_VSRN_SIZE_MASK);
        }

        else if((Size_X <=2048 && Size_X>=1024) || (Size_Y <=1024 && Size_Y>=512))
        {
            reg_write_mask_bits(&pDev->TX1_CTRL, 0x02<<BIT_TX1_VSRN_SIZE, BIT_TX1_VSRN_SIZE_MASK);
            reg_write_mask_bits(&pDev->TX2_CTRL, 0x02<<BIT_TX2_VSRN_SIZE, BIT_TX2_VSRN_SIZE_MASK);
        }
    }
    else
    {
        pDev->PPU_CTRL_b.PIXEL_LINE_Sel = 0;
        pDev->PPU_CTRL_b.PIXEL_LINE_Sel_HD = 0;
        pDev->TX1_CTRL_b.VSCRN_SIZE = 0;
        pDev->TX2_CTRL_b.VSCRN_SIZE = 0;
    }

    return rval;
}


/**
 *  \brief  Setting ppu burst length when using dram
 *
 *  \param [in] Burst_Length           enum of ppu_burst_length_t
 *  \return                            0: ok, other: fail
 *
 *  \details 
 */
int
PPU_Burst_Length_Setting(
    ppu_burst_length_t Burst_Length)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    if(Burst_Length == PPU_BURST_LENGTH_16)
    {
        reg_set_bit(&pDev->DMA_REQ, BIT_BURST_LENGTH);
    }
    else
    {
        reg_clear_bit(&pDev->DMA_REQ, BIT_BURST_LENGTH);
    }
    return rval;
}

/* Text */

/**
 *  \brief  Enable Text
 *
 *  \param [in] text                 enum ppu_text
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_Init(
    ppu_text_t text)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    sn_line_dma_t *pDev2 = (sn_line_dma_t*) SN_LINE_DMA_BASE;
    switch(text)
    {
        case TEXT1:
            reg_set_bit(&pDev->PPU_CTRL, BIT_TX1_EN);
            break;
        
        case TEXT2:
            reg_set_bit(&pDev->PPU_CTRL, BIT_TX2_EN);
            break;
        
        case TEXT3:
            reg_set_bit(&pDev->PPU_CTRL, BIT_TX3_EN);
            reg_set_bit(&pDev2->LINE_CTRL, BIT_LINE_DMA_EN);
            break;
        
        default:
            /* Wrong Text layer */
            return -1;
    }
    
    return rval;
}

/**
 *  \brief  Disable Text
 *
 *  \param [in] text                 enum ppu_text
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_Deinit(
    ppu_text_t text)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    switch(text)
    {
        case TEXT1:
            reg_clear_bit(&pDev->PPU_CTRL, BIT_TX1_EN);
            break;
        
        case TEXT2:
            reg_clear_bit(&pDev->PPU_CTRL, BIT_TX2_EN);
            break;
        
        case TEXT3:
            reg_clear_bit(&pDev->PPU_CTRL, BIT_TX3_EN);
            break;
        
        default:
            /* Wrong text layer */
            return -1;
    }
    
    return rval;
}

/**
 *  \brief  Clear VRAM value of Text
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] mode                 enum ppu_vram_clr_t
 *  \param [in] uiTemp_Addr          The temp area address (Min Area:0x200 Byte)
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int 
PPU_Text_VRAM_Init(
    ppu_text_t text,
    ppu_text_vram_clr_t mode,
    uint32_t uiTemp_Addr)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    uint32_t *Addr_Ptr,VRAM_Addr;
    uint8_t count;
    Addr_Ptr=(uint32_t *)(uiTemp_Addr);
    
    /*Initial Buffer */
    for(count=0;count<0x80;count++)
    {
        *(Addr_Ptr+count)=0x00000000;
    }        
    
    /*Initial VRAM*/
    switch(mode & 0x03)
    {
        case TX_ATTR_CLR:
            if(text == TEXT1)
            {
                VRAM_Addr=0x1000;
            }
            else if(text == TEXT2)
            {
                VRAM_Addr=0x2800;
            }
            
            for(count=0;count<0x0C;count++)
            {
                reg_write_bits(&pDev->DMA_DES_ADDRL, VRAM_Addr);
                reg_write_bits(&pDev->DMA_DES_ADDRH, 0x4005);
                reg_write_bits(&pDev->DMA_SRC_ADDRL, *Addr_Ptr);
                reg_write_bits(&pDev->DMA_SRC_ADDRH, (*Addr_Ptr)>>16);
                reg_write_bits(&pDev->DMA_LEN, (0x100)-1);  //unit is word 0x200 Byte
                reg_set_bit(&pDev->DMA_REQ, BIT_DMA_REQ_START);
                VRAM_Addr+=0x200;
                while(reg_read_mask_bits(&pDev->DMA_REQ, 0x01<<BIT_DMA_REQ_START) != 0);
            }
            
            break;
        
        case TX_PALETTE1_CLR:
            if(text == TEXT1)
            {
                reg_write_bits(&pDev->DMA_DES_ADDRL, 0x4000);
            }
            else if(text == TEXT2)
            {
                reg_write_bits(&pDev->DMA_DES_ADDRL, 0x4200);
            }
            
            reg_write_bits(&pDev->DMA_DES_ADDRH, 0x4005);
            reg_write_bits(&pDev->DMA_SRC_ADDRL, *Addr_Ptr);
            reg_write_bits(&pDev->DMA_SRC_ADDRH, (*Addr_Ptr)>>16);
            reg_write_bits(&pDev->DMA_LEN, (0x100)-1);  //unit is word 0x200Byte
            reg_set_bit(&pDev->DMA_REQ, BIT_DMA_REQ_START);
            while(reg_read_mask_bits(&pDev->DMA_REQ, 0x01<<BIT_DMA_REQ_START) != 0);
            
            break;
        
        case TX_PALETTE2_CLR:
            if(text == TEXT1)
            {
                reg_write_bits(&pDev->DMA_DES_ADDRL, 0x4400);
            }
            else if(text == TEXT2)
            {
                reg_write_bits(&pDev->DMA_DES_ADDRL, 0x4600);
            }

            reg_write_bits(&pDev->DMA_DES_ADDRH, 0x4005);
            reg_write_bits(&pDev->DMA_SRC_ADDRL, *Addr_Ptr);
            reg_write_bits(&pDev->DMA_SRC_ADDRH, (*Addr_Ptr)>>16);
            reg_write_bits(&pDev->DMA_LEN, (0x100)-1);  //unit is word 0x200Byte
            reg_set_bit(&pDev->DMA_REQ, BIT_DMA_REQ_START);
            while(reg_read_mask_bits(&pDev->DMA_REQ, 0x01<<BIT_DMA_REQ_START) != 0);
            break;
            
        default:
            /* Wrong vram clean mode */
            return -1;
    }
    
    return rval;
}

/**
 *  \brief  Set Text display offset
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] usX_Offset           Horizontal offset start display [9:0]
 *  \param [in] usY_Offset           Vertical offset start display   [8:0]
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_Position_Setting(
    ppu_text_t text,
    uint16_t usX_Offset, 
    uint16_t usY_Offset)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(text == TEXT1)
    {
        reg_write_bits(&pDev->TX1_X_POS, usX_Offset);
        reg_write_bits(&pDev->TX1_Y_POS, usY_Offset);
    }
    
    else if(text == TEXT2)
    {
        reg_write_bits(&pDev->TX2_X_POS, usX_Offset);
        reg_write_bits(&pDev->TX2_Y_POS, usY_Offset);
    }
    
    return rval;
}

/**
 *  \brief  Set Text Bitmap Mode Parameters
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] color                enum ppu_color_mode_t
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_BitmapMode_Init(
    ppu_text_t text,
    ppu_color_mode_t color)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(text == TEXT1)
    {
        reg_set_bit(&pDev->TX1_CTRL, BIT_BMP_EN);
        if(color == PPU_HIGH_COLOR)
        {
            reg_set_bit(&pDev->TX1_CTRL, BIT_HIGH_COLOR);
        }
        else
        {
            reg_write_mask_bits(&pDev->TX1_ATTR, color, TEXT_COLOR_MASK);
        }
    }
    
    else if(text == TEXT2)
    {
        reg_set_bit(&pDev->TX2_CTRL, BIT_BMP_EN);
        if(color == PPU_HIGH_COLOR)
        {
            reg_set_bit(&pDev->TX2_CTRL, BIT_HIGH_COLOR);
        }
        else
        {
            reg_write_mask_bits(&pDev->TX2_ATTR, color, TEXT_COLOR_MASK);
        }
    }
    
    else
    {
        /* Wrong text layer */
        return -1;
    }
    
    return rval;
}

/**
 *  \brief  Point Txet Number table address
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] uiNumPtr_Addr        Number table address in VRAM
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_NumberPtr_Setting(
    ppu_text_t text, 
    uint32_t uiNumPtr_Addr)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    uint32_t NumPtr_Addr_temp;
    NumPtr_Addr_temp = uiNumPtr_Addr;
    
    if(text == TEXT1)
    {
        reg_write_bits(&pDev->TX1_NUM_PTR, ((uiNumPtr_Addr&0xFFFF0000)|(((NumPtr_Addr_temp-0x1000)&0xFFFF)>>1))+0x1000);
    }
    
    else if(text == TEXT2)
    {
        reg_write_bits(&pDev->TX2_NUM_PTR, ((uiNumPtr_Addr&0xFFFF0000)|(((NumPtr_Addr_temp-0x1000)&0xFFFF)>>1))+0x1000);
    }
    
    else
    {
        /* Wrong text layer */
        return -1;
    }
    
    return rval;
}

/**
 *  \brief  Point Text Attribute table address
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] uiAttrPtr_Addr       Attribute table address in VRAM
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_AttributePtr_Setting(
    ppu_text_t text, 
    uint32_t uiAttrPtr_Addr)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    uint32_t AttrPtr_Addr_temp;
    AttrPtr_Addr_temp = uiAttrPtr_Addr;
    
    if(text == TEXT1)
    {
        reg_write_bits(&pDev->TX1_ATTR_PTR, ((uiAttrPtr_Addr&0xFFFF0000)|(((AttrPtr_Addr_temp-0x1000)&0xFFFF)>>1))+0x1000);
    }
    
    else if(text == TEXT2)
    {
        reg_write_bits(&pDev->TX2_ATTR_PTR, ((uiAttrPtr_Addr&0xFFFF0000)|(((AttrPtr_Addr_temp-0x1000)&0xFFFF)>>1))+0x1000);
    }
    
    else
    {
        /* Wrong text layer */
        return -1;
    }
    
    return rval;
}

/**
 *  \brief  Point Text Pattern data address
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] uiPatPtr_Addr        Text Pattern data address in ABH_RAM
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_PatternPtr_Setting(
    ppu_text_t text,
    uint32_t uiPatPtr_Addr)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    uint32_t PatPtr_Addr_temp;
    PatPtr_Addr_temp = uiPatPtr_Addr;
    
    if(text == TEXT1)
    {
        reg_write_bits(&pDev->TX1_PATT_ADDRH, (uiPatPtr_Addr&0xFFFF0000)>>16);
        reg_write_bits(&pDev->TX1_PATT_ADDRL, PatPtr_Addr_temp&0xFFFF);
    }
    
    else if(text == TEXT2)
    {
        reg_write_bits(&pDev->TX2_PATT_ADDRH, (uiPatPtr_Addr&0xFFFF0000)>>16);
        reg_write_bits(&pDev->TX2_PATT_ADDRL, PatPtr_Addr_temp&0xFFFF);
    }
    
    else
    {
        /* Wrong text layer */
        return -1;
    }
    
    return rval;
}

/**
 *  \brief  Set Text Display Depth Level
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] Depth                enum ppu_depth_level
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_DepthLevel_Setting(
    ppu_text_t text,
    ppu_depth_level_t Depth)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(text == TEXT1)
    {
        reg_write_mask_bits(&pDev->TX1_ATTR, ((Depth & 0x03)<<BIT_TEXT_DEPTH) , ((0x03)<<BIT_TEXT_DEPTH));
    }
    
    else if(text == TEXT2)
    {
        reg_write_mask_bits(&pDev->TX2_ATTR, ((Depth & 0x03)<<BIT_TEXT_DEPTH), ((0x03)<<BIT_TEXT_DEPTH));
    }
    
    // TODO: Check if text3 can set depth or not
    else
    {
        /* Wrong text layer */
        return -1;
    }
    return rval;
}

/**
 *  \brief  Enable/Disable Text WallPaper Effect
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] WAPEN                0/1 : Disable/Enable Wall Paper Effect
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_WallPaper_Setting(
    ppu_text_t text,
    uint8_t WAPEN)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(WAPEN == 1)
    {
        if(text == TEXT1)
        {
            reg_set_bit(&pDev->TX1_CTRL, BIT_TEXT_WAP);
        }
    
        else if(text == TEXT2)
        {
            reg_set_bit(&pDev->TX2_CTRL, BIT_TEXT_WAP);
        }
        
        else
        {
            /* Wrong text layer */
            return -1;
        }
    }
    
    else
    {
        if(text == TEXT1)
        {
            reg_clear_bit(&pDev->TX1_CTRL, BIT_TEXT_WAP);
        }
    
        else if(text == TEXT2)
        {
            reg_clear_bit(&pDev->TX2_CTRL, BIT_TEXT_WAP);
        }
        
        else
        {
            /* Wrong text layer */
            return -1;
        }
    }
    

    return rval;
}

/**
 *  \brief  Enable/Disable Text Vertiacl Compression Effect
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] VCMPEN               0/1 : Disable/Enable Vertiacl Compression
 *  \param [in] Scale                enum ppu_vcmp_scale
 *  \param [in] Offset               Shift the Vertiacl Compression Display [0:7]
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_Vertical_Compression_Setting(
    ppu_text_t text,
    uint8_t VCMPEN,
    ppu_vcmp_scale_t Scale,
    uint16_t Offset)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(VCMPEN == 1)
    {
        reg_write_bits(&pDev->VCMP_Offset, Offset);
        reg_write_bits(&pDev->VCMP_Scale, Scale);
        
        if(text == TEXT1)
        {
            reg_set_bit(&pDev->TX1_CTRL, BIT_VCMP_EN);
        }
        
        else if(text == TEXT2)
        {
            reg_set_bit(&pDev->TX2_CTRL, BIT_VCMP_EN);
        }
        
        else
        {
            /* Wrong text layer */
            return -1;
        }
        
    }
    else
    {
        if(text == TEXT1)
        {
            reg_clear_bit(&pDev->TX1_CTRL, BIT_VCMP_EN);
        }
        
        else if(text == TEXT2)
        {
            reg_clear_bit(&pDev->TX2_CTRL, BIT_VCMP_EN);
        }
        
        else
        {
            /* Wrong text layer */
            return -1;
        }
    }
    
    return rval;
}

/**
 *  \brief  Enable/Disable Text Blending Effect
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] BLDEN                0/1 : Disable/Enable Blending
 *  \param [in] BLD_Level            enum ppu_blending_level_t
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_Blending_Setting(
    ppu_text_t text,
    uint8_t BLDEN,
    ppu_blending_level_t BLD_Level)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(BLDEN == 1)
    {
        reg_write_bits(&pDev->Blend_CTRL, BLD_Level & 0x1F);
        if(text == TEXT1)
        {
            reg_set_bit(&pDev->TX1_CTRL, BIT_BLD_EN);
        }
        
        else if(text == TEXT2)
        {
            reg_set_bit(&pDev->TX2_CTRL, BIT_BLD_EN);
        }
        
        else
        {
            /* Wrong text layer */
            return -1;
        }
    }
    else
    {
        if(text == TEXT1)
        {
            reg_clear_bit(&pDev->TX1_CTRL, BIT_BLD_EN);
        }
        
        else if(text == TEXT2)
        {
            reg_clear_bit(&pDev->TX2_CTRL, BIT_BLD_EN);
        }
        
        else
        {
            /* Wrong text layer */
            return -1;
        }
    }
    
    return rval;
}

/**
 *  \brief  Text Support 30 Display Regions Enable or Disable 
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] Display_Region       [0:29] One of bit is the display region unit switch.
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_Vertical_Display_Init(
    ppu_text_t text,
    uint32_t Display_Region)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(text == TEXT1)
    {
        reg_write_bits(&pDev->TX1_DISP_CTRL, Display_Region);
    }
    
    else if(text == TEXT2)
    {
        reg_write_bits(&pDev->TX2_DISP_CTRL, Display_Region);
    }
    
    else
    {
        /* Wrong text layer */
        return -1;
    }
    
    return rval;
}

/**
 *  \brief  Set Text Character Mode Parameters
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] Char_Size            enum ppu_character_size_t
 *  \param [in] Color                enum ppu_color_mode_t 
 *  \param [in] VScrn_En             0:Disable
 *                                   1:Enable support source resolution 1024x512
 *                                   2:Enable support source resolution 2048*1024
 * 
 *  \return                          0: ok, other: fail
 *
 *  \details                         Character mode NOT support hi-color
 */
int 
PPU_Text_CharacterMode_Init(
    ppu_text_t text,
    ppu_character_size_t Char_Size,
    ppu_color_mode_t Color,
    uint8_t VScrn_En)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(text == TEXT1)
    {
        reg_clear_bit(&pDev->TX1_CTRL, BIT_BMP_EN);
        reg_write_mask_bits(&pDev->TX1_ATTR, Char_Size<<BIT_VSIZE, 0x03<<BIT_VSIZE);
        reg_write_mask_bits(&pDev->TX1_ATTR, (Char_Size>>2)<<BIT_HSIZE, 0x03<<BIT_HSIZE);
        reg_write_mask_bits(&pDev->TX1_ATTR, Color, TEXT_COLOR_MASK);
        reg_write_mask_bits(&pDev->TX1_CTRL, VScrn_En<<BIT_VSCRN_SIZE, VScrn_En<<BIT_VSCRN_SIZE);
    }
    else if(text == TEXT2)
    {
        reg_clear_bit(&pDev->TX2_CTRL, BIT_BMP_EN);
        reg_write_mask_bits(&pDev->TX2_ATTR, Char_Size<<BIT_VSIZE, 0x03<<BIT_VSIZE);
        reg_write_mask_bits(&pDev->TX2_ATTR, (Char_Size>>2)<<BIT_HSIZE, 0x03<<BIT_HSIZE);
        reg_write_mask_bits(&pDev->TX2_ATTR, Color, TEXT_COLOR_MASK);
        reg_write_mask_bits(&pDev->TX2_CTRL, VScrn_En<<BIT_VSCRN_SIZE, VScrn_En<<BIT_VSCRN_SIZE);
    }
    
    else
    {
        /* Wrong text layer */
        return -1;
    }
    
    return rval;
}

/**
 *  \brief  Set Text Character Mode Attribute Effect Parameters
 *
 *  \param [in] text                 enum ppu_text
 *  \param [in] AttrInReg            0: Attribute parameters reference VRAM
 *                                   1: Atterbute parameters reference register
 *  \param [in] flip                 enum ppu_flip_t
 *  \param [in] Palette_Sel          Palette Select 0~15 
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text_CharacterMode_Attribute_Setting(
    ppu_text_t text, 
    uint8_t AttrInReg, 
    ppu_flip_t flip, 
    uint8_t Palette_Sel)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    if(text == TEXT1)
    {
        if(AttrInReg == 1)
        {
            reg_set_bit(&pDev->TX1_CTRL, BIT_ATTRInREG);
            reg_write_mask_bits(&pDev->TX1_ATTR, (flip&0x01)<<BIT_HFLIP, (0x01)<<BIT_HFLIP);
            reg_write_mask_bits(&pDev->TX1_ATTR, ((flip>>1)&0x01)<<BIT_VFLIP, (0x01)<<BIT_VFLIP);
            reg_write_mask_bits(&pDev->TX1_ATTR, (Palette_Sel)<<BIT_PALETTE, (0xF)<<BIT_PALETTE);
        }
        else
        {
            reg_clear_bit(&pDev->TX1_CTRL, BIT_ATTRInREG);
        }
    }
    
    else if(text == TEXT2)
    {
        if(AttrInReg == 1)
        {
            reg_set_bit(&pDev->TX2_CTRL, BIT_ATTRInREG);
            reg_write_mask_bits(&pDev->TX2_ATTR, (flip&0x01)<<BIT_HFLIP, (0x01)<<BIT_HFLIP);
            reg_write_mask_bits(&pDev->TX2_ATTR, ((flip>>1)&0x01)<<BIT_VFLIP, (0x01)<<BIT_VFLIP);
            reg_write_mask_bits(&pDev->TX2_ATTR, (Palette_Sel)<<BIT_PALETTE, (0xF)<<BIT_PALETTE);
        }
        else
        {
            reg_clear_bit(&pDev->TX2_CTRL, BIT_ATTRInREG);
        }
    }
    
    else
    {
        /* Wrong text layer */
        return -1;
    }
    
    return rval;
}

/**
 *  \brief  Set the Text3 Line DMA Width & Height
 *
 *  \param [in] Width               Panel Width
 *  \param [in] Height              Panel Height
 *  \return                         0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text3_Line_DMA_Setting(
    uint32_t Width,
    uint32_t Height)
{
    int rval = 0;
    sn_line_dma_t *pDev2 = (sn_line_dma_t *) SN_LINE_DMA_BASE;
    reg_write_bits(&pDev2->PIXEL_NUM, Width);
    reg_write_bits(&pDev2->LINE_NUM, Height);
    
    return rval;
}

/**
 *  \brief  Set the Text3 Buffer on AHB_RAM Address
 *
 *  \param [in] Buf_No               enum ppu_text3_buf
 *  \param [in] Buf_Addr             Buffer address
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Text3_BufferAddress_Setting(
    ppu_text3_buf_t Buf_No,
    uint32_t Buf_Addr)
{
    int rval = 0;
    sn_line_dma_t *pDev = (sn_line_dma_t*) SN_LINE_DMA_BASE;
    
    if(Buf_No == TEXT3_BUF_1)
    {
        reg_write_bits(&pDev->LINE_ADDR_1, Buf_Addr);
    }
    else
    {
        reg_write_bits(&pDev->LINE_ADDR_0, Buf_Addr);
    }
    
    return rval;
}

/**
 *  \brief  Set scaling function for Text3
 *
 *  \param [in] ucEnscale            0/1 : Disable/Enable Scaling
 *  \param [in] ScaleSel             0/1 : Scale select X2/X4
 *  \param [in] Frame_StrAddr        Start frame data address
 *  \param [in] SrcWidth             Soucre width for scale function
 *  \param [in] SrcHight             Soucre height for scale function
 *  \return                          0: ok, other: fail
 *
 *  \details                         enlarge QQVGA size to QVGA or VGA size
 */
int
PPU_Text3_Scale_Setting(
    uint8_t Enscale,
    uint8_t ScaleSel,
    uint32_t Frame_StrAddr,
    uint16_t SrcWidth,
    uint16_t SrcHight)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    sn_scaling_dma_t *pDev2 = (sn_scaling_dma_t*) SN_SCALING_DMA_BASE;
    
    if(Enscale == 1)
    {
        // PPU Scaling Setting
        pDev->PPU_CTRL_b.PIXEL_LINE_Sel = ScaleSel;
        reg_write_bits(&pDev2->LINE_NUM, SrcHight);
        reg_write_bits(&pDev2->PIXEL_NUM, SrcWidth);
        
        // DMA Scaling Setting
        reg_write_bits(&pDev2->LINE_ADDR, Frame_StrAddr);
        pDev2->LINE_CTRL_b.SCALE_SEL = ScaleSel;
        reg_set_bit(&pDev2->LINE_CTRL, BIT_SCALING_DMA_EN);
    }
    else
    {
        reg_clear_bit(&pDev2->LINE_CTRL, BIT_SCALING_DMA_EN);
    }
    
    return rval;
}


/* Sprite */

/**
 *  \brief  Enable the PPU Sprite Display
 *
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int PPU_Sprite_Enable(void)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    reg_set_bit(&pDev->PPU_CTRL, BIT_SPR_EN);
    
    return rval;
}

/**
 *  \brief  Disable the PPU Sprite Display
 *
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int PPU_Sprite_Disable(void)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    
    reg_clear_bit(&pDev->PPU_CTRL, BIT_SPR_EN);
    
    return rval;
}

/**
 *  \brief  Set status of one sprite
 *
 *  \param [in] SprNo                Select number of sprite
 *  \param [in] status               enum ppu_sprite_status
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Single_Sprite_Setting(
    uint8_t SprNo,
    ppu_sprite_status_t status)
{
    int rval = 0;
    uint16_t *VSpr_Ptr;
    
    VSpr_Ptr = (uint16_t*)(0x40056800);
    if(status == SPR_ENABLE)
    {
        *(VSpr_Ptr+(SprNo*4)+3)|=0x8000;
    }
    else
    {
        *(VSpr_Ptr+(SprNo*4)+3)&=0x7FFF;
    }
    
    return rval;
}

/**
 *  \brief  Set blending status of sprite
 *
 *  \param [in] SprNo                Select number of sprite
 *  \param [in] status               enum ppu_sprite_status
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Sprite_Blending_Setting(
    uint8_t SprNo,
    ppu_sprite_status_t status)
{
    int rval = 0;
    uint16_t *VSpr_Ptr;
    VSpr_Ptr = (uint16_t*)(0x40056800);
    
    if(status == SPR_ENABLE)
    {
        *(VSpr_Ptr+(SprNo*4)+3)|=0x4000;
    }
    else
    {
        *(VSpr_Ptr+(SprNo*4)+3)&=0xBFFF;
    }
    
    return rval;
}

/**
 *  \brief  Point to Sprite Pattern table address in AHB-RAM
 *
 *  \param [in] PatPtr_Addr          Sprite pattern data address in AHB-RAM.
 *
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Sprite_PatternPtr_Setting(
    uint32_t PatPtr_Addr)
{
    int rval = 0; 
    sn_ppu_t *pDev = (sn_ppu_t*)SN_PPU_BASE;
    
    reg_write_bits(&pDev->SPR_PATT_ADDRL, PatPtr_Addr);
    reg_write_bits(&pDev->SPR_PATT_ADDRH, (PatPtr_Addr&0xFFFF0000)>>16);
    
    return rval;
}

/**
 *  \brief  Clear Sprite of VRAM value 
 *
 *  \param [in] mode                 enum ppu_vram_clr
 *  \param [in] Temp_Addr            The temp area address (Min Area:0x200 Byte)
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Sprite_VRAM_Init(
    ppu_spr_vram_clr_t mode,
    uint32_t Temp_Addr)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    uint32_t *Addr_Ptr,VRAM_Addr;
    uint8_t count;
    Addr_Ptr=(uint32_t *)(Temp_Addr);
    
    /*Initial Buffer */
    for(count=0;count<0x80;count++)
        *(Addr_Ptr+count)=0x00000000;
    
    switch(mode)
    {
        case SPR_NO_CLR:
            break;
        
        case SPR_PALETTE_CLR:
            VRAM_Addr = 0x4800;
            for(count=0;count<=0x10;count++)
            {
                reg_write_bits(&pDev->DMA_DES_ADDRL, VRAM_Addr);
                reg_write_bits(&pDev->DMA_DES_ADDRH, 0x4005);
                reg_write_bits(&pDev->DMA_SRC_ADDRL, *Addr_Ptr);
                reg_write_bits(&pDev->DMA_SRC_ADDRL, *Addr_Ptr>>16);
                reg_write_bits(&pDev->DMA_LEN, (0x100)-1);
                reg_set_bit(&pDev->DMA_REQ, BIT_DMA_REQ_START);
                VRAM_Addr+=0x200;
                while(reg_read_mask_bits(&pDev->DMA_REQ, 0x01<<BIT_DMA_REQ_START) != 0);
            }
            break;
        
        case SPR_PARAMETER_CLR:
            VRAM_Addr = 0x6800;
            for(count=0;count<0x04;count++)
            {
                reg_write_bits(&pDev->DMA_DES_ADDRL, VRAM_Addr);
                reg_write_bits(&pDev->DMA_DES_ADDRH, 0x4005);
                reg_write_bits(&pDev->DMA_SRC_ADDRL, *Addr_Ptr);
                reg_write_bits(&pDev->DMA_SRC_ADDRL, *Addr_Ptr>>16);
                reg_write_bits(&pDev->DMA_LEN, (0x100)-1);
                reg_set_bit(&pDev->DMA_REQ, BIT_DMA_REQ_START);
                VRAM_Addr+=0x200;
                while(reg_read_mask_bits(&pDev->DMA_REQ, 0x01<<BIT_DMA_REQ_START) != 0);
            }
            break;
        
        case SPR_ROTATION_CLR:
            VRAM_Addr = 0x7000;
            for(count=0;count<0x02;count++)
            {
                reg_write_bits(&pDev->DMA_DES_ADDRL, VRAM_Addr);
                reg_write_bits(&pDev->DMA_DES_ADDRH, 0x4005);
                reg_write_bits(&pDev->DMA_SRC_ADDRL, *Addr_Ptr);
                reg_write_bits(&pDev->DMA_SRC_ADDRL, (*Addr_Ptr)>>16);
                reg_write_bits(&pDev->DMA_LEN, (0x100)-1);
                reg_set_bit(&pDev->DMA_REQ, BIT_DMA_REQ_START);
                VRAM_Addr+=0x200;
                while(reg_read_mask_bits(&pDev->DMA_REQ, 0x01<<BIT_DMA_REQ_START) != 0);
            }
            break;
            
        default:
            /* Wrong text layer */
            return -1;
    }
    
    return rval;
}

/**
 *  \brief  Set sprite color mode 
 *
 *  \param [in] SprNo                Select number of sprite
 *  \param [in] Color                enum ppu_color_mode
 *  \return                          0: ok, other: fail
 *
 *  \details Sprite NOT support hi-color mode
 */
int
PPU_Sprite_ColorMode(
    uint8_t SprNo,
    ppu_color_mode_t Color)
{
    int rval = 0;
    uint16_t *VSpr_Ptr;
    VSpr_Ptr = (uint16_t*)0x40056800;
    *(VSpr_Ptr+(SprNo*4)+3)&=0xFFFC;
    *(VSpr_Ptr+(SprNo*4)+3)|=(Color&0x3);
    
    return rval;
}

/**
 *  \brief  Set sprite coordinate of the screen
 *
 *  \param [in] SprNo                Select number of sprite
 *  \param [in] XPosition            X axis position
 *  \param [in] YPosition            Y axis position
 *  \return                          0: ok, other: fail
 *
 *  \details 
 */
int
PPU_Sprite_Position_Setting(
    uint8_t SprNo,
    uint16_t XPosition,
    uint16_t YPosition)
{
    int rval = 0;
    sn_ppu_t *pDev = (sn_ppu_t*) SN_PPU_BASE;
    uint8_t    Flag;
    uint16_t *VSpr_Ptr;
    uint16_t panel_width = 0;
    uint16_t panel_height = 0;
    VSpr_Ptr=(uint16_t *)(0x40056800);
    
    if(pDev->PPU_CTRL_b.PIXEL_LINE_Sel == 1 || pDev->PPU_CTRL_b.PIXEL_LINE_Sel_HD == 1)
    {
        panel_width = pDev->OTHER_RES_DISP_SIZE_X;
        panel_height = pDev->OTHER_RES_DISP_SIZE_Y;
    }
    else
    {
        panel_width = 320;
        panel_height = 240;
    }
    if(XPosition<(panel_width/2))
    {
        XPosition=0xffff-(panel_width/2)+XPosition;
        Flag = 1;
    }
    else
    {
        XPosition=XPosition-(panel_width/2);
        Flag = 0;
    }
    if(YPosition<(panel_height/2))
    {
        YPosition=0xffff-(panel_height/2)+(YPosition);
    }

    else
    {
        YPosition=YPosition-(panel_height/2);
    }

    *(VSpr_Ptr+(SprNo*4)+1)&=0x0;
    *(VSpr_Ptr+(SprNo*4)+1)|=(XPosition&0x3FF);
    *(VSpr_Ptr+(SprNo*4)+2)&=0xFC00;
    *(VSpr_Ptr+(SprNo*4)+2)|=(YPosition&0x3FF);

    if(Flag==0)
        *(VSpr_Ptr+(SprNo*4)+3)&=~0x40;
    else
        *(VSpr_Ptr+(SprNo*4)+3)|=0x40;
    
    return rval;
}

/**
 *  \brief  Set sprite flip status
 *
 *  \param [in] SprNo                Select number of sprite
 *  \param [in] HVFlip               enum ppu_flip
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Sprite_HVFlip(
    uint8_t SprNo,
    ppu_flip_t HVFlip)
{
    int rval = 0;
    uint16_t *VSpr_Ptr;
    VSpr_Ptr=(uint16_t *)(0x40056800);
    *(VSpr_Ptr+(SprNo*4)+3)&=0xFFF3;
    *(VSpr_Ptr+(SprNo*4)+3)|=((HVFlip&0x3)<<2);
    
    return rval;
}

/**
 *  \brief  Set Sprite Depth
 *
 *  \param [in] SprNo                Select number of sprite
 *  \param [in] Depth                enum ppu_depth_level
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Sprite_Depth_Setting(
    uint8_t SprNo,
    ppu_depth_level_t Depth)
{
    int rval = 0;
    uint16_t *VSpr_Ptr;
    VSpr_Ptr=(uint16_t *)(0x40056800);
    *(VSpr_Ptr+(SprNo*4)+3)&=0xCFFF;
    *(VSpr_Ptr+(SprNo*4)+3)|=((Depth&0x3)<<12);
    
    return rval;
}

/**
 *  \brief  Select sprite palette bank
 *
 *  \param [in] SprNo                Select number of sprite
 *  \param [in] PalBk                Palette select for number of sprite
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Sprite_Palette_Bank(
    uint8_t SprNo,
    uint8_t PalBk)
{
    int rval = 0;
    uint16_t *VSpr_Ptr;
    VSpr_Ptr=(uint16_t *)(0x40056800);
    *(VSpr_Ptr+(SprNo*4)+3)&=0xF0FF;
    *(VSpr_Ptr+(SprNo*4)+3)|=((PalBk&0xF)<<8);
    
    return rval;
}

/**
 *  \brief  Set sprite rotation & scaling
 *
 *  \param [in] Ori_V_Size           Original height of sprite
 *  \param [in] Desired_V_Size       Desired height of sprite
 *  \param [in] Ori_H_Size           Original width of sprite
 *  \param [in] Desired_H_Size       Desired width of sprite
 *  \param [in] Angle                Angle of sprite rotation(0~360 degrees)
 *  \param [in] ExtSize              0/1/2/3: x1/x2/x4/x8 Size of sprite
 *  \param [in] index                Select number of sprite
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int PPU_Sprite_Rotate_Scale(
    int32_t Ori_V_Size,
    int32_t Desired_V_Size,
    int32_t Ori_H_Size,
    int32_t Desired_H_Size,
    uint32_t Angle,
    ppu_sprite_extsize_t ExtSize,
    uint32_t index)
{
    int rval = 0;
    int16_t *Zeta;
    uint16_t *VSpr_Ptr;
    int32_t Sin_angle, Cos_angle;
    int16_t A, B, C, D;
    int32_t Xref, Yref;
    VSpr_Ptr=(uint16_t *)(0x40056800);
    
    Zeta = (int16_t *)Rot_Scale_Table;
    Sin_angle = *(Zeta+Angle);
    Cos_angle = *(Zeta+(Angle+90));
    
    A = Cos_angle*(((Ori_H_Size<<14)/Desired_H_Size)>>6)>>8;
    B = Sin_angle*(((Ori_H_Size<<14)/Desired_H_Size)>>6)>>8;
    C = -Sin_angle*(((Ori_V_Size<<14)/Desired_V_Size)>>6)>>8;
    D = Cos_angle*(((Ori_V_Size<<14)/Desired_V_Size)>>6)>>8;
    
    switch(ExtSize)
    {
        // Normal Size
        case PPU_SPR_SIZE_X1:
            Xref = (Ori_H_Size<<7) - (Ori_H_Size/2)*A - (Ori_V_Size/2)*B;
            Yref = (Ori_V_Size<<7) - (Ori_H_Size/2)*C - (Ori_V_Size/2)*D;
            break;

        // Size x2
        case PPU_SPR_SIZE_X2:
            Xref = (Ori_H_Size<<7) - (Ori_H_Size)*A - (Ori_V_Size)*B;
            Yref = (Ori_V_Size<<7) - (Ori_H_Size)*C - (Ori_V_Size)*D;
            break;

        // Size x4
        case PPU_SPR_SIZE_X4:
            Xref = (Ori_H_Size<<7) - (2*Ori_H_Size)*A - (2*Ori_V_Size)*B;
            Yref = (Ori_V_Size<<7) - (2*Ori_H_Size)*C - (2*Ori_V_Size)*D;
            break;

        // Size x8
        case PPU_SPR_SIZE_X8:
            Xref = (Ori_H_Size<<7) - (4*Ori_H_Size)*A - (4*Ori_V_Size)*B;
            Yref = (Ori_V_Size<<7) - (4*Ori_H_Size)*C - (4*Ori_V_Size)*D;
            break;

      default:
            break;
    }
    /* Add 20180610*/
    *(VSpr_Ptr+(index*4)+3) |= ExtSize<<4;
    *(VSpr_Ptr+(index*4)+3) |= 0x01 << 7;
    /* Add 20180610*/

    Zeta = (int16_t *)(0x40057000+index*16);
    *Zeta++ = A;
    *Zeta++ = B;
    *Zeta++ = C;
    *Zeta++ = D;
    *Zeta++ = Xref;
    *Zeta++ = Xref>>16;
    *Zeta++ = Yref;
    *Zeta++ = Yref>>16;
    
    return rval;
}

/**
 *  \brief  get the version of ppu driver
 *
 *  \return                          version of ppu driver
 *
 *  \details
 */
uint32_t
PPU_GetVersion(void)
{
    return PPU_VERSION;
}
