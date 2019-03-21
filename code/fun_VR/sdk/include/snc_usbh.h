/**
 * @file
 * this	is USBH	header file, include this file before use
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#ifndef	USBH__H
#define	USBH__H

#if defined( __cplusplus )
extern "C"{
#endif

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include <queue.h>
#include <semphr.h>

#define	USBH_CONFIGURATION_NUM_MAX	0x02
#define	USBH_INTERFACE_NUM_MAX		0x09
#define	USBH_ENDPOINT_NUM_MAX		0x09
#define	USBH_CLASS_NUM_MAX		0x03

#define bStringLanguageLength			0x0A
#define bStringManufactureLength	0x40
#define bStringProductLength		0x40
#define bStringSerialNLength		0x40

#define	SUCCESS		0	/**< success */
#define	FAIL		1	/**< fail */

#define	MAX_QTD_PER_QH				16			/**< MAX QTD Per QH	(10*20KB/ENDP) */
#define USBH_UVC_BK_STREAM_BUF_SIZE		(MAX_QTD_PER_QH-1)*20*1024 //(MAX_QTD-1)*20k

#define	USBH_2ms	2/portTICK_RATE_MS
#define	USBH_5ms	5/portTICK_RATE_MS
#define	USBH_10ms	10/portTICK_RATE_MS
#define	USBH_20ms	20/portTICK_RATE_MS
#define	USBH_30ms	30/portTICK_RATE_MS
#define	USBH_40ms	40/portTICK_RATE_MS
#define	USBH_50ms	50/portTICK_RATE_MS
#define	USBH_60ms	60/portTICK_RATE_MS
#define	USBH_70ms	70/portTICK_RATE_MS
#define	USBH_80ms	80/portTICK_RATE_MS
#define	USBH_90ms	90/portTICK_RATE_MS
#define	USBH_100ms	100/portTICK_RATE_MS
#define	USBH_120ms	120/portTICK_RATE_MS
#define	USBH_200ms	200/portTICK_RATE_MS
#define	USBH_500ms	500/portTICK_RATE_MS
#define	USBH_1000ms	1000/portTICK_RATE_MS
#define	USBH_2000ms	2000/portTICK_RATE_MS
#define	USBH_3000ms	3000/portTICK_RATE_MS
#define	USBH_5000ms	5000/portTICK_RATE_MS
#define	USBH_10000ms	10000/portTICK_RATE_MS
#define	USBH_MAX	0xFFFFFFFF

// UVC steram format
#define	USBH_UVC_STREAM_UNKNOW		0
#define	USBH_UVC_STREAM_H264			1
#define	USBH_UVC_STREAM_MJPEG			2
#define	USBH_UVC_STREAM_YUV				3

// UVC stream resolution
#define USBH_UVC_STREAM_3264X2448		1
#define	USBH_UVC_STREAM_1920X1080		2
#define	USBH_UVC_STREAM_1280X800		3
#define	USBH_UVC_STREAM_1280X720		4
#define	USBH_UVC_STREAM_960X540			5
#define	USBH_UVC_STREAM_848X480			6
#define	USBH_UVC_STREAM_800X600			7
#define	USBH_UVC_STREAM_640X480			8
#define	USBH_UVC_STREAM_640X360			9
#define	USBH_UVC_STREAM_424X240			10
#define	USBH_UVC_STREAM_320X240			11
#define	USBH_UVC_STREAM_320X180			12
#define	USBH_UVC_STREAM_176X176			13
#define	USBH_UVC_STREAM_160X120			14

// UVC stream frame rate
#define	USBH_UVC_STREAM_5_FPS				0x001E8480
#define	USBH_UVC_STREAM_10_FPS			0x000F4240
#define	USBH_UVC_STREAM_15_FPS			0x000A2C2A
#define	USBH_UVC_STREAM_20_FPS			0x0007A120
#define	USBH_UVC_STREAM_25_FPS			0x00061a80
#define	USBH_UVC_STREAM_30_FPS			0x00051615

#define UVC_INFO(format, args...) printf("\r\nUVC_INFO:"format, ##args)	

#define	USBH_UVC_DATA_QUEUE_SIZE		10

#define max_stream_count	2

