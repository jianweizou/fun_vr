/**
* @file
* this is usb device driver file
* snx_udc.c
* @author IP2/Erick
*/
#ifndef __SONIX_USBD_SNC7320_PLATFORM_H
#define __SONIX_USBD_SNC7320_PLATFORM_H

#include "sonix_config.h"

#include "FreeRTOS.h"

#define USBD_MEM_BASE_ADDR              0x30000000        
#define USBD_MEM_STATIC_ADDR            0

#define USBD_EP0_MAXPACKET_SIZE			64
#define USBD_EP1_MAXPACKET_SIZE			16
#define USBD_EP2_MAXPACKET_SIZE			0
#define USBD_EP3_MAXPACKET_SIZE			1024
#define USBD_EP4_MAXPACKET_SIZE			1024
#define USBD_EP5_MAXPACKET_SIZE			0
#define USBD_EP6_MAXPACKET_SIZE			0
#define USBD_EP7_MAXPACKET_SIZE			1024 * 3
#define USBD_EP8_MAXPACKET_SIZE			0
#define USBD_EP9_MAXPACKET_SIZE			0
#define USBD_EP10_MAXPACKET_SIZE		0
#define USBD_EP11_MAXPACKET_SIZE		0
#define USBD_EP12_MAXPACKET_SIZE		512
#define USBD_EP13_MAXPACKET_SIZE		512
#define USBD_EP14_MAXPACKET_SIZE		512
#define USBD_EP15_MAXPACKET_SIZE		512

void* usbd_malloc(size_t size);

#endif
