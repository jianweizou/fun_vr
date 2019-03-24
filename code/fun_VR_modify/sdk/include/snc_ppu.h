/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_ppu.h
 *
 * @author Syuan Jhao
 * @version 0.1
 * @date 2018/04/30
 * @license
 * @description
 */

#ifndef __snc_ppu_H_wbEfWEnD_l0to_H5CO_sjxk_uPp3U7ZbsUhJ__
#define __snc_ppu_H_wbEfWEnD_l0to_H5CO_sjxk_uPp3U7ZbsUhJ__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum ppu_load_memory
{
    PPU_SRAM = 0,
    PPU_DDR, 
    PPU_SRAM_DDR,
    PPU_DDR_SRAM
} ppu_load_memory_t;
    
    
typedef enum ppu_text
{
    TEXT1 = 0,
    TEXT2,
    TEXT3
} ppu_text_t;

typedef enum ppu_text_vram_clr
{
    TX_NO_CLR = 0,
    TX_ATTR_CLR,
    TX_PALETTE1_CLR,
    TX_PALETTE2_CLR
} ppu_text_vram_clr_t;

typedef enum ppu_spr_vram_clr
{
    SPR_NO_CLR = 0,
    SPR_PALETTE_CLR,
    SPR_PARAMETER_CLR,
    SPR_ROTATION_CLR
} ppu_spr_vram_clr_t;

typedef enum ppu_color_mode
{
    PPU_COLOR_4 = 0,
    PPU_COLOR_16,
    PPU_NONE,
    PPU_COLOR_256,
    PPU_HIGH_COLOR
} ppu_color_mode_t;

typedef enum ppu_character_size
{
    CHAR_8x8 = 0,
    CHAR_8x16,
    CHAR_8x32,
    CHAR_8x64,
    CHAR_16x8,
    CHAR_16x16,
    CHAR_16x32,
    CHAR_16x64,
    CHAR_32x8,
    CHAR_32x16,
    CHAR_32x32,
    CHAR_32x64,
    CHAR_64x8,
    CHAR_64x16,
    CHAR_64x32,
    CHAR_64x64
} ppu_character_size_t;

typedef enum ppu_flip
{
    PPU_NO_FLIP = 0,
    PPU_HFLIP,
    PPU_VFLIP,
    PPU_HVFLIP
} ppu_flip_t;

typedef enum ppu_text3_buf
{
    TEXT3_BUF_0 = 0,
    TEXT3_BUF_1
} ppu_text3_buf_t;

typedef enum ppu_sprite_status
{
    SPR_DISABLE = 0,
    SPR_ENABLE
} ppu_sprite_status_t;

typedef enum ppu_brightness_level
{
    BR_LEVEL_0 = 0,
    BR_LEVEL_1,
    BR_LEVEL_2,
    BR_LEVEL_3,
    BR_LEVEL_4,
    BR_LEVEL_5,
    BR_LEVEL_6,
    BR_LEVEL_7,
    BR_LEVEL_8,
    BR_LEVEL_9,
    BR_LEVEL_10,
    BR_LEVEL_11,
    BR_LEVEL_12,
    BR_LEVEL_13,
    BR_LEVEL_14,
    BR_LEVEL_15,
    BR_LEVEL_16,
} ppu_brightness_level_t;

typedef enum ppu_birghtness_mode
{
    PPU_BRIGHTNESS_INCREASE = 0,
    PPU_BRIGHTNESS_DECREASE
} ppu_birghtness_mode_t;

typedef enum ppu_depth_level
{
    PPU_LAYER_0 = 0,
    PPU_LAYER_1,
    PPU_LAYER_2,
    PPU_LAYER_3,
    
} ppu_depth_level_t;

typedef enum ppu_bright_out
{
    PPU_BRIGHT_IN = 0,
    PPU_BRIGHT_OUT
} ppu_bright_out_t;

typedef enum ppu_vcmp_scale
{
    PPU_NO_VCMP = 0,
    PPU_VCMP_HALF,
    PPU_VCMP_ONE_THIRD,
    PPU_VCMP_ONE_QUARTER,
    PPU_VCMP_DOUBLE
} ppu_vcmp_scale_t;

