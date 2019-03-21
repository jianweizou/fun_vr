//#include "typedef.h"
#include <stdio.h>
#include "usb_io.h"
#include "rtmp.h"
#include "USBH.h"
//#include "USBH-CORE.h"
#include "common.h"
#include "rtmp_andes.h"
#include "system.h"
#include "MT7601/MT7601_FW.h"
#include "rx.h"

extern USBH_Device_Structure *DEV;
UCHAR CXBusy = 0;
UCHAR EPOUTBusy = 0, EPINBusy = 0;
static UCHAR failCount = 0;
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
extern AUTO_BKIN_STRUCTURE get_auto_bkin_struct(void);
#endif
extern uint8_t usbh_cx_xfr(USBH_Device_Structure *DEV, USBH_CX_XFR_REQ_Struct *CX_REQ); 
extern uint8_t usbh_bk_xfr(USBH_Device_Structure *DEV, USBH_BK_XFR_REQ_Struct *BK_REQ);
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
extern void set_auto_bkin_processed_cnt(int xfr_cnt);
#endif

NTSTATUS RTUSB_ReadMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUINT32			pValue)
{
	NTSTATUS	Status = 0;
	UINT32		localVal;

	Status = RTUSB_VendorRequest(
								pAd,
								(USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK),
								DEVICE_VENDOR_REQUEST_IN,
								0x7,
								0,
								Offset,
								&localVal,
								4);
	
	*pValue = le2cpu32(localVal);


	if (Status < 0)
		*pValue = 0xffffffff;

	DBGPRINT(RT_LOUD, ("[LOUD]MAC Read %X = %X", Offset, *pValue));
	return Status;
}


NTSTATUS RTUSB_WriteMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN 	BOOLEAN 		bWriteHigh)
{
	NTSTATUS Status;
	UINT32 localVal;

	if (Offset == RX_FILTR_CFG)
		DBGPRINT(RT_LOUD, ("[LOUD]%s: FTR = 0x%X \r\n", __FUNCTION__, Value));
	
	localVal = Value;
	Status = RTUSB_SingleWrite(pAd, Offset, (USHORT)(localVal & 0xffff), bWriteHigh);
	Status = RTUSB_SingleWrite(pAd, Offset + 2, (USHORT)((localVal & 0xffff0000) >> 16), bWriteHigh);

	DBGPRINT(RT_LOUD, ("[LOUD]MAC Write %X = %X", Offset, Value));
	return Status;
}


NTSTATUS RTUSB_SingleWrite(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	USHORT			Offset,
	IN	USHORT			Value,
	IN	BOOLEAN			WriteHigh)
{
	NTSTATUS	Status;

	Status = RTUSB_VendorRequest(
								pAd,
								USBD_TRANSFER_DIRECTION_OUT,
								DEVICE_VENDOR_REQUEST_OUT,
								(WriteHigh == TRUE) ? 0x10 : 0x2,
								Value,
								Offset,
								NULL,
								0);
	return Status;
}


NTSTATUS RTUSB_MultiWrite(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length,
	IN	BOOLEAN			bWriteHigh)
{
	NTSTATUS	Status;
	USHORT		Value;
	USHORT 		index 	= 0;
	USHORT    	resude 	= 0;
	PUCHAR    	pSrc 	= pData;

	resude = length % 2;
	length  += resude;
	
	do
	{
		Value =(USHORT)( *pSrc  | (*(pSrc + 1) << 8));
		Status = RTUSB_SingleWrite(pAd,Offset + index, Value, bWriteHigh);
		index +=2;
		length -= 2;
		pSrc = pSrc + 2;
    } while (length > 0);

	return Status;
}


