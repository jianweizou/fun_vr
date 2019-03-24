#include "sonix_config.h"

#define UAC_VERSION                                 0x0100
#define UAC_MAX_REQUEST_SIZE                        64

#define UAC_IN_INPUT_TERMINAL_ID                    1
#define UAC_IN_FEATURE_UNIT_ID                      2
#define UAC_IN_OUTPUT_TERMINAL_ID                   3
#define UAC_OUT_INPUT_TERMINAL_ID                   4
#define UAC_OUT_FEATURE_UNIT_ID                     5
#define UAC_OUT_OUTPUT_TERMINAL_ID                  6

#define UAC_INTF_CONTROL                            0
#define UAC_INTF_IN_STREAMING                       1
#define UAC_INTF_OUT_STREAMING                      2
#define UAC_INTF_MAX                      			(2+USBD_UAC_ISOC_OUT_SUPPORT)

#define UAC_STRING_ASSOCIATION_IDX                  0
#define UAC_STRING_CONTROL_IDX                      1
#define UAC_STRING_IN_STREAMING_ALT0_IDX            2
#define UAC_STRING_IN_STREAMING_ALT1_IDX            3
#define UAC_STRING_OUT_STREAMING_ALT0_IDX           4
#define UAC_STRING_OUT_STREAMING_ALT1_IDX           5

#define UAC_VENDOR_ID                               0x0C45  /* Sonix Technology Co. */
#define UAC_PRODUCT_ID                              0x8203  /* SN98660 UAC */
#define UAC_DEVICE_BCD                              0x0100  /* 1.00 */

static char uac_vendor_label[]  = "Sonix Technology Co., Ltd.";
static char uac_product_label[] = "SN7320 Audio";
static char uac_config_label[]  = "USB Audio";

#define UAC_DT_HEADER_LENGTH                10
#define UAC_DT_INUT_TERMINAL_LENGTH         12
#define UAC_DT_FEATURE_UNIT_LENGTH          (7+(2*2))
#define UAC_DT_OUTPUT_TERMINAL_LENGTH       9
#define USB_DT_STREAMING_CLS_LENGTH         7

#define UAC_IN_CHANNEL_MAX                  1       //6ch:6  2ch:2	
#define UAC_IN_LOGICAL_CHANNEL              1       //=(UAC_IN_CHANNEL_MAX>>1)
#define UAC_IN_SAMPLE_BIT_LENGTH            16      //6ch:24  2ch:16	
#define UAC_IN_EP_MAX_PACKET_SIZE           34     //= UAC_IN_CHANNEL_MAX*UAC_IN_SAMPLE_BIT_LENGTH/8*(MaxSamplingRate/1000+1)
#define	UAC_IN_MAX_PAYLOAD_SIZE             64*1000
#define	UAC_IN_MAX_SAMPLING_FREQ          	16000


#define UAC_OUT_CHANNEL_MAX                 2 
#define UAC_OUT_SAMPLE_BIT_LENGTH           16
#define UAC_OUT_EP_MAX_PACKET_SIZE          196
#define UAC_OUT_XFR_CNT                     100
#define UAC_OUT_MAX_PAYLOAD_SIZE            (UAC_OUT_EP_MAX_PACKET_SIZE*UAC_OUT_XFR_CNT)
#define	UAC_OUT_MAX_SAMPLING_FREQ          	48000


static struct usb_qualifier_descriptor	uac_qualifier_desc	=	{
    sizeof(struct usb_qualifier_descriptor),    // bLength
    USB_DT_DEVICE_QUALIFIER,                    // bDescriptorType
    0x0200,                                     // bcdUSB
    USB_CLASS_MISC,                             // bDeviceClass
    USB_CLASS_COMM,                             // bDeviceSubClass
    0x01,                                       // bDeviceProtocol
    0,                                          // bMaxPacketSize0      /* dynamic */
    0,                                          // bNumConfigurations   /* dynamic */
    0                                           // bRESERVED;
};


