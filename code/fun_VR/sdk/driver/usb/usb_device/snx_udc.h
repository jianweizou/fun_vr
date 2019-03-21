/**
*
* this is usb device driver file
* snx_udc.h
* author IP2/Luka
*/
#ifndef __SONIX_UDC_H
#define __SONIX_UDC_H

#include "sonix_config.h"

#include "ch9.h"
#include "composite.h"
#if defined( CONFIG_PLATFORM_SN7320 )
//#include "snc7320.h"
#include "snc_types.h"
#endif

#if defined(CONFIG_DMA_MULTI_MODE_ENABLE)
#define USBD_MULTI_LINK_WORKAROUND				1
#endif

//#define USBD_REG_BASE                           0x94400000
#define SN_USB_DEV_BASE									0x40100000UL	//SNC7320 NEW
#define USBD_REG_BASE							SN_USB_DEV_BASE
#define U32_BIT(BIT_INDEX_)                     ((unsigned int)(1ul << (BIT_INDEX_)))
#define U32_MASK(OFFSET_, WIDTH_)               ((unsigned int)(~(((WIDTH_) < 32 ? 0xFFFFFFFFul : 0x00ul) << (WIDTH_))  << (OFFSET_)))

// --------------------- Define max endpoint & interface number ----------------------------------
#define MAX_NUM_OF_ENDPOINTS_PAIRS              6
#define MAX_NUM_OF_INTERFACES_PER_CONFIGURATION 3

// --------------------- Define on-chip ram allocation ----------------------------------
//system register reserve 1k
#if defined(CONFIG_PLATFORM_ST53510)
// buffering for EP OUT
#define ENDPOINT_BUFFERING_0_0                  0//512b
#define ENDPOINT_BUFFERING_1_0                  0
#define ENDPOINT_BUFFERING_2_0                  0
#define ENDPOINT_BUFFERING_3_0                  0
#define ENDPOINT_BUFFERING_4_0                  1//iso out 1k
#define ENDPOINT_BUFFERING_5_0                  0
#define ENDPOINT_BUFFERING_6_0                  0
#define ENDPOINT_BUFFERING_7_0                  0
#define ENDPOINT_BUFFERING_8_0                  0
#define ENDPOINT_BUFFERING_9_0                  0
#define ENDPOINT_BUFFERING_10_0                 0
#define ENDPOINT_BUFFERING_11_0                 0
#define ENDPOINT_BUFFERING_12_0                 0
#define ENDPOINT_BUFFERING_13_0                 1//buk out 1k
#define ENDPOINT_BUFFERING_14_0                 0
#define ENDPOINT_BUFFERING_15_0                 0

// buffering for EP IN
#define ENDPOINT_BUFFERING_0_1                  0//512b
#define ENDPOINT_BUFFERING_1_1                  0//int 512b
#define ENDPOINT_BUFFERING_2_1                  0
#define ENDPOINT_BUFFERING_3_1                  1//iso in 1k
#define ENDPOINT_BUFFERING_4_1                  0
#define ENDPOINT_BUFFERING_5_1                  0//int 512b
#define ENDPOINT_BUFFERING_6_1                  0
#define ENDPOINT_BUFFERING_7_1                  2//iso 1k
#define ENDPOINT_BUFFERING_8_1                  0
#define ENDPOINT_BUFFERING_9_1                  0
#define ENDPOINT_BUFFERING_10_1                 0
#define ENDPOINT_BUFFERING_11_1                 0
#define ENDPOINT_BUFFERING_12_1                 1//buk in 1k
#define ENDPOINT_BUFFERING_13_1                 0
#define ENDPOINT_BUFFERING_14_1                 1//buk in 1k
#define ENDPOINT_BUFFERING_15_1                 0


