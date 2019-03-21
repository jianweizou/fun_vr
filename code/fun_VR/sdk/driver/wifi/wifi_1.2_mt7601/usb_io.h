#ifndef __USB_IO_H__
#define __USB_IO_H__

#include "types.h"
#include "rtmp_if.h"

#define MAX_VENDOR_REQ_RETRY_COUNT  		10
#define MAX_BUSY_COUNT 						10
#define DEVICE_VENDOR_REQUEST_OUT       	0x40
#define DEVICE_VENDOR_REQUEST_IN        	0xc0
#define USBD_TRANSFER_DIRECTION_OUT			0
#define USBD_TRANSFER_DIRECTION_IN			0
#define USBD_SHORT_TRANSFER_OK				0


/* FCE Related */
#define TX_CPU_PORT_FROM_FCE_BASE_PTR		0x09A0
#define TX_CPU_PORT_FROM_FCE_MAX_COUNT		0x09A4
#define TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX 0x09A8
#define FCE_PDMA_GLOBAL_CONF				0x09C4
#define FCE_PSE_CTRL						0x0800
#define FCE_CSO								0x0808
#define FCE_L2_STUFF						0x080c
#define FCE_SKIP_FS							0x0A6C
#define PER_PORT_PAUSE_ENABLE_CONTROL1		0x0A38



#define RTMP_IO_READ32(_A, _R, _pV)					RTUSB_ReadMACRegister((_A), (_R), (PUINT32) (_pV))
#define RTMP_IO_WRITE32(_A, _R, _V)					RTUSB_WriteMACRegister((_A), (_R), (UINT32) (_V), FALSE)

#define rtmp_ee_efuse_read16(_a, _b, _c) 			eFuseReadRegisters(_a, _b, 2, (PUSHORT)&(_c))
#define RT28xx_EEPROM_READ16	 					rtmp_ee_efuse_read16



NTSTATUS RTUSB_ReadMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUINT32			pValue);


NTSTATUS RTUSB_WriteMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN 	BOOLEAN 		bWriteHigh);


NTSTATUS RTUSB_SingleWrite(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	USHORT			Offset,
	IN	USHORT			Value,
	IN	BOOLEAN			WriteHigh);


NTSTATUS RTUSB_MultiWrite(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length,
	IN	BOOLEAN			bWriteHigh);


NTSTATUS RTUSB_VendorRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT32			TransferFlags,
	IN	UCHAR			RequestType,
	IN	UCHAR			Request,
	IN	USHORT			Value,
	IN	USHORT			Index,
	IN	PVOID			TransferBuffer,
	IN	UINT32			TransferBufferLength);


NTSTATUS RTUSB_VenderReset(
	IN	PRTMP_ADAPTER	pAd);


NTSTATUS RTUSB_WakeUp(
	IN	PRTMP_ADAPTER	pAd);


NTSTATUS RTUSB_EPOUThndl(			/* replace with APP_EPOUThandler */
	IN	PRTMP_ADAPTER 	pAd, 
	IN	UINT8 			ep, 
	IN	PUINT8 			add, 
	IN	UINT32 			len);


NTSTATUS RTUSB_BULKINhndl(			/* replace with APP_BULKIN */
	IN	PRTMP_ADAPTER 	pAd, 
	IN	UINT8 			ep,
	OUT	PUINT8 			add, 
	IN	UINT32 			len);


NTSTATUS RTUSB_ClearBULKINBuffer(
	IN PRTMP_ADAPTER 	pAd);


NDIS_STATUS RTUSB_LoadIVB(
	IN PRTMP_ADAPTER 	pAd);


NDIS_STATUS RTUSB_LoadFirmwareToAndes(
	IN PRTMP_ADAPTER 	pAd);


VOID RTUSB_IRQHandler(
	IN PRTMP_ADAPTER 	pAd);

UCHAR eFuseReadRegisters(
	IN	PRTMP_ADAPTER	pAd, 
	IN	USHORT 			Offset, 
	IN	USHORT 			Length, 
	OUT	PUSHORT 		pData);

#endif /*__USB_IO_H__ */