static struct usb_device_descriptor uac_device_desc = {
    sizeof(uac_device_desc),        // bLength
    USB_DT_DEVICE,                  // bDescriptorType
    0x0200,                         // bcdUSB
    USB_CLASS_MISC,                 // bDeviceClass
    USB_CLASS_COMM,                 // bDeviceSubClass
    0x01,                           // bDeviceProtocol
    0,                              // bMaxPacketSize0      /* dynamic */
    UAC_VENDOR_ID,                  // idVendor
    UAC_PRODUCT_ID,                 // idProduct
    UAC_DEVICE_BCD,                 // bcdDevice
    0,                              // iManufacturer        /* dynamic */
    0,                              // iProduct             /* dynamic */
    0,                              // iSerialNumber        /* dynamic */
    0                               // bNumConfigurations   /* dynamic */
};

static struct usb_interface_assoc_descriptor uac_iad  = {
    sizeof(uac_iad),                // bLength
    USB_DT_INTERFACE_ASSOCIATION,   // bDescriptorType
    0,                              // bFirstInterface
    UAC_INTF_MAX,                   // bInterfaceCount
    USB_CLASS_AUDIO,                // bFunctionClass
    USB_SUBCLASS_AUDIOCONTROL,      // bFunctionSubClass
    0x00,                           // bFunctionProtocol
    0                               // iFunction
};

static struct usb_interface_descriptor uac_control_intf  = {
    USB_DT_INTERFACE_SIZE,          // bLength
    USB_DT_INTERFACE,               // bDescriptorType
    UAC_INTF_CONTROL,               // bInterfaceNumber
    0,                              // bAlternateSetting
    0,                              // bNumEndpoints
    USB_CLASS_AUDIO,                // bInterfaceClass
    USB_SUBCLASS_AUDIOCONTROL,      // bInterfaceSubClass
    0x00,                           // bInterfaceProtocol
    0                               // iInterface
};

static struct uac_header_descriptor uac_control_header = {
    UAC_DT_HEADER_LENGTH,           // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType
    UAC_HEADER,                     // bDescriptorSubType
    UAC_VERSION,                    // bcdADC
    0,                              // wTotalLength /* dynamic */
    (UAC_INTF_MAX-1),               // bInCollection
    UAC_INTF_IN_STREAMING,          // baInterfaceNr[0]  
#if UAC_ISOC_OUT_SUPPORT       
    UAC_INTF_OUT_STREAMING          // baInterfaceNr[1]
#endif    
};


static struct uac_input_terminal_descriptor uac_in_input_terminal_desc = {
    UAC_DT_INUT_TERMINAL_LENGTH,    // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType 
    UAC_INPUT_TERMINAL,             // bDescriptorSubtype          
    UAC_IN_INPUT_TERMINAL_ID,       // bTerminalID
    UAC_INPUT_TERMINAL_MICROPHONE,  // wTerminalType     /* dynamic */                        
    0,                              // bAssocTerminal
    UAC_IN_CHANNEL_MAX,             // bNrChannels
    0,                              // wChannelConfig   /* dynamic */
    0,                              // iChannelNames
    0                               // iTerminal
};  


DECLARE_UAC_FEATURE_UNIT_DESCRIPTOR(UAC_IN_LOGICAL_CHANNEL);
static struct UAC_FEATURE_UNIT_DESCRIPTOR(UAC_IN_LOGICAL_CHANNEL) uac_in_feature_unit_desc = {
    UAC_DT_FEATURE_UNIT_SIZE(UAC_IN_LOGICAL_CHANNEL),    // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType
    UAC_FEATURE_UNIT,               // bDescriptorSubType
    UAC_IN_FEATURE_UNIT_ID,         // bUnitID
    UAC_IN_INPUT_TERMINAL_ID,       // bSourceID
    2,                              // bControlSize
    UAC_FU_MUTE|UAC_FU_VOLUME,      // bmControls[0]    /* dynamic */
    UAC_FU_MUTE|UAC_FU_VOLUME,      // bmControls[0]    /* dynamic */
    0  
};

static struct uac_output_terminal_descriptor uac_in_output_terminal_desc = {
    UAC_DT_OUTPUT_TERMINAL_LENGTH,  // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType
    UAC_OUTPUT_TERMINAL,            // bDescriptorSubtype	
    UAC_IN_OUTPUT_TERMINAL_ID,      // bTerminalID		
    UAC_TERMINAL_STREAMING,         // wTerminalType
    0,                              // bAssocTerminal
    UAC_IN_FEATURE_UNIT_ID,         // bSourceID
    0                               // iTerminal
};