#define	MJPEGFrameSize		(CONFIG_MODULE_USBH_MJPEG_SIZE * 3 * 1024)
#define	H264FrameSize		(CONFIG_MODULE_USBH_H264_SIZE * 3 * 1024)
#define	YUVFrameSize		1536*1024

// USBH	Xfr Type
#define	USBH_NONE_TYPE		0x00
#define	USBH_CX_TYPE			0x01
#define	USBH_BK_OUT_TYPE	0x02
#define	USBH_BK_IN_TYPE		0x03
#define	USBH_INT_OUT_TYPE	0x04
#define	USBH_INT_IN_TYPE	0x05
#define	USBH_ISO_OUT_TYPE	0x06
#define	USBH_ISO_IN_TYPE	0x07

typedef	enum
{
	USBH_UNKNOW_CLASS		=	0,
	USBH_WIFI_CLASS			= 1,
	USBH_MSC_CLASS			= 2,
	USBH_UVC_BULK_CLASS	= 4,
	USBH_UVC_ISO_CLASS	= 8,
	USBH_HID_CLASS			= 16,
	USBH_HUB_CLASS			=	32

}USBH_CLASS_DRV_INDEX;

typedef struct
{
	uint32_t	stream_id;
	uint32_t 	*ptr;
	uint32_t 	size;
	uint32_t 	framecnt;
	uint32_t	xact_errcnt;
	uint32_t	xact_TIMEOUT_cnt;
	uint32_t	xact_RXERR_cnt;
	uint32_t	xact_PID_ERR_cnt;
	uint32_t	xact_UVC_H_ERR_cnt;
	uint32_t	xact_CRC16_ERR_cnt;
	uint32_t	babblecnt;
	uint32_t	underflowcnt;
	uint32_t	discardcnt;
	uint32_t	errdiscardcnt;	
	char		fmt[8];
	char		width[8];
	char		height[8];	
	char		fps[8];	
	uint32_t	debug_msg:1;
	uint32_t	sd_record:1;
	uint32_t	usb_preview:1;
	uint32_t	lcm_preview:1;
	uint32_t	isH264:1;
	uint32_t	isIFrame:1;
	uint32_t	isOpen:1;
	uint32_t	reserve:25;
	
}USBH_UVC_APP_STREAM_STRUCTURE;

typedef struct
{  
	USBH_UVC_APP_STREAM_STRUCTURE	stream[max_stream_count];	
}USBH_UVC_APP_DEV_STRUCTURE;

typedef struct
{
	uint32_t 			sd_record_enable;	
	uint32_t 			usb_preview_enable;		
	uint32_t 			lcm_preview_enable;	
	uint32_t 			debug_enable;	
	uint32_t 			err_debug_enable;
	USBH_UVC_APP_DEV_STRUCTURE	dev[10];
	
}USBH_UVC_APP_STRUCTURE;

extern USBH_UVC_APP_STRUCTURE usbh_uvc_app;

extern TaskHandle_t	xTASK_HDL_UVC_STREAM_XFR;
extern QueueHandle_t		usbh_queue_uvc_mw;
extern QueueHandle_t		usbh_queue_uvc_app;

typedef struct
{
	uint8_t		bED_Length;
	uint8_t		bED_bDescriptorType;
	uint8_t		bED_EndpointAddress;
	uint8_t		bED_bmAttributes;
	uint8_t		bED_wMaxPacketSizeLowByte;
	uint8_t		bED_wMaxPacketSizeHighByte;
	uint8_t		bED_Interval;
	uint8_t		bED_Toggle;

}USBH_EndPoint_Descriptor_Struct;

typedef struct
{
	uint8_t		bClass_LENGTH;
	uint8_t		bClaNumberss;
	uint8_t		bClassVerLowByte;
	uint8_t		bClassVerHighByte;
	uint8_t		bCityNumber;
	uint8_t		bFollowDescriptorNum;
	uint8_t		bReport;
	uint8_t		bLengthLowByte;
	uint8_t		bLengthHighByte;

}USBH_Class_Descriptor_Struct;