NTSTATUS RTUSB_VendorRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT32			TransferFlags,
	IN	UCHAR			RequestType,
	IN	UCHAR			Request,
	IN	USHORT			Value,
	IN	USHORT			Index,
	IN	PVOID			TransferBuffer,
	IN	UINT32			TransferBufferLength)	
{
	NTSTATUS  STATUS;
	PUCHAR  pSETUP_DATA_BUFF = NULL;
	USBH_CX_XFR_REQ_Struct	CX_REQ;

	if (pAd->USBPlugOut)
		return NDIS_FAILURE;

	while(CXBusy) 	vTaskDelay(1);

	CXBusy = 1;

	NdisZeroMemory(&CX_REQ, sizeof(CX_REQ));

	if (TransferBufferLength)
	{

		pSETUP_DATA_BUFF = pvPortMalloc(TransferBufferLength+16);
		if (pSETUP_DATA_BUFF == NULL) 
		{	
			DBGPRINT(RT_ERROR, ("[ERR]:%s: SETUP_DATA_BUFF Allocate fail...\r\n", __FUNCTION__));			
		  	return NDIS_FAILURE;
		}
	}

	CX_REQ.pBUFF = (PUINT32)(pSETUP_DATA_BUFF) ;
	CX_REQ.CLASS[0] = RequestType;//0x80;
	CX_REQ.CLASS[1] = Request;
	CX_REQ.CLASS[2] = Value & 0xFF;
	CX_REQ.CLASS[3] = (Value >> 8) & 0xFF;
	CX_REQ.CLASS[4] = Index & 0xFF;
	CX_REQ.CLASS[5] = (Index >> 8)& 0xFF;
	CX_REQ.CLASS[6] = TransferBufferLength & 0xFF;
	CX_REQ.CLASS[7] = (TransferBufferLength >> 8) & 0xFF;
	
	CX_REQ.wValue = Value;
	CX_REQ.wIndex = Index;
	CX_REQ.SIZE = TransferBufferLength;
			
	if (RequestType == DEVICE_VENDOR_REQUEST_IN)
	{	
		CX_REQ.CMD = USBH_CX_CMD_CLASS_Read;
	}
	else if (TransferBufferLength == 0)
	{
		CX_REQ.CMD = USBH_CX_CMD_CLASS_NoneData;
	}
	else
	{
		CX_REQ.CMD = USBH_CX_CMD_CLASS_Write;
		NdisMoveMemory(pSETUP_DATA_BUFF, TransferBuffer,	TransferBufferLength);
	}
	
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if (STATUS){
		DBGPRINT(RT_ERROR, ("[ERR]%s: STATUS %d, ENUM FAIL,GOING TO ERROR HANDLING %d\r\n", 
							__FUNCTION__, STATUS, TransferBufferLength));
		if (STATUS == NDIS_PLUG_OUT)
			pAd->USBPlugOut = TRUE;
	}
	else if ((STATUS == NDIS_SUCCESS)&& (RequestType == DEVICE_VENDOR_REQUEST_IN)) 
		NdisMoveMemory(TransferBuffer, (PUCHAR)pSETUP_DATA_BUFF, TransferBufferLength);

	if (pSETUP_DATA_BUFF)
		vPortFree(pSETUP_DATA_BUFF);

	CXBusy = 0;

	return STATUS;
}

NTSTATUS RTUSB_VenderReset(
	IN	PRTMP_ADAPTER	pAd)
{
	NTSTATUS	Status;
 
	Status = RTUSB_VendorRequest(
								pAd,
								USBD_TRANSFER_DIRECTION_OUT,
								DEVICE_VENDOR_REQUEST_OUT,
								0x01,
								0x1,
								0,
								NULL,
								0);

	DBGPRINT(RT_TRACE, ("[TRACE]<---%s\r\n", __FUNCTION__));
	return Status;
}


