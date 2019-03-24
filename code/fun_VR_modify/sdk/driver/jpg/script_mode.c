/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file script_mode.c
 *
 * @author Bokai Wang
 * @version 0.1
 * @date 2018/01/10
 * @license
 * @description
 */

#include <stdio.h>
#include <string.h>
#include "snc_script.h"
#include "snc_jpeg.h"
#include "snc_csc.h"
#include "snc_idma.h"
#include "snc_ppu.h"
#include "snc_tft.h"
#include "register_7320.h"
#define SCRIPT_VERSION        0x73200000
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct script_init_flow
{
    struct script_step_info
    {
        uint32_t ip_adr;
        uint32_t ip_data;
        union 
        {
            uint8_t step_info;
            struct
            {
                uint8_t jmp_step : 1;
                uint8_t loop_en  : 1;
            }step_info_t;
        };
    }script_step_info_t;

    struct script_irq_info
    {
        uint8_t irq_num;
        union
        {
            uint8_t irq_info;
            struct
            {
                uint8_t jifn_int  : 1;
                uint8_t cpu_write_en  : 1;
                uint8_t wait_irq_en : 1;
            }irq_info_t;
        };
    }script_irq_info_t;
}script_init_flow_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern void JPEG_DEC_Idma_Out(uint32_t wram_addr, uint32_t len);
extern void JPEG_DEC_Idma_In(uint32_t wram_addr, uint32_t len);
extern void CSC_Idma_Out(uint32_t wram_addr, uint32_t len);
extern void CSC_Idma_In(uint32_t wram_addr, uint32_t len);
uint32_t script_proc_done = 0;
uint8_t en_tx3_flag = 0;
uint8_t HBLK;
uint8_t VBLK;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void SCRIPT_Enable(void)
{
    SN_SCRIPT_Type_t *SN_SCRIPT = (SN_SCRIPT_Type_t*)(SN_SCRIPT_BASE);
    NVIC_ClearPendingIRQ(SCRIPT_IRQn); 
    NVIC_EnableIRQ(SCRIPT_IRQn);
    SN_SCRIPT->SCRIPT_INIT_b.Active = 1;
}

uint8_t SCRIPT_Status(void)
{
    SN_SCRIPT_Type_t *SN_SCRIPT = (SN_SCRIPT_Type_t*)(SN_SCRIPT_BASE);
    return SN_SCRIPT->SCRIPT_INIT_b.Active;
}

void SCRIPT_Rst(void)
{
    SN_SCRIPT_Type_t *SN_SCRIPT = (SN_SCRIPT_Type_t*)(SN_SCRIPT_BASE);
    SN_SCRIPT->SCRIPT_INIT_b.Reset = 1;
}

uint8_t SCRIPT_Cur_Step(void)
{
    SN_SCRIPT_Type_t *SN_SCRIPT = (SN_SCRIPT_Type_t*)(SN_SCRIPT_BASE);
    return SN_SCRIPT->SCRIPT_INIT_b.Current_Step;
}

uint8_t SCRIPT_Cur_Loop(void)
{
    SN_SCRIPT_Type_t *SN_SCRIPT = (SN_SCRIPT_Type_t*)(SN_SCRIPT_BASE);
    return SN_SCRIPT->SCRIPT_INIT_b.Current_loop;
}

void SCRIPT_Set_Total_Loop(uint8_t total_loop)
{
    SN_SCRIPT_Type_t *SN_SCRIPT = (SN_SCRIPT_Type_t*)(SN_SCRIPT_BASE);
    SN_SCRIPT->SCRIPT_INIT_b.Total_loop = total_loop;
}

//void SCRIPT_Set_Step(uint8_t step_num, uint32_t reg_addr, uint32_t set_data, uint8_t jmp_step, uint8_t irq_num, uint8_t loop_en, uint8_t J_IFN_int, uint8_t cpu_w_en, uint8_t wait_irq_en)
//{
//    SN_SCRIPT_Type_t *SN_SCRIPT = (SN_SCRIPT_Type_t*)(SN_SCRIPT_BASE);
//    SCRIPT_STEP_Type *step;
//    step = (SCRIPT_STEP_Type*)&SN_SCRIPT->STEP1;
//    step = (SCRIPT_STEP_Type*)(step+(step_num-1));

//    step->SET_ADDR = reg_addr;
//    step->SET_DATA = set_data;
//    step->SCRIPT_STEP_b.Jmp_step = jmp_step;
//    step->SCRIPT_STEP_b.IRQ = irq_num;
//    step->SCRIPT_STEP_b.Loop_en = loop_en;
//    step->SCRIPT_STEP_b.J_IFN_int = J_IFN_int;
//    step->SCRIPT_STEP_b.CPU_w_en = cpu_w_en;
//    step->SCRIPT_STEP_b.wait_int_en = wait_irq_en;
//}

void SCRIPT_Set_Step_Info(uint8_t step_num, uint32_t reg_addr, uint32_t set_data, uint8_t jmp_step, uint8_t loop_en)
{
    SN_SCRIPT_Type_t *SN_SCRIPT = (SN_SCRIPT_Type_t*)(SN_SCRIPT_BASE);
    SCRIPT_STEP_Type *step;
    step = (SCRIPT_STEP_Type*)&SN_SCRIPT->STEP1;
    step = (SCRIPT_STEP_Type*)(step+(step_num-1));

    step->SET_ADDR = reg_addr;
    step->SET_DATA = set_data;
    step->SCRIPT_STEP_b.Jmp_step = jmp_step;
    step->SCRIPT_STEP_b.Loop_en = loop_en;
}

void SCRIPT_Set_Step_Irq(uint8_t step_num, uint8_t irq_num, uint8_t J_IFN_int, uint8_t cpu_w_en, uint8_t wait_irq_en)
{
    SN_SCRIPT_Type_t *SN_SCRIPT = (SN_SCRIPT_Type_t*)(SN_SCRIPT_BASE);
    SCRIPT_STEP_Type *step;
    step = (SCRIPT_STEP_Type*)&SN_SCRIPT->STEP1;
    step = (SCRIPT_STEP_Type*)(step+(step_num-1));

    step->SCRIPT_STEP_b.IRQ = irq_num;
    step->SCRIPT_STEP_b.J_IFN_int = J_IFN_int;
    step->SCRIPT_STEP_b.CPU_w_en = cpu_w_en;
    step->SCRIPT_STEP_b.wait_int_en = wait_irq_en;
}

