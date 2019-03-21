 /**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file mpu.c
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/04/11
 * @license
 * @description
 */
 
#include "snc_types.h"
#include "snc_core_mpu.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define MPU_VERSION         0x73200000

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

static uint32_t 
__MPU_Set_Access(  mpu_attr_pri_t privilege, 
                   mpu_attr_unpri_t unprivilege
                )
{
    uint32_t ret = MPU_RET_SUCCESS;
    uint32_t setting = 0;
    
    do{
        if(privilege < unprivilege)
        {
            ret = MPU_RET_INVAILD_ATTRIBUTE;
            break;
        }
        
    
        switch(privilege)
        {
            case MPU_PRI_NONE:
                setting = 0;
                break;
            
            case MPU_PRI_RW:
                setting = unprivilege;
                break;
            
            case MPU_PRI_RO:
                setting = 4 | unprivilege;
                break;
        }
        /* Clear setting to zero */
        MPU->RASR &= ~(MPU_RASR_AP_Msk);
        MPU->RASR |= (( setting << MPU_RASR_AP_Pos) & MPU_RASR_AP_Msk);
        
    }while(0);
    
    return ret;
}


uint32_t MPU_Enable()
{
    /* MPU Common Setting */
    MPU->CTRL       |= MPU_CTRL_PRIVDEFENA_Msk;     /* privileged default enable */
//    MPU->CTRL       |= MPU_CTRL_HFNMIENA_Msk;       /* Enable MPU even in NMI/HardFault IRQ  */
    SCB->SHCSR      |= SCB_SHCSR_MEMFAULTENA_Msk;   /* Enable MPU Fault IRQ */
    
    /* Enable MPU */
    NVIC_EnableIRQ(MemManage_IRQn);
    MPU->CTRL       |= MPU_CTRL_ENABLE_Msk;
    
    return MPU_RET_SUCCESS;
}


uint32_t MPU_Disable()
{
    MPU->CTRL       &= ~MPU_CTRL_ENABLE_Msk;
    NVIC_DisableIRQ(MemManage_IRQn);
    
    return MPU_RET_SUCCESS;
}


uint32_t MPU_Config_Region(mpu_region_set_t *p)
{
    uint32_t ret = 0;

    do{
        /* Check settings. */
        if(p->index > 7)
        {
            ret = MPU_RET_INVAILD_INDEX;
            break;
        }
        
        if((p->size < 4) || (p->size > 31))
        {
            ret = MPU_RET_INVAILD_SIZE;
            break;
        }
        
        MPU->RBAR      |= MPU_RBAR_VALID_Msk;
        
        /* #.RNR: Region number register -------------------------------*/
        MPU->RNR        = p->index;
        
        /* #.RBSR: Region base address register. -----------------------*/
        MPU->RBAR       = (p->start_addr & MPU_RBAR_ADDR_Msk);  /* Update RBAR addr and region setting */
        
        /* #.RASR: Region Attribute and Size Register ------------------*/
        MPU->RASR       = (p->size << MPU_RASR_SIZE_Pos) & MPU_RASR_SIZE_Msk
                            ;  /* Assign Regions size */
        
        if(__MPU_Set_Access(p->privilege_access, p->unprivilege_access))
        {
            ret = MPU_RET_INVAILD_ATTRIBUTE;
            break;
        }
        
        /* Enable current region */
        MPU->RASR   |= MPU_CTRL_ENABLE_Msk;
        
    }while(0);
    
    return ret;
}

uint32_t MPU_Disable_Region(uint32_t region_num)
{
    /* Disable MPU */
    MPU->CTRL       &= ~MPU_CTRL_ENABLE_Msk;
    NVIC_DisableIRQ(MemManage_IRQn);
    
    MPU->RBAR       |= MPU_RBAR_VALID_Msk;
    MPU->RNR         = region_num;
    MPU->RASR       &= ~MPU_CTRL_ENABLE_Msk;
    
    /* Enable MPU */
    NVIC_EnableIRQ(MemManage_IRQn);
    SCB->SHCSR      |= SCB_SHCSR_MEMFAULTENA_Msk;  /* Enable MPU Fault IRQ */
    MPU->CTRL       |= MPU_CTRL_ENABLE_Msk;
    
    return MPU_RET_SUCCESS;
}

uint32_t MPU_Init(uint32_t total_region , mpu_region_set_t *pTable)
{
    uint32_t ret = 0;
    uint32_t Regions_mpu = ((MPU->TYPE >> 8) & 0xFF);

    if(Regions_mpu == 0)
    {
        ret = MPU_RET_NOT_SUPPORT;
    }
    
    /* Disable MPU */
    MPU_Disable();
    
    /* Initial MPU config */
    for(int i=0; i<total_region; i++)
    {
        MPU_Config_Region(pTable + i);
    }
    
    /* Enable MPU */
    MPU_Enable();
    
    return ret;
}


/**
 *  \brief: Return driver version.
 */
uint32_t MPU_GetVersion(void)
{
    return MPU_VERSION;
}