NTSTATUS RTUSB_EPOUThndl(			/* replace with APP_EPOUThandler */
	IN	PRTMP_ADAPTER 	pAd, 
	IN	UINT8 			ep, 
	IN	PUINT8 			add, 
	IN	UINT32 			len)
{
	NTSTATUS  				STATUS;
	USBH_BK_XFR_REQ_Struct	BK_REQ;
	UINT32 					val = 0;

	if (pAd->USBPlugOut)
		return NDIS_FAILURE;

	while (EPOUTBusy) vTaskDelay(1);

	EPOUTBusy = 1;
	
	NdisZeroMemory((PUCHAR)&BK_REQ, sizeof(USBH_BK_XFR_REQ_Struct));

	BK_REQ.pBUFF		=	(uint32_t *)(((uint32_t)add) + val);
	BK_REQ.XfrType		=	EHCI_BK_OUT_TYPE;
	BK_REQ.NUM			=	(ep == 8)? 1 : 2;
	BK_REQ.SIZE 		=	len;

	STATUS = usbh_bk_xfr((USBH_Device_Structure*)DEV,(USBH_BK_XFR_REQ_Struct*)&BK_REQ);
	if (STATUS != NDIS_SUCCESS)
	{
		if (STATUS == NDIS_PLUG_OUT)
			pAd->USBPlugOut = TRUE;

		RTMP_IO_READ32(pAd, TX_STA_CNT0, &val);
		RTMP_IO_READ32(pAd, TX_STA_CNT1, &val);
		RTMP_IO_READ32(pAd, TX_STA_CNT2, &val);
		
		DBGPRINT(RT_ERROR, ("[ERR]%s: BULK Out TIME OUT STATUS=%d	ERROR=%d FAIL.  Retry=%d   Fail=%d\r\n", 
							__FUNCTION__, STATUS, BK_REQ.SIZE, BK_REQ.ACT_SIZE, failCount));

		if( failCount++ >3)
		{
			DBGPRINT(RT_ERROR, ("\nSystem Reboot from  APP_EPOUThandler\n\n\r\n"));
//            ResetClrRemap();             
			NVIC_SystemReset();
			while(1);
		}

		

//		if (bulk_timeout_cb)
//			bulk_timeout_cb();
	} else failCount = 0;
	
	EPOUTBusy = 0;

	return STATUS;
}


NDIS_STATUS RTUSB_BULKINhndl(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			ep, 
	OUT PUCHAR 			add, 
	IN UINT32 			len)
{	
	NDIS_STATUS  			STATUS;
	PUINT 					pBULK_DATA_BUFF;
	USBH_BK_XFR_REQ_Struct	BK_REQ;

	if (pAd->USBPlugOut)
		return NDIS_FAILURE;

	while(EPINBusy) vTaskDelay(1);

	EPINBusy = 1;

	pBULK_DATA_BUFF = pvPortMalloc(1600);
	if (pBULK_DATA_BUFF == NULL) 
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s:BULK_IN_DATA_BUFF Allocate fail...!!\r\n", __FUNCTION__));		
		return NDIS_FAILURE;
	}

	BK_REQ.XfrType		=	EHCI_BK_IN_TYPE;
	BK_REQ.NUM			=	(ep == 4) ? 1:2 ;
	BK_REQ.SIZE 		=	len;
	BK_REQ.pBUFF		=	pBULK_DATA_BUFF;
	
	STATUS = usbh_bk_xfr((USBH_Device_Structure*)DEV, (USBH_BK_XFR_REQ_Struct*)&BK_REQ);
	if (STATUS != NDIS_SUCCESS) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: STATUS: %d BULK IN FAIL,GOING TO ERROR HANDLING\r\n", __FUNCTION__, STATUS));
		if (STATUS == NDIS_PLUG_OUT)
			pAd->USBPlugOut = TRUE;
	}
	else NdisMoveMemory(add, pBULK_DATA_BUFF, len);
	
	vPortFree(pBULK_DATA_BUFF);

	EPINBusy = 0;

	return STATUS;
}

NTSTATUS RTUSB_ClearBULKINBuffer(
	IN PRTMP_ADAPTER 	pAd)
{
	AUTO_BKIN_STRUCTURE AUTO_BK_IN;

	if (!pAd) {
		DBGPRINT(RT_ERROR, ("[ERR]%s: Wifi doesn't initialization!!\r\n", __FUNCTION__));
		return NDIS_FAILURE;
	}

	if (DEV == NULL) {
		DBGPRINT(RT_WARN, ("[WARN]%s: USB device doesn't initialization!!\r\n", __FUNCTION__));
		return NDIS_FAILURE;
	}

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);
	
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
	AUTO_BK_IN = get_auto_bkin_struct();
