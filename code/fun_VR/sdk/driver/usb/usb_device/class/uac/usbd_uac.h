/**
* @file
* this is usb device driver file
* usbd_uac.h
* @author IP2/Luka
*/
#ifndef __SONIX_USBD_UAC_H
#define __SONIX_USBD_UAC_H

#include "sonix_config.h"

#include "ch9.h"
#include "composite.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_MODULE_USBD_UAC_DEBUG)
#define USBD_UAC_PRINT(fmt, args...) printf("\r[USBD_UAC]"fmt"\r", ##args)
#else
#define USBD_UAC_PRINT(...)     ((void) 0)
#endif
#define USBD_UAC_PRINT_ERR(fmt, args...) printf("\r[USBD_UAC](error)%s:"fmt"\r", __func__, ##args)


/*user defined*/
//#define UAC_ISOC_OUT_SUPPORT 						1
#define UAC_FU_VOLUME_MIN                           0x000F
#define UAC_FU_VOLUME_MAX                           0xFFFF
#define UAC_FU_VOLUME_RES                           0x0001

/*spec defined*/
/* A.2 Audio Interface Subclass Codes */
#define USB_SUBCLASS_AUDIOCONTROL                   0x01
#define USB_SUBCLASS_AUDIOSTREAMING                 0x02
#define USB_SUBCLASS_MIDISTREAMING                  0x03

#define AS_GENERAL                                  0x01

/* A.5 Audio Class-Specific AC Interface Descriptor Subtypes */
#define UAC_HEADER                                  0x01
#define UAC_INPUT_TERMINAL                          0x02
#define UAC_OUTPUT_TERMINAL                         0x03
#define UAC_MIXER_UNIT                              0x04
#define UAC_SELECTOR_UNIT                           0x05
#define UAC_FEATURE_UNIT                            0x06
#define UAC1_PROCESSING_UNIT                        0x07
#define UAC1_EXTENSION_UNIT                         0x08

/*A.6 Audio Class-specific AS Interface Descriptor Subtypes*/
#define UAC_GENERAL                     0x01
#define UAC_FORMAT_TYPE                 0x02
#define UAC_FORMAT_SPECIFIC             0x03

/*A.8 Audio Class-Specific Endpoint Descriptor Subtypes*/
#define UAC_DESCRIPTOR_UNDEFINED        0x00
#define UAC_EP_GENERAL                  0x01


/* A.10.2 Feature Unit Control Selectors */
#define UAC_FU_MUTE                                 0x01
#define UAC_FU_VOLUME                               0x02
#define UAC_FU_BASS                                 0x03
#define UAC_FU_MID                                  0x04
#define UAC_FU_TREBLE                               0x05
#define UAC_FU_GRAPHIC_EQUALIZER                    0x06
#define UAC_FU_AUTOMATIC_GAIN                       0x07
#define UAC_FU_DELAY                                0x08
#define UAC_FU_BASS_BOOST                           0x09
#define UAC_FU_LOUDNESS                             0x0a

/* Terminals - 2.1 USB Terminal Types */
#define UAC_TERMINAL_UNDEFINED                      0x100
#define UAC_TERMINAL_STREAMING                      0x101
#define UAC_TERMINAL_VENDOR_SPEC                    0x1FF

/* Terminals - 2.2 Input Terminal Types */
#define UAC_INPUT_TERMINAL_UNDEFINED                0x200
#define UAC_INPUT_TERMINAL_MICROPHONE               0x201
#define UAC_INPUT_TERMINAL_DESKTOP_MICROPHONE       0x202
#define UAC_INPUT_TERMINAL_PERSONAL_MICROPHONE      0x203
#define UAC_INPUT_TERMINAL_OMNI_DIR_MICROPHONE      0x204
#define UAC_INPUT_TERMINAL_MICROPHONE_ARRAY         0x205
#define UAC_INPUT_TERMINAL_PROC_MICROPHONE_ARRAY    0x206


/* Terminals - 2.3 Output Terminal Types */
#define UAC_OUTPUT_TERMINAL_UNDEFINED                   0x300
#define UAC_OUTPUT_TERMINAL_SPEAKER                     0x301
#define UAC_OUTPUT_TERMINAL_HEADPHONES                  0x302
#define UAC_OUTPUT_TERMINAL_HEAD_MOUNTED_DISPLAY_AUDIO  0x303
#define UAC_OUTPUT_TERMINAL_DESKTOP_SPEAKER             0x304
#define UAC_OUTPUT_TERMINAL_ROOM_SPEAKER                0x305
#define UAC_OUTPUT_TERMINAL_COMMUNICATION_SPEAKER       0x306
#define UAC_OUTPUT_TERMINAL_LOW_FREQ_EFFECTS_SPEAKER    0x307