#elif defined(CONFIG_PLATFORM_SN98660)
#define ENDPOINT_BUFFERING_0_0                  0//512b
#define ENDPOINT_BUFFERING_1_0                  0
#define ENDPOINT_BUFFERING_2_0                  1//buk out 1k
#define ENDPOINT_BUFFERING_3_0                  0
#define ENDPOINT_BUFFERING_4_0                  0
#define ENDPOINT_BUFFERING_5_0                  1//buk out 1k
#define ENDPOINT_BUFFERING_6_0                  0
#define ENDPOINT_BUFFERING_7_0                  0
#define ENDPOINT_BUFFERING_8_0                  0
#define ENDPOINT_BUFFERING_9_0                  0
#define ENDPOINT_BUFFERING_10_0                 0
#define ENDPOINT_BUFFERING_11_0                 0
#define ENDPOINT_BUFFERING_12_0                 0
#define ENDPOINT_BUFFERING_13_0                 0
#define ENDPOINT_BUFFERING_14_0                 0
#define ENDPOINT_BUFFERING_15_0                 0

// buffering for EP IN
#define ENDPOINT_BUFFERING_0_1                  0//512b
#define ENDPOINT_BUFFERING_1_1                  1//in 1k
#define ENDPOINT_BUFFERING_2_1                  1//in 1k
#define ENDPOINT_BUFFERING_3_1                  0//int 512b
#define ENDPOINT_BUFFERING_4_1                  0
#define ENDPOINT_BUFFERING_5_1                  0
#define ENDPOINT_BUFFERING_6_1                  0
#define ENDPOINT_BUFFERING_7_1                  0
#define ENDPOINT_BUFFERING_8_1                  0
#define ENDPOINT_BUFFERING_9_1                  0
#define ENDPOINT_BUFFERING_10_1                 0
#define ENDPOINT_BUFFERING_11_1                 0
#define ENDPOINT_BUFFERING_12_1                 0
#define ENDPOINT_BUFFERING_13_1                 0
#define ENDPOINT_BUFFERING_14_1                 0
#define ENDPOINT_BUFFERING_15_1                 0


#elif defined(CONFIG_PLATFORM_SN7320) 
// buffering for EP OUT
#define ENDPOINT_BUFFERING_0_0                  0//512b
#define ENDPOINT_BUFFERING_1_0                  0
#define ENDPOINT_BUFFERING_2_0                  0
#define ENDPOINT_BUFFERING_3_0                  0
#define ENDPOINT_BUFFERING_4_0                  1//iso out 1k
#define ENDPOINT_BUFFERING_5_0                  0
#define ENDPOINT_BUFFERING_6_0                  0
#define ENDPOINT_BUFFERING_7_0                  0
#define ENDPOINT_BUFFERING_8_0                  0
#define ENDPOINT_BUFFERING_9_0                  0
#define ENDPOINT_BUFFERING_10_0                 0
#define ENDPOINT_BUFFERING_11_0                 0
#define ENDPOINT_BUFFERING_12_0                 0
#define ENDPOINT_BUFFERING_13_0                 1//buk out 1k
#define ENDPOINT_BUFFERING_14_0                 0
#define ENDPOINT_BUFFERING_15_0                 1//buk out 1k

// buffering for EP IN
#define ENDPOINT_BUFFERING_0_1                  0//512b
#define ENDPOINT_BUFFERING_1_1                  0//int 512b
#define ENDPOINT_BUFFERING_2_1                  0
#define ENDPOINT_BUFFERING_3_1                  1//iso in 1k
#define ENDPOINT_BUFFERING_4_1                  0
#define ENDPOINT_BUFFERING_5_1                  0//int 512b
#define ENDPOINT_BUFFERING_6_1                  0
#define ENDPOINT_BUFFERING_7_1                  2//iso 1k
#define ENDPOINT_BUFFERING_8_1                  0
#define ENDPOINT_BUFFERING_9_1                  0
#define ENDPOINT_BUFFERING_10_1                 0
#define ENDPOINT_BUFFERING_11_1                 0
#define ENDPOINT_BUFFERING_12_1                 1//buk in 1k
#define ENDPOINT_BUFFERING_13_1                 0
#define ENDPOINT_BUFFERING_14_1                 1//buk in 1k
#define ENDPOINT_BUFFERING_15_1                 0
#endif