#endif
	pAd->RxUSBDataPtr = (PUCHAR) AUTO_BK_IN.start_addr;
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
	set_auto_bkin_processed_cnt(AUTO_BK_IN.xfr_cnt);
#endif
	
	return NDIS_SUCCESS;
}

NDIS_STATUS RTUSB_LoadIVB(
	IN PRTMP_ADAPTER 	pAd)
{
	NDIS_STATUS Status = NDIS_SUCCESS;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	Status = RTUSB_VendorRequest(pAd,
								 USBD_TRANSFER_DIRECTION_OUT,
								 DEVICE_VENDOR_REQUEST_OUT,
								 0x01,
								 0x12,
								 0x00,
								 pChipCap->FWImageName + 32,
								 64);
	if (Status)
	{
		DBGPRINT(RT_ERROR, ("[ERR]%s: Upload IVB Fail\r\n", __FUNCTION__));
		return Status;
	}

	RTMPusecDelay(1);

	vTaskDelay(10);	
	return Status;
}



NDIS_STATUS RTUSB_LoadFirmwareToAndes(
	IN PRTMP_ADAPTER 	pAd)
{
	NDIS_STATUS Status = NDIS_SUCCESS;
	PUCHAR DataBuffer;
	TXINFO_NMAC_CMD *TxInfoCmd;	
	INT32 SentLen;
	UINT32 CurLen;
	UINT32 MACValue, Loop = 0;
	USHORT Value;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	UINT32 ILMLen=0, DLMLen=0;

    DataBuffer = (PUCHAR)pvPortMalloc(15360);
	if (DataBuffer == NULL)
	{
		DBGPRINT(RT_ERROR,("[ERR]%s: DownLoad FW malloc Error\r\n", __FUNCTION__));
		return NDIS_FAILURE;
	}
	pChipCap->FWImageName= (PUCHAR)DP0_MT7601_FW;
	memcpy((void*)DataBuffer,(void*)pChipCap->FWImageName,20);
	
	if (pChipCap->IsComboChip)
	{
loadfw_protect:
		RTUSB_ReadMACRegister(pAd, SEMAPHORE_00, &MACValue);
		Loop++;

		if (((MACValue & 0x01) == 0) && (Loop < 10000))
			goto loadfw_protect;
	}

	
	Status = RTUSB_WriteMACRegister(pAd, 0x80, 0xFF200003, FALSE);
	
	/* Enable USB_DMA_CFG */
	Status = RTUSB_WriteMACRegister(pAd, USB_DMA_CFG, 0xC00000, FALSE);

	/* Check MCU if ready */
	Status = RTUSB_ReadMACRegister(pAd, COM_REG0, &MACValue);

	if (MACValue == 0x01)
		goto error0;

	RTMP_IO_WRITE32(pAd, 0x94c, 0x0);
	RTMP_IO_WRITE32(pAd, 0x800, 0x0);

	RTUSB_VenderReset(pAd);

	RTMPusecDelay(5);
	
	ILMLen = (*(DataBuffer + 3) << 24) | (*(DataBuffer + 2) << 16) |
			 (*(DataBuffer + 1) << 8) | (*DataBuffer);

	DLMLen = (*(DataBuffer + 7) << 24) | (*(DataBuffer + 6) << 16) |
			 (*(DataBuffer + 5) << 8) | (*(DataBuffer + 4));

	RTMP_IO_WRITE32(pAd, 0xa44, 0x0);
	RTMP_IO_WRITE32(pAd, 0x230, 0x84210);
	RTMP_IO_WRITE32(pAd, 0x400, 0x80c00);
	RTMP_IO_WRITE32(pAd, 0x800, 0x01);
	RTMP_IO_READ32(pAd, 0x0404, &MACValue);
	MACValue |= 0xF;
	RTMP_IO_WRITE32(pAd, 0x0404, MACValue);

	/* Enable FCE */
	RTMP_IO_WRITE32(pAd, FCE_PSE_CTRL, 0x01);

	/* Enable USB_DMA_CFG */
	RTMP_IO_WRITE32(pAd, USB_DMA_CFG, 0xC00000);

#ifdef MT7601
	if ( IS_MT7601(pAd) )
	{
		USB_DMA_CFG_STRUC UsbCfg;

		RTMP_IO_READ32(pAd, USB_DMA_CFG, &UsbCfg.word);
		UsbCfg.field.TxClear = 1;
		RTMP_IO_WRITE32(pAd, USB_DMA_CFG, UsbCfg.word);
		UsbCfg.field.TxClear = 0;
		RTMP_IO_WRITE32(pAd, USB_DMA_CFG, UsbCfg.word);
	}
#endif /* MT7601 */

	/* FCE tx_fs_base_ptr */
	RTMP_IO_WRITE32(pAd, TX_CPU_PORT_FROM_FCE_BASE_PTR, 0x400230);

	/* FCE tx_fs_max_cnt */
	RTMP_IO_WRITE32(pAd, TX_CPU_PORT_FROM_FCE_MAX_COUNT, 0x01); 

	/* FCE pdma enable */
	RTMP_IO_WRITE32(pAd, FCE_PDMA_GLOBAL_CONF, 0x44);  

	/* FCE skip_fs_en */
	RTMP_IO_WRITE32(pAd, FCE_SKIP_FS, 0x03);

	CurLen = 0x40; 

	/* Loading ILM */
	while (1)
	{
		SentLen = (ILMLen - CurLen) >= 14336 ? 14336 : (ILMLen - CurLen);

		if (SentLen > 0)
		{
			memset(DataBuffer, 0x0,  sizeof(*TxInfoCmd));
			TxInfoCmd = (TXINFO_NMAC_CMD *)DataBuffer;
			TxInfoCmd->info_type = CMD_PACKET;
			TxInfoCmd->pkt_len = SentLen;
			TxInfoCmd->d_port = CPU_TX_PORT;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)TxInfoCmd, TYPE_TXINFO);
#endif
			memcpy((void*)(DataBuffer+ sizeof(*TxInfoCmd)),(void*)(pChipCap->FWImageName+32 +CurLen),SentLen);
			memset(DataBuffer + sizeof(*TxInfoCmd) + SentLen, 0x0, 4);

			Value = CurLen & 0xFFFF;		
			
			/* Set FCE DMA descriptor */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x230,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_ERROR, ("%d: Set FCE DMA descriptor fail\r\n", __LINE__));
				goto error2;
			}
			
			Value = ((CurLen & 0xFFFF0000) >> 16);

			/* Set FCE DMA descriptor */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x232,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_ERROR, ("%d: Set FCE DMA descriptor fail\r\n", __LINE__));
				goto error2;
			}

			CurLen += SentLen;

			while ((SentLen % 4) != 0)
				SentLen++;

			Value = ((SentLen << 16) & 0xFFFF);

			/* Set FCE DMA length */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x234,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_ERROR, ("%d: Set FCE DMA length fail\r\n", __LINE__));
				goto error2;
			}
			
			Value = (((SentLen << 16) & 0xFFFF0000) >> 16);

			/* Set FCE DMA length */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x236,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_ERROR, ("%d: Set FCE DMA length fail\r\n", __LINE__));
				goto error2;
			}
			
			RTUSB_EPOUThndl(pAd, 4, DataBuffer,  SentLen + sizeof(*TxInfoCmd) + 4);
			
			RTMP_IO_READ32(pAd, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, &MACValue);
			MACValue++;
			RTMP_IO_WRITE32(pAd, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, MACValue);
		}
		else
		{
			break;
		}		

		/* Check if DMA done */
		Loop = 0;
		do
		{
			RTMP_IO_READ32(pAd, COM_REG1, &MACValue);
			if (MACValue & 0x80000000)			// DDONE 0x400234, bit[31]
				break;
			Loop++;
			RTMPusecDelay(5);
		} while (Loop <= 100);
		
	}

	CurLen = 0x00;

	/* Loading DLM */
	while (1)
	{
		SentLen = (DLMLen - CurLen) >= 14336 ? 14336 : (DLMLen - CurLen);

		if (SentLen > 0)
		{
			TxInfoCmd = (TXINFO_NMAC_CMD *)DataBuffer;
			TxInfoCmd->info_type = CMD_PACKET;
			TxInfoCmd->pkt_len = SentLen;
			TxInfoCmd->d_port = CPU_TX_PORT;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)TxInfoCmd, TYPE_TXINFO);
