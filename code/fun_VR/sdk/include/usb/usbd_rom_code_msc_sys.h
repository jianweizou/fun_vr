#ifndef	USB_ROM_CODE_MSC_SYS_H
#define	USB_ROM_CODE_MSC_SYS_H

#include "snx_udc.h"

/*Debug*/
//#define	FLAG0_ENABLE	SN_GPIO1->MODE_b.MODE14 = 1;
//#define	FLAG1_ENABLE	SN_GPIO2->MODE_b.MODE0 = 1;
//#define	FLAG2_ENABLE	SN_GPIO2->MODE_b.MODE1 = 1;
//#define	FLAG3_ENABLE	SN_GPIO2->MODE_b.MODE2 = 1;

//#define	FLAG0(x)	SN_GPIO1->DATA_b.DATA14=x;
//#define	FLAG1(x)	SN_GPIO2->DATA_b.DATA0=x;
//#define	FLAG2(x)	SN_GPIO2->DATA_b.DATA1=x;
//#define	FLAG3(x)	SN_GPIO2->DATA_b.DATA2=x;

/*define*/
enum std_read_write_10
{
	STD_USBD_SDCARD0=0,
	STD_USBD_SDIO,
};

enum vendor_read_write_10
{
	VENDOR_USBD_SDCARD0=0,
	VENDOR_USBD_SDIO,
	VENDOR_USBD_SPIFC,
	VENDOR_USBD_PARALLEL,
};

/*struct*/
struct usbd_sys_lun
{
	uint8_t		storage;		/*storage info*/
	uint32_t	total_size; /*Byte*/
};

struct usbd_sys_std
{
	struct usbd_sys_lun lun[2];	
	uint8_t (*write_callback)(uint8_t lun, uint32_t addr, uint32_t len, uint8_t* buf);
	uint8_t (*read_callback)(uint8_t lun, uint32_t addr, uint32_t len, uint8_t* buf);
};

/*Variable*/
extern trb_struct_t *g_ep0_trb_addr;
extern trb_struct_t *g_ep12_trb_addr;
extern trb_struct_t *g_ep13_trb_addr;
extern uint8_t *g_bulk_in_buf_addr;
extern uint8_t *g_bulk_out_buf_addr;
extern uint32_t g_bulk_in_buf_size;
extern uint32_t g_bulk_out_buf_size;
extern struct usbd_sys_std g_usbd_sys_std;

/*Function*/
extern void usb_dev_msc_init(void);
extern void usb_dev_msc_uninit(void);
extern uint8_t usb_dev_msc_routine(void);
extern uint8_t usb_dev_bulk_in_buffer_setup(uint8_t *addr, uint32_t size);
extern uint8_t usb_dev_bulk_out_buffer_setup(uint8_t *addr, uint32_t size);
extern void usb_dev_ep0_trb_buffer_setup(trb_struct_t *addr);
extern void usb_dev_ep12_trb_buffer_setup(trb_struct_t *addr);
extern void usb_dev_ep13_trb_buffer_setup(trb_struct_t *addr);

#endif