//========================================================================================
// --------------------- Events codes ----------------------------------
#define USBEV_EVENT_DEV_UNCONFIGURE             0
#define USBEV_EVENT_DEV_CONFIGURE               1
#define USBEV_EVENT_DEV_DISCONNECT              2
#define USBEV_EVENT_DEV_CONNECT                 3
#define USBEV_EVENT_HOT_RESET                   4
#define USBEV_EVENT_WARM_RESET                  5
#define USBEV_EVENT_U1_ENTER                    6
#define USBEV_EVENT_U1_EXIT                     7
#define USBEV_EVENT_U2_ENTER                    8
#define USBEV_EVENT_U2_EXIT                     9
#define USBEV_EVENT_U3_ENTER                    10
#define USBEV_EVENT_U3_EXIT                     11
#define USBEV_EVENT_SELECT_INTERFACE            12
#define USBEV_EVENT_ITP_SOF                     13
#define USBEV_EVENT_EP_PRIME                    14
#define USBEV_EVENT_EP_START                    15
#define USBEV_EVENT_EP_STOP                     16
#define USBEV_EVENT_EP_REJECT                   17
#define USBEV_EVENT_EP_CLEAR_STALL              18
#define USBEV_EVENT_SID_ERROR                   19
#define USBEV_EVENT_CLEAR_FEATURE_ENDPOINT      20
#define USBEV_EVENT_L2_ENTER                    21
#define USBEV_EVENT_L1_ENTER                    22
#define USBEV_EVENT_L2_EXIT                     23
#define USBEV_EVENT_L1_EXIT                     24
#define USBEV_EVENT_WAKE_INT                    25

// ------------------ Status codes --------------------------------------
#define USBEV_STATUS_SUCCESS                    0
#define USBEV_STATUS_SUCCESS_NRDY               0x80
#define USBEV_STATUS_CANCELED                   1
#define USBEV_STATUS_DATA_OVERFLOW              2
#define USBEV_STATUS_NO_OPENED                  3
#define USBEV_STATUS_ENDPOINT_STALLED           4
#define USBEV_STATUS_PENDING                    5
#define USBEV_STATUS_STALL                      6

// ------------------ Status codes --------------------------------------
#define TRANSFER_STATUS_SUCCESS                 0
#define TRANSFER_STATUS_TRBERR                  1
#define TRANSFER_STATUS_OUTSMM                  2

#define FLSUH_STATUS_SUCCESS                    0
#define FLUSH_STATUS_TIMEOUT                    1

// --------------- Types definitions ------------------------------------

#define EP_MSC_BULK_IN                          0x81
#define EP_MSC_BULK_OUT                         0x02

//==========================================================

#define USBRM_EP_ENABLE                         U32_MASK(0, 1)
#define USBRM_EP_TYPE                           U32_MASK(1, 2)

//==========================================================
// USB Device test command
#define USBD_TEST_J                             1
#define USBD_TEST_K                             2
#define USBD_TEST_SE0_NAK                       3
#define USBD_TEST_PACKET                        4
#define USBD_TEST_FORCE_ENABLE                  5

// Values
#define USBRV_TM_TEST_J                         0ul
#define USBRV_TM_TEST_K                         1ul
#define USBRV_TM_SE0_NAK                        2ul
#define USBRV_TM_TEST_PACKET                    3ul

#define USBRF_TESTMODE_ENABLE                   U32_BIT(9)
#define USBRM_TESTMODE_SEL                      U32_MASK(10, 2)
#define USBRD_SET_TESTMODE(TESTMODE_)           (((TESTMODE_) << 10 & USBRM_TESTMODE_SEL) | USBRF_TESTMODE_ENABLE)
#define USBRD_GET_TESTMODE(USBR_CMD_)           ((UINT32)(((USBR_CMD_) & USBRM_TESTMODE_SEL)) >> 10)
//==========================================================