static struct uac_input_terminal_descriptor uac_out_input_terminal_desc = {
    UAC_DT_INUT_TERMINAL_LENGTH,     // bLength                      
    USB_DT_CS_INTERFACE,             // bDescriptorType               
    UAC_INPUT_TERMINAL,              // bDescriptorSubtype          
    UAC_OUT_INPUT_TERMINAL_ID,       // bTerminalID 
    UAC_TERMINAL_STREAMING,          // wTerminalType                          
    0,                               // bAssocTerminal
    UAC_OUT_CHANNEL_MAX,             // bNrChannels                                     
    3,                               // wChannelConfig
    0,                               // iChannelNames
    0                                // iTerminal
};

DECLARE_UAC_FEATURE_UNIT_DESCRIPTOR(0);
static struct UAC_FEATURE_UNIT_DESCRIPTOR(0) uac_out_feature_unit_desc = {
    UAC_DT_FEATURE_UNIT_SIZE(0),    // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType
    UAC_FEATURE_UNIT,               // bDescriptorSubType
    UAC_OUT_FEATURE_UNIT_ID,        // bUnitID
    UAC_OUT_INPUT_TERMINAL_ID,      // bSourceID
    2,                              // bControlSize
    UAC_FU_MUTE|UAC_FU_VOLUME,      // bmControls[0] 
    0
};

static struct uac_output_terminal_descriptor uac_out_output_terminal_desc = {
    UAC_DT_OUTPUT_TERMINAL_LENGTH,  // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType
    UAC_OUTPUT_TERMINAL,            // bDescriptorSubtype
    UAC_OUT_OUTPUT_TERMINAL_ID,     // bTerminalID
    UAC_OUTPUT_TERMINAL_SPEAKER,    // wTerminalType
    UAC_OUT_FEATURE_UNIT_ID,        // bAssocTerminal
    UAC_OUT_FEATURE_UNIT_ID,        // bSourceID
    0                               // iTerminal
};

static struct usb_interface_descriptor uac_in_streaming_intf_alt0  = {
    USB_DT_INTERFACE_SIZE,          // bLength
    USB_DT_INTERFACE,               // bDescriptorType
    UAC_INTF_IN_STREAMING,          // bInterfaceNumber
    0,                              // bAlternateSetting
    0,                              // bNumEndpoints
    USB_CLASS_AUDIO,                // bInterfaceClass
    USB_SUBCLASS_AUDIOSTREAMING,    // bInterfaceSubClass
    0x00,                           // bInterfaceProtocol
    0                               // iInterface
};
static struct usb_interface_descriptor uac_in_streaming_intf_alt1  = {
    USB_DT_INTERFACE_SIZE,          // bLength
    USB_DT_INTERFACE,               // bDescriptorType
    UAC_INTF_IN_STREAMING,          // bInterfaceNumber
    1,                              // bAlternateSetting
    1,                              // bNumEndpoints
    USB_CLASS_AUDIO,                // bInterfaceClass
    USB_SUBCLASS_AUDIOSTREAMING,    // bInterfaceSubClass
    0x00,                           // bInterfaceProtocol
    0                               // iInterface
};

static struct uac_streaming_cls_descriptor uac_in_streaming_cls_intf  = {
    USB_DT_STREAMING_CLS_LENGTH,    // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType
    UAC_GENERAL,                    // bDescriptorSubtype
    UAC_IN_OUTPUT_TERMINAL_ID,      // bTerminalLink
    1,                              // bDelay
    UAC_FORMAT_TYPE_I_PCM           // wFormatTag
};

static struct usb_interface_descriptor uac_out_streaming_intf_alt0  = {
    USB_DT_INTERFACE_SIZE,          // bLength
    USB_DT_INTERFACE,               // bDescriptorType
    UAC_INTF_OUT_STREAMING,         // bInterfaceNumber
    0,                              // bAlternateSetting
    0,                              // bNumEndpoints
    USB_CLASS_AUDIO,                // bInterfaceClass
    USB_SUBCLASS_AUDIOSTREAMING,    // bInterfaceSubClass
    0x00,                           // bInterfaceProtocol
    0                               // iInterface
};
static struct usb_interface_descriptor uac_out_streaming_intf_alt1  = {
    USB_DT_INTERFACE_SIZE,          // bLength
    USB_DT_INTERFACE,               // bDescriptorType
    UAC_INTF_OUT_STREAMING,         // bInterfaceNumber
    1,                              // bAlternateSetting
    1,                              // bNumEndpoints
    USB_CLASS_AUDIO,                // bInterfaceClass
    USB_SUBCLASS_AUDIOSTREAMING,    // bInterfaceSubClass
    0x00,                           // bInterfaceProtocol
    0                               // iInterface
};

