/** \defgroup USBH_FUN USBH functions
 *  \ingroup USBH_MODULE
 *  \brief Control Transfer and Bulk Transfer API Introduction
*/


/**
 * @file
 * this	is  USBH-CORE file
 * USBH-CORE.c
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#include "sonix_config.h"

#if defined(CONFIG_USBH_FREE_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#endif

#if defined( CONFIG_SN_GCC_SDK )
#include <generated/snx_sdk_conf.h> 
#include <nonstdlib.h>
extern void flush_dcache_area(unsigned int start_addr, unsigned	int size); 
#endif 

#if defined( CONFIG_SN_KEIL_SDK )
#include <stdlib.h>
#endif 

#if defined( CONFIG_XILINX_SDK )
#include <stdlib.h>
#include <stdio.h>
#include <xil_cache.h>
#endif

#include <string.h>
#include "USBH.h"

//#include <cli/printlog.h>
// Global 
volatile USBH_Device_Structure USBH_DEV[USBH_MAX_PORT];
#if defined( CONFIG_DUAL_HOST )	
volatile USBH_Device_Structure USBH_DEV_2[USBH_MAX_PORT];
#endif // End of if defined( CONFIG_DUAL_HOST )

uint8_t	USBH_GETDESCRIPTOR_DEVICE[]						=	{0x80,0x06,0x00,0x01,0x00,0x00,0x00,0x00};
uint8_t	USBH_GETDESCRIPTOR_CONFIG[]						=	{0x80,0x06,0x00,0x02,0x00,0x00,0x00,0x00};
uint8_t	USBH_SETADDRESS[]											=	{0x00,0x05,0x01,0x00,0x00,0x00,0x00,0x00};
uint8_t	USBH_SETCONFIGURATION[]								=	{0x00,0x09,0x01,0x00,0x00,0x00,0x00,0x00};
uint8_t	USBH_GETDESCRIPTOR_STR_LANG[]					=	{0x80,0x06,0x00,0x03,0x00,0x00,0x00,0x00};
uint8_t	USBH_GETDESCRIPTOR_STR_Manufacturer[]	=	{0x80,0x06,0x00,0x03,0x00,0x00,0x00,0x00};
uint8_t	USBH_GETDESCRIPTOR_STR_Product[]			=	{0x80,0x06,0x00,0x03,0x00,0x00,0x00,0x00};
uint8_t	USBH_GETDESCRIPTOR_STR_Serial[]				=	{0x80,0x06,0x00,0x03,0x00,0x00,0x00,0x00};
uint8_t	USBH_GETSTATUS_DEVICE[]								=	{0x80,0x00,0x00,0x00,0x00,0x00,0x02,0x00};
uint8_t	USBH_GETSTATUS_INTERFACE[]						=	{0x81,0x00,0x00,0x00,0x00,0x00,0x02,0x00};
uint8_t	USBH_GETSTATUS_ENDPOINT[]							=	{0x82,0x00,0x00,0x00,0x00,0x00,0x02,0x00};
uint8_t	USBH_CLEAR_FEATURE[]									=	{0x02,0x01,0x00,0x00,0x81,0x00,0x00,0x00};
uint8_t	USBH_SET_INTERFACE[]									=	{0x01,0x0B,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t	USBH_HID_GETDESCRIPTOR_REPORT[]				=	{0x81,0x06,0x00,0x22,0x00,0x00,0x00,0x00};

#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
uint8_t usbh_reset_count = 0;
#endif

#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)
extern automount_info_usbh	auto_mount_info_usbh[2];
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
#else
	QueueHandle_t	USBH_QUEUE_STD_ISO_REQ = NULL;
#endif

	QueueHandle_t	USBH_QUEUE_BULK_REQ[10] = {NULL}; // USBH_MAX_PORT*2
	QueueHandle_t	USBH_QUEUE_BULK_REQ_DATA = NULL;
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
#else
	osMessageQId		USBH_QUEUE_STD_ISO_REQ = NULL;		
#endif
	osMessageQId	USBH_QUEUE_BULK_REQ[10] = {NULL};

	osMessageQId	USBH_QUEUE_BULK_REQ_DATA = NULL;
#endif
		
#if !defined (CONFIG_WIFI_SDK)
AUTO_BKIN_STRUCTURE AUTO_BK_IN;
#endif
// Porting EHCI	function
static inline void usbh_enable_xfr(void	*EP)
{
	ehci_enable_xfr((EHCI_ENDP_STRUCT *)EP);
}

static inline void usbh_disable_xfr(void *EP)
{
	ehci_disable_xfr((EHCI_ENDP_STRUCT*)EP);
}

static inline uint8_t usbh_chk_xfr_result(void *EP)
{
	return ehci_chk_xfr_result((EHCI_ENDP_STRUCT*)EP);
}

static inline void usbh_cx_struct_init(void *EP, USBH_CX_XFR_REQ_Struct	*CX_REQ)
{
	ehci_cx_struct_init((EHCI_ENDP_STRUCT*)EP,(EHCI_CX_XFR_REQ_Struct*)CX_REQ);
}

static inline void usbh_bk_struct_init(void* EP, USBH_BK_XFR_REQ_Struct	*BK_REQ)
{
	ehci_bk_struct_init((EHCI_ENDP_STRUCT*)EP,(EHCI_BK_XFR_REQ_Struct*)BK_REQ);
}

static inline void usbh_int_struct_init(void* EP, USBH_INT_XFR_REQ_Struct *INT_REQ)
{
	ehci_int_struct_init((EHCI_ENDP_STRUCT*)EP,(EHCI_INT_XFR_REQ_Struct*)INT_REQ);
}

void usbh_iso_struct_init(void* EP, USBH_ISO_XFR_REQ_Struct *ISO_REQ)
{
	ehci_iso_struct_init((EHCI_ENDP_STRUCT*)EP,(EHCI_ISO_XFR_REQ_Struct*)ISO_REQ);
}

static inline uint32_t usbh_get_cx_act_size(void *EP,USBH_CX_XFR_REQ_Struct *CX_REQ) {
	return ehci_get_cx_act_size((EHCI_ENDP_STRUCT*)EP,(EHCI_CX_XFR_REQ_Struct*)CX_REQ);
}

static inline uint32_t usbh_get_bk_act_size(void *EP,USBH_BK_XFR_REQ_Struct *BK_REQ) {
	return ehci_get_bk_act_size((EHCI_ENDP_STRUCT*)EP,(EHCI_BK_XFR_REQ_Struct*)BK_REQ);
}

extern uint8_t ehci_parser_dev(USBH_Device_Structure *DEV, uint8_t FUN);
uint8_t	usbh_parser_dev(USBH_Device_Structure *DEV, uint8_t FUN){
	return ehci_parser_dev(DEV, FUN);
}

void usbh_core_init(void)
{
	memset((uint8_t*)(&USBH_DEV[0]),0,USBH_MAX_PORT*sizeof(USBH_DEV[0]));// clear all device structure
}

static inline uint8_t usbh_parser_cfgdes(USBH_Device_Structure *DEV, uint8_t *pbData)
{
	uint8_t		i,j,k;
	uint32_t	desc_boundary;

	//<1>.Copy Configuration 1~2
	if (DEV->DEVDes.bCONFIGURATION_NUMBER > USBH_CONFIGURATION_NUM_MAX){
		USBH_INFO("??? Analysis	Configuration Fail(bCONFIGURATION_NUMBER>Max)...");
		while(1);
	}

	for	(i = 0;	i < DEV->DEVDes.bCONFIGURATION_NUMBER; i++) {
		memcpy(&(DEV->CFGDes[i]),pbData,USBH_CONFIGURATION_LENGTH);
		pbData=pbData+USBH_CONFIGURATION_LENGTH;

		//<2>.Copy Interface 1~5
		if (DEV->CFGDes[i].bINTERFACE_NUMBER> USBH_INTERFACE_NUM_MAX) {
			USBH_INFO("??? Analysis Interface Fail(bINTERFACE_NUMBER>Max)...");
			while(1);
		}
		if(*(pbData+1)	== USBH_INTERFACE){		// INTERFACE Descriptor	
			for(j = 0;j < DEV->CFGDes[i].bINTERFACE_NUMBER; j++) {
				memcpy(&(DEV->CFGDes[i].Interface[j]), pbData, USBH_INTERFACE_LENGTH);
				pbData = pbData	+ USBH_INTERFACE_LENGTH;
				//<3>.Copy Class HID
				if (DEV->CFGDes[i].Interface[j].bInterfaceClass	== 3) {	//Only support 1 class
					memcpy(&(DEV->CFGDes[i].Interface[j].CLASSDes[0]), pbData, *pbData);
					pbData = pbData + (*pbData);
				}
				//<4>.Copy Endpoint
				for (k = 0; k <	DEV->CFGDes[i].Interface[j].bEP_NUMBER;	k++) {
					memcpy(&(DEV->CFGDes[i].Interface[j].ENDPDes[k]), pbData, USBH_ENDPOINT_LENGTHX);
					pbData = pbData + USBH_ENDPOINT_LENGTHX;
				}//Endpoint
			}//Interface
		}else if(*(pbData+1) == USBH_INTERFACE_ASSOCIATION){	// INTERFACE_ASSOCIATION Descriptor Type 
			desc_boundary = (uint32_t)(pbData+(DEV->CFGDes->bTotalLengthHighByte<<8)+DEV->CFGDes->bTotalLengthLowByte)-USBH_CONFIGURATION_LENGTH;
			j=0;
			k=0;
			do{
				//if((*pbData == 0x09) && (*(pbData+1) == USBH_INTERFACE) && (*(pbData+2) == j)&& (*(pbData+4) != 0)){
				if((*pbData == 0x09) &&	(*(pbData+1) ==	USBH_INTERFACE)	&& (*(pbData+2)	== j)){
					j++;
					k=0;
					memcpy(&(DEV->CFGDes[i].Interface[j-1]), pbData, USBH_INTERFACE_LENGTH);
					pbData = pbData + USBH_INTERFACE_LENGTH;
				}else if(*pbData == 0x07){
					memcpy(&(DEV->CFGDes[i].Interface[j-1].ENDPDes[k]),	pbData,	USBH_ENDPOINT_LENGTHX);			
					pbData = pbData + USBH_ENDPOINT_LENGTHX;
					k++;
					DEV->CFGDes[i].Interface[j-1].bAlternateSetting = k;
					DEV->CFGDes[i].Interface[j-1].bEP_NUMBER = k;
				}else{
					pbData = pbData + (*pbData);
					if(*pbData == 0) break;
				}
			}while(((uint32_t)pbData) <((uint32_t)desc_boundary));
		}
	}//Configuration

	return (1);
}


static void usbh_dumpstring (uint8_t *pbTemp, uint8_t bSize)
{
	uint32_t i;
	for(i = 0;i < bSize; i = i + 2) {
		USBH_INFO("%c",*(pbTemp	+ i));
	}
	USBH_INFO("\r\n");
}


void usbh_print_device_info (USBH_Device_Structure *DEV)
{
	uint16_t hwVID,hwPID;
	uint32_t i,j,k,h;
	uint8_t *pbData;
	uint8_t bData,bMultSource,bMult;
	uint32_t wSize;

	//<1>.Print Device Information
	USBH_INFO("\r\n");
	USBH_INFO(  "******************************************************\r\n");
	USBH_INFO(  "************ Device Enumerate Information ************\r\n");
	USBH_INFO(  "******************************************************\r\n");

	switch(DEV->SPEED) {
		case 0:
			USBH_INFO("\r\n  >>> Device Speed: Full Speed...\r\n");
			break;
		case 1:
			USBH_INFO("\r\n  >>> Device Speed: Low Speed...\r\n");
			break;
		case 2:
			USBH_INFO("\r\n  >>> Device Speed: High Speed...\r\n");
			break;
	}
	hwVID = (DEV->DEVDes.bVIDHighByte);
	hwVID = (hwVID << 8) |(DEV->DEVDes.bVIDLowByte);
	hwPID = (DEV->DEVDes.bPIDHighByte);
	hwPID = (hwPID << 8) |(DEV->DEVDes.bPIDLowByte);

	USBH_INFO("  >>> VID=0x%x / PID=0x%x / USB Ver=0x%x%x / Ed0 MaxSize=%d\r\n", hwVID,	hwPID, DEV->DEVDes.bVerHighByte, DEV->DEVDes.bVerLowByte, DEV->DEVDes.bEP0MAXPACKETSIZE);
	USBH_INFO("  >>> Manufacturer:");
	usbh_dumpstring(&(DEV->bStringManufacture[2]), DEV->bStringManufacture[0]);

	USBH_INFO("  >>> Product:");
	usbh_dumpstring(&(DEV->bStringProduct[2]),DEV->bStringProduct[0]);

	USBH_INFO("  >>> Serial:");
	usbh_dumpstring(&(DEV->bStringSerialN[2]),DEV->bStringSerialN[0]);

	USBH_INFO("  >>>(Hex)");
	pbData = (uint8_t*)&(DEV->DEVDes);
	for (h = 0; h < 18; h++) {
		USBH_INFO(" %02x",*pbData++);
	}
	USBH_INFO("\r\n");

	//<2>.Configuration
	for (i = 0; i < DEV->DEVDes.bCONFIGURATION_NUMBER; i++) {
		pbData=(uint8_t*) &(DEV->CFGDes[i]);;
		USBH_INFO("  ********* Configuration[%d] = %d *********\r\n",i,DEV->CFGDes[i].bConfigurationValue);
		USBH_INFO("  >>>(Hex)");
		for (h = 0; h < USBH_CONFIGURATION_LENGTH; h++) {
			USBH_INFO("  %02x",*pbData++);
		}
		USBH_INFO("\r\n");
		//<3>.Interface
		for (j = 0; j < DEV->CFGDes[i].bINTERFACE_NUMBER; j++) {
			pbData = (uint8_t*)&(DEV->CFGDes[i].Interface[j]);
			USBH_INFO("     ****** Interface[%d] = %d ******\r\n", j, DEV->CFGDes[i].Interface[j].bInterfaceNumber);
			USBH_INFO("     >>>(Hex)");
			for (h = 0; h < USBH_INTERFACE_LENGTH; h++) {
				USBH_INFO("  %02x",*pbData++);
			}
			USBH_INFO("\r\n");
			//Print the Class Information
			if (DEV->CFGDes[i].Interface[j].bInterfaceClass == 3) { //Only support 1 class
				USBH_INFO("     >>>(Hex) HID Device ...\n");
				pbData = (uint8_t*)	&(DEV->CFGDes[i].Interface[j].CLASSDes[0]);
				USBH_INFO("     >>>");
				for (h = 0; h < DEV->CFGDes[i].Interface[j].CLASSDes[0].bClass_LENGTH; h++) {
					USBH_INFO("  %02x",*pbData++);
				}
				USBH_INFO("\r\n");
			}
			//<4>.Endpoint
			for (k = 0; k < DEV->CFGDes[i].Interface[j].bEP_NUMBER; k++) {
				pbData = (uint8_t*)	&(DEV->CFGDes[i].Interface[j].ENDPDes[k]);
				wSize = DEV->CFGDes[i].Interface[j].ENDPDes[k].bED_wMaxPacketSizeHighByte;
				wSize = ((wSize	<< 8) +	DEV->CFGDes[i].Interface[j].ENDPDes[k].bED_wMaxPacketSizeLowByte);
				bMultSource = wSize >> 11;
				wSize = wSize&0x7FF;

				USBH_INFO("       *** EndPoint[%d] = %d ***\r\n", k,(DEV->CFGDes[i].Interface[j].ENDPDes[k].bED_EndpointAddress) & 0x0F);

				switch (DEV->CFGDes[i].Interface[j].ENDPDes[k].bED_bmAttributes &0x03) {
					case USBH_ED_ISO:
						USBH_INFO("       >>> ISO");
						break;
					case USBH_ED_BULK:
						USBH_INFO("       >>> Bulk");
						break;
					case USBH_ED_INT:
						USBH_INFO("       >>> Interrupt");
						break;
					case USBH_ED_Control:
						USBH_INFO("       >>> Control");
						break;
					default:
						USBH_INFO("  ??? Data Error...\r\n");
				}

				bData = (DEV->CFGDes[i].Interface[j].ENDPDes[k].bED_EndpointAddress) & BIT7;
				if (bData > 0) {
					USBH_INFO("  -In");
				} else {
					USBH_INFO("  -Out");
				}
				switch(bMultSource) {
					case 1:
						bMult=2;
						break;
					case 2:
						bMult=3;
						break;
					default:
						bMult=1;
						break;
				}
				USBH_INFO("  (MaxSize=%d ,Interval=%d ,HighBandWidth=%d )\r\n"
						 , wSize, DEV->CFGDes[i].Interface[j].ENDPDes[k].bED_Interval, bMult);
				USBH_INFO("       >>>(Hex)");


				for (h = 0; h < USBH_ENDPOINT_LENGTHX; h++) {
					USBH_INFO("  %02x",*pbData++);
				}
				USBH_INFO("\r\n");
			}//Endpoint
		}//Interface
	}//Configuration
}
/** \defgroup USBH_FUN USBH functions
 * \ingroup USBH_MODULE
 * 
 * @{
 */