// Values
#define USBRV_EP_DISABLED                       0ul
#define USBRV_EP_ENABLED                        1ul
// Defines
#define USBRD_EP_ENABLE(EP_ENABLE_VAL_)         ((EP_ENABLE_VAL_) & USBRM_EP_ENABLE)
#define USBRD_EP_ENABLED                        USBRD_EP_ENABLE(USBRV_EP_ENABLED)
#define USBRD_EP_DISABLED                       USBRD_EP_ENABLE(USBRV_EP_DISABLED)
#define USBRD_EP_TYPE(EP_TYPE_VAL_)             ((EP_TYPE_VAL_) << 1 & USBRM_EP_TYPE)
#define USBRD_EP_CONTROL                        USBRD_EP_TYPE(USBRV_EP_CONTROL)
#define USBRD_EP_ISOCHRONOUS                    USBRD_EP_TYPE(USBRV_EP_ISOCHRONOUS)
#define USBRD_EP_BULK                           USBRD_EP_TYPE(USBRV_EP_BULK)
#define USBRD_EP_INTERRUPT                      USBRD_EP_TYPE(USBRV_EP_INTERRUPT)
#ifdef USB3_SUPPORT
#define USBRD_EP_ENDIAN(ENDIAN_VAL_)            ((ENDIAN_VAL_) << 7 & USBRM_EP_ENDIAN)
#define USBRD_EP_LITTLE_ENDIAN                  USBRD_EP_ENDIAN(USBRV_EP_LITTLE_ENDIAN)
#define USBRD_EP_BIG_ENDIAN                     USBRD_EP_ENDIAN(USBRV_EP_BIG_ENDIAN)
#endif
#define USBRD_EP_MAXBURST(MAX_BURST_VAL_)       ((MAX_BURST_VAL_) << 8 & USBRM_EP_MAXBURST)
#define USBRD_EP_MULT(MULT_VAL_)                ((MULT_VAL_) << 14 & USBRM_EP_MULT)
#define USBRD_EP_MAXPKSIZE(MAX_PK_SIZE_VAL_)    (((uint32_t)MAX_PK_SIZE_VAL_) << 16 & USBRM_EP_MAXPKSIZE)
#define USBRD_EP_BUFFERING(BUFFERING_VAL_)      (((uint32_t)BUFFERING_VAL_) << 27 & USBRM_EP_BUFFERING)

#define USBRD_GET_EP_TYPE(EP_CFG_REG_VAL_)      (((EP_CFG_REG_VAL_) & USBRM_EP_TYPE) >> 1)
#ifdef USB3_SUPPORT
#define USBRD_GET_EP_ENDIAN(EP_CFG_REG_VAL_)    (((EP_CFG_REG_VAL_) & USBRM_EP_ENDIAN) >> 7)
#endif
#define USBRD_GET_EP_MAXBURST(EP_CFG_REG_VAL_)  (((EP_CFG_REG_VAL_) & USBRM_EP_MAXBURST) >> 8)
#define USBRD_GET_EP_MULT(EP_CFG_REG_VAL_)      (((EP_CFG_REG_VAL_) & USBRM_EP_MULT) >> 14)
#define USBRD_GET_EP_MAXPKSIZE(EP_CFG_REG_VAL_) (((EP_CFG_REG_VAL_) & USBRM_EP_MAXPKSIZE) >> 16)
#define USBRD_GET_EP_BUFFERING(EP_CFG_REG_VAL_) (((EP_CFG_REG_VAL_) & USBRM_EP_BUFFERING) >> 27)

#define USBRM_EP_MAXBURST                       U32_MASK(8, 4)
#define USBRM_EP_MULT                           U32_MASK(14, 2)
#define USBRM_EP_MAXPKSIZE                      U32_MASK(16, 11)
#define USBRM_EP_BUFFERING                      U32_MASK(27, 5)

// Masks
#define USBRM_EP_NUM                            U32_MASK(0, 4)
#define USBRM_EP_DIR                            U32_MASK(7, 1)
#define USBRM_EP_ADDR                           (USBRM_EP_DIR | USBRM_EP_NUM)

// Endpoint Data transfer direction
#define USB_EP_OUT                              (0 << 7)
#define USB_EP_IN                               (1 << 7)