typedef enum ppu_blending_level
{
    BLD_LEVEL_0 = 0,
    BLD_LEVEL_1,
    BLD_LEVEL_2,
    BLD_LEVEL_3,
    BLD_LEVEL_4,
    BLD_LEVEL_5,
    BLD_LEVEL_6,
    BLD_LEVEL_7,
    BLD_LEVEL_8,
    BLD_LEVEL_9,
    BLD_LEVEL_10,
    BLD_LEVEL_11,
    BLD_LEVEL_12,
    BLD_LEVEL_13,
    BLD_LEVEL_14,
    BLD_LEVEL_15,
    BLD_LEVEL_16,
} ppu_blending_level_t;

typedef enum ppu_sprite_extsize
{
    PPU_SPR_SIZE_X1 = 0,
    PPU_SPR_SIZE_X2,
    PPU_SPR_SIZE_X8,
    PPU_SPR_SIZE_X4
} ppu_sprite_extsize_t;

typedef enum ppu_other_resolution_status
{
    PPU_OTHER_RES_DISABLE = 0,
    PPU_OTHER_RES_ENABLE
} ppu_other_resolution_status_t;

typedef enum ppu_burst_length
{
    PPU_BURST_LENGTH_8 = 0,
    PPU_BURST_LENGTH_16
} ppu_burst_length_t;

//=============================================================================
//                  Macro Definition
//=============================================================================
#define VRAM_TX1_AttrNum_Addr       0x40051000UL
#define VRAM_TX1_Num_Addr           0x40051800UL
#define VRAM_TX1_Palette_Addr1      0x40054000UL
#define VRAM_TX1_Palette_Addr2      0x40054400UL
#define VRAM_TX2_AttrNum_Addr       0x40052800UL
#define VRAM_TX2_Num_Addr           0x40053000UL
#define VRAM_TX2_Palette_Addr1      0x40054200UL
#define VRAM_TX2_Palette_Addr2      0x40054600UL
#define VRAM_SPR_Parameter_Addr     0x40056800UL
#define VRAM_SPR_Palette_Addr       0x40054800UL

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
    ppu_load_memory_t Mem_Device);

/**
 *  \brief  PPU Deinitial
 *
 *  \param 
 *  \return                 0: ok, other: fail
 *
 *  \details
 */
int
PPU_Deinit(void);

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
    uint8_t B);

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
    uint32_t uiDataLength);

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
    uint16_t Data);

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
    uint32_t uiAddr);

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
    ppu_birghtness_mode_t BRM);

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
    uint16_t VEnd);

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
    uint16_t Size_Y);

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
    ppu_burst_length_t Burst_Length);

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
    ppu_text_t text);

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
    ppu_text_t text);

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
    uint32_t uiTemp_Addr);

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
    uint16_t usY_Offset);

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
    ppu_color_mode_t color);

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
PPU_Text_CharacterMode_Init(
    ppu_text_t text,
    ppu_character_size_t Char_Size,
    ppu_color_mode_t Color,
    uint8_t ucVScrn_En);

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
    uint8_t Palette_Sel);

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
    uint32_t uiNumPtr_Addr);

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
    uint32_t uiAttrPtr_Addr);

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
    uint32_t uiPatPtr_Addr);

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
    uint32_t Display_Region);

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
    ppu_depth_level_t Depth);

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
    uint8_t WAPEN);

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
    uint16_t Offset);

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
    ppu_blending_level_t BLD_Level);

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
    uint32_t Height);

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
    uint32_t Buf_Addr);

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
    uint16_t SrcHight);

/* Sprite */

/**
 *  \brief  Enable the PPU Sprite Display
 *
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Sprite_Enable(void);

/**
 *  \brief  Disable the PPU Sprite Display
 *
 *  \return                          0: ok, other: fail
 *
 *  \details
 */
int
PPU_Sprite_Disable(void);

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
    ppu_sprite_status_t status);

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
    ppu_sprite_status_t status);

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
    uint32_t PatPtr_Addr);

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
    uint32_t Temp_Addr);

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
    ppu_color_mode_t Color);

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
    uint16_t YPosition);

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
    ppu_flip_t HVFlip);

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
    ppu_depth_level_t Depth);

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
    uint8_t PalBk);

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
int
PPU_Sprite_Rotate_Scale(
    int32_t Ori_V_Size,
    int32_t Desired_V_Size,
    int32_t Ori_H_Size,
    int32_t Desired_H_Size,
    uint32_t Angle,
    ppu_sprite_extsize_t ExtSize,
    uint32_t index);

/**
 *  \brief  get the version of ppu driver
 *
 *  \return                          version of ppu driver
 *
 *  \details
 */
uint32_t 
PPU_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif
