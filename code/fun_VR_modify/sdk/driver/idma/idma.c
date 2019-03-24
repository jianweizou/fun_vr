/**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file idma.c
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/03/5
 * @license
 * @description
 */

#include <string.h>
#include "snc_idma.h"
#include "snc_dram.h"   /* for dram register */
#include "snc_system.h" /* for sys0 dram channel */
#include "reg_util.h"

#include "register_7320.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define IDMA_VERSION        0x73200000

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint32_t transfer_busy = 0;    /* bit0-idma0, bit1-idma1 */

//=============================================================================
//                  Private Function Definition
//=============================================================================
static sn_idma_t*
_idma_get_handle(
    idma_ch_t   channel)
{
    sn_idma_t       *pDev = 0;
    switch( channel )
    {
        case IDMA_CH_0:             pDev = (sn_idma_t*)SN_IDMA_0_BASE;              break;
        case IDMA_CH_1:             pDev = (sn_idma_t*)SN_IDMA_1_BASE;              break;
        case IDMA_CH_SPIFC:         pDev = (sn_idma_t*)SN_IDMA_SPIFC_BASE;          break;
        case IDMA_CH_SDNF:          pDev = (sn_idma_t*)SN_IDMA_SDNF_BASE;           break;
        case IDMA_CH_SDIO:          pDev = (sn_idma_t*)SN_IDMA_SDIO_BASE;           break;
        case IDMA_CH_CIS:           pDev = (sn_idma_t*)SN_IDMA_CIS_BASE;            break;
        case IDMA_CH_CSC_IN:        pDev = (sn_idma_t*)SN_IDMA_CSC_IN_BASE;         break;
        case IDMA_CH_CSC_OUT:       pDev = (sn_idma_t*)SN_IDMA_CSC_OUT_BASE;        break;
        case IDMA_CH_JPEG_ENC_IN:   pDev = (sn_idma_t*)SN_IDMA_JPEG_ENC_IN_BASE;    break;
        case IDMA_CH_JPEG_ENC_OUT:  pDev = (sn_idma_t*)SN_IDMA_JPEG_ENC_OUT_BASE;   break;
        case IDMA_CH_JPEG_DEC_IN:   pDev = (sn_idma_t*)SN_IDMA_JPEG_DEC_IN_BASE;    break;
        case IDMA_CH_JPEG_DEC_OUT:  pDev = (sn_idma_t*)SN_IDMA_JPEG_DEC_OUT_BASE;   break;
        case IDMA_CH_SPI0:          pDev = (sn_idma_t*)SN_IDMA_SPI0_BASE;           break;
        case IDMA_CH_SPI1:          pDev = (sn_idma_t*)SN_IDMA_SPI1_BASE;           break;

        default:    break;
    }
    return pDev;
}




/**
 *  \brief Check if destination address is legal
 *  
 *  \param [in] channel: IDMA channel
 *  \param [in] des_addr: IDMA detination address
 *  
 */
static uint32_t
_idma_chk_addr(
    idma_ch_t       channel,
    idma_setting_t  *pSetting)
{
    uint32_t ret = 0;
    uint32_t dram_accessible_ch_table[] = { IDMA_CH_0,
                                            IDMA_CH_1,
                                            IDMA_CH_CIS,
                                            IDMA_CH_JPEG_ENC_OUT,
                                            IDMA_CH_JPEG_DEC_IN,
                                            0xFF
                                          };
    do
    {
        /* Exception conditions */
        if(pSetting->ram_addr == IDMA_ADDR_IGNORE)
        {
            ret = IDMA_RET_SUCCESS;
            break;
        }
        
        /* Check DRAM address */
        if( pSetting->ram_addr >= DRAM_BASE )
        {
            /* Check if available to access DRAM */
            uint32_t * ptr = dram_accessible_ch_table;
            while( *ptr != 0xFF )
            {
                if(*ptr  == channel )
                    break;
                ++ptr;
            }
            
            /* Need to redirect the buffer. */
            if( *ptr == 0xFF )
            {
                ret = IDMA_RET_DRAM_REDIRECT;
                break;
            }
            
            /* Check if channel is enabled */
            uint32_t pos_mask = 0;
            uint32_t ch_sel  = 0;
            switch(channel)
            {
                case IDMA_CH_0:
                    pos_mask = (SYSTEM_DDR_USER_DMA0_IN * 2);
                    break;
                case IDMA_CH_1:
                    pos_mask = (SYSTEM_DDR_USER_DMA1_IN * 2);
                    break;
                case IDMA_CH_JPEG_DEC_IN:
                    pos_mask = (SYSTEM_DDR_USER_JPEG_D_IN * 2);
                    break;
                case IDMA_CH_JPEG_ENC_OUT:
                    pos_mask = (SYSTEM_DDR_USER_JPEG_E_OUT * 2);
                    break;
                case IDMA_CH_CIS:
                    pos_mask = (SYSTEM_DDR_USER_CIS * 2);
                    break;
                default:
                    break;
            }
            ch_sel = (*(uint32_t*)SN_SYS0_DRAM_CHL_SEL >> pos_mask) & 3;
            
            if(ch_sel == 0)
            {
                ret = IDMA_RET_DRAM_CH_UNSELECTED;
                break;
            }
            
            #if 0   /* Synchronize the setting of dram and idma. */
            if(p_dev->CH_CTRL_b.BURST_NUM > 16)
            {
                uint32_t ch = 0;
                
                /* Check the current setting */
                if(channel == IDMA_CH_0)
                {
                    ch = (*(uint32_t*)SN_SYS0_DRAM_CHL_SEL >> 8) & 3;
                }
                if(channel == IDMA_CH_1)
                {
                    ch = (*(uint32_t*)SN_SYS0_DRAM_CHL_SEL >> 0) & 3;
                }

                ExtMem_Set_Burst_Num(   (ch - 1), 
                            p_dev->CH_CTRL_b.BURST_NUM, 
                            p_dev->CH_CTRL_b.BURST_NUM);
            }
            #endif
        }

        /* Check SRAM address */
        else if( (pSetting->ram_addr < SRAM_BASE)  )
        {
            ret = IDMA_RET_INVALID_ADDRESS;
            break;
        }         
        
        if( (pSetting->ram_addr >= (SRAM_BASE + SRAM_SIZE)) && (pSetting->ram_addr < DRAM_BASE) )
        {
            ret = IDMA_RET_INVALID_ADDRESS;
            break;
        }
        
    }while(0);
    
    return ret;
}

static uint32_t 
_idma_redirect_to_sram(
    idma_ch_t       channel,
    idma_setting_t  *pSetting)
{
    sn_idma_t   *p_dev      = _idma_get_handle(channel);
    sn_idma_t   *p_transfer;
    
    uint32_t    dram_ch;
    uint32_t    dram_ch_pos;
    uint32_t    buf_size    = pSetting->transfer.size_buff;
    uint32_t    burst_unit __attribute((unused));
    
    
    /* transfer buffer check *********************************************/
    uint32_t buf_tail = (uint32_t)pSetting->transfer.p_buff + pSetting->transfer.size_buff;
    
    if( (uint32_t)pSetting->transfer.p_buff < SRAM_BASE ||
        buf_tail > (SRAM_BASE + SRAM_SIZE))
    {
        return IDMA_RET_INVALID_TRANSFER_BUF;
    }
    
    /* transfer channel check */
    if(pSetting->transfer.ch == IDMA_TRANSFER_CH_0)
    {
        p_transfer = (sn_idma_t*)SN_IDMA_0_BASE;
        dram_ch_pos = 8;
        
       
    }
    else if(pSetting->transfer.ch == IDMA_TRANSFER_CH_1)
    {
        p_transfer    = (sn_idma_t*)SN_IDMA_1_BASE;
        dram_ch_pos = 0;
    }
    else
    {
        return IDMA_RET_TRANSFER_CH_UNSELECTED;
    }
    
    /* Busy state check - prevent form re-entry */
    if(transfer_busy & pSetting->transfer.ch)
    {
        return IDMA_RET_BUSY;
    }
    else
    {
        transfer_busy |= pSetting->transfer.ch;
    }
    
    dram_ch = (*(uint32_t*)SN_SYS0_DRAM_CHL_SEL >> dram_ch_pos) & 3;
    if(dram_ch == 0)
    {
        return IDMA_RET_TRANSFER_CH_UNSELECTED;
    }
     
    if(pSetting->transfer.size_buff < 4)
    {
        return IDMA_RET_INVALID_TRANSFER_BUF;
    }
    
    /* Caculate the burst number *********************************************/   
//    for(burst_unit = 1024; burst_unit>=4; burst_unit>>=1)
//    {
//        if((buf_size & (buf_size & burst_unit)) == 0)
//        {
//            break;
//        }
//    }
//    
//    IDMA_Set_Burst_Unit( channel, (burst_unit>>2) );
//    
//    /* If burst number greater than 16 word. */ 
//    if(burst_unit > 64)
//    {
//        ExtMem_Set_Burst_Num(   (dram_ch - 1), 
//                    (burst_unit>>2), 
//                    (burst_unit>>2));
//    }
//    else
//    {
//        ExtMem_Set_Burst_Num(   (dram_ch - 1), 0, 0);
//    }
    /* Set the burst number */
    IDMA_Set_Burst_Unit( (idma_ch_t)pSetting->transfer.ch , (idma_burst_t)p_dev->CH_CTRL_b.BURST_NUM );
    
    
    /* If polling method --------------------------------------------------------- */ 
//    if(pSetting->is_polling == 1)
    {
        uint32_t todo_size = pSetting->length;
        uint32_t done_size = 0;
        do
        {
            uint32_t do_size;
            
            if(todo_size > buf_size)
            {
                do_size = buf_size;
            }
            else
            {
                do_size = todo_size;
            }
            /* Start the IDMA procedure. */
                
            /* Check Read or write */
            switch(pSetting->direction)
            {
                case IDMA_DIRECTION_DIP_2_RAM:
                    /* #Direction: DIP -> DRAM _______________________________________ */
                
                    /* Step 1: DIP -> SRAM  */
                    p_dev->CH_DIP_ADDR       = pSetting->dip_addr + done_size;
                    p_dev->CH_RAM_ADDR       = (uint32_t)pSetting->transfer.p_buff;
                    p_dev->CH_LEN            = do_size;
                    p_dev->CH_CTRL          |= IDMA_CTRL_RDIP_Msk | IDMA_CTRL_START_Msk;
                    
                    while(p_dev->CH_CTRL_b.START);
                    
                    /* Step 2: SRAM -> DRAM(IDMA1)  */
                    p_transfer->CH_DIP_ADDR    = (uint32_t)pSetting->transfer.p_buff;
                    p_transfer->CH_RAM_ADDR    = pSetting->ram_addr + done_size;
                    p_transfer->CH_LEN         = do_size;
                    p_transfer->CH_CTRL       |= IDMA_CTRL_RDIP_Msk | IDMA_CTRL_START_Msk;
                                    
                    while(p_transfer->CH_CTRL_b.START);
                
                    break;
                
                case IDMA_DIRECTION_RAM_2_DIP:
                    /* #Direction: DRAM -> DIP _______________________________________ */

                    /* Step 1: DRAM -> SRAM(IDMA1)  */
                    p_transfer->CH_DIP_ADDR    = pSetting->ram_addr + done_size;
                    p_transfer->CH_RAM_ADDR    = (uint32_t)pSetting->transfer.p_buff;
                    p_transfer->CH_LEN         = do_size;
                    p_transfer->CH_CTRL        |= IDMA_CTRL_RDIP_Msk | IDMA_CTRL_START_Msk;

                    while(p_transfer->CH_CTRL_b.START);
                    
                    /* Step 2: SRAM -> DIP  */
                    p_dev->CH_DIP_ADDR       = pSetting->dip_addr + done_size;
                    p_dev->CH_RAM_ADDR       = (uint32_t)pSetting->transfer.p_buff;
                    p_dev->CH_LEN            = do_size;
                    p_dev->CH_CTRL          &= ~IDMA_CTRL_RDIP_Msk; /* from ram -> dip */
                    p_dev->CH_CTRL          |= IDMA_CTRL_START_Msk;
                
                    while(p_dev->CH_CTRL_b.START);
                                    
                    break;
            }
            
            todo_size -= do_size;
            done_size += do_size;
            
        }while(todo_size);
    }
    
    /* Remove busy state */
    transfer_busy &= ~pSetting->transfer.ch;
    
    #if 0
    else
    /* If interrupt method ---------------------------------------------------- */ 
    {
        if(idma1_task_t.is_busy)
        {
            return IDMA_RET_BUSY;
        }
        
        idma1_task_t.ch_dma1 = IDMA_CH_1;
        idma1_task_t.ch_dev  = channel;
        
        IRQn_Type irq_first_step;
        
        memcpy(&idma1_task_t.setting, pSetting, sizeof(idma_setting_t));
        
        /* Find IRQn*/
        switch(channel)
        {
            case IDMA_CH_0:         irq_first_step = IDMA0_IRQn;        break;  /* For test */
            case IDMA_CH_SDIO:      irq_first_step = SDIO_DMA_IRQn;     break;
            case IDMA_CH_SDNF:      irq_first_step = SDNF_DMA_IRQn;     break;
            case IDMA_CH_SPI0:      irq_first_step = SPI0_DMA_IRQn;     break;
            case IDMA_CH_SPI1:      irq_first_step = SPI1_DMA_IRQn;     break;
            default:
                return IDMA_RET_INVALID_ADDRESS;
        }
        
        NVIC_EnableIRQ(IDMA1_IRQn);
        NVIC_EnableIRQ(irq_first_step);
        
        /* Check Read or write */
        if(pSetting->direction == IDMA_DIRECTION_DIP_2_RAM)
        {
                /* #Direction: DRAM -> DIP _______________________________________ */
                irq_first_step = IDMA1_IRQn;
        }

        /* Software trigger INT */
        NVIC->STIR  = irq_first_step;
    }
    #endif
    
    return IDMA_RET_SUCCESS;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================

/**
 *  \brief  Start IDMA transmission 
 *
 *  \param [in] channel     channel of IDMA (enum idma_ch)
 *  \param [in] pSetting    setting info
 *                              if (pSetting == 0), it will wait IDMA idle and return
 *  \return
 *
 *  \details
 */
uint32_t
IDMA_Start(
    idma_ch_t       channel,
    idma_setting_t  *pSetting)
{
    int         ret = 0;
    sn_idma_t   *p_dev = 0;

    do {
        uint32_t    value = 0;

        p_dev = _idma_get_handle(channel);
        if( p_dev == 0 )
        {
            ret = IDMA_RET_INVALID_CHANNEL;
            break;
        }
        
        // wait idle
        while( reg_read_mask_bits(&p_dev->CH_CTRL, 0x1) ) {}

        if( !pSetting )
            break;
        
        if( (pSetting->dip_addr & 0x3) || (pSetting->ram_addr & 0x3) )
        {
            /* Not aligned with 4 */        
            ret = IDMA_RET_UNALIGNED_ADDRESS;
            break;
        }
        
        if( pSetting->length & 0x3 )
        {
            /* Not aligned with 4 */        
            ret = IDMA_RET_INVALID_LENGTH;
            break;
        }
        
        ret = _idma_chk_addr(channel, pSetting);
        
        /** Redirect destination buffer to SRAM *********************************/
        if(ret == IDMA_RET_DRAM_REDIRECT)
        {
            ret = _idma_redirect_to_sram(channel, pSetting);
            break;  /* The procedure is different from ordinary idma flow */
        }
        else if(ret != IDMA_RET_SUCCESS)
        {
            break;
        }
        
        /* Start the IDMA procedure. */
        reg_write_bits(&p_dev->CH_DIP_ADDR, pSetting->dip_addr);
        reg_write_bits(&p_dev->CH_LEN     , pSetting->length  );
        
        
        if( pSetting->ram_addr == IDMA_ADDR_IGNORE )
        {
            reg_write_bits(&p_dev->CH_RAM_ADDR, 0);
        }
        else{
            reg_write_bits(&p_dev->CH_RAM_ADDR, pSetting->ram_addr);
        }
                
        value |= IDMA_CTRL_START_Msk;
        
        
        if( ( pSetting->direction == IDMA_DIRECTION_DIP_2_RAM ) ||
            ( channel == IDMA_CH_0 || channel == IDMA_CH_1 ))       /* Fixed Direction */
        {
            value |= IDMA_CTRL_RDIP_Msk;
        }
            
        reg_write_mask_bits(&p_dev->CH_CTRL, 
                            value, 
                            (IDMA_CTRL_RDIP_Msk | IDMA_CTRL_START_Msk));

    } while(0);

    return ret;
}

/**
 *  \brief  Restart IDMA
 *
 *  \param [in] channel     channel of IDMA (enum idma_ch)
 *  
 *  \return 0- success, 1- IDMA busy.
 *
 *  \details Restart the IDMA without resetting the register, usually used with continue mode.
 */
    
uint32_t
IDMA_Restart(idma_ch_t       channel)
{
    sn_idma_t   *pDev = 0;
    pDev = _idma_get_handle(channel);
    
    if(pDev->CH_CTRL_b.START)
    {
        return 1;
    }
    else
    {
        pDev->CH_CTRL_b.START = 1;
        return 0;
    }
}
/**
 *  \brief  Stop IDMA transmission for special usage.
 *
 *  \param [in] channel     channel of IDMA (enum idma_ch)
 *
 *  \details
 */
uint32_t
IDMA_Stop(
    idma_ch_t       channel)
{
    sn_idma_t   *pDev   = _idma_get_handle(channel);
    
    reg_write_mask_bits(&pDev->CH_CTRL, 
                    0, 
                    IDMA_CTRL_START_Msk);
    return 0;
}

/**
 *  \brief  Return current transsmit byte number of IDMA for special usage.
 *
 *  \param [in] channel     channel of IDMA (enum idma_ch)
 *
 *  \return the complete byte number of current transmit DMA procedure.
 */
uint32_t
IDMA_Get_Cutrrent_Length(
    idma_ch_t       channel)
{
    sn_idma_t   *pDev   = _idma_get_handle(channel);
    
    return pDev->CH_LEN;
}

/**
 *  \brief Configure the IDMA extra functionality.
 *  
 *  \param [in] channel : IDMA channel select
 *  \param [in] config  : enum idma_cfg_t, can be or by each selections.
 *  \return             : 0- success, else- fail
 *  
 */
uint32_t
IDMA_Extra_Config(
    idma_ch_t   channel,
    uint32_t    config)
{
    sn_idma_t   *pDev   = _idma_get_handle(channel);
    uint32_t    mask    = 0;
    
    if( !pDev )
    {
        return IDMA_RET_FAIL;
    }
    
    if( config >= IDMA_CFG_INVALID)
    {
        return IDMA_RET_FAIL;
    }
    
    /* Repeat config */
    mask = config & (0x03 << 0);
    if(mask)
    {
        if(mask & IDMA_CFG_CONTINUE_DIS)
        {
            pDev->CH_CTRL_b.CONTINUE = 0;
        }
        else if(mask & IDMA_CFG_CONTINUE_EN)
        {
            pDev->CH_CTRL_b.CONTINUE = 1;
        }
    }
    
    /* Scalar Config */
    mask = config & (0x07 << 2);
    if(mask)
    {
        if(mask & IDMA_CFG_SCALAR_DIS)
        {
            pDev->CH_CTRL_b.SCALAR = 0;
        }
        else if(mask & IDMA_CFG_SCALAR_HALF)
        {
            pDev->CH_CTRL_b.SCALAR = 1;
        }
        else if(mask & IDMA_CFG_SCALAR_A_QUARTER)
        {
            pDev->CH_CTRL_b.SCALAR = 2;
        }
    }
    
    /* AD config */
    mask = config & (0x03 << 5);
    if(mask)
    {
        if(mask & IDMA_CFG_CIS_AD_DIS)
        {
            pDev->CH_CTRL_b.AD_PASS = 0;
        }
        else if(mask & IDMA_CFG_CIS_AD_EN)
        {
            pDev->CH_CTRL_b.AD_PASS = 1;
        }
            
    }

    return 0;
}

/**
 *  \brief Set the burst number of DRAM, uint is 4Byte(Word).
 *  
 *  \param [in] channel     : IDMA channel select
 *  \param [in] burst_num   : burst data length of DRAM, defined @idma_burst_t

*  \return                 : 0- success, else- fail
 *  
 */

uint32_t
IDMA_Set_Burst_Unit(
    idma_ch_t       channel,
    idma_burst_t    burst_num)
{
    sn_idma_t   *pDev = _idma_get_handle(channel);
    
    if( !pDev )
    {
        return IDMA_RET_FAIL;
    }
    
    if(!burst_num)
    {
        return IDMA_RET_FAIL;
    }
    
    if(burst_num > 256)
    {
        return IDMA_RET_FAIL;
    }
    
    pDev->CH_CTRL_b.BURST_NUM   = burst_num;
    
    return 0;
}

/**
 *  \brief  Get IDMA status
 *
 *  \param [in] channel     enum idma_ch
 *  \return                 0: idle, other: busy
 *
 *  \details
 */
uint32_t
IDMA_Get_Status(
    idma_ch_t   channel)
{
    uint32_t ret    = 0;
    sn_idma_t *pDev = 0;

    do {
        pDev = _idma_get_handle(channel);
        if( !pDev )
        {
            ret = IDMA_RET_FAIL;
            break;
        }
        
//        /* Check if IDMA redirection */
//        if(channel == idma1_task_t.ch_dev)
//        {
//            ret = idma1_task_t.is_busy;
//        }
//        else
        {
            ret = reg_read_mask_bits(&pDev->CH_CTRL, 0x1);
        }

    } while(0);

    return ret;
}


#if 0
/* DIP->RAM: DMA_IP -> DMA_1 */
/* RAM->DIP: DMA_1  -> DMA_IP */

/**
 *  When IRQ_IDMA1 pending means IDMA1 was finished and need to trigger DMA_IP
 */
uint32_t IDMA1_IRQ_UpdateStatus()
{
    uint32_t todo_size     = idma1_task_t.setting.length;
    uint32_t do_size;
    sn_idma_t * p_dev      =  _idma_get_handle(idma1_task_t.ch_dev);

    if(todo_size > IDMA_LOCAL_BUF_SIZE)
    {
        do_size = IDMA_LOCAL_BUF_SIZE;
    }
    else if (todo_size > 0)
    {
        do_size = todo_size;
    }
    else
    {
        return 0;
    }
    
    /* Start the IDMA procedure. */
        
    /* Check Read or write */
    switch(idma1_task_t.setting.direction)
    {
        case IDMA_DIRECTION_DIP_2_RAM:
            /* #Direction: DIP -> DRAM - IP-DMA exe first ______________________ */
            /**
             * DIP  -> SRAM  (via IP_DMA)
             * SRAM -> DRAM  (via IDMA1)
             */
            /* If is the begaining of the procedure. */
        
            if(idma1_task_t.is_busy == 0)
            {
                idma1_task_t.is_busy = 1;
            }
            else /* If the procedure is running */
            {
                idma1_task_t.setting.length -= do_size;
                idma1_task_t.done_size      += do_size;
                
                if(idma1_task_t.setting.length == 0)
                {
                    idma1_task_t.is_busy = 0;
                    return 1;
                }
            }
            
            /* Step 1: DIP -> SRAM(DMA-IP)  */
            p_dev->CH_DIP_ADDR       = (uint32_t)idma1_task_t.setting.dip_addr + idma1_task_t.done_size;
            p_dev->CH_RAM_ADDR       = (uint32_t)pSetting->transfer.p_buff;
            p_dev->CH_LEN            = do_size;
            p_dev->CH_CTRL          |= IDMA_CTRL_RDIP_Msk | IDMA_CTRL_START_Msk;
            
             
            break;
        
        case IDMA_DIRECTION_RAM_2_DIP:
            /* #Direction: DRAM -> DIP - IDMA1 exe first _______________________ */
            /**
             * DRAM -> SRAM (via IDMA1)
             * SRAM -> DIP  (via IP_DMA)
             */

            /* Step 2: SRAM -> DIP  */
            p_dev->CH_DIP_ADDR       = (uint32_t)idma1_task_t.setting.dip_addr + idma1_task_t.done_size;
            p_dev->CH_RAM_ADDR       = (uint32_t)pSetting->transfer.p_buff;
            p_dev->CH_LEN            = do_size;
            p_dev->CH_CTRL          &= ~IDMA_CTRL_RDIP_Msk; /* from ram -> dip */
            p_dev->CH_CTRL          |= IDMA_CTRL_START_Msk;
                            
            break;
    }
    return 0;
}


//DIP
uint32_t IDMA0_IRQ_UpdateStatus()
{
    uint32_t todo_size     = idma1_task_t.setting.length;
    uint32_t do_size;
    sn_idma_t * p_dma1     = _idma_get_handle(idma1_task_t.ch_dma1);

    if(todo_size > IDMA_LOCAL_BUF_SIZE)
    {
        do_size = IDMA_LOCAL_BUF_SIZE;
        
    }
    else if (todo_size > 0)
    {
        do_size = todo_size;
    }
    else
    {
        return 0;
    }
    
    /* Start the IDMA procedure. */
        
    /* Check Read or write */
    switch(idma1_task_t.setting.direction)
    {
        case IDMA_DIRECTION_RAM_2_DIP:
            /* #Direction: DRAM -> DIP - IDMA1 exe first _______________________ */
            /**
             * DRAM -> SRAM (via IDMA1)
             * SRAM -> DIP  (via IP_DMA)
             */

            /* If is the begaining of the procedure. */
            if(idma1_task_t.is_busy == 0)
            {
                idma1_task_t.is_busy = 1;
                

            }
            else /* If the procedure is running */
            {
                idma1_task_t.setting.length -= do_size;
                idma1_task_t.done_size      += do_size;
                
                if(idma1_task_t.setting.length == 0)
                {
                    idma1_task_t.is_busy = 0;
                    return 1;
                }
            }

            /* Config the IDMA */
            p_dma1->CH_DIP_ADDR       = (uint32_t)idma1_task_t.setting.ram_addr + idma1_task_t.done_size;
            p_dma1->CH_RAM_ADDR       = (uint32_t)pSetting->transfer.p_buff;
            p_dma1->CH_LEN            = do_size;
            p_dma1->CH_CTRL          |= IDMA_CTRL_RDIP_Msk | IDMA_CTRL_CONTINUE_MSK;
            
            /* Trigger the start bit, address info will increse automaticlly */
            p_dma1->CH_CTRL             |= IDMA_CTRL_START_Msk;

            break;
        
        case IDMA_DIRECTION_DIP_2_RAM:
            /* #Direction: DIP -> DRAM - IP-DMA exe first ______________________ */
            /**
             * DIP -> SRAM   (via IP_DMA)
             * SRAM -> DRAM  (via IDMA1)
             */
            /* If is the begaining of the procedure. */
            /* Step 2: SRAM -> DIP  */
            p_dma1->CH_DIP_ADDR       = (uint32_t)pSetting->transfer.;
            p_dma1->CH_RAM_ADDR       = (uint32_t)idma1_task_t.setting.ram_addr + idma1_task_t.done_size;
            p_dma1->CH_LEN            = do_size;
            p_dma1->CH_CTRL          |= IDMA_CTRL_RDIP_Msk | IDMA_CTRL_CONTINUE_MSK | IDMA_CTRL_START_Msk;
                            
            break;
    }
    return 0;
}














__irq void IDMA1_IRQHandler()
{
    IDMA1_IRQ_UpdateStatus();
}


__irq void IDMA0_IRQHandler()
{
    IDMA0_IRQ_UpdateStatus();
}
#endif


uint32_t
IDMA_GetVersion(void)
{
    return IDMA_VERSION;
}