#define USBRV_EP_CONTROL                        0
#define USBRV_EP_ISOCHRONOUS                    1
#define USBRV_EP_BULK                           2
#define USBRV_EP_INTERRUPT                      3

#define USBRM_USBSPEED                          U32_MASK(4, 3)
#define USBRM_LPMST                             U32_MASK(18, 2)


// Defines
#define USBRD_DRBL(EP_NUM_, EP_DIR_)            (1 << (EP_NUM_) << ((EP_DIR_) ? 16 : 0))


#define get_ep_num(EPADDR_)                     ((EPADDR_) & 0x0F)

#define get_ep_dir(EPADDR_)                     ((EPADDR_) >> 7 & 0x01)

#define USBD_GET_EPADDR(EPNUM_, EPDIR_)         (((EPNUM_) & 0x0F) | ((EPDIR_) << 7 & 0x80))


#define USBRD_GET_FADDR(STS_REG_VAL_)           (unsigned int)(((STS_REG_VAL_) & USBRM_FADDR) >> 8)


// Defines
#define USBRD_EP_INTERRUPT_EN(EP_NUM_, EP_DIR_) (1 << (EP_NUM_) << ((EP_DIR_) ? 16 : 0))


/*************/
/*      defined  */
/*************/
#define USBD_EP_TOTAL_NUMBER                    16

/*************/
/*      enum     */
/*************/
typedef enum _usbd_speed_mode {
	UNDEFINED_MODE,
	USBD_LS_MODE,
	USBD_FS_MODE,
	USBD_HS_MODE,
	USBD_SS_MODE
} usbd_speed_mode;

enum ep0_state {
	EP0_SETUP_PHASE,
	EP0_IN_DATA_PHASE,
	EP0_OUT_DATA_PHASE,
	EP0_END_XFER_PHASE,
	EP0_STALL,
};

// EPX state Flags

#define USBD_EP_STATE_TOC                       U32_BIT(0)      /*      transaction on complete */
#define USBD_EP_STATE_IN                        U32_BIT(1)
#define USBD_EP_STATE_OUT                       U32_BIT(2)
#define USBD_EP_STATE_ERR                       U32_BIT(3)
#define USBD_EP_STATE_EMPTY                     U32_BIT(4)
#define USBD_EP_STATE_BUSY                      U32_BIT(5)
#define USBD_EP_STATE_TFOC                      U32_BIT(6)      /*      transfer on complete */
#define USBD_EP_STATE_0_PACKET_TAIL             U32_BIT(7)
#define USBD_EP_STATE_ISOERR					U32_BIT(8)
#define USBD_EP_STATE_TSTOP						U32_BIT(9)		/*		transfer stop		*/ //erk+
#define USBD_EP_STATE_UVC_ERR                   U32_BIT(24)
#define USBD_EP_STATE_UVC_EOF                   U32_BIT(25)
#define USBD_EP_STATE_UVC_FID                   U32_BIT(26)
#define USBD_EP_STATE_UVC_STILL                 U32_BIT(27)
#define USBD_EP_STATE_DISABLE                   U32_BIT(31)



/*************/
/*      struct   */
/*************/


struct _sonix_udc;

struct snx_request;

struct _sonix_udc_ep {
	struct usb_ep                               *ep;
	struct _sonix_udc                           *dev;
	unsigned char                               num;
	unsigned char                               bEndpointAddress;
	//test
	struct snx_request                          *req;
};

struct _sonix_udc {
	struct usb_gadget                           gadget;
	struct _sonix_udc_ep                        ep[USBD_EP_TOTAL_NUMBER];
	struct usb_gadget_driver                    *driver;
	enum ep0_state                              ep0state;
};

struct snx_request {
	struct usb_request                          req;
};


// pointer to TRB for ep0 IN/OUT
typedef struct {
	uint32_t                                    data_buffer_pointer;
	uint32_t                                    length_and_burst;
	uint32_t                                    control;
} trb_struct_t;

int usb_gadget_register_driver(struct usb_gadget_driver *driver);
int usb_gadget_unregister_driver(struct usb_gadget_driver *driver);