#endif
			memcpy((void*)(DataBuffer+ sizeof(*TxInfoCmd)),(void*)(pChipCap->FWImageName+32+ILMLen +CurLen),SentLen);
			memset(DataBuffer + sizeof(*TxInfoCmd) + SentLen + 4, 0x0, 4);

			Value = ((CurLen + 0x80000) & 0xFFFF);

			/* Set FCE DMA descriptor */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x230,
										 NULL,
										 0);


			if (Status)
			{
				DBGPRINT(RT_ERROR, ("(%d)Set FCE DMA descriptor fail\r\n", __LINE__));
				goto error2;
			}
			
			Value = (((CurLen + 0x80000) & 0xFFFF0000) >> 16);

			/* Set FCE DMA descriptor */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x232,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_ERROR, ("(%d)Set FCE DMA descriptor fail\r\n", __LINE__));
				goto error2;
			}			

			CurLen += SentLen;

			while ((SentLen % 4) != 0)
				SentLen++;

			Value = ((SentLen << 16) & 0xFFFF);

			/* Set FCE DMA length */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x234,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_ERROR, ("(%d)Set FCE DMA length fail\r\n", __LINE__));
				goto error2;
			}
			
			Value = (((SentLen << 16) & 0xFFFF0000) >> 16);

			/* Set FCE DMA length */
			Status = RTUSB_VendorRequest(pAd,
										 USBD_TRANSFER_DIRECTION_OUT,
										 DEVICE_VENDOR_REQUEST_OUT,
										 0x42,
										 Value,
										 0x236,
										 NULL,
										 0);

			if (Status)
			{
				DBGPRINT(RT_ERROR, ("(%d)Set FCE DMA length fail\r\n", __LINE__));
				goto error2;
			}

			RTUSB_EPOUThndl(pAd, 4, DataBuffer,  SentLen + sizeof(*TxInfoCmd) + 4);
			RTUSB_ReadMACRegister(pAd, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, &MACValue);
			MACValue++;
			RTUSB_WriteMACRegister(pAd, TX_CPU_PORT_FROM_FCE_CPU_DESC_INDEX, MACValue, FALSE);
		}
		else
		{
			break;
		}		

		RTMPusecDelay(5);
	}

	/* Upload new 64 bytes interrupt vector */
	Status = RTUSB_LoadIVB(pAd);
	
	/* Check MCU if ready */
	Loop = 0;
	do
	{
		RTMP_IO_READ32(pAd, COM_REG0, &MACValue);
		if (MACValue == 0x1)
			break;
		RTMPusecDelay(10);
		Loop++;
	} while (Loop <= 100);

	DBGPRINT(RT_TRACE, ("%s: COM_REG0(0x%x) = 0x%x\r\n", __FUNCTION__, COM_REG0, MACValue));

	if (MACValue != 0x1)
		Status = NDIS_FAILURE;
	