typedef struct
{
	uint8_t bINTERFACE_LENGTH;		// bLength
	uint8_t bDT_INTERFACE;			// bDescriptorType INTERFACE
	uint8_t bInterfaceNumber;		// bInterfaceNumber
	uint8_t bAlternateSetting;		// bAlternateSetting
	uint8_t bEP_NUMBER;			// bNumEndpoints(excluding endpoint	zero)
	uint8_t bInterfaceClass;		// bInterfaceClass
	uint8_t bInterfaceSubClass;		// bInterfaceSubClass
	uint8_t bInterfaceProtocol;		// bInterfaceProtocol
	uint8_t bInterface;			// iInterface

	//USBH_Interface_Association_Descriptor_Struct  ASSOCDes;	    
	USBH_Class_Descriptor_Struct		CLASSDes[USBH_CLASS_NUM_MAX];
	USBH_EndPoint_Descriptor_Struct		ENDPDes[USBH_ENDPOINT_NUM_MAX];	   
	
}USBH_Interface_Descriptor_Struct;

typedef struct
{
	uint16_t	SelfPowered:1;
	uint16_t	RemoteWakeup:1; 
	uint16_t	RESERVE:14;	
	
}USBH_STANDARD_STATUS_Struct;

typedef struct usbh_configuration_descriptor_struct
{
	uint8_t  bCONFIG_LENGTH;		// bLength
	uint8_t  bDT_CONFIGURATION;		// bDescriptorType CONFIGURATION
	uint8_t  bTotalLengthLowByte;		// wTotalLength, include all descriptors
	uint8_t  bTotalLengthHighByte;
	uint8_t  bINTERFACE_NUMBER;		// bNumInterface
	uint8_t  bConfigurationValue;		// bConfigurationValue
	uint8_t  bConfiguration;		// iConfiguration
	uint8_t  bAttribute;			// bmAttribute
	uint8_t  bMaxPower;			// iMaxPower (2mA	units)
	USBH_Interface_Descriptor_Struct	Interface[USBH_INTERFACE_NUM_MAX];

}USBH_Configuration_Descriptor_Struct;

typedef struct usbh_device_descriptor_struct
{
	uint8_t bDEVICE_LENGTH;		// bLength
	uint8_t bDT_DEVICE;		// bDescriptorType
	uint8_t bVerLowByte;		// bcdUSB
	uint8_t bVerHighByte;

	uint8_t bDeviceClass;		// bDeviceClass
	uint8_t bDeviceSubClass;	// bDeviceSubClas;
	uint8_t bDeviceProtocol;	// bDeviceProtocol
	uint8_t bEP0MAXPACKETSIZE;	// bMaxPacketSize0

	uint8_t bVIDLowByte;		// idVendor
	uint8_t bVIDHighByte;
	uint8_t bPIDLowByte;		// idProduct
	uint8_t bPIDHighByte;
	uint8_t bRNumLowByte;		// bcdDeviceReleaseNumber
	uint8_t bRNumHighByte;

	uint8_t bManufacturer;		// iManufacturer
	uint8_t bProduct;		// iProduct
	uint8_t bSerialNumber;		// iSerialNumber
	uint8_t bCONFIGURATION_NUMBER;	// bNumConfigurations	  
}USBH_Device_Descriptor_Struct;

typedef struct
{
	uint32_t	size;
	void		*ptr;

}USBH_REOSURCE_BUFF_Struct;

typedef struct
{
	//<1>.ENUM Information
	uint32_t				device_id;
	uint32_t				ENUM_SM;					//!< Enum State Machine
	uint32_t				SPEED;						//!< USB Speed , High speed	, Full speed or	Low speed
	uint32_t				bDevIsConnected;				//!< Reserve for HUB Class
	uint32_t				bAdd;						//!< Device	Address
	uint32_t				CLASS_DRV;					//!< Class Driver attribute
	void					*CLASS_STRUCT_PTR;				//!< Class Structure pointer
	USBH_REOSURCE_BUFF_Struct		BUFF;

	//<2>.Descriptor Information
	USBH_Device_Descriptor_Struct		DEVDes;						//!< USB Device Descriptor 
	USBH_Configuration_Descriptor_Struct	CFGDes[USBH_CONFIGURATION_NUM_MAX];		//!< USB Configuration descriptor
	USBH_STANDARD_STATUS_Struct		STDSTS;

	//<3>.String  Information
	uint8_t					bStringLanguage[bStringLanguageLength];				//!< USB String language
	uint8_t					bStringManufacture[bStringManufactureLength];	//!< USB String Manufacture
	uint8_t					bStringProduct[bStringProductLength];		//!< USB String Product
	uint8_t					bStringSerialN[bStringSerialNLength];		//!< USB String Serial Number
}USBH_Device_Structure;