/*************/
/*      function */
/*************/
void usbd_process( void *pvParameters );
void usbd_ep0_process( void *pvParameters );
//void udc_irq(void);

void udc_init(void);
void udc_disable(void);

void snx_udc_power_on(void);
void snx_udc_power_off(void);
void snx_udc_fw_reconnection(void);

struct usb_request *get_usbreq(struct usb_ep *ep);

/*************/
/*      variable */
/*************/
static unsigned int (*usbd_ep_function[USBD_EP_TOTAL_NUMBER])(unsigned int ep_sts,unsigned int dir);
static unsigned int usbd_ep0_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep1_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep2_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep3_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep4_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep5_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep6_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep7_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep8_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep9_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep10_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep11_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep12_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep13_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep14_function( unsigned int ep_sts, unsigned int dir);
static unsigned int usbd_ep15_function( unsigned int ep_sts, unsigned int dir);


//=================================================================================
// USBSS_DEV Registers
//=================================================================================

#define USBD_REG_CONF                           (USBD_REG_BASE + 0x00)
#define USBD_REG_STS                            (USBD_REG_BASE + 0x04)
#define USBD_REG_CMD                            (USBD_REG_BASE + 0x08)
#define USBD_REG_ITPN                           (USBD_REG_BASE + 0x0C)
#define USBD_REG_LPM                            (USBD_REG_BASE + 0x10)
#define USBD_REG_IEN                            (USBD_REG_BASE + 0x14)
#define USBD_REG_ISTS                           (USBD_REG_BASE + 0x18)
#define USBD_REG_EP_SEL                         (USBD_REG_BASE + 0x1C)
#define USBD_REG_EP_TRADDR                      (USBD_REG_BASE + 0x20)
#define USBD_REG_EP_CFG                         (USBD_REG_BASE + 0x24)
#define USBD_REG_EP_CMD                         (USBD_REG_BASE + 0x28)
#define USBD_REG_EP_STS                         (USBD_REG_BASE + 0x2C)
#define USBD_REG_EP_STS_SID                     (USBD_REG_BASE + 0x30)
#define USBD_REG_EP_STS_EN                      (USBD_REG_BASE + 0x34)
#define USBD_REG_DRBL                           (USBD_REG_BASE + 0x38)
#define USBD_REG_EP_IEN                         (USBD_REG_BASE + 0x3C)
#define USBD_REG_EP_ISTS                        (USBD_REG_BASE + 0x40)
#define USBD_REG_SE0                            (USBD_REG_BASE + 0x70)
#define USBD_REG_MISC_CTRL                      (USBD_REG_BASE + 0x78)
#define USBD_REG_EP_NEW_MAXPKTSIZE              (USBD_REG_BASE + 0x7C)
#define USBD_REG_ISTS_CLR                       (USBD_REG_BASE + 0x80)
#define USBD_REG_EP_STS_CLR                     (USBD_REG_BASE + 0x84)
#define USBD_REG_RAM_SIZE                       (USBD_REG_BASE + 0x88)
//=================================================================================
// USB_CONF - Global Configuration Register
//=================================================================================

// Flags
#define USBD_CONF_BIT_CFGRST                    U32_BIT(0)
#define USBD_CONF_BIT_CFGSET                    U32_BIT(1)
#define USBD_CONF_BIT_USBDIS                    U32_BIT(4)
#define USBD_CONF_BIT_LENDIAN                   U32_BIT(5)
#define USBD_CONF_BIT_BENDIAN                   U32_BIT(6)
#define USBD_CONF_BIT_SWRST                     U32_BIT(7)
#define USBD_CONF_BIT_DSING                     U32_BIT(8)
#define USBD_CONF_BIT_DMULT                     U32_BIT(9)
#define USBD_CONF_BIT_DMAOFFEN                  U32_BIT(10)
#define USBD_CONF_BIT_DMAOFFDS                  U32_BIT(11)
#define USBD_CONF_BIT_FORCEFSCLR                U32_BIT(12)
#define USBD_CONF_BIT_FORCEFSSET                U32_BIT(13)
#define USBD_CONF_BIT_DEVEN                     U32_BIT(14)
#define USBD_CONF_BIT_DEVDS                     U32_BIT(15)
#define USBD_CONF_BIT_L1EN                      U32_BIT(16)
#define USBD_CONF_BIT_L1DS                      U32_BIT(17)
#define USBD_CONF_BIT_CLK2OFFEN                 U32_BIT(18)
#define USBD_CONF_BIT_CLK2OFFDS                 U32_BIT(19)
#define USBD_CONF_BIT_LGO_L0                    U32_BIT(20)
//=================================================================================
// USB_IEN - USB Interrupt Enable Register
//=================================================================================