error2:
error0: 	
	if (pChipCap->IsComboChip)
		RTUSB_WriteMACRegister(pAd, SEMAPHORE_00, 0x1, FALSE);
	
	vPortFree(DataBuffer);
	return Status;
}

VOID RTUSB_IRQHandler(
	IN PRTMP_ADAPTER 	pAd)
{
	int i, frm_cnt;
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
	AUTO_BKIN_STRUCTURE AUTO_BK_IN;

	AUTO_BK_IN = get_auto_bkin_struct();
#endif
	pAd->RxUSBDataPtr = (PUCHAR) AUTO_BK_IN.start_addr;
	frm_cnt = AUTO_BK_IN.xfr_cnt;

	if (AUTO_BK_IN.buff_full)
		DBGPRINT(RT_WARN, ("[WARN]%s: AUTO BULK IN BUFF FULL: %d\r\n", __FUNCTION__, frm_cnt));


	while (frm_cnt > 0)
	{
		for (i = 0; i < frm_cnt; i++)
		{
			RxDoneInterruptHandle(pAd);
#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
			set_auto_bkin_processed_cnt(1);
#endif
			pAd->RxUSBDataPtr += PACKET_SIZE;
			if ((UINT32)pAd->RxUSBDataPtr >= AUTO_BK_IN.boundary_addr) 
				pAd->RxUSBDataPtr = (PUCHAR)AUTO_BK_IN.base_addr;
		}

#if defined (CONFIG_MODULE_USB_WIFI_CLASS)
		AUTO_BK_IN = get_auto_bkin_struct();
#endif
		pAd->RxUSBDataPtr = (PUCHAR) AUTO_BK_IN.start_addr;
		frm_cnt = AUTO_BK_IN.xfr_cnt;
		
		if (AUTO_BK_IN.buff_full)
			DBGPRINT(RT_WARN, ("[WARN]%s: AUTO BULK IN BUFF FULL: %d\r\n", __FUNCTION__, frm_cnt));
	}
}