typedef	struct
{
	uint32_t	dev_id;		
	uint32_t	stream_id;	
	uint32_t	size;
	uint32_t	*ptr;
	uint32_t	ring_buff_end;
	uint32_t	ring_buff_start;
	uint32_t	xact_err;
	uint32_t	babble;
	uint32_t	discard;
	uint32_t	underflow;
	uint32_t	errdiscard;
	uint32_t	stream_xfr_type;
}USBH_UVC_MW_STRUCTURE;

typedef	struct
{
	uint32_t	cmd;
	uint32_t	size;
	uint32_t	block_len;
	uint32_t	act_size;
	uint8_t		*pbuff;	
	uint32_t	lba;
	uint8_t		cbw[31];
	uint8_t		csw[13];
	uint8_t		lun;
}MSC_REQ_Struct;

typedef	struct AUTO_BKIN{
	uint32_t	base_addr;
	uint32_t	boundary_addr;
	uint32_t	start_addr;
	uint8_t		xfr_cnt;
	uint8_t		buff_full;
}AUTO_BKIN_STRUCTURE;

typedef	struct
{
	uint32_t		CMD;
	uint32_t		SIZE;
	uint32_t		*pBUFF;	
	uint8_t			CLASS[8];
	uint16_t		wValue;
	uint16_t		wIndex;
	uint16_t		TimeoutMsec;
	
}USBH_CX_XFR_REQ_Struct;

typedef	struct usbh_bk_xfr_req_struct
{
	uint32_t		XfrType;
	uint32_t		NUM;
	uint32_t		SIZE;
	uint32_t		*pBUFF;
	uint32_t		TimeoutMsec;
}USBH_BK_XFR_REQ_Struct;

void usbh_freertos_init(void);
void usbh_freertos_init_2(void);
uint32_t usbh_uvc_init(uint8_t ID);

uint32_t uvc_get_info(void);
uint32_t uvc_start(USBH_Device_Structure *UVC_DEV, uint8_t bFmt, uint8_t bRes, uint32_t dwFps, uint32_t *ptr, uint32_t size, uint8_t intf_start_index);
void uvc_stream_complete(USBH_UVC_MW_STRUCTURE	*usbh_uvc_mw);
uint8_t uvc_streamid_to_devid(uint8_t stream_id);
void uvc_unregister_streamid(USBH_Device_Structure *UVC_DEV, uint32_t dwStreamID);
uint8_t	uvc_stop(USBH_Device_Structure *UVC_DEV, uint8_t stream_id);
void uvc_clean_bk_buf(void);
uint8_t uvc_still_trigger(USBH_Device_Structure *UVC_DEV);

uint32_t usbh_msc_init(uint8_t ID);
uint8_t msc_ready(uint8_t device_id);
uint8_t msc_get_capacity(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req);
uint8_t msc_read(USBH_Device_Structure *dev, void *pdata_buf, uint32_t size, uint32_t lba);
uint8_t msc_write(USBH_Device_Structure *dev, void *pdata_buf, uint32_t size, uint32_t lba);

uint8_t	usbh_cx_xfr(USBH_Device_Structure *DEV, USBH_CX_XFR_REQ_Struct *CX_REQ);
uint8_t usbh_bk_xfr(USBH_Device_Structure *DEV, USBH_BK_XFR_REQ_Struct *BK_REQ);

uint32_t usbh_wifi_init(uint8_t ID);
AUTO_BKIN_STRUCTURE get_auto_bkin_struct(void);
void set_auto_bkin_processed_cnt(int xfr_cnt);
void usbh_auto_bkin_init(void);
void usbh_auto_bkin_uninit(void);




#endif //USBH__H
