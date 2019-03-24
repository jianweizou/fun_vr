/**
* @file
* this is usb device driver file
* snx_udc.c
* @author IP2/Erick
*/
#include "sonix_config.h"

#include "FreeRTOS.h"
#include "snc7320_platform.h"

uint32_t USBD_STATIC_MEM_START_ADDR = USBD_MEM_BASE_ADDR;
uint32_t addr_old = 0, addr_next = 0;

inline void USBD_VariableInit(uint32_t start_addr)
{
    addr_old = start_addr;
    addr_next = start_addr;
}

void* usbd_malloc(size_t size)
{  
#if USBD_MEM_STATIC_ADDR == 1
 	uint8_t align_num = 4;
    
    USBD_VariableInit(USBD_STATIC_MEM_START_ADDR);
	addr_old = addr_next;
    if((addr_old % align_num) != 0)
        addr_old = ((addr_old / align_num) + 1) * align_num;
    addr_next = addr_old + size;
    return (void *)addr_old;      
#else 
    return pvPortMalloc(size); 
#endif
}