// Flags

#define USBD_IEN_BIT_ITPIEN                     U32_BIT(10)
#define USBD_IEN_BIT_WAKEIEN                    U32_BIT(11)
#define USBD_IEN_BIT_CON2IEN                    U32_BIT(16)
#define USBD_IEN_BIT_DIS2IEN                    U32_BIT(17)
#define USBD_IEN_BIT_UHSFSRESIEN                U32_BIT(18)
#define USBD_IEN_BIT_L2ENTIEN                   U32_BIT(20)
#define USBD_IEN_BIT_L2EXTIEN                   U32_BIT(21)
#define USBD_IEN_BIT_L1ENTIEN                   U32_BIT(24)
#define USBD_IEN_BIT_L1EXTIEN                   U32_BIT(25)
#define USBD_IEN_BIT_CFGRSESIEN                 U32_BIT(26)
#define USBD_IEN_BIT_REGTOIEN                   U32_BIT(27)


//=================================================================================
// USB_ISTS - USB Interrupt Status Register
//=================================================================================

// Flags

#define USBD_ISTS_BIT_ITPI                      U32_BIT(10)
#define USBD_ISTS_BIT_WAKEI                     U32_BIT(11)
#define USBD_ISTS_BIT_CON2I                     U32_BIT(16)
#define USBD_ISTS_BIT_DIS2I                     U32_BIT(17)
#define USBD_ISTS_BIT_U2RESI                    U32_BIT(18)
#define USBD_ISTS_BIT_L2ENTI                    U32_BIT(20)
#define USBD_ISTS_BIT_L2EXTI                    U32_BIT(21)
#define USBD_ISTS_BIT_L1ENTI                    U32_BIT(24)
#define USBD_ISTS_BIT_L1EXTI                    U32_BIT(25)
#define USBD_ISTS_BIT_CFGRESI                   U32_BIT(26)
#define USBD_ISTS_BIT_REGTOI                    U32_BIT(27)
//=================================================================================
// USB_EP_CMD - Endpoint Command Register
//=================================================================================

// Flags
#define USBD_EP_CMD_BIT_EPRST                   U32_BIT(0)
#define USBD_EP_CMD_BIT_SSTALL                  U32_BIT(1)
#define USBD_EP_CMD_BIT_CSTALL                  U32_BIT(2)
#define USBD_EP_CMD_BIT_ERDY                    U32_BIT(3)
#define USBD_EP_CMD_BIT_CMPL                    U32_BIT(5)
#define USBD_EP_CMD_BIT_DRDY                    U32_BIT(6)
#define USBD_EP_CMD_BIT_DFLUSH                  U32_BIT(7)
//=================================================================================
// USB_EP_STS - Endpoint Status Register
//=================================================================================
#define USBD_EP_STS_BIT_SETUP                   U32_BIT(0)
#define USBD_EP_STS_BIT_STALL                   U32_BIT(1)
#define USBD_EP_STS_BIT_IOC                     U32_BIT(2)
#define USBD_EP_STS_BIT_ISP                     U32_BIT(3)
#define USBD_EP_STS_BIT_DESCMIS                 U32_BIT(4)
#define USBD_EP_STS_BIT_TRBERR                  U32_BIT(7)
#define USBD_EP_STS_BIT_DBUSY                   U32_BIT(9)
#define USBD_EP_STS_BIT_BUFEMPTY                U32_BIT(10)
#define USBD_EP_STS_BIT_CCS                     U32_BIT(11)
#define USBD_EP_STS_BIT_OUTSMM                  U32_BIT(14)
#define USBD_EP_STS_BIT_ISOERR                  U32_BIT(15)
#define USBD_EP_STS_BIT_HOSTPP                  U32_BIT(16)
#define USBD_EP_STS_BIT_OUTQ_VAL                U32_BIT(28)
#define USBD_EP_STS_BIT_STPWAIT                 U32_BIT(31)
//=================================================================================
// EP_STS_EN - Endpoint Status Register Enable
//=================================================================================