/** \fn	uint8_t	usbh_cx_xfr(USBH_Device_Structure *DEV,USBH_CX_XFR_REQ_Struct *CX_REQ)
 * \brief control transfer function
 * 
 * \param DEV target USB Device
 * 
  * \param CX_REQ control transfer request
 * 
 * \return status ,	SUCCESS	or  FAIL
 */
 /** @}	*/
uint8_t	usbh_cx_xfr(USBH_Device_Structure *DEV, USBH_CX_XFR_REQ_Struct *CX_REQ)
{
	uint32_t	EPNUM					= 0;
	uint8_t		status				= FAIL;
	uint8_t		*BUFF					= NULL;
	uint16_t	TimeoutMsec		= 0;
	uint32_t 	CXTimeout			= 0;
	
#ifdef CONFIG_MODULE_USBH_CX_TIMEOUT
	uint32_t 	ConfigTimeout	= 0;
#endif
		
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
	do{
		BUFF	=	pvPortMalloc(16,GFP_DMA, MODULE_DRI_USBH);
	}while(BUFF	== NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK ) 
	do{
		BUFF	=	pvPortMalloc(16);
	}while(BUFF	== NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK ) 
	BUFF = (uint8_t *)malloc(16);
#endif
#endif

	CX_REQ->SETUP_CMD_BUFF	=	BUFF;
	CX_REQ->STS_DUMMY_BUFF	=	BUFF+8;

	// Find CX QH 
	for(EPNUM=0;EPNUM<MAX_QH_PER_DEVICE;EPNUM++){
		if(DEV->EP[EPNUM].XfrType == USBH_CX_TYPE) {

			DEV->iEPNUM = EPNUM;

			break;
		}
	}

	if(EPNUM == MAX_QH_PER_DEVICE) {
		if(DEV->bDevIsConnected == 0) {
			USBH_DBG("CX XFR DEV_NOT_EXIST");
			return DEV_NOT_EXIST;
		} else {
			USBH_DBG("CX XFR FAIL");
			return FAIL;
		}
	}

	//Disable EP Xfr
	usbh_disable_xfr((void *)&DEV->EP[EPNUM]);

	//Set SETUP	DATA
	switch (CX_REQ->CMD){
		case USBH_CX_CMD_GETDESCRIPTOR_DEVICE:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_GETDESCRIPTOR_DEVICE,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		case USBH_CX_CMD_GETDESCRIPTOR_CONFIG:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_GETDESCRIPTOR_CONFIG,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		case USBH_CX_CMD_SETADDRESS:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_SETADDRESS,8);
			CX_REQ->SETUP_CMD_BUFF[2]	=	(uint8_t)CX_REQ->wValue;
			CX_REQ->SETUP_CMD_BUFF[3]	=	(uint8_t)(CX_REQ->wValue>>8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	0;
			CX_REQ->SETUP_CMD_BUFF[7]	=	0;
			CX_REQ->CX_Case = CX_NoneData;
			break;
		case USBH_CX_CMD_SETCONFIGURATION:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_SETCONFIGURATION,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	0;
			CX_REQ->SETUP_CMD_BUFF[7]	=	0;
			CX_REQ->CX_Case = CX_NoneData;
			break;
		case USBH_CX_GETDESCRIPTOR_STR_LANG:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_GETDESCRIPTOR_STR_LANG,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		case USBH_CX_CMD_GETDESCRIPTOR_STR_Manufacturer:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_GETDESCRIPTOR_STR_Manufacturer,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->SETUP_CMD_BUFF[2]	=	(uint8_t)(CX_REQ->wValue);
			CX_REQ->SETUP_CMD_BUFF[4]	=	(uint8_t)(CX_REQ->wIndex);
			CX_REQ->SETUP_CMD_BUFF[5]	=	(uint8_t)(CX_REQ->wIndex>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		case USBH_CX_CMD_GETDESCRIPTOR_STR_Product:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_GETDESCRIPTOR_STR_Product,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->SETUP_CMD_BUFF[2]	=	(uint8_t)(CX_REQ->wValue);
			CX_REQ->SETUP_CMD_BUFF[4]	=	(uint8_t)(CX_REQ->wIndex);
			CX_REQ->SETUP_CMD_BUFF[5]	=	(uint8_t)(CX_REQ->wIndex>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		case USBH_CX_CMD_GETDESCRIPTOR_STR_Serial:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_GETDESCRIPTOR_STR_Serial,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->SETUP_CMD_BUFF[2]	=	(uint8_t)(CX_REQ->wValue);
			CX_REQ->SETUP_CMD_BUFF[4]	=	(uint8_t)(CX_REQ->wIndex);
			CX_REQ->SETUP_CMD_BUFF[5]	=	(uint8_t)(CX_REQ->wIndex>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		case USBH_CX_CMD_GETSTATUS_DEVICE:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_GETSTATUS_DEVICE,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->SETUP_CMD_BUFF[2]	=	(uint8_t)(CX_REQ->wValue);
			CX_REQ->SETUP_CMD_BUFF[4]	=	(uint8_t)(CX_REQ->wIndex);
			CX_REQ->SETUP_CMD_BUFF[5]	=	(uint8_t)(CX_REQ->wIndex>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		case USBH_CX_CMD_GETSTATUS_INTERFACE:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_GETSTATUS_INTERFACE,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->SETUP_CMD_BUFF[2]	=	(uint8_t)(CX_REQ->wValue);
			CX_REQ->SETUP_CMD_BUFF[4]	=	(uint8_t)(CX_REQ->wIndex);
			CX_REQ->SETUP_CMD_BUFF[5]	=	(uint8_t)(CX_REQ->wIndex>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		case USBH_CX_CMD_GETSTATUS_ENDPOINT:
			memcpy(CX_REQ->SETUP_CMD_BUFF,USBH_GETSTATUS_ENDPOINT,8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->SETUP_CMD_BUFF[2]	=	(uint8_t)(CX_REQ->wValue);
			CX_REQ->SETUP_CMD_BUFF[4]	=	(uint8_t)(CX_REQ->wIndex);
			CX_REQ->SETUP_CMD_BUFF[5]	=	(uint8_t)(CX_REQ->wIndex>>8);
			CX_REQ->CX_Case = CX_Read;
			break; 
		case USBH_CX_CMD_CLEAR_FEATURE:
			USBH_CLEAR_FEATURE[4] = CX_REQ->wIndex;
			memcpy(CX_REQ->SETUP_CMD_BUFF, USBH_CLEAR_FEATURE, 8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	0;
			CX_REQ->SETUP_CMD_BUFF[7]	=	0;
			CX_REQ->CX_Case = CX_NoneData;
			break;
		case USBH_CX_CMD_SET_INTERFACE:
			memcpy(CX_REQ->SETUP_CMD_BUFF, USBH_SET_INTERFACE, 8);
			CX_REQ->SETUP_CMD_BUFF[2]	=	(uint8_t)(CX_REQ->wValue);
			CX_REQ->SETUP_CMD_BUFF[3]	=	(uint8_t)(CX_REQ->wValue>>8);
			CX_REQ->SETUP_CMD_BUFF[4]	=	(uint8_t)(CX_REQ->wIndex);
			CX_REQ->SETUP_CMD_BUFF[5]	=	(uint8_t)(CX_REQ->wIndex>>8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	0;
			CX_REQ->SETUP_CMD_BUFF[7]	=	0;
			CX_REQ->CX_Case = CX_NoneData;
			break;
		case USBH_CX_CMD_CLASS_Read:
			memcpy(CX_REQ->SETUP_CMD_BUFF, CX_REQ->CLASS, 8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		case USBH_CX_CMD_CLASS_Write:		
			memcpy(CX_REQ->SETUP_CMD_BUFF, CX_REQ->CLASS, 8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->CX_Case = CX_Write;
			break;
		case USBH_CX_CMD_CLASS_NoneData:	
			memcpy(CX_REQ->SETUP_CMD_BUFF, CX_REQ->CLASS, 8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	0;
			CX_REQ->SETUP_CMD_BUFF[7]	=	0;
			CX_REQ->CX_Case = CX_NoneData;
			break;	
		case USBH_HID_CX_CMD_GETDESCRIPTOR_REPORT:
			memcpy(CX_REQ->SETUP_CMD_BUFF, USBH_HID_GETDESCRIPTOR_REPORT, 8);
			CX_REQ->SETUP_CMD_BUFF[6]	=	(uint8_t)(CX_REQ->SIZE);
			CX_REQ->SETUP_CMD_BUFF[7]	=	(uint8_t)(CX_REQ->SIZE>>8);
			CX_REQ->CX_Case = CX_Read;
			break;
		default :
			USBH_DBG("CX Command not support");
			return FAIL;
	}

	// Init CX Structure
	usbh_cx_struct_init((void *)&DEV->EP[EPNUM],CX_REQ);

	// Flush
#if defined( CONFIG_SN_GCC_SDK )	
	flush_dcache_area((uint32_t)&BUFF[0], 16);
	if(CX_REQ->SIZE){
		flush_dcache_area((uint32_t)CX_REQ->pBUFF, CX_REQ->SIZE);
	}
#endif	

	// Enable XFR
	usbh_enable_xfr((void *)&DEV->EP[EPNUM]);

	// Wait for	XFR done
	
#ifdef CONFIG_MODULE_USBH_CX_TIMEOUT
	ConfigTimeout = CONFIG_MODULE_USBH_CX_TIMEOUT;
	if(ConfigTimeout == 10000000)
		CXTimeout = USBH_MAX;
	else
	{
#if defined (CONFIG_USBH_FREE_RTOS)
	CXTimeout = ConfigTimeout/portTICK_RATE_MS;
	//TimeoutMsec	=	(((CX_REQ->TimeoutMsec/portTICK_RATE_MS) > CXTimeout) ? CX_REQ->TimeoutMsec/portTICK_RATE_MS : CXTimeout);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	CXTimeout = ConfigTimeout;
#endif
	}
#else
	CXTimeout = USBH_CX_TIMEOUT;
#endif	//end of #ifdef CONFIG_MODULE_USBH_CX_TIMEOUT
	
#if defined (CONFIG_USBH_FREE_RTOS)
	TimeoutMsec	=	(((CX_REQ->TimeoutMsec/portTICK_RATE_MS) > CXTimeout) ? CX_REQ->TimeoutMsec/portTICK_RATE_MS : CXTimeout);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	TimeoutMsec	=	(((CX_REQ->TimeoutMsec) > CXTimeout) ? CX_REQ->TimeoutMsec : CXTimeout);
#endif
//#endif

#if defined (CONFIG_USBH_FREE_RTOS)
	if(xSemaphoreTake((SemaphoreHandle_t)DEV->EP[EPNUM].SEM, TimeoutMsec) == pdFALSE){
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	if(osSemaphoreWait(DEV->EP[EPNUM].SEM, TimeoutMsec) != osOK){
#endif
		// Disable XFR
		ehci_stop_xfr((void *)&DEV->EP[EPNUM]);

		if(DEV->bDevIsConnected == 0) {
			USBH_DBG("CX XFR _at_ QH:0x%x DEV_NOT_EXIST", (uint32_t )DEV->EP[EPNUM].pQH);
			status = DEV_NOT_EXIST;
		}
		else {
			USBH_DBG("CX TIMEOUT(%d) FAIL , _at_ QH:0x%x Fail", TimeoutMsec, (uint32_t)DEV->EP[EPNUM].pQH);
			DEV->EP[EPNUM].status = EHCI_QTD_STATUS_TIMEOUT;
			status	=	FAIL;
		}
	}else{
		if(DEV->bDevIsConnected == 0) {
			ehci_stop_xfr((void *)&DEV->EP[EPNUM]);
			USBH_DBG("CX XFR _at_ QH:0x%x DEVICE_NOT_EXIST",(uint32_t)DEV->EP[EPNUM].pQH);
			status	=	DEV_NOT_EXIST;
		}
		else {
			if (usbh_chk_xfr_result((void *) &DEV->EP[EPNUM]) == SUCCESS) {
				if (CX_REQ->CX_Case != CX_NoneData) {
					CX_REQ->ACT_SIZE = usbh_get_cx_act_size((void *) &DEV->EP[EPNUM], CX_REQ);
				}
				status = SUCCESS;
			} else {
				USBH_DBG("CX XFR _at_ QH:0x%x Fail", (uint32_t )DEV->EP[EPNUM].pQH);
				status = FAIL;
			}
		}
	}
#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree(BUFF);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	free(BUFF);
#endif
	return status;
}

/** \defgroup USBH_FUN USBH functions
 * \ingroup USBH_MODULE
 * 
 * @{
 */

/** \fn	uint8_t	usbh_bk_xfr(USBH_Device_Structure *DEV,USBH_BK_XFR_REQ_Struct *BK_REQ)
 * \brief bulk transfer function
 * 
 * \param DEV target USB Device
 * 
  * \param BK_REQ bulk transfer request
 * 
 * \return status ,	SUCCESS	or  FAIL
 */
/** @} */
uint8_t usbh_bk_xfr(USBH_Device_Structure *DEV, USBH_BK_XFR_REQ_Struct *BK_REQ) {

	uint32_t EPNUM = 0, j = 0;
	uint32_t TimeoutMsec = 0;
	uint32_t BulkTimeout = 0;
#ifdef CONFIG_MODULE_USBH_WIFI_BK_OUT_TIMEOUT
	uint32_t ConfigTimeout = 0;
#endif 
	
	// Find BK#	QH 
	for (EPNUM = 1; EPNUM < MAX_QH_PER_DEVICE; EPNUM++) {
		if (DEV->EP[EPNUM].XfrType == BK_REQ->XfrType) {
			j++;
			if (j == BK_REQ->NUM) {
				DEV->iEPNUM = EPNUM;

				break;
			}
		}
	}

	if(EPNUM == MAX_QH_PER_DEVICE) {
		if(DEV->bDevIsConnected == 0) {
			USBH_DBG("BK XFR DEV_NOT_EXIST");
			return DEV_NOT_EXIST;
		} else {
			USBH_DBG("BK XFR FAIL");
			return FAIL;
		}
	}

	// Resource	Check
#if defined (CONFIG_USBH_FREE_RTOS)
	if ( xSemaphoreTake(DEV->EP[EPNUM].Mutex, USBH_1000ms) == pdFALSE) {
		USBH_DBG("USBH Reource Timeout Error");
		DEV->EP[EPNUM].status = EHCI_QTD_STATUS_TIMEOUT;
		return FAIL;
	}
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	if (osMutexWait(DEV->EP[EPNUM].Mutex, USBH_1000ms) != osOK) {
		USBH_DBG("USBH Reource Timeout Error");
		DEV->EP[EPNUM].status = EHCI_QTD_STATUS_TIMEOUT;
		return FAIL;
	}
#endif

	// Disable EP XFR
	usbh_disable_xfr((void *)&DEV->EP[EPNUM]);

	// Init BK Structure
	usbh_bk_struct_init((void *)&DEV->EP[EPNUM],BK_REQ);

	//Flush & Set Timeout value	
#if defined( CONFIG_SN_GCC_SDK )	
	if(BK_REQ->XfrType == USBH_BK_OUT_TYPE){
		if(BK_REQ->SIZE){
			flush_dcache_area((uint32_t)BK_REQ->pBUFF, BK_REQ->SIZE);
		}
	}
#endif	

	// Enable QH
	usbh_enable_xfr((void *)&DEV->EP[EPNUM]);

	// Wait for	XFR done
	if(BK_REQ->XfrType == USBH_BK_OUT_TYPE){
		if(DEV->CLASS_DRV == USBH_WIFI_CLASS){
#ifdef CONFIG_MODULE_USBH_WIFI_BK_OUT_TIMEOUT
			ConfigTimeout = CONFIG_MODULE_USBH_WIFI_BK_OUT_TIMEOUT;
			if(ConfigTimeout == 10000000)
				BulkTimeout = USBH_MAX;
			else
#if defined (CONFIG_USBH_FREE_RTOS)
			BulkTimeout = ConfigTimeout/portTICK_RATE_MS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			BulkTimeout = ConfigTimeout;
#endif
#else
			BulkTimeout = USBH_BK_OUT_TIMEOUT;
#endif
		}
		else if(DEV->CLASS_DRV == USBH_MSC_CLASS){
#ifdef CONFIG_MODULE_USBH_MSC_BK_OUT_TIMEOUT
			ConfigTimeout = CONFIG_MODULE_USBH_MSC_BK_OUT_TIMEOUT;
			if(ConfigTimeout == 10000000)
				BulkTimeout = USBH_MAX;
			else
#if defined (CONFIG_USBH_FREE_RTOS)
			BulkTimeout = ConfigTimeout/portTICK_RATE_MS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			BulkTimeout = ConfigTimeout;
#endif
#else
			BulkTimeout = USBH_BK_OUT_TIMEOUT;
#endif
		}
		else {
			BulkTimeout = USBH_BK_OUT_TIMEOUT;
		}
	}else{
		if(DEV->CLASS_DRV == USBH_WIFI_CLASS){
#ifdef CONFIG_MODULE_USBH_WIFI_BK_IN_TIMEOUT
			ConfigTimeout = CONFIG_MODULE_USBH_WIFI_BK_IN_TIMEOUT;
			if(ConfigTimeout == 10000000)
				BulkTimeout = USBH_MAX;
			else
#if defined (CONFIG_USBH_FREE_RTOS)
			BulkTimeout = ConfigTimeout/portTICK_RATE_MS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			BulkTimeout = ConfigTimeout;
#endif
#else
			BulkTimeout = USBH_WIFI_BK_IN_TIMEOUT;
#endif
		} else if(DEV->CLASS_DRV == USBH_MSC_CLASS){
#ifdef CONFIG_MODULE_USBH_MSC_BK_IN_TIMEOUT
			ConfigTimeout = CONFIG_MODULE_USBH_MSC_BK_IN_TIMEOUT;
			if(ConfigTimeout == 10000000)
				BulkTimeout = USBH_MAX;
			else
#if defined (CONFIG_USBH_FREE_RTOS)
			BulkTimeout = ConfigTimeout/portTICK_RATE_MS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			BulkTimeout = ConfigTimeout;
#endif
#else
			BulkTimeout = USBH_BK_IN_TIMEOUT;
#endif
		} else {
			BulkTimeout = USBH_BK_IN_TIMEOUT;
		}
	}

	//TimeoutMsec	= (((BK_REQ->TimeoutMsec) > BulkTimeout) ? BK_REQ->TimeoutMsec : BulkTimeout);
	if( BK_REQ->TimeoutMsec > 0 )
		TimeoutMsec = BK_REQ->TimeoutMsec;
	else
		TimeoutMsec = BulkTimeout;

#if defined (CONFIG_USBH_FREE_RTOS)
	if (xSemaphoreTake((SemaphoreHandle_t )DEV->EP[EPNUM].SEM, TimeoutMsec) == pdFAIL) {		
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	if (osSemaphoreWait(DEV->EP[EPNUM].SEM, TimeoutMsec) != osOK) {
#endif
		if (DEV->bDevIsConnected == 0) {
			// determine actually transfer size
			//BK_REQ->ACT_SIZE = usbh_get_bk_act_size((void *) &DEV->EP[EPNUM], BK_REQ);

			ehci_stop_xfr((void *) &DEV->EP[EPNUM]);
			// release resource
#if defined (CONFIG_USBH_FREE_RTOS)
			xSemaphoreGive(DEV->EP[EPNUM].Mutex);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osMutexRelease(DEV->EP[EPNUM].Mutex);
#endif
			
			USBH_DBG("BK XFR _at_ QH:0x%x DEV NOT EXIST", (uint32_t )DEV->EP[EPNUM].pQH);
			return DEV_NOT_EXIST;
		} else {
			// Disable XFR
			ehci_stop_xfr((void *) &DEV->EP[EPNUM]);
			// determine actually transfer size
			BK_REQ->ACT_SIZE = usbh_get_bk_act_size((void *) &DEV->EP[EPNUM], BK_REQ);
			// release resource
			if(DEV->EP[EPNUM].Mutex == NULL)
				USBH_DBG("%s : Mutex is NULL !!", __func__);
#if defined (CONFIG_USBH_FREE_RTOS)
			xSemaphoreGive(DEV->EP[EPNUM].Mutex);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osMutexRelease(DEV->EP[EPNUM].Mutex);
#endif
			USBH_DBG("BK TIMEOUT FAIL , _at_ QH:0x%x Fail, EPNUM = %d, (%d)",(uint32_t)DEV->EP[EPNUM].pQH, EPNUM, TimeoutMsec);
			DEV->EP[EPNUM].status = EHCI_QTD_STATUS_TIMEOUT;
			return FAIL;
		}
	} else {
		if(DEV->bDevIsConnected == 0) {
			// determine actually transfer size
			//BK_REQ->ACT_SIZE = usbh_get_bk_act_size((void *) &DEV->EP[EPNUM], BK_REQ);

			ehci_stop_xfr((void *)&DEV->EP[EPNUM]);
#if defined (CONFIG_USBH_FREE_RTOS)
			xSemaphoreGive(DEV->EP[EPNUM].Mutex);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osMutexRelease(DEV->EP[EPNUM].Mutex);
#endif
			USBH_DBG("BK XFR _at_ QH:0x%x DEVICE NOT EXIST", (uint32_t )DEV->EP[EPNUM].pQH);
			return DEV_NOT_EXIST;
		}
		
		if (usbh_chk_xfr_result((void *) &DEV->EP[EPNUM]) == SUCCESS) {
			// determine actually transfer size
			BK_REQ->ACT_SIZE = usbh_get_bk_act_size((void *) &DEV->EP[EPNUM], BK_REQ);

			// record last toggle bit
			DEV->EP[EPNUM].DataTog = DEV->EP[EPNUM].pQH->bOverlay_DT;
			if (BK_REQ->XfrType == USBH_BK_IN_TYPE) {
#if defined( CONFIG_SN_GCC_SDK )				
				if (BK_REQ->SIZE) {
					flush_dcache_area((uint32_t) BK_REQ->pBUFF, BK_REQ->SIZE);
				}
#endif
			}
#if defined (CONFIG_USBH_FREE_RTOS)
			xSemaphoreGive(DEV->EP[EPNUM].Mutex);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osMutexRelease(DEV->EP[EPNUM].Mutex);
#endif
			return SUCCESS;
		} else {
			// determine actually transfer size
			BK_REQ->ACT_SIZE = usbh_get_bk_act_size((void *)&DEV->EP[EPNUM], BK_REQ);

			if (DEV->EP[EPNUM].status == EHCI_QTD_STATUS_Halted) {
				DEV->EP[EPNUM].DataTog = 0;
			}

#if defined (CONFIG_USBH_FREE_RTOS)
			xSemaphoreGive(DEV->EP[EPNUM].Mutex);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
			osMutexRelease(DEV->EP[EPNUM].Mutex);
#endif
			USBH_DBG("BK XFR _at_ QH:0x%x Fail", (uint32_t )DEV->EP[EPNUM].pQH);
			return FAIL;
		}
	}
}

uint8_t	usbh_int_xfr(USBH_Device_Structure *DEV, USBH_INT_XFR_REQ_Struct *INT_REQ) 
{
	uint32_t EPNUM = 0,j=0;
	uint32_t tempsize = 0;
	uint32_t timeout = 0;
#ifdef CONFIG_MODULE_USBH_INT_IN_TIMEOUT	
	uint32_t ConfigTimeout	= 0;
#endif 	
	// Find INT#	QH 
	for(EPNUM=1;EPNUM<MAX_QH_PER_DEVICE;EPNUM++){
		if (DEV->EP[EPNUM].XfrType == INT_REQ->XfrType)	{
			j++;
			if(j == INT_REQ->NUM) break;
		}
	}

	// Disable EP XFR
	usbh_disable_xfr((void *)&DEV->EP[EPNUM]);

	// Init INT	Structure
	usbh_int_struct_init((void *)&DEV->EP[EPNUM],INT_REQ);

	//Flush & Set Timeout value	
	if(INT_REQ->XfrType	== USBH_INT_OUT_TYPE){
#if defined( CONFIG_SN_GCC_SDK )		
		if(INT_REQ->SIZE){
			flush_dcache_area((uint32_t)INT_REQ->pBUFF, INT_REQ->SIZE);
		}
#endif
		timeout	= USBH_INT_OUT_TIMEOUT;
	}else{
		//timeout	= USBH_INT_IN_TIMEOUT;

#ifdef CONFIG_MODULE_USBH_INT_IN_TIMEOUT
		ConfigTimeout = CONFIG_MODULE_USBH_INT_IN_TIMEOUT;
		if(ConfigTimeout == 10000000)
			timeout = USBH_MAX;
		else
#if defined (CONFIG_USBH_FREE_RTOS)
		timeout = ConfigTimeout/portTICK_RATE_MS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		timeout = ConfigTimeout;
#endif
#else
		timeout	= USBH_INT_IN_TIMEOUT;
#endif

	}
	// Enable QH
	usbh_enable_xfr((void *)&DEV->EP[EPNUM]);

	// Wait for	XFR done
#if defined (CONFIG_USBH_FREE_RTOS)
	if(xSemaphoreTake((SemaphoreHandle_t)DEV->EP[EPNUM].SEM,timeout) == pdFAIL){
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	if(osSemaphoreWait(DEV->EP[EPNUM].SEM,timeout) != osOK){
#endif
		
		// Disable XFR
		ehci_stop_xfr((void *)&DEV->EP[EPNUM]);
		USBH_DBG("INT TIMEOUT(%d) FAIL , _at_ QH:0x%x Fail", timeout, (uint32_t)DEV->EP[EPNUM].pQH);
		return FAIL;
	}else{
		if( usbh_chk_xfr_result((void *)&DEV->EP[EPNUM]) == SUCCESS ){
			// determine actually transfer size
			j=0;
			while(1){
				tempsize += DEV->EP[EPNUM].pQTD[j]->bTotalBytes;
				if(DEV->EP[EPNUM].pQTD[j]->bTerminate){
					break;
				}else{
					j++;
				}
			};
			INT_REQ->ACT_SIZE	=	INT_REQ->SIZE-tempsize;
			// record last toggle bit
			DEV->EP[EPNUM].DataTog = DEV->EP[EPNUM].pQH->bOverlay_DT;
			if(INT_REQ->XfrType	== 	USBH_INT_IN_TYPE){
#if defined( CONFIG_SN_GCC_SDK )
				if(INT_REQ->SIZE){
					flush_dcache_area((uint32_t)INT_REQ->pBUFF, INT_REQ->SIZE);
				}
#endif
			}
			return SUCCESS;
		}else{
			USBH_DBG("INT XFR _at_ QH:0x%x Fail",(uint32_t)DEV->EP[EPNUM].pQH);
			return FAIL;
		}
	}
}


uint8_t	usbh_iso_xfr(USBH_Device_Structure *DEV, USBH_ISO_XFR_REQ_Struct *ISO_REQ) 
{
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
	uint32_t EPNUM = 0;//, j=0;	
	// Find ISO	EPNUM 
	for(EPNUM=1;EPNUM<MAX_QH_PER_DEVICE;EPNUM++){
		if (DEV->EP[EPNUM].XfrType == ISO_REQ->XfrType)	{
			//j++;
			//if(j == ISO_REQ->NUM) break;
			
			if(EPNUM == ISO_REQ->NUM) break;
		}
	}	

	// Stream START / STOP
	if(ISO_REQ->START){		
		// Init ISO Structure		
		usbh_iso_struct_init((void *)&DEV->EP[EPNUM],ISO_REQ);		
		usbh_enable_xfr((void *)&DEV->EP[EPNUM]);
	}else{
		usbh_disable_xfr((void *)&DEV->EP[EPNUM]);
	}
	return SUCCESS; 
#else
	
	USBH_STD_ISO_QUEUE_STRUCT	STD_ISO_REQ;

	STD_ISO_REQ.DEV		= (uint32_t *)&DEV;
	STD_ISO_REQ.EP		= (uint32_t *)&DEV->EP[ISO_REQ->NUM];
	STD_ISO_REQ.ISO_REQ	= (uint32_t *)ISO_REQ;	

	// Stream START / STOP
	if(ISO_REQ->START){
		ISO_REQ->STD_ISO_SM	=	1;
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueSend(USBH_QUEUE_STD_ISO_REQ, &STD_ISO_REQ, 0);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessagePut(USBH_QUEUE_STD_ISO_REQ, &STD_ISO_REQ, 0);
#endif
	}else{
		ISO_REQ->STD_ISO_SM	=	0;
	}
	return SUCCESS; 

#endif 
}

uint8_t usbh_cx_clear_feature(USBH_Device_Structure *dev, USBH_CX_XFR_REQ_Struct *cx_req, BK_EP_INDEX EP) {

	uint8_t status = SUCCESS;

	cx_req->CMD = USBH_CX_CMD_CLEAR_FEATURE;
	cx_req->wValue = 0x00;

	if (EP == BULK_IN_EP) {
		int i = 1;
		for (i = 1; i < MAX_QH_PER_DEVICE; i++) {
			if (dev->EP[i].XfrType == EHCI_BK_IN_TYPE) {
				USBH_CLEAR_FEATURE[4] = 0x80 | i;
				dev->EP[i].DataTog = 0;
				break;
			}
		}
	} else if (EP == BULK_OUT_EP) {
		int i = 1;
		for (i = 1; i < MAX_QH_PER_DEVICE; i++) {
			if (dev->EP[i].XfrType == EHCI_BK_OUT_TYPE) {
				USBH_CLEAR_FEATURE[4] = 0x00 | i;
				dev->EP[i].DataTog = 0;
				break;
			}
		}
	} else {
		return FAIL;
	}

	status = usbh_cx_xfr((USBH_Device_Structure*)dev, (USBH_CX_XFR_REQ_Struct*)cx_req);

	return status;
}

uint8_t usbh_enum(USBH_Device_Structure * DEV, uint8_t addr) {
	USBH_CX_XFR_REQ_Struct CX_REQ;
	uint8_t STATUS = FAIL;
	uint8_t	*SETUP_DATA_BUFF = NULL;

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
	do{
		SETUP_DATA_BUFF		=	pvPortMalloc(bConfigurationDescLength,GFP_DMA, MODULE_DRI_USBH);
	}while(SETUP_DATA_BUFF == NULL);
#endif 	
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
	do{
		SETUP_DATA_BUFF 	=	pvPortMalloc(bConfigurationDescLength);
	}while(SETUP_DATA_BUFF == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
	SETUP_DATA_BUFF = (uint8_t *)malloc(bConfigurationDescLength);
#endif
#endif
	
	memset(&CX_REQ, 0, sizeof(CX_REQ));		
	
	// 1. Get Device descriptor	: Addr0	ENDP0 LEN=8
	CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_DEVICE;
	CX_REQ.SIZE	=	8;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS != SUCCESS){
		USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 1");
		goto err;
	}
#if defined( CONFIG_SN_GCC_SDK )	
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	memcpy(&DEV->DEVDes,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
		
	// 2. Set Device Address  :	Addr0 ENDP0
	CX_REQ.CMD		=	USBH_CX_CMD_SETADDRESS;
	CX_REQ.wValue	=	addr;	// Address = 1 
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS != SUCCESS){
		USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 2");
		goto err;		
	}					
	// Apply New Parameter 
	DEV->EP->pQH->bMaxPacketSize	=	DEV->DEVDes.bEP0MAXPACKETSIZE;		
	DEV->EP->pQH->bDeviceAddress	=	addr;		
	DEV->bAdd											=	addr;
		
	// 3. Get Device descriptor	: Addr1	ENDP0 LEN=18
	CX_REQ.CMD		=	USBH_CX_CMD_GETDESCRIPTOR_DEVICE;
	CX_REQ.SIZE		=	18;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS != SUCCESS){
		USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 3");
		goto err;
	}
#if defined( CONFIG_SN_GCC_SDK )
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	memcpy(&DEV->DEVDes,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);		

	// 4. Get Configure	Descriptor : Addr1 ENDP0 LEN=9
	CX_REQ.CMD		=	USBH_CX_CMD_GETDESCRIPTOR_CONFIG;
	CX_REQ.SIZE		=	9;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS != SUCCESS){
		USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 4");
		goto err;			
	}
#if defined( CONFIG_SN_GCC_SDK )	
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	memcpy(&DEV->CFGDes[0],CX_REQ.pBUFF,CX_REQ.ACT_SIZE);		
		
	// 5. Get Configure	Descriptor : Addr1 ENDP0 LEN=wTotalLength
	CX_REQ.CMD		=	USBH_CX_CMD_GETDESCRIPTOR_CONFIG;
	CX_REQ.SIZE		=	(DEV->CFGDes[0].bTotalLengthHighByte<<8)|DEV->CFGDes[0].bTotalLengthLowByte;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS != SUCCESS){
		USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 5");
		goto err;			
	}		
#if defined( CONFIG_SN_GCC_SDK )
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	usbh_parser_cfgdes(DEV,(uint8_t*)CX_REQ.pBUFF);
	memcpy(DEV->bData, (uint8_t*)CX_REQ.pBUFF, CX_REQ.ACT_SIZE);

	// 6. Get Language	
	CX_REQ.CMD		=	USBH_CX_GETDESCRIPTOR_STR_LANG;
	CX_REQ.SIZE		=	4;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS != SUCCESS){
		USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 6");
		goto err;
	}		
#if defined( CONFIG_SN_GCC_SDK )	
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	memcpy(&DEV->bStringLanguage,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
		
	// 7. Get Product String
	if(DEV->DEVDes.bProduct){
		CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_STR_Product;
		CX_REQ.SIZE	=	bStringProductLength;
		CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
		CX_REQ.wValue	=	DEV->DEVDes.bProduct;
		CX_REQ.wIndex	=	(DEV->bStringLanguage[3]<<8)|DEV->bStringLanguage[2];
		STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
		if(STATUS != SUCCESS){
			USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 7");
			goto err;			
		}		
#if defined( CONFIG_SN_GCC_SDK )		
		if(CX_REQ.ACT_SIZE){
			flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
		}
#endif
		memcpy(&DEV->bStringProduct,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);		
	}
		
// 8. Get Manufacture String
	if(DEV->DEVDes.bManufacturer){
		CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_STR_Manufacturer;
		CX_REQ.SIZE	=	bStringManufactureLength;
		CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
		CX_REQ.wValue	=	DEV->DEVDes.bManufacturer;
		CX_REQ.wIndex	=	(DEV->bStringLanguage[3]<<8)|DEV->bStringLanguage[2];
		STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
		if(STATUS != SUCCESS){
			USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 8");
			goto err;			
		}		
#if defined( CONFIG_SN_GCC_SDK )
		if(CX_REQ.ACT_SIZE){
			flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
		}
#endif
		memcpy(&DEV->bStringManufacture,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
			
	// 9. Get Serial Number String
	if(DEV->DEVDes.bSerialNumber){
		CX_REQ.CMD		=	USBH_CX_CMD_GETDESCRIPTOR_STR_Serial;
		CX_REQ.SIZE		=	bStringSerialNLength;
		CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
		CX_REQ.wValue	=	DEV->DEVDes.bSerialNumber;
		CX_REQ.wIndex	=	(DEV->bStringLanguage[3]<<8)|DEV->bStringLanguage[2];
		STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
		if(STATUS != SUCCESS){
			USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 9");
			goto err;		
		}		
#if defined( CONFIG_SN_GCC_SDK )		
		if(CX_REQ.ACT_SIZE){
			flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
		}
#endif
		memcpy(&DEV->bStringSerialN,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}	
			
	// 10. Set Config
	CX_REQ.CMD		=	USBH_CX_CMD_SETCONFIGURATION;	
	CX_REQ.wValue	=	DEV->DEVDes.bCONFIGURATION_NUMBER;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS != SUCCESS){
		USBH_DBG("ENUM FAIL,GOING TO ERROR HANDLING 10");
		goto err;		
	}
	usbh_print_device_info(DEV);
#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree(SETUP_DATA_BUFF);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	free(SETUP_DATA_BUFF);
#endif
	return SUCCESS;
	
err:
#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree(SETUP_DATA_BUFF);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	free(SETUP_DATA_BUFF);
#endif
	return FAIL;
}

void usbh_root_enum(void)
{
	USBH_Device_Structure *DEV;
	
	DEV	=	(USBH_Device_Structure*)&USBH_DEV[USBH_ROOT];

	usbh_parser_dev(DEV,USBH_PARSER_ROOT_DET);

	switch(DEV->ENUM_SM){
	case USBH_ROOT_PLUG_OUT_STATE:
		USBH_DBG("Enter USBH_ROOT_PLUG_OUT_STATE !");
		if(DEV->CLASS_DRV == USBH_HUB_CLASS){
			usbh_parser_dev(DEV,USBH_PARSER_ROOT_HUB_DEQ);
		}else{
			usbh_parser_dev(DEV,USBH_PARSER_ROOT_DEQ);
		}
		DEV->ENUM_SM	=   USBH_ROOT_DET_STATE;
		break;
		
	case USBH_ROOT_PLUG_IN_STATE:
		USBH_DBG("Enter USBH_ROOT_PLUG_IN_STATE !");
		if(usbh_parser_dev(DEV,USBH_PARSER_ROOT_RST) == SUCCESS){
			usbh_parser_dev(DEV,USBH_PARSER_CX_ENQU);
			DEV->ENUM_SM	=   USBH_ROOT_ENUM_STATE;
		}else{
			DEV->ENUM_SM	=	USBH_ROOT_ERR_HDL_STATE;
		}
		break;
		
	case USBH_ROOT_ENUM_STATE:
		USBH_DBG("Enter USBH_ROOT_ENUM_STATE !");		
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)		
		dev_enum_time = USBH_100ms;
#endif		
		if(usbh_enum(DEV,1)	==	SUCCESS){
			DEV->ENUM_SM	=	USBH_ROOT_PARSER_STATE;
		}else{
			DEV->ENUM_SM	=	USBH_ROOT_ERR_HDL_STATE;
		}
		break;
		
	case USBH_ROOT_PARSER_STATE:
		USBH_DBG("Enter USBH_ROOT_PARSER_STATE !");		
		usbh_parser_dev(DEV,USBH_PARSER_NONCX_ENQU);
		DEV->ENUM_SM	=	USBH_ROOT_ACTIVE_STATE;
		break;
	
	case USBH_ROOT_ACTIVE_STATE:
		USBH_DBG("Enter USBH_ROOT_ACTIVE_STATE !");
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
		dev_enum_time = USBH_500ms;
#endif
#if defined (CONFIG_WIFI_SDK)
		/* is the driver added for this device */
		if (FAIL == usbh_device_add(DEV))
			usbh_parser_dev(DEV,USBH_PARSER_WAKE_CLASS_DRV);
#else
		usbh_parser_dev(DEV,USBH_PARSER_WAKE_CLASS_DRV);
#endif // end of if defined (CONFIG_WIFI_SDK)
		DEV->ENUM_SM	=	USBH_ROOT_DET_STATE;
		break;
		
	case USBH_ROOT_ERR_HDL_STATE:
		USBH_DBG("Enter USBH_ROOT_ERR_HDL_STATE !");

#if defined( CONFIG_FS_RECOVERY_PATCH )
		USBH_DBG("Enable CONFIG_FS_RECOVERY_PATCH !");
	
		usbh_uninit();
		
		usbh_init();
#else
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)
		if( usbh_reset_count < MAX_USBH_RESET_COUNT ) {
			usbh_reset_count++;
			DEV->bDevIsConnected = 0;
		}
#endif
#endif
		DEV->ENUM_SM	=	USBH_ROOT_DET_STATE;

		break;
	case USBH_ROOT_HALT_STATE:
		USBH_DBG("Enter USBH_ROOT_HALT_STATE !");
		DEV->ENUM_SM	=	USBH_ROOT_DET_STATE;
		break;
	}
}

#if defined(CONFIG_AUTOMOUNT)
uint8_t usbh_force_root_plug_out(void) {
	uint8_t		status;
	USBH_Device_Structure * DEV;

	USBH_DBG("Perform force root plug out");

	DEV = (USBH_Device_Structure*) &USBH_DEV[USBH_ROOT];
	DEV->ENUM_SM = USBH_ROOT_PLUG_OUT_STATE;

#if defined (CONFIG_USBH_FREE_RTOS)
	if( xSemaphoreTake(auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT, USBH_3000ms) == pdFALSE ){
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	if( osSemaphoreWait(auto_mount_info_usbh[0].xSEM_USBH_PLUG_OUT, USBH_3000ms) != osOK ){
#endif
		status = FAIL;
		USBH_DBG("Perform force root plug out FAIL !");
	}
	else {
		status = SUCCESS;
		USBH_DBG("Perform force root plug out SUCCESS !");
	}

	return status;
}
#else
void usbh_force_root_plug_out(void)
{
	USBH_Device_Structure * DEV;
	
	USBH_DBG("Perform force root plug out");	
	
#if defined(CONFIG_MODULE_USBH_MSC_MOTOCAM)	
	usbh_reset_count = 0;
#endif	
	
	DEV			=	(USBH_Device_Structure*)&USBH_DEV[USBH_ROOT];	
	DEV->ENUM_SM		=   	USBH_ROOT_PLUG_OUT_STATE;
#if defined (CONFIG_USBH_FREE_RTOS)
	vTaskDelay(100000);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osDelay(100000);
#endif
}
#endif


#if defined( CONFIG_DUAL_HOST )	
extern uint8_t	ehci_parser_dev_2(USBH_Device_Structure *DEV, uint8_t FUN);
uint8_t	usbh_parser_dev_2(USBH_Device_Structure *DEV, uint8_t FUN){
	return ehci_parser_dev_2(DEV,FUN);
}

void usbh_core_init_2(void)
{
	memset((uint8_t*)(&USBH_DEV_2[0]),0,USBH_MAX_PORT*sizeof(USBH_DEV_2[0]));// clear all device structure 
}

uint8_t	usbh_enum_2(USBH_Device_Structure *DEV, uint8_t addr)
{
	USBH_CX_XFR_REQ_Struct	CX_REQ;
	uint8_t STATUS = FAIL;
	uint8_t	*SETUP_DATA_BUFF = NULL;

#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
	do{
		SETUP_DATA_BUFF		=	pvPortMalloc(bConfigurationDescLength,GFP_DMA, MODULE_DRI_USBH);
	}while(SETUP_DATA_BUFF == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
	do{
		SETUP_DATA_BUFF 	=	pvPortMalloc(bConfigurationDescLength);
	}while(SETUP_DATA_BUFF == NULL);
#endif
#endif
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK )
	SETUP_DATA_BUFF = (uint8_t *)malloc(bConfigurationDescLength);
#endif
#endif
	
	memset(&CX_REQ,0,sizeof(CX_REQ));		
	
	// 1. Get Device descriptor	: Addr0	ENDP0 LEN=8
	CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_DEVICE;
	CX_REQ.SIZE	=	8;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		USBH_DBG("ENUM FAIL1,GOING TO ERROR HANDLING");
		goto err;
	}
#if defined( CONFIG_SN_GCC_SDK )	
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	memcpy(&DEV->DEVDes,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
		
	// 2. Set Device Address  :	Addr0 ENDP0
	CX_REQ.CMD	=	USBH_CX_CMD_SETADDRESS;
	CX_REQ.wValue	=	addr;	// Address = 1 
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		USBH_DBG("ENUM FAIL2,GOING TO ERROR HANDLING");
		goto err;		
	}					
	// Apply New Parameter 
	DEV->EP->pQH->bMaxPacketSize	=	DEV->DEVDes.bEP0MAXPACKETSIZE;		
	DEV->EP->pQH->bDeviceAddress	=	addr;		
	DEV->bAdd			=	addr;
		
	// 3. Get Device descriptor	: Addr1	ENDP0 LEN=18
	CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_DEVICE;
	CX_REQ.SIZE	=	18;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		USBH_DBG("ENUM FAIL3,GOING TO ERROR HANDLING");
		goto err;
	}
#if defined( CONFIG_SN_GCC_SDK )	
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	memcpy(&DEV->DEVDes,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);		

	// 4. Get Configure	Descriptor : Addr1 ENDP0 LEN=9
	CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_CONFIG;
	CX_REQ.SIZE	=	9;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		USBH_DBG("ENUM FAIL4,GOING TO ERROR HANDLING");
		goto err;			
	}
#if defined( CONFIG_SN_GCC_SDK )	
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	memcpy(&DEV->CFGDes[0],CX_REQ.pBUFF,CX_REQ.ACT_SIZE);		
		
	// 5. Get Configure	Descriptor : Addr1 ENDP0 LEN=wTotalLength
	CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_CONFIG;
	CX_REQ.SIZE	=	(DEV->CFGDes[0].bTotalLengthHighByte<<8)|DEV->CFGDes[0].bTotalLengthLowByte;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		USBH_DBG("ENUM FAIL5,GOING TO ERROR HANDLING");
		goto err;			
	}		
#if defined( CONFIG_SN_GCC_SDK )	
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	usbh_parser_cfgdes(DEV,(uint8_t*)CX_REQ.pBUFF);
	memcpy(DEV->bData, (uint8_t*)CX_REQ.pBUFF, CX_REQ.ACT_SIZE);

	// 6. Get Language	
	CX_REQ.CMD	=	USBH_CX_GETDESCRIPTOR_STR_LANG;
	CX_REQ.SIZE	=	4;
	CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		USBH_DBG("ENUM FAIL6,GOING TO ERROR HANDLING");
		goto err;
	}		
#if defined( CONFIG_SN_GCC_SDK )	
	if(CX_REQ.ACT_SIZE){
		flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}
#endif
	memcpy(&DEV->bStringLanguage,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);			
		
	// 7. Get Manufacture String
	if(DEV->DEVDes.bManufacturer){
		CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_STR_Manufacturer;
		CX_REQ.SIZE	=	bStringManufactureLength;
		CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
		CX_REQ.wValue	=	DEV->DEVDes.bManufacturer;
		CX_REQ.wIndex	=	(DEV->bStringLanguage[3]<<8)|DEV->bStringLanguage[2];
		STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
		if(STATUS == FAIL){
			USBH_DBG("ENUM FAIL7,GOING TO ERROR HANDLING");
			goto err;			
		}		
#if defined( CONFIG_SN_GCC_SDK )
		if(CX_REQ.ACT_SIZE){
			flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
		}
#endif
		memcpy(&DEV->bStringManufacture,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);			
	}
		
	// 8. Get Product String
	if(DEV->DEVDes.bProduct){
		CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_STR_Product;
		CX_REQ.SIZE	=	bStringProductLength;
		CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
		CX_REQ.wValue	=	DEV->DEVDes.bProduct;
		CX_REQ.wIndex	=	(DEV->bStringLanguage[3]<<8)|DEV->bStringLanguage[2];
		STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
		if(STATUS == FAIL){
			USBH_DBG("ENUM FAIL8,GOING TO ERROR HANDLING");
			goto err;			
		}		
#if defined( CONFIG_SN_GCC_SDK )		
		if(CX_REQ.ACT_SIZE){
			flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
		}
#endif
		memcpy(&DEV->bStringProduct,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);		
	}
			
	// 9. Get Serial Number String
	if(DEV->DEVDes.bSerialNumber){
		CX_REQ.CMD	=	USBH_CX_CMD_GETDESCRIPTOR_STR_Serial;
		CX_REQ.SIZE	=	bStringSerialNLength;
		CX_REQ.pBUFF	=	(uint32_t*)SETUP_DATA_BUFF;
		CX_REQ.wValue	=	DEV->DEVDes.bSerialNumber;
		CX_REQ.wIndex	=	(DEV->bStringLanguage[3]<<8)|DEV->bStringLanguage[2];
		STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
		if(STATUS == FAIL){
			USBH_DBG("ENUM FAIL9,GOING TO ERROR HANDLING");
			goto err;		
		}		
#if defined( CONFIG_SN_GCC_SDK )		
		if(CX_REQ.ACT_SIZE){
			flush_dcache_area((uint32_t)CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
		}
#endif
		memcpy(&DEV->bStringSerialN,CX_REQ.pBUFF,CX_REQ.ACT_SIZE);
	}	
			
	// 10. Set Config
	CX_REQ.CMD	=	USBH_CX_CMD_SETCONFIGURATION;	
	CX_REQ.wValue	=	DEV->DEVDes.bCONFIGURATION_NUMBER;
	STATUS = usbh_cx_xfr((USBH_Device_Structure*)DEV,(USBH_CX_XFR_REQ_Struct*)&CX_REQ);
	if(STATUS == FAIL){
		USBH_DBG("ENUM FAILA,GOING TO ERROR HANDLING");
		goto err;		
	}
	usbh_print_device_info(DEV);

#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree(SETUP_DATA_BUFF);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	free(SETUP_DATA_BUFF);
#endif
	return SUCCESS;
	
err:
#if defined (CONFIG_USBH_FREE_RTOS)
	vPortFree(SETUP_DATA_BUFF);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	free(SETUP_DATA_BUFF);
#endif
	return FAIL;
}

void usbh_root_enum_2(void)
{
	USBH_Device_Structure *DEV;
	
	DEV	=	(USBH_Device_Structure*)&USBH_DEV_2[USBH_ROOT];

	usbh_parser_dev_2(DEV,USBH_PARSER_ROOT_DET);

	switch(DEV->ENUM_SM){
		case USBH_ROOT_PLUG_OUT_STATE:
			USBH_DBG("Enter USBH_ROOT_PLUG_OUT_STATE !");
			if(DEV->CLASS_DRV == USBH_HUB_CLASS){
				usbh_parser_dev_2(DEV,USBH_PARSER_ROOT_HUB_DEQ);
			}else{
				usbh_parser_dev_2(DEV,USBH_PARSER_ROOT_DEQ);
			}
			DEV->ENUM_SM	=   USBH_ROOT_DET_STATE;
			break;		
		case USBH_ROOT_PLUG_IN_STATE:
			USBH_DBG("Enter USBH_ROOT_PLUG_IN_STATE !");
			if(usbh_parser_dev_2(DEV,USBH_PARSER_ROOT_RST) == SUCCESS){
				usbh_parser_dev_2(DEV,USBH_PARSER_CX_ENQU);
				DEV->ENUM_SM	=   USBH_ROOT_ENUM_STATE;
			}else{
				DEV->ENUM_SM	=	USBH_ROOT_ERR_HDL_STATE;
			}
			break;
		case USBH_ROOT_ENUM_STATE:
			USBH_DBG("Enter USBH_ROOT_ENUM_STATE !");	
			if(usbh_enum_2(DEV,1)	==	SUCCESS){
				DEV->ENUM_SM	=	USBH_ROOT_PARSER_STATE;
			}else{
				DEV->ENUM_SM	=	USBH_ROOT_ERR_HDL_STATE;
			}
			break;
		case USBH_ROOT_PARSER_STATE:
			USBH_DBG("Enter USBH_ROOT_PARSER_STATE !");		
			usbh_parser_dev_2(DEV,USBH_PARSER_NONCX_ENQU);
			DEV->ENUM_SM	=	USBH_ROOT_ACTIVE_STATE;
			break;
		case USBH_ROOT_ACTIVE_STATE:
			USBH_DBG("Enter USBH_ROOT_ACTIVE_STATE !");			
#if defined (CONFIG_WIFI_SDK)
			/* is the driver added for this device */
			if (FAIL == usbh_device_add(DEV))
				usbh_parser_dev(DEV,USBH_PARSER_WAKE_CLASS_DRV)
#else
			usbh_parser_dev_2(DEV,USBH_PARSER_WAKE_CLASS_DRV);
#endif
			DEV->ENUM_SM	=	USBH_ROOT_DET_STATE;
			break;
		case USBH_ROOT_ERR_HDL_STATE:
			USBH_DBG("Enter USBH_ROOT_ERR_HDL_STATE !");
		
#if defined( CONFIG_FS_RECOVERY_PATCH )
			USBH_DBG("Enable CONFIG_FS_RECOVERY_PATCH !");
			
			usbh_uninit_2();
		
			usbh_init_2();
#endif
			DEV->ENUM_SM	=	USBH_ROOT_DET_STATE;		
		
			break;
		case USBH_ROOT_HALT_STATE:
			USBH_DBG("Enter USBH_ROOT_HALT_STATE !");
			DEV->ENUM_SM	=	USBH_ROOT_DET_STATE;
			break;
	}
}

void usbh_force_root_plug_out_2(void){
	USBH_Device_Structure *DEV;
	
	USBH_DBG("Perform force root plug out");	
	
	DEV			=	(USBH_Device_Structure*)&USBH_DEV_2[USBH_ROOT];	
	DEV->ENUM_SM		=   	USBH_ROOT_PLUG_OUT_STATE;	
	
#if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)	
#if defined (CONFIG_USBH_FREE_RTOS)
	if( xSemaphoreTake(auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT, USBH_3000ms) == pdFALSE )
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	if( osSemaphoreWait(auto_mount_info_usbh[1].xSEM_USBH_PLUG_OUT, USBH_3000ms) != osOK )
#endif
	{
//		status = FAIL;
		USBH_DBG("Perform force root plug out FAIL !");
	}
	else {
//		status = SUCCESS;
		USBH_DBG("Perform force root plug out SUCCESS !");
	}
#endif	//endof #if defined(CONFIG_AUTOMOUNT) || defined(CONFIG_RECORD)	
	
#if defined (CONFIG_USBH_FREE_RTOS)
	vTaskDelay(100000);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	osDelay(100000);
#endif
}

#endif // End of if defined( CONFIG_DUAL_HOST )

