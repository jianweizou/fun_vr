 /**
 * Copyright 2018, SONIX Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of SONIX Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of SONIX Corporation.
 */
/** @file snc_core_mpu.h
 *
 * @author Joseph Wang
 * @version 0.1
 * @date 2018/04/11
 * @license
 * @description
 */

#ifndef __M3_CORE_MPU_H_80b97d5d_af71_4390_ab58_9e77fbf68e34__
#define __M3_CORE_MPU_H_80b97d5d_af71_4390_ab58_9e77fbf68e34__

#ifdef __cplusplus
extern "C" {
#endif

#include "snc_types.h"

//==============================================================================================
//                  MPU
//==============================================================================================
    
typedef enum mpu_return
{
    MPU_RET_SUCCESS,
    MPU_RET_FAIL,
    MPU_RET_INVAILD_INDEX,
    MPU_RET_INVAILD_SIZE,
    MPU_RET_INVAILD_ATTRIBUTE,
    MPU_RET_NOT_SUPPORT,
    
}mpu_return_t;

typedef enum mpu_region_idx
{
    MPU_REGION_0    ,
    MPU_REGION_1    ,
    MPU_REGION_2    ,
    MPU_REGION_3    ,
    MPU_REGION_4    ,
    MPU_REGION_5    ,
    MPU_REGION_6    ,
    MPU_REGION_7    ,

}mpu_region_idx_t;

typedef enum mpu_size
{
    MPU_REGION_SIZE_32B    = 4,
    MPU_REGION_SIZE_64B    ,
    MPU_REGION_SIZE_128B   ,
    MPU_REGION_SIZE_256B   ,
    MPU_REGION_SIZE_512B   ,
    
    MPU_REGION_SIZE_1K     ,
    MPU_REGION_SIZE_2K     ,
    MPU_REGION_SIZE_4K     ,
    MPU_REGION_SIZE_8K     ,
    MPU_REGION_SIZE_16K    ,
    MPU_REGION_SIZE_32K    ,
    MPU_REGION_SIZE_64K    ,
    MPU_REGION_SIZE_128K   ,
    MPU_REGION_SIZE_256K   ,
    MPU_REGION_SIZE_512K   ,
    
    MPU_REGION_SIZE_1M     ,
    MPU_REGION_SIZE_2M     ,
    MPU_REGION_SIZE_4M     ,
    MPU_REGION_SIZE_8M     ,
    MPU_REGION_SIZE_16M    ,
    MPU_REGION_SIZE_32M    ,
    MPU_REGION_SIZE_64M    ,
    MPU_REGION_SIZE_128M   ,
    MPU_REGION_SIZE_256M   ,
    MPU_REGION_SIZE_512M   ,
    
    MPU_REGION_SIZE_1G     ,
    MPU_REGION_SIZE_2G     ,
    MPU_REGION_SIZE_4G     ,
    
}mpu_size_t;

typedef enum mpu_attr_pri
{
    MPU_PRI_NONE    = 1,
    MPU_PRI_RO      ,
    MPU_PRI_RW      ,

}mpu_attr_pri_t;


typedef enum mpu_attr_unpri
{
    MPU_UNPRI_NONE  = 1,
    MPU_UNPRI_RO    ,
    MPU_UNPRI_RW    ,
    
}mpu_attr_unpri_t;


typedef struct mpu_region_set
{
    mpu_region_idx_t    index;
    uint32_t            start_addr;
    mpu_size_t          size;
    
    mpu_attr_pri_t      privilege_access;
    mpu_attr_unpri_t    unprivilege_access;
    
}mpu_region_set_t;


/* #. MPU Functions ------------------------------------------------------*/
/**
 *  \brief: MPU_Enable
 *  
 *  \return: values @mpu_return_t
 *  
 *  \details: Enable memory protect unit with regions of been activated.
 *  
 */
uint32_t MPU_Enable(void);


/**
 *  \brief: MPU_Disable
 *  
 *  \return: values @mpu_return_t
 *  
 *  \details: Disable memory protect unit.
 *  
 */
uint32_t MPU_Disable(void);


/**
 *  \brief: MPU_Config_Region
 *  
 *  \param [in] p: MPU region config structure @mpu_region_set_t
 *  \return: values @mpu_return_t
 *  
 *  \details: Config specific MPU region and activate the "region setting",
 *            each region can be assigned with activate/deactivate, 
 *            the function MPU_tnable() will only effects on activate regions.
 *  
 */
uint32_t MPU_Config_Region(mpu_region_set_t *p);


/**
 *  \brief: MPU_Deactivate_Region
 *  
 *  \param [in] region_num: number of MPU region
 *  \return: values @mpu_return_t
 *  
 *  \details: Deactivate specific region by it's number.
 *  
 */
uint32_t MPU_Deactivate_Region(uint32_t region_num);


/**
 *  \brief: MPU_Init
 *  
 *  \param [in] region_num: Number of MPU activate regions.
 *  \param [in] p: The pointer to an array of @mpu_region_set_t
 *  \return: values @mpu_return_t
 *  
 *  \details: Config and enable all the regions in array table.
 *  
 */
uint32_t MPU_Init(uint32_t region_num, mpu_region_set_t *p);

/**
 *  \brief: Return driver version.
 */
uint32_t MPU_GetVersion(void);



#ifdef __cplusplus
}
#endif

#endif