/* Formats - A.2 Format Type Codes */
#define UAC_FORMAT_TYPE_UNDEFINED                   0x0
#define UAC_FORMAT_TYPE_I                           0x1
#define UAC_FORMAT_TYPE_II                          0x2
#define UAC_FORMAT_TYPE_III                         0x3
#define UAC_EXT_FORMAT_TYPE_I                       0x81
#define UAC_EXT_FORMAT_TYPE_II                      0x82
#define UAC_EXT_FORMAT_TYPE_III                     0x83

/* Formats - A.1.1 Audio Data Format Type I	*/
#define UAC_FORMAT_TYPE_I_UNDEFINED                 0x0
#define UAC_FORMAT_TYPE_I_PCM                       0x1
#define UAC_FORMAT_TYPE_I_PCM8                      0x2
#define UAC_FORMAT_TYPE_I_IEEE_FLOAT                0x3
#define UAC_FORMAT_TYPE_I_ALAW                      0x4
#define UAC_FORMAT_TYPE_I_MULAW                     0x5

#define UAC_EP_CS_ATTR_SAMPLE_RATE                  0x01
#define UAC_EP_CS_ATTR_PITCH_CONTROL                0x02
#define UAC_EP_CS_ATTR_FILL_MAX                     0x80

/* A.6 Audio Class-Specific AS Interface Descriptor Subtypes */
#define UAC_AS_GENERAL                              0x01
#define UAC_FORMAT_TYPE                             0x02
#define UAC_FORMAT_SPECIFIC                         0x03

/* A.9 Audio Class-Specific Request Codes */
#define UAC_SET_                                    0x00
#define UAC_GET_                                    0x80

#define UAC__CUR                                    0x1
#define UAC__MIN                                    0x2
#define UAC__MAX                                    0x3
#define UAC__RES                                    0x4
#define UAC__MEM                                    0x5

#define UAC_SET_CUR                                 (UAC_SET_ | UAC__CUR)
#define UAC_GET_CUR                                 (UAC_GET_ | UAC__CUR)
#define UAC_SET_MIN                                 (UAC_SET_ | UAC__MIN)
#define UAC_GET_MIN                                 (UAC_GET_ | UAC__MIN)
#define UAC_SET_MAX                                 (UAC_SET_ | UAC__MAX)
#define UAC_GET_MAX                                 (UAC_GET_ | UAC__MAX)
#define UAC_SET_RES                                 (UAC_SET_ | UAC__RES)
#define UAC_GET_RES                                 (UAC_GET_ | UAC__RES)
#define UAC_SET_MEM                                 (UAC_SET_ | UAC__MEM)
#define UAC_GET_MEM                                 (UAC_GET_ | UAC__MEM)




/* 4.3.2  Class-Specific AC Interface Descriptor */
struct uac_header_descriptor {
	unsigned char  bLength;                 /* 8 + n */
	unsigned char  bDescriptorType;         /* USB_DT_CS_INTERFACE */
	unsigned char  bDescriptorSubtype;      /* UAC_MS_HEADER */
	unsigned short bcdADC;                  /* 0x0100 */
	unsigned short wTotalLength;            /* includes Unit and Terminal desc. */
	unsigned char  bInCollection;           /* n */
#ifdef 	USBD_UAC_ISOC_OUT_SUPPORT
	unsigned char  baInterfaceNr[1+USBD_UAC_ISOC_OUT_SUPPORT];        /* [n] */
#else
	unsigned char  baInterfaceNr[1];
#endif	
} __attribute__ ((packed));

/* 4.7.2.4 Input terminal descriptor */
struct uac_input_terminal_descriptor {
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bTerminalID;
    unsigned short wTerminalType;
    unsigned char  bAssocTerminal;
    unsigned char  bNrChannels;
    unsigned short wChannelConfig;
    unsigned char  iChannelNames;
    unsigned char  iTerminal;
} __attribute__((packed));

/* 4.3.2.5 Feature Unit Descriptor */
struct uac_feature_uint_descriptor{  
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bUnitID;
    unsigned char  bSourceID;
    unsigned char  bControlSize;
    unsigned short  bmaControls;         /* variable length */ 
    unsigned char  iFeature;
} __attribute__((packed));