static struct uac_streaming_cls_descriptor uac_out_streaming_cls_intf  = {
    USB_DT_STREAMING_CLS_LENGTH,    // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType
    UAC_GENERAL,                    // bDescriptorSubtype
    UAC_OUT_INPUT_TERMINAL_ID,      // bTerminalLink
    1,                              // bDelay
    UAC_FORMAT_TYPE_I_PCM           // wFormatTag
};


DECLARE_UAC_FORMAT_TYPE_I_DISCRETE_DESCRIPTOR(1);
static struct UAC_FORMAT_TYPE_I_DISCRETE_DESCRIPTOR(1) uac_in_format_desc = {
    UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(1),    // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType
    UAC_FORMAT_TYPE,                // bDescriptorSubtype
    UAC_FORMAT_TYPE_I,              // bFormatType
    UAC_IN_CHANNEL_MAX,             // bNrChannels
    (UAC_IN_SAMPLE_BIT_LENGTH+7)/8, // bSubframeSize
    UAC_IN_SAMPLE_BIT_LENGTH,       // bBitResolution
    1,                              // bSamFreqType
    //0x80,0xBB,0x00,              	// = 48000
    0x80,0x3E,0x00,              	// = 16000
};


static struct usb_endpoint_descriptor uac_in_streaming_ep = {
    sizeof(struct usb_endpoint_descriptor),                 // bLength
    USB_DT_ENDPOINT,                                        // bDescriptorType
    USB_DIR_IN,                                             // bEndpointAddress     /* dynamic */
    (USB_ENDPOINT_XFER_ISOC|USB_ENDPOINT_SYNC_ASYNC),       // bmAttributes
    UAC_IN_EP_MAX_PACKET_SIZE,                              // wMaxPacketSize
    4,                                                      // wInterval
    0,
    0
};

static struct uac_streaming_endpoint_cls_descriptor uac_in_streaming_cls_ep = {
    sizeof(struct uac_streaming_endpoint_cls_descriptor),   // bLength
    USB_DT_CS_ENDPOINT,                                     // bDescriptorType
    UAC_EP_GENERAL,                                         // bDescriptorSubType
    1,                                                      // bmAttributes    //D0:Sampling Frequency, D1:Pitch
    0,                                                      // bLockDelayUnits
    0x192                                                   // wLockDelay
};

static struct UAC_FORMAT_TYPE_I_DISCRETE_DESCRIPTOR(1) uac_out_format_desc = {
    UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(1),    // bLength
    USB_DT_CS_INTERFACE,            // bDescriptorType
    UAC_FORMAT_TYPE,                // bDescriptorSubtype
    UAC_FORMAT_TYPE_I,              // bFormatType
    UAC_OUT_CHANNEL_MAX,            // bNrChannels
    (UAC_OUT_SAMPLE_BIT_LENGTH+7)/8,// bSubframeSize
    UAC_OUT_SAMPLE_BIT_LENGTH,      // bBitResolution
    1,                              // bSamFreqType
    0x80,0xbb,0x00                  // =48000
};

static struct usb_endpoint_descriptor uac_out_streaming_ep = {
    sizeof(struct usb_endpoint_descriptor),                 // bLength
    USB_DT_ENDPOINT,                                        // bDescriptorType
    USB_DIR_OUT,                                            // bEndpointAddress     /* dynamic */
    (USB_ENDPOINT_XFER_ISOC|USB_ENDPOINT_SYNC_ASYNC),       // bmAttributes
    UAC_OUT_EP_MAX_PACKET_SIZE,                             // wMaxPacketSize
    4,                                                      // wInterval
    0,
    0
};

static struct uac_streaming_endpoint_cls_descriptor uac_out_streaming_cls_ep = {
    sizeof(struct uac_streaming_endpoint_cls_descriptor),   // bLength
    USB_DT_CS_ENDPOINT,                                     // bDescriptorType
    UAC_EP_GENERAL,                                         // bDescriptorSubType
    1,                                                      // bmAttributes     //D0:Sampling Frequency, D1:Pitch
    0,                                                      // bLockDelayUnits
    0x192                                                   // wLockDelay
};

