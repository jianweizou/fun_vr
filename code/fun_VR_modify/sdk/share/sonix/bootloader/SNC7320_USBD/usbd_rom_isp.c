/*_____ I N C L U D E S ____________________________________________________*/
#include "snc_types.h"
#include "snc7320_sys_reg.h"
#include "usbd_rom_isp.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/* usbd */
#define	IRQ_QUEUE_VALID	0
#define	IRQ_QUEUE_EMPTY	1

struct _usb_irq_status {
    uint32_t usb_event_flag;
    uint32_t eps_event_flag;
    uint32_t ep_status_flag;
};

struct _usb_irq_event
{
	uint8_t irq_trig_from;
	struct _usb_irq_status ep_irq_status;
};

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void usbd_rom_isp(void)
{
    /* Force Switch to IHRC 12M , Patch for UTMI Clock Gated RDY */
    SN_SYS0->OSC_CTRL_b.IHRCEN = 1;                                                     /* Enable IHRC */
    while(SN_SYS0->SYS0_CSST_b.IHRC_RDY == 0);                                          /* While IHEC RDY */       
    SN_SYS0->SYS_CLKCFG_b.SYSCLKSEL = 0;                                                /* System Clk Sel 0:IHRC */
    while(SN_SYS0->SYS_CLKCFG_b.SYSCLKSEL != SN_SYS0->SYS_CLKCFG_b.SYSCLKSEL);          /* Wait System Clk switch */
    while(SN_SYS0->SYS_CLKCFG_b.SYSCLKST != SN_SYS0->SYS_CLKCFG_b.SYSCLKSEL);           /* Wait System Clk switch */
    SN_SYS0->OSC_CTRL_b.PLL_EN = 0;                                                     /* Disable PLL */
    SN_SYS0->OSC_CTRL_b.HXTLEN = 0;                                                     /* Disable HXTAL */
    
    /* Remap ISR */
    *(uint32_t*)88 = 0x080076b1;                                                        /* remap USB IRQ to Rom ISR */
    *(uint32_t*)132 = 0x08004127;                                                       /* remap NF IRQ to Rom ISR */
    *(uint32_t*)252 = 0x08009c97;                                                       /* remap SPIDMA0 ECC IRQ to Rom ISR */
    *(uint32_t*)260 = 0x08009ccd;                                                       /* remap SPIDMA1 ECC IRQ to Rom ISR */
    
    /* UTMI RDY Gated Disable , Patch for UTMI Clock Gated RDY */
    void (*usb_dev_msc_init)(void) = (void(*)(void))0x0800a919;                         /* refer to symbol */
    usb_dev_msc_init();                                                                 /* Rom Code udc msc init */
    *((uint32_t*)(0x40100070)) = 0;                                                     /* Disable UTMI RDY Waiting */
    memset( (void*)0x1803c235, 0, 420 );                                                /* Clear Rom code queue buf overflow */

    /* Rouint */
    void (*usbd_rom_isp_routine)(void) = (void(*)(void))0x0800a9df;                     /* refer to symbol */
    usbd_rom_isp_routine();                                                             /* Execute USBD MSC ISP Routine */
}

void usbd_isp(void)
{
	struct _usb_irq_event usbd_msc_irq;
    
	/* Main Routine */
    uint8_t (*udc_irq_event_queue_receive)(struct _usb_irq_event *udc_irq_event) = (uint8_t (*)(struct _usb_irq_event *udc_irq_event))0x08006029;
    void (*usbd_process)( struct _usb_irq_status usbd_irq_status ) = (void (*)( struct _usb_irq_status usbd_irq_status))0x080075a3;

	while(1)
	{
		/*USB Device Event*/
		if( udc_irq_event_queue_receive(&usbd_msc_irq) == IRQ_QUEUE_VALID )
		{	
			usbd_process(usbd_msc_irq.ep_irq_status);
		}
    }
}