#define UAC_DT_FEATURE_UNIT_SIZE(ch)            (7 + ((ch) + 1) * 2)
#define UAC_FEATURE_UNIT_DESCRIPTOR(ch)         _UAC_FEATURE_UNIT_DESCRIPTOR(ch) //to expand parameter "ch" 
#define _UAC_FEATURE_UNIT_DESCRIPTOR(ch)        uac_feature_uint_descriptor_##ch

#define DECLARE_UAC_FEATURE_UNIT_DESCRIPTOR(ch) \
struct UAC_FEATURE_UNIT_DESCRIPTOR(ch){ \
    unsigned char  bLength; \
    unsigned char  bDescriptorType; \
    unsigned char  bDescriptorSubtype; \
    unsigned char  bUnitID; \
    unsigned char  bSourceID; \
    unsigned char  bControlSize; \
    unsigned short  bmaControls[ch+1]; \
    unsigned char  iFeature;    \
} __attribute__((packed))

/* 4.3.2.2 Output Terminal Descriptor */
struct uac_output_terminal_descriptor {
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bTerminalID;
    unsigned short wTerminalType;
    unsigned char  bAssocTerminal;
    unsigned char  bSourceID;
    unsigned char  iTerminal;
} __attribute__ ((packed));


struct uac_streaming_cls_descriptor {
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bTerminalLink;
    unsigned char  bDelay;
    unsigned short wFormatTag;
} __attribute__((packed));

    struct uac_format_type_i_discrete_descriptor {
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bFormatType;
    unsigned char  bNrChannels;
    unsigned char  bSubframeSize;
    unsigned char  bBitResolution;
    unsigned char  bSamFreqType;
    unsigned char  tSamFreq[][3];                                                 
} __attribute__ ((packed));

#define UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(n)     (8 + (n * 3))
#define UAC_FORMAT_TYPE_I_DISCRETE_DESCRIPTOR(n)    uac_format_type_i_discrete_descriptor_##n

#define DECLARE_UAC_FORMAT_TYPE_I_DISCRETE_DESCRIPTOR(n) \
struct UAC_FORMAT_TYPE_I_DISCRETE_DESCRIPTOR(n) { \
    unsigned char  bLength; \
    unsigned char  bDescriptorType; \
    unsigned char  bDescriptorSubtype; \
    unsigned char  bFormatType; \
    unsigned char  bNrChannels; \
    unsigned char  bSubframeSize; \
    unsigned char  bBitResolution; \
    unsigned char  bSamFreqType; \
    unsigned char  tSamFreq[n][3]; \
} __attribute__ ((packed))  
      
struct uac_streaming_endpoint_cls_descriptor {
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubType;
    unsigned char  bmAttributes;
    unsigned char  bLockDelayUnits;
    unsigned short  wLockDelay;
} __attribute__((__packed__));

/* All UAC descriptors have these 3 fields at the beginning */
struct uac_descriptor_header {
    uint8_t             bLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubType;
} __attribute__((packed));



/* callback function type define */
typedef int (*usbd_drv_uac_start_record_cb_t)(uint32_t SamplesPerSec);
typedef void (*usbd_drv_uac_stop_record_cb_t)(void);
typedef int (*usbd_drv_uac_start_play_cb_t)(uint32_t SamplesPerSec);
typedef void (*usbd_drv_uac_stop_play_cb_t)(void);

typedef int (*usbd_drv_uac_ctrl_volume_cb_t)(unsigned char channel, unsigned char ctrl, unsigned short *volume);
typedef int (*usbd_drv_uac_ctrl_mute_cb_t)(unsigned char channel, unsigned char ctrl, unsigned char *enable);


/* uac callback functions */
void usbd_drv_uac_start_record_reg_cb(usbd_drv_uac_start_record_cb_t cb);
void usbd_drv_uac_stop_record_reg_cb(usbd_drv_uac_stop_record_cb_t cb);
void usbd_drv_uac_start_play_reg_cb(usbd_drv_uac_start_play_cb_t cb);
void usbd_drv_uac_stop_play_reg_cb(usbd_drv_uac_stop_play_cb_t cb);
void usbd_drv_uac_ctrl_volume_reg_cb(usbd_drv_uac_ctrl_volume_cb_t cb);
void usbd_drv_uac_ctrl_mute_reg_cb(usbd_drv_uac_ctrl_mute_cb_t cb);

int usbd_uac_init(void);
int usbd_uac_uninit(void);

void usbd_uac_drv_do_record(unsigned char *audio_buf, unsigned int audio_len);
int usbd_uac_drv_do_play(unsigned char *audio_buf, unsigned int audio_len);	
void usbd_uac_dqueue(void);

#ifdef __cplusplus
}
#endif


#endif  /* __SONIX_USBD_UAC_H */

