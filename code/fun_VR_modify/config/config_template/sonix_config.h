#ifndef __sonix_config_H_wdfR99A5_lOAW_HG5a_swbf_uYkNdWroKhtZ__
#define __sonix_config_H_wdfR99A5_lOAW_HG5a_swbf_uYkNdWroKhtZ__

#ifdef __cplusplus
extern "C" {
#endif

// <<< Use Configuration Wizard in Context Menu >>>
//=============================================================================
//                      Version
//=============================================================================
#define CONFIG_VERSION      20180612
//=============================================================================
// <h>                  System 
//=============================================================================
 // <e> Enable Debug Port
// <i> CONFIG_DBG_PORT_UART0
#define CONFIG_ENABLE_DBG_PORT   1
#if (CONFIG_ENABLE_DBG_PORT)
    // <o> Uart Debug Port
    //    <0=> Uart0
    //    <1=> Uart1
    //    <2=> Disable Uart Debug Port
    #define CONFIG_UART_DBG_PORT    0
#endif
// </e>

// <e> Enable Customizing Core_1
// <i> CONFIG_CORE_1_DEVP_ENABLE
#define CONFIG_CORE_1_DEVP_ENABLE  0
#if (CONFIG_CORE_1_DEVP_ENABLE)
    //  <o> Core selection  <0=> Core_0
    //                      <1=> Core_1
    //  <i> CONFIG_CORE_SELECT
    //  <i> Default: Core 0

    #define CONFIG_CORE_SELECT  0

    #include "cores_share_info.h"
#endif
// </e>


// </h>

//=============================================================================
// <h>                 Platform
//=============================================================================
// <e> Enable FreeRTOS
// <i> CONFIG_OS_FREERTOS
#define CONFIG_OS_FREERTOS  1
#if (CONFIG_OS_FREERTOS)
    // <o> FreeRTOS Heap Size (Unit: K Byte)
    // <i> CONFIG_FREERTOS_HEAP_SIZE Default: 80 KBytes
    #define CONFIG_FREERTOS_HEAP_SIZE   80

#endif
// </e>
// </h>

//=============================================================================
// <h>                 Project Private Options
//=============================================================================

// </h>

// <<< end of configuration section >>>


#ifdef __cplusplus
}
#endif

#endif