void _422scal_3ratio_step_init(script_handle_info_t *script)
{
    script_init_flow_t scaling_3[0x1D] = {
        {//SN_IDMA_CSC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40026110,
            .script_step_info_t.ip_data = script->csc_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40026110,
            .script_step_info_t.ip_data = script->csc_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {
            .script_step_info_t.ip_adr = 0,
            .script_step_info_t.ip_data = 0,
            .script_step_info_t.step_info = 1|1<<1,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0,
        },
    };
    for(int sccipt_loop = 0;sccipt_loop < 0x1E;sccipt_loop++)
    {
        SCRIPT_Set_Step_Info(sccipt_loop+1, scaling_3[sccipt_loop].script_step_info_t.ip_adr, 
                                            scaling_3[sccipt_loop].script_step_info_t.ip_data, 
                                            scaling_3[sccipt_loop].script_step_info_t.step_info_t.jmp_step, 
                                            scaling_3[sccipt_loop].script_step_info_t.step_info_t.loop_en); 
        SCRIPT_Set_Step_Irq(sccipt_loop+1,  scaling_3[sccipt_loop].script_irq_info_t.irq_num, 
                                            scaling_3[sccipt_loop].script_irq_info_t.irq_info_t.jifn_int,
                                            scaling_3[sccipt_loop].script_irq_info_t.irq_info_t.cpu_write_en, 
                                            scaling_3[sccipt_loop].script_irq_info_t.irq_info_t.wait_irq_en);
    }
}

void _422scal_2ratio_step_init(script_handle_info_t *script)
{
    script_init_flow_t scaling_2[0x15] = {
        {//SN_IDMA_CSC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40026110,
            .script_step_info_t.ip_data = script->csc_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40026110,
            .script_step_info_t.ip_data = script->csc_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = JPEG_DEC_OUT_DMA_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {
            .script_step_info_t.ip_adr = 0,
            .script_step_info_t.ip_data = 0,
            .script_step_info_t.step_info = 1|1<<1,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0,
        },
    };
    for(int sccipt_loop = 0;sccipt_loop < 0x15;sccipt_loop++)
    {
        SCRIPT_Set_Step_Info(sccipt_loop+1, scaling_2[sccipt_loop].script_step_info_t.ip_adr, 
                                            scaling_2[sccipt_loop].script_step_info_t.ip_data, 
                                            scaling_2[sccipt_loop].script_step_info_t.step_info_t.jmp_step, 
                                            scaling_2[sccipt_loop].script_step_info_t.step_info_t.loop_en); 
        SCRIPT_Set_Step_Irq(sccipt_loop+1,  scaling_2[sccipt_loop].script_irq_info_t.irq_num, 
                                            scaling_2[sccipt_loop].script_irq_info_t.irq_info_t.jifn_int,
                                            scaling_2[sccipt_loop].script_irq_info_t.irq_info_t.cpu_write_en, 
                                            scaling_2[sccipt_loop].script_irq_info_t.irq_info_t.wait_irq_en);
    }
}

void _422_step_init(script_handle_info_t *script)
{
    script_init_flow_t yuv422_init[0xF] = {
        {//SN_IDMA_CSC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40026110,
            .script_step_info_t.ip_data = script->csc_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_H
            .script_step_info_t.ip_adr = 0x40026000+0x1C,
            .script_step_info_t.ip_data = (script->jpeg_out_buf_1)>>16,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40026110,
            .script_step_info_t.ip_data = script->csc_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_H
            .script_step_info_t.ip_adr = 0x40026000+0x1C,
            .script_step_info_t.ip_data = (script->jpeg_out_buf_0)>>16,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_IN_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {
            .script_step_info_t.ip_adr = 0,
            .script_step_info_t.ip_data = 0,
            .script_step_info_t.step_info = 1|1<<1,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0,
        },
    };
    for(int sccipt_loop = 0;sccipt_loop < 0xF;sccipt_loop++)
    {
        SCRIPT_Set_Step_Info(sccipt_loop+1, yuv422_init[sccipt_loop].script_step_info_t.ip_adr, 
                                            yuv422_init[sccipt_loop].script_step_info_t.ip_data, 
                                            yuv422_init[sccipt_loop].script_step_info_t.step_info_t.jmp_step, 
                                            yuv422_init[sccipt_loop].script_step_info_t.step_info_t.loop_en); 
        SCRIPT_Set_Step_Irq(sccipt_loop+1,  yuv422_init[sccipt_loop].script_irq_info_t.irq_num, 
                                            yuv422_init[sccipt_loop].script_irq_info_t.irq_info_t.jifn_int,
                                            yuv422_init[sccipt_loop].script_irq_info_t.irq_info_t.cpu_write_en, 
                                            yuv422_init[sccipt_loop].script_irq_info_t.irq_info_t.wait_irq_en);
    }
}

void _420_step_init(script_handle_info_t *script)
{
    script_init_flow_t yuv420_init[0x15] = {
        {//TX3_ADDR
            .script_step_info_t.ip_adr = 0x40050100+0x4,
            .script_step_info_t.ip_data = script->csc_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//TX3_ADDR
            .script_step_info_t.ip_adr = 0x40050100+0x8,
            .script_step_info_t.ip_data = script->csc_out_buf_1 + script->csc_out_buf_len /2,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40026110,
            .script_step_info_t.ip_data = script->csc_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_H
            .script_step_info_t.ip_adr = 0x40026000+0x1C,
            .script_step_info_t.ip_data = (script->jpeg_out_buf_0)>>16,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_OUT_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//TX3_ADDR
            .script_step_info_t.ip_adr = 0x40050100+0x4,
            .script_step_info_t.ip_data = script->csc_out_buf_0,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//TX3_ADDR
            .script_step_info_t.ip_adr = 0x40050100+0x8,
            .script_step_info_t.ip_data = script->csc_out_buf_0 + script->csc_out_buf_len /2,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = PPU_HBLK_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40026110,
            .script_step_info_t.ip_data = script->csc_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_H
            .script_step_info_t.ip_adr = 0x40026000+0x1C,
            .script_step_info_t.ip_data = (script->jpeg_out_buf_1)>>16,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_CSC->CSC_DMA_DATA_IN_Addr_L
            .script_step_info_t.ip_adr = 0x40026000+0x18,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_CSC_IN->CH_CTRL
            .script_step_info_t.ip_adr = 0x40026100+0x8,
            .script_step_info_t.ip_data = 0x9,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_RAM_ADDR
            .script_step_info_t.ip_adr = 0x40028110,
            .script_step_info_t.ip_data = script->jpeg_out_buf_1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = CSC_DMA_OUT_IRQn,
            .script_irq_info_t.irq_info = 0|1<<1|1<<2,
        },
        {//SN_IDMA_JPEG_DEC_OUT->CH_CTRL
            .script_step_info_t.ip_adr = 0x40028110+0x8,
            .script_step_info_t.ip_data = 0x1,
            .script_step_info_t.step_info = 0,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0|1<<1|0<<2,
        },
        {
            .script_step_info_t.ip_adr = 0,
            .script_step_info_t.ip_data = 0,
            .script_step_info_t.step_info = 1|1<<1,
            .script_irq_info_t.irq_num = 0,
            .script_irq_info_t.irq_info = 0,
        },
    };
    for(int sccipt_loop = 0;sccipt_loop < 0x15;sccipt_loop++)
    {
        SCRIPT_Set_Step_Info(sccipt_loop+1, yuv420_init[sccipt_loop].script_step_info_t.ip_adr, 
                                            yuv420_init[sccipt_loop].script_step_info_t.ip_data, 
                                            yuv420_init[sccipt_loop].script_step_info_t.step_info_t.jmp_step, 
                                            yuv420_init[sccipt_loop].script_step_info_t.step_info_t.loop_en); 
        SCRIPT_Set_Step_Irq(sccipt_loop+1,  yuv420_init[sccipt_loop].script_irq_info_t.irq_num, 
                                            yuv420_init[sccipt_loop].script_irq_info_t.irq_info_t.jifn_int,
                                            yuv420_init[sccipt_loop].script_irq_info_t.irq_info_t.cpu_write_en, 
                                            yuv420_init[sccipt_loop].script_irq_info_t.irq_info_t.wait_irq_en);
    }
}

int SCRIPT_Init(script_handle_info_t *script)
{
    
    if(script->en_scal)
    {
        if(script->frame_h / script->scal_frame_h == 3)
        {
            _422scal_3ratio_step_init(script);
        }
        else if(script->frame_h / script->scal_frame_h == 2)
        {
            _422scal_2ratio_step_init(script);
        }
    }
    else
    {
        if(script->jpeg_fmt == JPEG_YCBCR_420)
        {
            _420_step_init(script);
        }
        else if(script->jpeg_fmt == JPEG_YCBCR_422)
        {
            _422_step_init(script);
        }
    }
    return 0;
}

int SCRIPT_Proc(script_handle_info_t *script)
{
    if(script->en_scal)
    {
        if(script->frame_h / script->scal_frame_h == 3)
        {
            script_proc_done = 0;
            
            JPEG_DEC_Idma_In(script->jpeg_in_buf + (script->header_length/4*4), script->jpeg_bistrm_size);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT) || IDMA_Get_Status(IDMA_CH_CSC_IN));
            
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT) || IDMA_Get_Status(IDMA_CH_CSC_IN));
            
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT) || IDMA_Get_Status(IDMA_CH_CSC_IN));
            
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            CSC_Idma_Out(script->csc_out_buf_1, script->csc_out_buf_len);
            
            while(IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT) || IDMA_Get_Status(IDMA_CH_CSC_IN));
            
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT) || IDMA_Get_Status(IDMA_CH_CSC_IN));
            
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            
            if(!en_tx3_flag)
            {
                PPU_Background_Color (0X1F,0X3F,0X1F);
                PPU_Text3_Line_DMA_Setting(script->scal_frame_w, script->scal_frame_h);
                PPU_Text3_BufferAddress_Setting(TEXT3_BUF_0, script->csc_out_buf_0);
                PPU_Text3_BufferAddress_Setting(TEXT3_BUF_1, script->csc_out_buf_1);
                PPU_Text_Init(TEXT3);
                PPU_Init(PPU_SRAM);
                TFT_HBLK_Interrupt_Enable(1);
                TFT_VBLK_Interrupt_Enable(1);
                en_tx3_flag = 1;
            }
            SCRIPT_Rst();
            SCRIPT_Set_Total_Loop((script->scal_frame_h-16-16)/16);
            
            HBLK = 0;
            
            SCRIPT_Enable();
            while(!script_proc_done);
            
            HBLK = 0;
            while(HBLK != 1);
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            
            while(IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            
            while(IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            
            while(IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            
            HBLK = 0;
            while(HBLK != 1);
            CSC_Idma_Out(script->csc_out_buf_1, script->csc_out_buf_len);
            
            while(IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT) || IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            
            while(VBLK == 0);
            VBLK = 0;
            
            
        }
        else if(script->frame_h / script->scal_frame_h == 2)
        {
            script_proc_done = 0;
            
            JPEG_DEC_Idma_In(script->jpeg_in_buf + (script->header_length/4*4), script->jpeg_bistrm_size);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT) && IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT) && IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            CSC_Idma_Out(script->csc_out_buf_1, script->csc_out_buf_len);
            
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT) && IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT) && IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            
            if(!en_tx3_flag)
            {
                PPU_Background_Color (0X1F,0X3F,0X1F);
                PPU_Text3_Line_DMA_Setting(script->scal_frame_w, script->scal_frame_h);
                PPU_Text3_BufferAddress_Setting(TEXT3_BUF_0, script->csc_out_buf_0);
                PPU_Text3_BufferAddress_Setting(TEXT3_BUF_1, script->csc_out_buf_1);
                PPU_Text_Init(TEXT3);
                PPU_Init(PPU_SRAM);
                TFT_HBLK_Interrupt_Enable(1);
                TFT_VBLK_Interrupt_Enable(1);
                en_tx3_flag = 1;
            }
            SCRIPT_Rst();
            SCRIPT_Set_Total_Loop((script->scal_frame_h-16-16)/16);
            HBLK = 0;
            
            SCRIPT_Enable();
            while(!script_proc_done);
            HBLK = 0;
            while(HBLK != 1);
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            
            while(IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            
            while(IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            
            HBLK = 0;
            while(HBLK != 1);
            CSC_Idma_Out(script->csc_out_buf_1, script->csc_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_IN));
            CSC_Idma_In(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT) && IDMA_Get_Status(IDMA_CH_CSC_IN));
            while(VBLK == 0);
            VBLK = 0;
        }
    }
    else
    {
        if(script->jpeg_fmt == JPEG_YCBCR_420)
        {
            script_proc_done = 0;
            
            JPEG_DEC_Idma_In(script->jpeg_in_buf + (script->header_length/4*4), script->jpeg_bistrm_size);
            
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_0, script->csc_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT));

            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            CSC_Idma_Out(script->csc_out_buf_1, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_1, script->csc_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT));
            
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            SCRIPT_Rst();
            SCRIPT_Set_Total_Loop(5);
            if(!en_tx3_flag)
            {
                PPU_Background_Color (0X1F,0X3F,0X1F);
                PPU_Text3_Line_DMA_Setting(320, 240);
                PPU_Text3_BufferAddress_Setting(TEXT3_BUF_0, script->csc_out_buf_0);
                PPU_Text3_BufferAddress_Setting(TEXT3_BUF_1, script->csc_out_buf_0 + script->csc_out_buf_len /2);
                PPU_Text_Init(TEXT3);
                PPU_Init(PPU_SRAM);
                TFT_HBLK_Interrupt_Enable(1);
                TFT_VBLK_Interrupt_Enable(1);
                en_tx3_flag = 1;
            }
            SCRIPT_Enable();

            while(!script_proc_done);
            
            HBLK = 0;
            while(HBLK != 1);
            PPU_Text3_BufferAddress_Setting(TEXT3_BUF_0, script->csc_out_buf_1);
            HBLK = 0;
            while(HBLK != 1);
            PPU_Text3_BufferAddress_Setting(TEXT3_BUF_1, script->csc_out_buf_1 + script->csc_out_buf_len /2);
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_0, script->csc_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT));
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            
            HBLK = 0;
            while(HBLK != 1);
            PPU_Text3_BufferAddress_Setting(TEXT3_BUF_0, script->csc_out_buf_0);
            HBLK = 0;
            while(HBLK != 1);
            PPU_Text3_BufferAddress_Setting(TEXT3_BUF_1, script->csc_out_buf_0 + script->csc_out_buf_len /2);
            CSC_Idma_Out(script->csc_out_buf_1, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_1, script->csc_out_buf_len);

            HBLK = 0;
            while(HBLK != 1);
            PPU_Text3_BufferAddress_Setting(TEXT3_BUF_0, script->csc_out_buf_1);
            HBLK = 0;
            while(HBLK != 1);
            PPU_Text3_BufferAddress_Setting(TEXT3_BUF_1, script->csc_out_buf_1 + script->csc_out_buf_len /2);
            
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_0, script->csc_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT));
            
            HBLK = 0;
            while(HBLK != 1);
            PPU_Text3_BufferAddress_Setting(TEXT3_BUF_0, script->csc_out_buf_0);
            HBLK = 0;
            while(HBLK != 1);
            PPU_Text3_BufferAddress_Setting(TEXT3_BUF_1, script->csc_out_buf_0 + script->csc_out_buf_len /2);
            while(VBLK == 0);
            VBLK = 0;
        }
        else if(script->jpeg_fmt == JPEG_YCBCR_422)
        {
            script_proc_done = 0;
            
            JPEG_DEC_Idma_In(script->jpeg_in_buf + (script->header_length/4*4), script->jpeg_bistrm_size);
            
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_0, script->csc_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT));

            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            CSC_Idma_Out(script->csc_out_buf_1, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_1, script->csc_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT));
            
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_0, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            JPEG_DEC_Idma_Out(script->jpeg_out_buf_1, script->jpeg_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_JPEG_DEC_OUT));
            
            
            if(!en_tx3_flag)
            {
                PPU_Background_Color (0X1F,0X3F,0X1F);
                PPU_Text3_Line_DMA_Setting(320, 240);
                PPU_Text3_BufferAddress_Setting(TEXT3_BUF_0, script->csc_out_buf_0);
                PPU_Text3_BufferAddress_Setting(TEXT3_BUF_1, script->csc_out_buf_1);
                PPU_Text_Init(TEXT3);
                PPU_Init(PPU_SRAM);
                TFT_HBLK_Interrupt_Enable(1);
                TFT_VBLK_Interrupt_Enable(1);
                en_tx3_flag = 1;
            }
            SCRIPT_Rst();
            SCRIPT_Set_Total_Loop((script->frame_h-16-16)/16);
            HBLK = 0;
            while(HBLK != 1);
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_0, script->csc_out_buf_len);
            
            SCRIPT_Enable();

            while(!script_proc_done);
            
            HBLK = 0;
            while(HBLK != 1);
            CSC_Idma_Out(script->csc_out_buf_1, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_1, script->csc_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT));
            
            HBLK = 0;
            while(HBLK != 1);
            CSC_Idma_Out(script->csc_out_buf_0, script->csc_out_buf_len);
            CSC_Idma_In(script->jpeg_out_buf_0, script->csc_out_buf_len);
            while(IDMA_Get_Status(IDMA_CH_CSC_OUT));
            while(VBLK == 0);
            VBLK = 0;
        }
    }
    return 0;
}

void SCRIPT_Set_Hblk(void)
{
    HBLK = 1;
}

void SCRIPT_Set_Vblk(void)
{
    VBLK = 1;
}

void SCRIPT_End(void)
{
    script_proc_done = 1;
}

/**
 *  \brief get script library version
 *
 *  \param 
 *  \return script library version
 *
 *  \details
 */
uint32_t
SCRIPT_GetVersion(void)
{
    return SCRIPT_VERSION;
}
