 /**
 *
 * this is usb device driver file
 * usb_device.h
 * author IP2/Luka
 */
#ifndef __USB_DEVICE_H
#define __USB_DEVICE_H

#include "sonix_config.h"
#include "snc_log_util.h"

#if defined(CONFIG_OS_FREERTOS)
    #include <FreeRTOS.h>
    #include <semphr.h>
#endif

#if defined(CONFIG_SN_GCC_SDK)
    #include <bsp.h>
    #include <task.h>
    #include <interrupt.h>
#endif

#include "stdio.h"

#define ERK_MOD 1

//#define USBD_PRINT(fmt, args...) printf
//#define USBD_PRINT(fmt, args...) printf("[USBD]"fmt, ##args)
#define USBD_PRINT(fmt, args...)

#ifdef __cplusplus
extern "C"{
#endif

typedef enum {
    USBD_MODE_MSC = 0,
    USBD_MODE_UVC,
    USBD_MODE_HID,
    USBD_MODE_UAC,
    USBD_MODE_UNKNOWN = 255,
} usbd_class_mode_t;

typedef enum {
       USBD_SUBMODE_NONE= 0,
    USBD_SUBMODE_BULK ,
    USBD_SUBMODE_ISO,
    USBD_SUBMODE_UNKNOWN = 255,
} usbd_class_submode_t;


typedef enum {
    USBD_POWER_OFF = 0,
    USBD_POWER_ON,
} usbd_power_mode_t;



#define USBD_DEF_CLASS_MODE                     USBD_MODE_MSC

#define USBD_PKG                                "USB_Device"
#define USBD_CFG_CLASS_MODE                     "class_mode"
#define USBD_CFG_HOTPLUG_MODE                   "hotplug_mode"
#define USBD_CFG_HOTPLUG_GPIO_NUM               "hotplug_gpio_num"
#define USBD_CFG_HOTPLUG_GPIO_TRIG_LEV          "hotplug_gpio_trig_lev"


#ifndef true
    #define true    1
#endif

#ifndef false
    #define false   0
#endif



/* USB device functions */
//void USBDClassDrvInit(void);
//void usbd_drv_task_init(void);
//void usbd_drv_task_uninit(void);
void USBDevice_init(void);
void USBDevice_uninit(void);

#if defined(CONFIG_SN_KEIL_SDK)
    #define outl(addr, value)           (*((volatile unsigned int  *)(addr)) = value)
    #define inl(addr)                   (*((volatile unsigned int *)(addr)))
    #define min(_a, _b)                 (_a <_b) ? _a :_b
    #define USBD_PLUGIN_DELAY_TIME      (300)
    #define USBD_PLUGOUT_DELAY_TIME     (1000)
#endif

#define ESUCCESS                0       /* success */
#define EDOM                    33      /* Math argument out of domain of func */
#define ENOMEM                  12      /* Out of Memory */
#define EINVAL                  22      /* Invalid argument */
#define ENOSPC                  28      /* No space left on device */
#define EOPNOTSUPP              95      /* Operation not supported on transport endpoint */
#define EBUSY                   16      /* Device or resource busy */
#define ENODEV                  19      /* No such device */
#define EALREADY                114     /* Operation already in progress */
#define EINPROGRESS             115     /* Operation now in progress */
#define ENOTSUPP                524     /* Operation is not supported */
#define EPROTO                  71      /* Protocol error */
#define ECONNABORTED            103     /* Software caused connection abort */
#define ESHUTDOWN               108     /* Cannot send after transport endpoint shutdown */
#define ECONNRESET              104     /* Connection reset by peer */

#define EIO                     5       /* I/O error */
#define EOVERFLOW               75      /* Value too large for defined data type */
#define EAGAIN                  11      /* Try again */

#define EINTR                   4       /* Interrupted system call */


#ifdef __cplusplus
}
#endif


#endif