UCHAR eFuseReadRegisters(
	IN	PRTMP_ADAPTER	pAd, 
	IN	USHORT 			Offset, 
	IN	USHORT 			Length, 
	OUT	PUSHORT 		pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	USHORT	efuseDataOffset;
	UINT32	data;
	UINT32 efuse_ctrl_reg = EFUSE_CTRL_3290;
	
	RTMP_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);

	/*Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	/*Use the eeprom logical address and covert to address to block number*/
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/*Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 0.*/
	eFuseCtrlStruc.field.EFSROM_MODE = 0;

	/*Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.*/
	eFuseCtrlStruc.field.EFSROM_KICK = 1;
	
	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, efuse_ctrl_reg, data);

	/*Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.*/
	i = 0;
	while (i < 500)
	{	
		/*rtmp.HwMemoryReadDword(EFUSE_CTRL, (DWORD *) &eFuseCtrlStruc, 4);*/
		RTMP_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);
		if (eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			break;
		}	
		RTMPusecDelay(2);
		i++;	
	}

	/*if EFSROM_AOUT is not found in physical address, write 0xffff*/
	if (eFuseCtrlStruc.field.EFSROM_AOUT == 0x3f)
	{
		for (i = 0; i < Length/2; i++)
			*(pData + 2*i) = 0xffff;
	}
	else
	{
		/*Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x590-0x59C)*/
		efuseDataOffset =  EFUSE_DATA0_3290 + (Offset & 0xC);
		/*In RTMP_IO_READ32 will automatically execute 32-bytes swapping*/
		RTMP_IO_READ32(pAd, efuseDataOffset, &data);
		/*Decide the upper 2 bytes or the bottom 2 bytes.*/
		/* Little-endian		S	|	S	Big-endian*/
		/* addr	3	2	1	0	|	0	1	2	3*/
		/* Ori-V	D	C	B	A	|	A	B	C	D*/
		/*After swapping*/
		/*		D	C	B	A	|	D	C	B	A*/
		/*Return 2-bytes*/
		/*The return byte statrs from S. Therefore, the little-endian will return BA, the Big-endian will return DC.*/
		/*For returning the bottom 2 bytes, the Big-endian should shift right 2-bytes.*/
#ifdef RT_BIG_ENDIAN
		data = data << (8*((Offset & 0x3)^0x2));		  
#else
		data = data >> (8*(Offset & 0x3));		
#endif /* RT_BIG_ENDIAN */
		
		NdisMoveMemory(pData, &data, Length);
	}

	return (UCHAR) eFuseCtrlStruc.field.EFSROM_AOUT;
}