// Flags
#define USBD_EP_STS_BIT_SETUPEN                 U32_BIT(0)
#define USBD_EP_STS_BIT_DESCMISEN               U32_BIT(4)
#define USBD_EP_STS_BIT_TRBERREN                U32_BIT(7)
#define USBD_EP_STS_BIT_OUTSMMEN                U32_BIT(14)
#define USBD_EP_STS_BIT_ISOERREN                U32_BIT(15)
#define USBD_EP_STS_BIT_STPWAITEN               U32_BIT(31)
//=================================================================================
// EP_IEN - Endpoints Interrupt Enable Register
//=================================================================================

// Flags

#define USBD_EP_IEN_BIT_EP0_OUT                 U32_BIT(0)
#define USBD_EP_IEN_BIT_EP0_IN                  U32_BIT(16)

//=================================================================================
// USBD_REG_MISC_CTRL - Control Register, Optional by Project
//=================================================================================
#define USBD_MISC_CTRL_BIT_HLENO               	U32_BIT(3)
#define USBD_MISC_CTRL_BIT_HEADER2B            	U32_BIT(4)
#define USBD_MISC_CTRL_BIT_SCR_PTS_EXIST        U32_BIT(5)
#define USBD_MISC_CTRL_BIT_FRAME_SRC_EN         U32_BIT(6)
#define USBD_MISC_CTRL_BIT_UVC_ISO_EN       	U32_BIT(7)
#define USBD_MISC_CTRL_BIT_RETRY_EN             U32_BIT(8)
#define USBD_MISC_CTRL_BIT_REG_ACC_TO_EN		U32_BIT(16)
#define USBD_MISC_CTRL_BIT_TMDBPS_EN            U32_BIT(26)


//=================================================================================
// USBD_REG_EP_NEW_MAXPKTSIZE - Set EP New Max Packet Size
//=================================================================================
#define USBD_EP_NEW_MAXPKTSIZE_EN               U32_BIT(31)

//-----------------------------------------------------------------------------
// TRB -
//-----------------------------------------------------------------------------
// Flags
#define TRB_C                                   U32_BIT(0)
#define TRB_TC                                  U32_BIT(1)
#define TRB_ISP                                 U32_BIT(2)
#define TRB_F                                   U32_BIT(3)
#define TRB_CH                                  U32_BIT(4)
#define TRB_IOC                                 U32_BIT(5)
#define UVC_ERR          						U32_BIT(6)
#define UVC_EOF            						U32_BIT(7)
#define UVC_FID            						U32_BIT(8)
#define UVC_STILL          						U32_BIT(9)
#define TRB_NORMAL                              U32_BIT(10)
#define TRB_LINK                                (U32_BIT(11)|U32_BIT(12))
#define TRB_BurstLength_Single                  U32_BIT(24)
#define TRB_BurstLength_INCR4                   U32_BIT(26)
#define TRB_BurstLength_INCR8                   U32_BIT(27)
#define TRB_BurstLength_INCR16                  U32_BIT(28)

#define TRB_MULTI_LINK_CNT						14
#define TRB_MAX_SIZE                            65536
#define TRB_BurstLength                         TRB_BurstLength_INCR16

// need to fix
#define USBRD_GET_USBSPEED(STS_REG_VAL_)        ((unsigned int)((STS_REG_VAL_) & USBRM_USBSPEED) >> 4)

struct _usb_irq_status {
	uint32_t usb_event_flag;
	uint32_t eps_event_flag;
	uint32_t ep_status_flag;
};

#endif
