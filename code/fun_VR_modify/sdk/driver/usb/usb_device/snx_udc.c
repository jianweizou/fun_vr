/**
* @file
* this is usb device driver file
* snx_udc.c
* @author IP2/Luka
*/
#include "sonix_config.h"

#include "FreeRTOS.h"
#include "task.h"
//#include "INTC.h"
#include "queue.h"
#include "semphr.h"

#include <stddef.h>
#include <string.h>

#include "usb_device.h"
#include "composite.h"
#include "snx_udc.h"
#include "portmacro.h"

#if defined( CONFIG_PLATFORM_SN7320 )
#include "snc7320_platform.h"
#endif

volatile static xQueueHandle  usbd_irq_Queue;
volatile static xQueueHandle  usbd_ep0_Queue;
static uint32_t TRB_SLICE_LENGTH[USBD_EP_TOTAL_NUMBER];

static trb_struct_t *trb_ep[USBD_EP_TOTAL_NUMBER];
static trb_struct_t *trb_ep0_in;
static trb_struct_t *trb_ep0_out;
static trb_struct_t *ep4_ping_normal_trb = 0;
static trb_struct_t *ep4_pong_normal_trb = 0;
static trb_struct_t	*ep4_ping_link_trb = 0;
static trb_struct_t *ep4_pong_link_trb = 0;

struct usb_ctrlrequest *setup_packet;

static struct usb_ep_ops snx_ep_ops;
struct _sonix_udc snx_udc;
struct usb_ep _usb_ep[15];
static uint8_t g_odd = 0;
static uint32_t ep_rst_ready = 0;
static uint32_t ep_gioc_idx[USBD_EP_TOTAL_NUMBER];
static SemaphoreHandle_t usbd_ep0_out_finish;

#if !defined(CONFIG_DMA_MULTI_MODE_ENABLE)
static uint8_t g_start_ep0_out_drbl = 0;
#endif
/**
* @brief interface function - register driver

*/
void udc_enable(void);

void udelay(uint32_t value)
{
	uint32_t i;
	for (i=0; i<60*value; i++) {
		__nop();
	}
}


static inline unsigned int udc_read(unsigned int addr)
{
	return (*((volatile unsigned int *)(addr)));
	//return inl(addr);
}

static inline void udc_write(unsigned int addr, unsigned int value)
{
	outl((unsigned int*)addr, value);
}

void snx_udc_ep0_start_TRB(unsigned char endp_addr, unsigned int* buffer, unsigned short buffer_size);


/**
* @brief interface function - write assigning endpoint data to fifo
* @param req the pointer of enpoint request
* @param num enpoitn number
* @param dir enpoitn direction
* @return value of error code.
*/
static /*inline*/ void snx_udc_epx_write_packet(struct snx_request *req,uint32_t num,uint32_t dir, uint32_t zero_packet_tail)
{
#if defined(CONFIG_DMA_MULTI_MODE_ENABLE)
	char 				*ep_name;
	uint32_t volatile   ccs = 0, ep_sts = 0;
	uint32_t            data_size_for_last_transfer = 0, cnt = 0;
	uint32_t            i = 0;
	uint32_t			remainder_data_size = 0;
	static trb_struct_t *tempTRB = NULL;
	
	udc_write(USBD_REG_EP_SEL, (num | dir));
	ep_sts = udc_read(USBD_REG_EP_STS);
	
	ccs = (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs = (ccs >> 11);
	
	data_size_for_last_transfer = req->req.length;
	tempTRB = trb_ep[num];
	
	ep_name = (char*)snx_udc.ep[num].ep->name;
	
	if(ep_name[4] == 'i' && ep_name[5] == 'n' && ep_name[6] == 't'){ //transfer type = INT
		TRB_SLICE_LENGTH[num] = snx_udc.ep[num].ep->maxpacket;
		remainder_data_size = data_size_for_last_transfer % snx_udc.ep[num].ep->maxpacket;
	}
	else{ //transfer type = BULK		
		TRB_SLICE_LENGTH[num] = 65536;
		remainder_data_size = data_size_for_last_transfer % snx_udc.ep[num].ep->maxpacket;
	}
	
	data_size_for_last_transfer -= remainder_data_size;
	
	if(data_size_for_last_transfer > 0){
		while(data_size_for_last_transfer > TRB_SLICE_LENGTH[num]){
			if(ep_name[4] == 'i' && ep_name[5] == 'n' && ep_name[6] == 't'){ //transfer type = INT
				//Link serveral LINK TRB
				for(i = 0; i < TRB_MULTI_LINK_CNT; i++){
					tempTRB->data_buffer_pointer = (uint32_t)(tempTRB+1);
					tempTRB->length_and_burst = 0;
					tempTRB->control = TRB_LINK | TRB_CH | ccs;
					tempTRB++;
				}			
			}	
			
			tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
			tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength;
			tempTRB->control = TRB_NORMAL | ccs;

			data_size_for_last_transfer = data_size_for_last_transfer - (uint32_t)TRB_SLICE_LENGTH[num];
			tempTRB++;
			cnt++;		
		}
		
		if(ep_name[4] == 'i' && ep_name[5] == 'n' && ep_name[6] == 't'){ //transfer type = INT
			//Link serveral LINK TRB
			for(i = 0; i < TRB_MULTI_LINK_CNT; i++){
				tempTRB->data_buffer_pointer = (uint32_t)(tempTRB+1);
				tempTRB->length_and_burst = 0;
				tempTRB->control = TRB_LINK | TRB_CH | ccs;
				tempTRB++;
			}			
		}	
		
		//Normal TRB
		tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
		tempTRB->length_and_burst = data_size_for_last_transfer | TRB_BurstLength;
		tempTRB->control = TRB_NORMAL | ccs;
		tempTRB++;		
	}
	
	if(remainder_data_size == 0){
		//Link 0 byte Normal TRB
		if((data_size_for_last_transfer&0x01FF)==0 &&(zero_packet_tail)) {
			cnt++;
			tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
			tempTRB->length_and_burst = (uint32_t)(0| TRB_BurstLength);
			tempTRB->control = TRB_NORMAL | TRB_ISP | TRB_IOC | ccs;
			tempTRB++;
		}	
		else{
			tempTRB--;
			tempTRB->control = TRB_NORMAL | TRB_ISP | TRB_IOC | ccs;
			tempTRB++;
		}
	}
	else{
		//Link serveral LINK TRB
		for(i = 0; i < TRB_MULTI_LINK_CNT; i++){
			tempTRB->data_buffer_pointer = (uint32_t)(tempTRB+1);
			tempTRB->length_and_burst = 0;
			tempTRB->control = TRB_LINK | TRB_CH | ccs;
			tempTRB++;
		}
		//Normal TRB
		tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]) + data_size_for_last_transfer;
		tempTRB->length_and_burst = remainder_data_size | TRB_BurstLength;	
		tempTRB->control = TRB_NORMAL | TRB_ISP | TRB_IOC | ccs;
		tempTRB++;		
	}
	
	//Link TRB
	tempTRB->data_buffer_pointer = (uint32_t)(trb_ep[num]);
	tempTRB->length_and_burst = 0;
	tempTRB->control = TRB_LINK | TRB_TC | TRB_CH;
	
	//Set TRB Addr
	udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep[num]);
	
	//Enable DRBL
	udc_write(USBD_REG_DRBL, USBRD_DRBL(num, dir));	
#else	
	uint32_t volatile   ccs,ep_sts;
	uint32_t            data_size_for_last_transfer,cnt;
	//uint32_t            i = 0;
	static trb_struct_t *tempTRB;

	if(num==0x03)
		TRB_SLICE_LENGTH[num] = 192;
	else
		TRB_SLICE_LENGTH[num] = 65536;
	udc_write(USBD_REG_EP_SEL, (num | dir));

	ep_sts = udc_read(USBD_REG_EP_STS);
	ccs = (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs = (ccs >> 11);

	cnt = 0;
	data_size_for_last_transfer = req->req.length ;
	tempTRB = trb_ep[num];

	while(data_size_for_last_transfer > TRB_SLICE_LENGTH[num]) {
		tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
		tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength;
		tempTRB->control = TRB_NORMAL | ccs |(TRB_IOC|TRB_ISP);

		data_size_for_last_transfer = data_size_for_last_transfer - (uint32_t)TRB_SLICE_LENGTH[num];
		tempTRB++;
		cnt++;
	}

	tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) +(cnt*TRB_SLICE_LENGTH[num]);
	tempTRB->length_and_burst = data_size_for_last_transfer | TRB_BurstLength;
	tempTRB->control = TRB_NORMAL | ccs |(TRB_IOC|TRB_ISP);
	tempTRB++;	

	if((data_size_for_last_transfer&0x01FF)==0 &&(zero_packet_tail)) {
		cnt++;
		tempTRB->data_buffer_pointer = (uint32_t)&(req->req.buf[cnt * TRB_MAX_SIZE]);
		tempTRB->length_and_burst = (uint32_t)(0| TRB_BurstLength);
		tempTRB->control = TRB_NORMAL | TRB_ISP | TRB_IOC | ccs;
		tempTRB++;
	}

	ep_gioc_idx[num] = cnt;

	tempTRB->data_buffer_pointer = (uint32_t)(trb_ep[num]);
	tempTRB->length_and_burst = 0;
	tempTRB->control = TRB_LINK | TRB_TC | TRB_CH;

	udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep[num ]);
	udc_write(USBD_REG_DRBL, USBRD_DRBL(num, dir));
#endif	
}
/**
* @brief interface function - read assigning endpoint data from fifo
* @param req the pointer of enpoint request
* @param num enpoitn number
* @param dir enpoitn direction
* @return value of error code.
*/
static /*inline*/  void snx_udc_epx_read_packet(struct snx_request *req,uint32_t num,uint32_t dir)
{
#if defined(CONFIG_DMA_MULTI_MODE_ENABLE)
	uint32_t volatile   ccs = 0, ep_sts = 0;
	uint32_t            data_size_for_last_transfer = 0, cnt = 0, i = 0;
	static trb_struct_t *tempTRB;
	
	USBD_PRINT("NEWTD: Creating TRB for ep:%02X buffer:%08X size:%d\r\n",num|(dir << 7),(unsigned int)req->req.buf,(unsigned int)req->req.length);
	
	udc_write(USBD_REG_EP_SEL, (num | dir));
	ep_sts = udc_read(USBD_REG_EP_STS);
	
	ccs = (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs = (ccs >> 11);	
	
	if(num != 0x04){
		TRB_SLICE_LENGTH[num] = 65536;
	}
	
	data_size_for_last_transfer = req->req.length;
	tempTRB = trb_ep[num];	
	
	//Prepare serveral Normal TRB when data_size_for_last_transfer > TRB_SLICE_LENGTH 
	while(data_size_for_last_transfer > TRB_SLICE_LENGTH[num]){
		if(num == 0x04){
			while(i < 8){
				tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
				tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength_INCR16;
				tempTRB->control = TRB_NORMAL | ccs;
				tempTRB = tempTRB++;
				i++;				
			}
		}
		tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
		tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength;
		tempTRB->control = TRB_NORMAL | ccs;

		data_size_for_last_transfer = data_size_for_last_transfer - (uint32_t)TRB_SLICE_LENGTH[num];
		tempTRB++;
		cnt++;	
		i = 0;
	}
	
	if(num == 0x04){
		while(i < 8){
			tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
			tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength_INCR16;
			tempTRB->control = TRB_NORMAL | ccs;
			tempTRB = tempTRB++;
			i++;				
		}
		tempTRB = tempTRB--;
		tempTRB->control = TRB_NORMAL | TRB_IOC | TRB_ISP | ccs;
		tempTRB = tempTRB++;
	}	
	else{
		//Prepare Last Normal TRB
		tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
		tempTRB->length_and_burst = data_size_for_last_transfer | TRB_BurstLength;
		tempTRB->control = TRB_NORMAL | TRB_IOC | TRB_ISP | ccs;
		tempTRB++;	
	}
	
	//Link TRB
	tempTRB->data_buffer_pointer = (uint32_t)(trb_ep[num]);
	tempTRB->length_and_burst = 0;
	tempTRB->control = TRB_LINK | TRB_TC | TRB_CH;

	//Set TRB Addr
	udc_write(USBD_REG_EP_SEL, (num | dir));
	udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep[num]);
	
	//Enable DRBL
	udc_write(USBD_REG_DRBL, USBRD_DRBL(num, dir));	
#else	
	uint32_t volatile ccs,ep_sts;

	//USBD_DBG("NEWTD: Creating TRB for ep:%02X buffer:%08X size:%d\r\n",epnum|(epdir << 7),(unsigned int)req->req.buf,(unsigned int)req->req.length);

	udc_write(USBD_REG_EP_SEL, (num | dir));


	ep_sts = udc_read(USBD_REG_EP_STS);
	ccs = (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs = (ccs >> 11);

	trb_ep[num ]->data_buffer_pointer = (uint32_t)req->req.buf;
	trb_ep[num ]->length_and_burst = (uint32_t)req->req.length | TRB_BurstLength;
	trb_ep[num ]->control = TRB_NORMAL | TRB_ISP | TRB_IOC | ccs;

	udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep[num]);
	udc_write(USBD_REG_DRBL, USBRD_DRBL(num, dir));

#endif
	return ;
}

static /*inline*/ void snx_udc_epx_read_packet_iso(struct snx_request *req, uint32_t num, uint32_t dir, int32_t sts)
{
	uint32_t volatile   ccs,ep_sts;
	uint32_t data_size_for_last_transfer,cnt;
	static trb_struct_t *tempTRB;
	
	USBD_PRINT("NEWTD: Creating TRB for ep:%02X buffer:%08X size:%d\r\n",num|(dir << 7),(unsigned int)req->req.buf,(unsigned int)req->req.length);
	
	if((udc_read(USBD_REG_STS)&U32_BIT(3)) == 0) {
		udc_write(USBD_REG_CONF,  USBD_CONF_BIT_DEVEN | USBD_CONF_BIT_DMULT);
	}

	udc_write(USBD_REG_EP_SEL, (num | dir));

	ep_sts = udc_read(USBD_REG_EP_STS);
	ccs = (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs = (ccs >> 11);
	cnt = 0;
	data_size_for_last_transfer = req->req.length;

	tempTRB = trb_ep[num];
	if(num == 0x03) {
		while(data_size_for_last_transfer > TRB_SLICE_LENGTH[num]) {
			tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
			tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength;
			tempTRB->control = TRB_NORMAL | ccs;

			data_size_for_last_transfer = data_size_for_last_transfer - (uint32_t)TRB_SLICE_LENGTH[num];
			tempTRB = tempTRB++;
			cnt++;
		}
		tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) +(cnt*TRB_SLICE_LENGTH[num]);
		tempTRB->length_and_burst = data_size_for_last_transfer | TRB_BurstLength;
		tempTRB->control = TRB_NORMAL | ccs |(TRB_IOC|TRB_ISP);
	}
	else {
		while(data_size_for_last_transfer > TRB_SLICE_LENGTH[num]) {
			tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
			tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength;
			tempTRB->control = TRB_NORMAL | ccs;

			if(sts & USBD_EP_STATE_UVC_FID)
				tempTRB->control |= ( UVC_FID );
			else
				tempTRB->control &= ~( UVC_FID );

			data_size_for_last_transfer = data_size_for_last_transfer - (uint32_t)TRB_SLICE_LENGTH[num];
			tempTRB = tempTRB++;
			cnt++;

		}

		tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
		tempTRB->length_and_burst = data_size_for_last_transfer | TRB_BurstLength;
		tempTRB->control = TRB_NORMAL | ccs ;

		if(sts & USBD_EP_STATE_UVC_FID)
			tempTRB->control |= ( UVC_FID );
		else
			tempTRB->control &= ~( UVC_FID );

		if((sts & USBD_EP_STATE_UVC_EOF)) {
			tempTRB->control |= ( UVC_EOF );

			tempTRB = tempTRB++;

			tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
			tempTRB->length_and_burst = 0 | TRB_BurstLength;
			tempTRB->control = (TRB_NORMAL | ccs|TRB_IOC|TRB_ISP);

			if(sts & USBD_EP_STATE_UVC_FID)
				tempTRB->control |= ( UVC_FID );
			else
				tempTRB->control &= ~( UVC_FID );
		}
		else {
//            tempTRB->control = TRB_NORMAL | ccs |(TRB_IOC|TRB_ISP);
			tempTRB->control |= (TRB_IOC|TRB_ISP);
		}
	}
	tempTRB = tempTRB++;
	tempTRB->data_buffer_pointer = (uint32_t)(trb_ep[num]);
	tempTRB->length_and_burst = 0;
	tempTRB->control = (uint32_t)((TRB_LINK | TRB_TC | TRB_CH ));

	udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep[num ]);
	udc_write(USBD_REG_DRBL, USBRD_DRBL(num, dir));

	return ;
}

static void snx_udc_epx_write_packet_iso(struct snx_request *req, uint32_t num, uint32_t dir, uint32_t sts)
{
	uint8_t j = 0;
	uint32_t volatile ccs, ep_sts;
	uint32_t data_size_for_last_transfer, cnt;
	static trb_struct_t *tempTRB;
	uint32_t idx = 0;	
	
	udc_write(USBD_REG_EP_SEL, (num | dir));
	ep_sts = udc_read(USBD_REG_EP_STS);
	ccs = (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs = (ccs >> 11);	
	
	cnt = 0;
	data_size_for_last_transfer = req->req.length;	
	tempTRB = trb_ep[num];
	
	ep4_ping_normal_trb = tempTRB;
	if(num == 0x04){
		if((sts & USBD_EP_STATE_TSTOP) == USBD_EP_STATE_TSTOP){
			if(ep4_ping_link_trb != NULL && (ep4_ping_link_trb->control & TRB_TC) != TRB_TC)
				ep4_ping_link_trb->control |= TRB_TC;
			if(ep4_pong_link_trb != NULL && (ep4_pong_link_trb->control & TRB_TC) != TRB_TC)
				ep4_pong_link_trb->control |= TRB_TC;
		}
		else{
			//=================================Ping TD=================================		
			while(data_size_for_last_transfer > TRB_SLICE_LENGTH[num]){
				while(j<8){
					tempTRB->data_buffer_pointer = (uint32_t)(req->req.ping_buf) + (cnt*TRB_SLICE_LENGTH[num]);				
					tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength;
					tempTRB->control = TRB_NORMAL | ccs;
					tempTRB = tempTRB++;
					j++;
					idx++;
				}

				data_size_for_last_transfer -= TRB_SLICE_LENGTH[num];
				cnt++;
				j = 0;
			}

			while(j<8){
				tempTRB->data_buffer_pointer = (uint32_t)(req->req.ping_buf) +(cnt*TRB_SLICE_LENGTH[num]);				
				tempTRB->length_and_burst = data_size_for_last_transfer | TRB_BurstLength;
				tempTRB->control = TRB_NORMAL | ccs;			
				tempTRB = tempTRB++;
				j++;
				idx++;
			}

			tempTRB = tempTRB--;
			tempTRB->control = TRB_NORMAL | TRB_IOC | TRB_ISP | ccs;
			tempTRB = tempTRB++;
			
			ep4_ping_link_trb = tempTRB;
			
			tempTRB->data_buffer_pointer = (uint32_t)(tempTRB+1);
			tempTRB->length_and_burst = 0;
			tempTRB->control = TRB_LINK | TRB_CH | ccs;					
			tempTRB = tempTRB++;
			//=================================Pong TD=================================
			ep4_pong_normal_trb = tempTRB;
			data_size_for_last_transfer = req->req.length;
			j = 0;
			idx = 0;
			cnt = 0;
			while(data_size_for_last_transfer > TRB_SLICE_LENGTH[num]){
				while(j<8){
					tempTRB->data_buffer_pointer = (uint32_t)(req->req.pong_buf) + (cnt*TRB_SLICE_LENGTH[num]);				
					tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength;
					tempTRB->control = TRB_NORMAL | ccs;
					tempTRB = tempTRB++;
					j++;
					idx++;
				}

				data_size_for_last_transfer -= TRB_SLICE_LENGTH[num];
				cnt++;
				j = 0;
			}

			while(j<8){
				tempTRB->data_buffer_pointer = (uint32_t)(req->req.pong_buf) +(cnt*TRB_SLICE_LENGTH[num]);				
				tempTRB->length_and_burst = data_size_for_last_transfer | TRB_BurstLength;
				tempTRB->control = TRB_NORMAL | ccs;			
				tempTRB = tempTRB++;
				j++;
				idx++;
			}

			tempTRB = tempTRB--;
			tempTRB->control = TRB_NORMAL | ccs | (TRB_IOC|TRB_ISP);
			tempTRB = tempTRB++;	
			
			tempTRB->data_buffer_pointer = (uint32_t)(trb_ep[num]);
			tempTRB->length_and_burst = 0;
			tempTRB->control = TRB_LINK | TRB_CH | ccs;		

			ep4_pong_link_trb = tempTRB;
			ep_gioc_idx[num] = idx;
			udc_write(USBD_REG_EP_SEL, (num | dir)); //erk
			udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep[num]);
			udc_write(USBD_REG_DRBL, USBRD_DRBL(num, dir));				
		}
	}	

	return;
}
/**
* @brief interface function - prepare assigning endpoint data with fifo
* @param req the pointer of enpoint request
* @param num enpoitn number
* @param dir enpoitn direction
* @return value of error code.
*/
static /*inline*/  int snx_udc_epx_prepare_packet(struct snx_request *req,uint32_t num,uint32_t dir)
{
#if defined(CONFIG_DMA_MULTI_MODE_ENABLE)
	uint32_t volatile   ccs = 0, ep_sts = 0;
	uint32_t            data_size_for_last_transfer = 0, cnt = 0;
	static trb_struct_t *tempTRB;
	
	udc_write(USBD_REG_EP_SEL, (num | dir));
	ep_sts = udc_read(USBD_REG_EP_STS);
	
	ccs = (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs = (ccs >> 11);	
	
	TRB_SLICE_LENGTH[num] = 65536;
	data_size_for_last_transfer = req->req.length;
	tempTRB = trb_ep[num];	
	
	//Prepare serveral Normal TRB if data_size_for_last_transfer > TRB_SLICE_LENGTH 
	while(data_size_for_last_transfer > TRB_SLICE_LENGTH[num]){
		tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
		tempTRB->length_and_burst = TRB_SLICE_LENGTH[num] | TRB_BurstLength;
		tempTRB->control = TRB_NORMAL | ccs;

		data_size_for_last_transfer = data_size_for_last_transfer - (uint32_t)TRB_SLICE_LENGTH[num];
		tempTRB++;
		cnt++;		
	}
	
	//Prepare Last Normal TRB
	tempTRB->data_buffer_pointer = (uint32_t)(req->req.buf) + (cnt*TRB_SLICE_LENGTH[num]);
	tempTRB->length_and_burst = data_size_for_last_transfer | TRB_BurstLength;
	tempTRB->control = TRB_NORMAL | TRB_IOC | TRB_ISP | ccs;
	tempTRB++;	
	
	//Link TRB
	tempTRB->data_buffer_pointer = (uint32_t)(trb_ep[num]);
	tempTRB->length_and_burst = 0;
	tempTRB->control = TRB_LINK | TRB_TC | TRB_CH;

	//Set TRB Addr
	udc_write(USBD_REG_EP_SEL, (num | dir));
	udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep[num]);
	
	//Enable DRBL
	udc_write(USBD_REG_DRBL, USBRD_DRBL(num, dir));		
#else	
	uint32_t volatile ccs, ep_sts;

	USBD_PRINT("NEWTD: Creating TRB for ep:%02X buffer:%08X size:%d\n",num|(dir << 7),(unsigned int)req->req.buf,(unsigned int)req->req.length);

	udc_write(USBD_REG_EP_SEL, (num | dir));
	ep_sts = udc_read(USBD_REG_EP_STS);
	ccs = (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs = (ccs >> 11);

	trb_ep[num]->data_buffer_pointer = (uint32_t)req->req.buf;
	trb_ep[num]->length_and_burst = (uint32_t)req->req.length | TRB_BurstLength;
	trb_ep[num]->control = TRB_NORMAL | TRB_ISP | TRB_IOC | ccs;

	udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep[num]);
	udc_write(USBD_REG_DRBL, USBRD_DRBL(num, dir));
#endif
	return 0;
}

static inline struct snx_request *to_snx_req(struct usb_request *req)
{
	return (struct snx_request *)req->parent_req;
}

static inline struct _sonix_udc_ep *to_snx_ep(struct usb_ep *ep)
{
	return (struct _sonix_udc_ep *)ep->parent_ep;
}

/*
struct usb_request* get_usbreq(struct usb_ep *ep)
{
	struct _sonix_udc_ep *_ep;
	_ep = (struct _sonix_udc_ep *)ep->parent_ep;
	return &(_ep->req->req);
}
*/
/**
* @brief interface function - reset usb endpoint status
* @param _ep the pointer of usb enpoint
* @return value of error code.
*/
static int snx_udc_reset(struct usb_ep *_ep)
{
	struct _sonix_udc_ep   *ep;

	USBD_PRINT( "%s\r\n",__FUNCTION__);

	ep = to_snx_ep(_ep);
	ep->ep->state = 0;
	return 0;
}
/**
* @brief interface function - execute udc queue with assigning endpoint
* @param _ep the pointer of usb enpoint
* @param _req the pointer of enpoint request
* @return value of error code.
*/
static int32_t snx_udc_queue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct snx_request      *req;
	struct _sonix_udc_ep    *ep;
	struct _sonix_udc       *udc;
	volatile uint32_t	    eps_sts = 0;
	volatile uint32_t		tempEpNum = 0;
	volatile uint32_t		tempEpsts = 0;	

	req = to_snx_req(_req);
	ep = to_snx_ep(_ep);
	USBD_PRINT( "%s num : %d len=%d ep->state : %08X\n",__FUNCTION__, ep->num,req->req.length,ep->ep->state);


	udc = ep->dev;
	if ( !udc->driver /*|| udc->gadget.speed == USB_SPEED_UNKNOWN)*/) {
		return -ESHUTDOWN;
	}

//	NVIC_DisableIRQ(USBDEV_IRQn);

	if(ep->num==0) {
		switch (udc->ep0state) {
		case EP0_OUT_DATA_PHASE:
			memset(req->req.buf,0,req->req.length);
			snx_udc_ep0_start_TRB(USB_DIR_OUT, (uint32_t*)req->req.buf, req->req.length);
			udc_write(USBD_REG_EP_SEL, ep->num);
			while (udc_read(USBD_REG_EP_CMD) & USBD_EP_CMD_BIT_ERDY);
			udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_ERDY);            //out stage->data1 ok & ack ok
			if(usbd_ep0_out_finish)
				xSemaphoreTake(usbd_ep0_out_finish, portMAX_DELAY);
			break;
		case EP0_IN_DATA_PHASE:
		default:
			snx_udc_ep0_start_TRB(USB_DIR_IN, (uint32_t*)req->req.buf, req->req.length);
			udc_write(USBD_REG_EP_SEL, ep->num | USB_DIR_IN);
			eps_sts = udc_read(USBD_REG_EP_CMD);
			while(eps_sts & USBD_EP_CMD_BIT_ERDY) {
				eps_sts = udc_read(USBD_REG_EP_CMD);
			}
			udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CMPL | USBD_EP_CMD_BIT_ERDY); //status stage
			udc_write(USBD_REG_EP_SEL, ep->num | USB_DIR_OUT);
			break;
		}
#if 0		
		switch (udc->ep0state) {
		case EP0_OUT_DATA_PHASE:
			memset(req->req.buf,0,req->req.length);
			snx_udc_ep0_start_TRB(USB_DIR_OUT, (uint32_t*)req->req.buf, req->req.length);
			udc_write(USBD_REG_EP_SEL, ep->num);
			while (udc_read(USBD_REG_EP_CMD) & USBD_EP_CMD_BIT_ERDY);
			udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_ERDY);            //out stage->data1 ok & ack ok

			//while(((udc_read(USBD_REG_DRBL) & 0x1) == 0x1));
			//while(((udc_read(USBD_REG_EP_STS) & USBD_EP_STS_BIT_DBUSY) == USBD_EP_STS_BIT_DBUSY));
			while((udc_read(USBD_REG_EP_STS) & (USBD_EP_STS_BIT_ISP|USBD_EP_STS_BIT_IOC)) == 0){
				if((udc_read(USBD_REG_EP_STS) & USBD_EP_STS_BIT_OUTQ_VAL) && ((udc_read(USBD_REG_EP_STS) >> 24) & 0x0F) != 0x0){
					tempEpNum = ((udc_read(USBD_REG_EP_STS) >> 24) & 0x0F);
					if(tempEpNum != 0){
						udc_write(USBD_REG_EP_SEL, tempEpNum);
						tempEpsts = udc_read(USBD_REG_EP_STS);
						tempEpsts |= USBD_EP_STS_BIT_DESCMIS;
						tempEpsts &= 0x80FFFFFF;
						
						usbd_ep_function[tempEpNum](tempEpsts, USB_DIR_OUT);
						udc_write(USBD_REG_EP_SEL, (uint32_t)tempEpNum);
						udc_write(USBD_REG_EP_STS_CLR, tempEpsts);
					}
					udc_write(USBD_REG_EP_SEL, USB_DIR_OUT);
				}				
			}
			udc_write(USBD_REG_EP_STS_CLR, USBD_EP_STS_BIT_ISP| USBD_EP_STS_BIT_IOC/* | USBD_EP_STS_BIT_TRBERR*/);
			trb_ep0_out->data_buffer_pointer = (uint32_t)setup_packet; //restore buffer address
			trb_ep0_out->length_and_burst = (8) | TRB_BurstLength; //restore trb length
			udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CMPL); 
			//udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CMPL);
			//udc_write(USBD_REG_EP_STS_CLR, USBD_EP_STS_BIT_ISP| USBD_EP_STS_BIT_IOC/* | USBD_EP_STS_BIT_TRBERR*/);
			break;

		case EP0_IN_DATA_PHASE:
		default:
			snx_udc_ep0_start_TRB(USB_DIR_IN, (uint32_t*)req->req.buf, req->req.length);
			udc_write(USBD_REG_EP_SEL, ep->num | USB_DIR_IN);
			eps_sts = udc_read(USBD_REG_EP_CMD);
			while(eps_sts & USBD_EP_CMD_BIT_ERDY) {
				eps_sts = udc_read(USBD_REG_EP_CMD);
			}
			udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CMPL | USBD_EP_CMD_BIT_ERDY); //status stage
			udc_write(USBD_REG_EP_SEL, ep->num | USB_DIR_OUT);
			//break;
		}
		udc->ep0state = EP0_SETUP_PHASE;
#endif		
	}
	else if(ep->num == 12 ) {
		if(!(ep->ep->state & USBD_EP_STATE_BUSY)) {
			ep->ep->state &= ~USBD_EP_STATE_TOC;
			ep->ep->state &= ~USBD_EP_STATE_TFOC;
			ep->ep->state |= USBD_EP_STATE_BUSY;
			snx_udc_epx_write_packet(req,ep->num, USB_DIR_IN, (ep->ep->state&USBD_EP_STATE_0_PACKET_TAIL));
		}
	}
	else if(ep->num == 13 ) {
		if(!(ep->ep->state & USBD_EP_STATE_BUSY)) {
			ep->ep->state &= ~USBD_EP_STATE_TOC;
			ep->ep->state |= USBD_EP_STATE_BUSY;
			snx_udc_epx_read_packet(req,ep->num,USB_DIR_OUT);
		}
	}
	else if(ep->num == 14 ) {
		if(!(ep->ep->state & USBD_EP_STATE_BUSY)) {
			ep->ep->state &= ~USBD_EP_STATE_TOC;
			ep->ep->state &= ~USBD_EP_STATE_TFOC;
			ep->ep->state |= USBD_EP_STATE_BUSY;
			snx_udc_epx_write_packet(req,ep->num, USB_DIR_IN, (ep->ep->state&USBD_EP_STATE_0_PACKET_TAIL));
		}
	}	
	else if(ep->num == 5 ) {
		if(!(ep->ep->state & USBD_EP_STATE_BUSY)) {
			ep->ep->state &= ~USBD_EP_STATE_TOC;
			ep->ep->state |= USBD_EP_STATE_BUSY;
			snx_udc_epx_write_packet(req,ep->num, USB_DIR_IN, (ep->ep->state&USBD_EP_STATE_0_PACKET_TAIL));
		}
	}
	else if(ep->num == 7 ) {
		if(!(ep->ep->state & USBD_EP_STATE_BUSY)) {
			TRB_SLICE_LENGTH[ep->num] = ep->ep->s.payload_xfr_size - 12;
			ep->ep->state &= ~USBD_EP_STATE_TOC;
			ep->ep->state |= USBD_EP_STATE_BUSY;
			snx_udc_epx_read_packet_iso(req,ep->num,USB_DIR_IN,ep->ep->state);
		}
	}
	else if(ep->num == 3 ) {
		if(!(ep->ep->state & USBD_EP_STATE_BUSY)) {
            TRB_SLICE_LENGTH[ep->num] = ep->ep->s.packet_xfr_size;
			ep->ep->state &= ~USBD_EP_STATE_TOC;
			ep->ep->state |= USBD_EP_STATE_BUSY;
			snx_udc_epx_read_packet_iso(req, ep->num, USB_DIR_IN, ep->ep->state);
		}
	}
	else if(ep->num == 4) {
		if(!(ep->ep->state & USBD_EP_STATE_BUSY)){
            TRB_SLICE_LENGTH[ep->num] = ep->ep->s.packet_xfr_size;
			ep->ep->state &= ~USBD_EP_STATE_TOC;
			ep->ep->state |= USBD_EP_STATE_BUSY;
			snx_udc_epx_write_packet_iso(req, ep->num, USB_DIR_OUT, ep->ep->state);
		}	
	}
	else if(ep->num == 1 ) {
		if(!(ep->ep->state & USBD_EP_STATE_BUSY)) {
			ep->ep->state &= ~USBD_EP_STATE_TOC;
			ep->ep->state |= USBD_EP_STATE_BUSY;
			snx_udc_epx_write_packet(req, ep->num, USB_DIR_IN, (ep->ep->state&USBD_EP_STATE_0_PACKET_TAIL));
		}
	}

	NVIC_EnableIRQ(USBDEV_IRQn);

	return 0;
}
/**
* @brief interface function - stop udc queue with assigning endpoint
* @param _ep the pointer of usb enpoint
* @return value of error code.
*/
static int32_t snx_udc_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct snx_request      *req;
	struct _sonix_udc_ep    *ep;
	struct _sonix_udc       *udc;	
	
	req = to_snx_req(_req);
	ep = to_snx_ep(_ep);
	USBD_PRINT( "%s num : %d len=%d ep->state : %08X\n",__FUNCTION__, ep->num,req->req.length,ep->ep->state);
	
	udc = ep->dev;
	if ( !udc->driver /*|| udc->gadget.speed == USB_SPEED_UNKNOWN)*/) {
		return -ESHUTDOWN;
	}
	
	if(ep->num == 4) {
		ep->ep->state |= USBD_EP_STATE_TSTOP;
		snx_udc_epx_write_packet_iso(req, ep->num, USB_DIR_OUT, ep->ep->state);
		ep->ep->state &= ~USBD_EP_STATE_TSTOP;
	}	
	
	return 0;
}
/**
* @brief interface function - read/write assigning endpoint data to fifo
* @param req the pointer of enpoint request
* @param num enpoitn number
* @param dir enpoitn direction
* @return value of error code.
*/
void snx_udc_epx_rw_packet(uint8_t *buf, uint32_t len)
{
	uint32_t volatile ccs,ep_sts;
	uint32_t remainder,idx = 0;

	trb_struct_t *trb;

	udc_write(USBD_REG_EP_SEL, (12 | USB_DIR_IN));
	ep_sts	= udc_read(USBD_REG_EP_STS);
	ccs 	= (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs 	= (ccs >> 11);

	remainder	= len;
	trb			= trb_ep[12];

	while(remainder > TRB_MAX_SIZE) {
		trb->data_buffer_pointer	= (uint32_t)(&(buf[idx * 65536]));
		trb->length_and_burst		= (uint32_t)(TRB_MAX_SIZE| TRB_BurstLength_INCR16);
		trb->control				= TRB_NORMAL | ccs| TRB_ISP | TRB_IOC;
		remainder					-= TRB_MAX_SIZE;
		trb++;
		idx++;

	}

	trb->data_buffer_pointer = (uint32_t)(&(buf[idx * 65536]));
	trb->length_and_burst = (uint32_t)(remainder| TRB_BurstLength_INCR16);

	trb->control = TRB_NORMAL | TRB_ISP | TRB_IOC | ccs;
	trb++;


	trb->data_buffer_pointer = (uint32_t)(trb_ep[12]);
	trb->length_and_burst = 0;
	trb->control = TRB_LINK | TRB_TC | TRB_CH;
	udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep[12]);
	udc_write(USBD_REG_DRBL, USBRD_DRBL(12, USB_DIR_IN));
	USBD_PRINT( "snx_udc_epx_rw_packet\r\n" );
}

static int32_t snx_udc_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc);
static int32_t snx_udc_ep_disable(struct usb_ep *_ep);
static struct usb_request *snx_udc_alloc_request(struct usb_ep *_ep);
static void snx_udc_free_request(struct usb_ep *_ep, struct usb_request *_req);
static int32_t snx_udc_set_halt(uint32_t ep, uint32_t value);

unsigned char get_ep_buffering(unsigned char index, unsigned char dir)
{
	if (dir == USB_DIR_OUT) {
		switch (index) {
		case 0:
			return ENDPOINT_BUFFERING_0_0;
		case 1:
			return ENDPOINT_BUFFERING_1_0;
		case 2:
			return ENDPOINT_BUFFERING_2_0;
		case 3:
			return ENDPOINT_BUFFERING_3_0;
		case 4:
			return ENDPOINT_BUFFERING_4_0;
		case 5:
			return ENDPOINT_BUFFERING_5_0;
		case 6:
			return ENDPOINT_BUFFERING_6_0;
		case 7:
			return ENDPOINT_BUFFERING_7_0;
		case 8:
			return ENDPOINT_BUFFERING_8_0;
		case 9:
			return ENDPOINT_BUFFERING_9_0;
		case 10:
			return ENDPOINT_BUFFERING_10_0;
		case 11:
			return ENDPOINT_BUFFERING_11_0;
		case 12:
			return ENDPOINT_BUFFERING_12_0;
		case 13:
			return ENDPOINT_BUFFERING_13_0;
		case 14:
			return ENDPOINT_BUFFERING_14_0;
		case 15:
			return ENDPOINT_BUFFERING_15_0;
		}
	} else {
		switch (index) {
		case 0:
			return ENDPOINT_BUFFERING_0_1;
		case 1:
			return ENDPOINT_BUFFERING_1_1;
		case 2:
			return ENDPOINT_BUFFERING_2_1;
		case 3:
			return ENDPOINT_BUFFERING_3_1;
		case 4:
			return ENDPOINT_BUFFERING_4_1;
		case 5:
			return ENDPOINT_BUFFERING_5_1;
		case 6:
			return ENDPOINT_BUFFERING_6_1;
		case 7:
			return ENDPOINT_BUFFERING_7_1;
		case 8:
			return ENDPOINT_BUFFERING_8_1;
		case 9:
			return ENDPOINT_BUFFERING_9_1;
		case 10:
			return ENDPOINT_BUFFERING_10_1;
		case 11:
			return ENDPOINT_BUFFERING_11_1;
		case 12:
			return ENDPOINT_BUFFERING_12_1;
		case 13:
			return ENDPOINT_BUFFERING_13_1;
		case 14:
			return ENDPOINT_BUFFERING_14_1;
		case 15:
			return ENDPOINT_BUFFERING_15_1;
		}
	}
	return 0;
}
/**
* @brief interface function - closes all opened ep_handlers
*/
void close_all_handlers(void)
{
	if (ep_rst_ready == 0) {
		return;
	}

	udc_write(USBD_REG_EP_SEL, (unsigned int) 0x00);
	udc_write(USBD_REG_EP_TRADDR, (unsigned int)(trb_ep0_out));
	udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_EPRST);
	while((udc_read(USBD_REG_EP_CMD) & USBD_EP_CMD_BIT_EPRST) != 0);
	udelay(3);
	udc_write(USBD_REG_EP_TRADDR, (unsigned int)(trb_ep0_out));

	udc_write(USBD_REG_EP_SEL, (unsigned int) 0x80);
	udc_write(USBD_REG_EP_TRADDR, (unsigned int)(trb_ep0_in));
	udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_EPRST);
	while((udc_read(USBD_REG_EP_CMD) & USBD_EP_CMD_BIT_EPRST) != 0);
	udelay(3);
	udc_write(USBD_REG_EP_TRADDR, (unsigned int)(trb_ep0_in));
#if !defined(CONFIG_DMA_MULTI_MODE_ENABLE)
	g_start_ep0_out_drbl = 0;
#endif	
}
/**
* @brief interface function - prepare endpoint 0 TRB with USBD IP
* @param endp_addr the address of endpoint
* @param buffer the pointer of buffer
* @param buffer_size the size of buffer
*/
extern usbd_class_submode_t snx_usbd_submode;

void snx_udc_ep0_start_TRB(unsigned char endp_addr, unsigned int* buffer, unsigned short buffer_size)
{
	unsigned int  ep_sts = 0, ccs = 0;
	//unsigned int  ep_num = 0;		
	static trb_struct_t *tempTRB = NULL;
	uint32_t trb_addr = 0;
#if defined(CONFIG_DMA_MULTI_MODE_ENABLE)
	uint32_t i = 0;	
	uint32_t data_size_for_last_transfer = 0, cnt = 0;
#endif
	
	USBD_PRINT("NEWTD: Creating TRB for ep:%02X buffer:%08X size:%d\r\n",endp_addr,(uint32_t)(buffer),buffer_size);

#if defined(CONFIG_DMA_SINGLE_MODE_ENABLE)
	if(snx_usbd_submode == USBD_SUBMODE_ISO){
		if(udc_read(USBD_REG_STS)&U32_BIT(3)) {
			udc_write(USBD_REG_CONF,  USBD_CONF_BIT_DEVEN | USBD_CONF_BIT_DSING);
		}
	}
#endif
	udc_write(USBD_REG_EP_SEL, (uint32_t) endp_addr);

	ep_sts = udc_read(USBD_REG_EP_STS);
	ccs = (ep_sts & USBD_EP_STS_BIT_CCS);
	ccs = (ccs >> 11);

	tempTRB = ((endp_addr & USB_DIR_MASK) == USB_DIR_IN) ? trb_ep0_in : trb_ep0_out;

#if defined(CONFIG_DMA_MULTI_MODE_ENABLE)
	data_size_for_last_transfer = buffer_size;
	TRB_SLICE_LENGTH[0] = 64;
	
	if((endp_addr & USB_DIR_MASK) == USB_DIR_IN){
		while(data_size_for_last_transfer > TRB_SLICE_LENGTH[0]){
			//Link serveral LINK TRB
			for(i = 0; i < TRB_MULTI_LINK_CNT; i++){
				tempTRB->data_buffer_pointer = (uint32_t)(tempTRB+1);
				tempTRB->length_and_burst = 0;
				tempTRB->control = TRB_LINK | TRB_CH | ccs;
				tempTRB++;
			}
		
			//Normal TRB
			tempTRB->data_buffer_pointer = (uint32_t)buffer + (cnt * TRB_SLICE_LENGTH[0]);
			tempTRB->length_and_burst = TRB_SLICE_LENGTH[0] | TRB_BurstLength;	
			tempTRB->control = TRB_NORMAL | ccs;
			tempTRB++;
			
			data_size_for_last_transfer -= TRB_SLICE_LENGTH[0];
			cnt++;
		}
		
		//Link serveral LINK TRB
		for(i = 0; i < TRB_MULTI_LINK_CNT; i++){
			tempTRB->data_buffer_pointer = (uint32_t)(tempTRB+1);
			tempTRB->length_and_burst = 0;
			tempTRB->control = TRB_LINK | TRB_CH | ccs;
			tempTRB++;
		}
		
		//Normal TRB
		tempTRB->data_buffer_pointer = (uint32_t)buffer + (cnt * TRB_SLICE_LENGTH[0]);
		tempTRB->length_and_burst = data_size_for_last_transfer | TRB_BurstLength;	
//		tempTRB->control = TRB_NORMAL | TRB_IOC | TRB_ISP | ccs;
		tempTRB->control = TRB_NORMAL | ccs;
		tempTRB++;
		
		//Link TRB
		tempTRB->data_buffer_pointer = (uint32_t)trb_ep0_in;
		tempTRB->length_and_burst = 0;
		tempTRB->control = TRB_LINK | TRB_CH | TRB_TC | ccs;		
		
		//Set ERDY
		while (udc_read(USBD_REG_EP_CMD) & USBD_EP_CMD_BIT_ERDY);
		udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_ERDY);
		
		//Set TRB Addr
		trb_addr = (uint32_t)trb_ep0_in;
		udc_write(USBD_REG_EP_SEL, (uint32_t) endp_addr);
		udc_write(USBD_REG_EP_TRADDR, trb_addr);

		//Enable DRBL 
		udc_write(USBD_REG_DRBL, USBRD_DRBL(endp_addr & USBRM_EP_NUM, endp_addr & USBRM_EP_DIR));		
	}
	else{ //USB_DIR_OUT
		//Normal TRB
		tempTRB->data_buffer_pointer = (uint32_t)buffer;
		tempTRB->length_and_burst = buffer_size | TRB_BurstLength;	
		tempTRB->control =  TRB_NORMAL | TRB_IOC | TRB_ISP | ccs;
		tempTRB++;	
		
		//Link TRB
		tempTRB->data_buffer_pointer = (uint32_t)trb_ep0_out;
		tempTRB->length_and_burst = 0;
		//tempTRB->control = TRB_LINK | TRB_CH | ccs;
		tempTRB->control = TRB_LINK | TRB_CH | TRB_TC | ccs;		

		//Set TRB Addr
		trb_addr = (uint32_t)trb_ep0_out;
		udc_write(USBD_REG_EP_SEL, (uint32_t) endp_addr);
		udc_write(USBD_REG_EP_TRADDR, trb_addr);	

		//Enable DRBL 
		udc_write(USBD_REG_DRBL, USBRD_DRBL(endp_addr & USBRM_EP_NUM, endp_addr & USBRM_EP_DIR));
	}
#else
	//Normal TRB
	tempTRB->data_buffer_pointer = (uint32_t)buffer;
	tempTRB->length_and_burst = buffer_size | TRB_BurstLength;	
	//tempTRB->control = ((endp_addr & USB_DIR_MASK) == USB_DIR_IN) ? (TRB_NORMAL|ccs) : (TRB_NORMAL|TRB_IOC|TRB_ISP|ccs);
	tempTRB->control = TRB_NORMAL | TRB_IOC | TRB_ISP | ccs;
	tempTRB++;
		
	//Link TRB
	tempTRB->data_buffer_pointer = ((endp_addr & USB_DIR_MASK) == USB_DIR_IN) ? (uint32_t)trb_ep0_in : (uint32_t)trb_ep0_out;
	tempTRB->length_and_burst = 0;
	tempTRB->control = ((endp_addr & USB_DIR_MASK) == USB_DIR_IN) ? (TRB_LINK|TRB_CH|TRB_TC|ccs) : (TRB_LINK|TRB_CH|ccs);
		
	//Set TRB Addr
	trb_addr = ((endp_addr & USB_DIR_MASK) == USB_DIR_IN) ? (uint32_t)trb_ep0_in : (uint32_t)trb_ep0_out;
	udc_write(USBD_REG_EP_SEL, (uint32_t) endp_addr);
	udc_write(USBD_REG_EP_TRADDR, trb_addr);

	//Enable DRBL 
	if((endp_addr & USB_DIR_MASK) == USB_DIR_IN){
		//Set ERDY
		while (udc_read(USBD_REG_EP_CMD) & USBD_EP_CMD_BIT_ERDY);
		udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_ERDY);
		
		udc_write(USBD_REG_DRBL, USBRD_DRBL(endp_addr & USBRM_EP_NUM, endp_addr & USBRM_EP_DIR));
	}
	else{
		if((udc_read(USBD_REG_STS) & U32_BIT(3)) == 0){ //single dma
			udc_write(USBD_REG_DRBL, USBRD_DRBL(endp_addr & USBRM_EP_NUM, endp_addr & USBRM_EP_DIR));
		}
		else{ //multi dma
			if(!g_start_ep0_out_drbl){
				udc_write(USBD_REG_DRBL, USBRD_DRBL(endp_addr & USBRM_EP_NUM, endp_addr & USBRM_EP_DIR));
				g_start_ep0_out_drbl = 1;
			}
		}
	}	
#endif
//#if 0	
//	trb_ep[0]->data_buffer_pointer =  (uint32_t)buffer;
//	trb_ep[0]->length_and_burst = buffer_size | TRB_BurstLength;
//	if(endp_addr==0x00)
//		trb_ep[0]->control = TRB_NORMAL | ccs | TRB_IOC| TRB_ISP;
//	else
//		trb_ep[0]->control = TRB_NORMAL | ccs ;

//	tempTRB = trb_ep[0];
//	tempTRB++;
//	tempTRB->data_buffer_pointer = (uint32_t)trb_ep[0];
//	tempTRB->length_and_burst = 0;
//	tempTRB->control = TRB_LINK | ccs | TRB_CH;

//	if((udc_read(USBD_REG_STS) & U32_BIT(3)) == 0){ //single dma		
//		udc_write(USBD_REG_EP_SEL, (uint32_t) endp_addr);
//		udc_write(USBD_REG_EP_TRADDR, (unsigned int) trb_ep[0]);
//		udc_write(USBD_REG_DRBL, USBRD_DRBL(endp_addr & USBRM_EP_NUM, endp_addr & USBRM_EP_DIR));
//	}
//	else{ //multi dma		
//		if(endp_addr==0x00){ //ep0_out
//			if(!g_start_ep0_out_drbl){	
//				udc_write(USBD_REG_EP_SEL, (uint32_t) endp_addr);
//				udc_write(USBD_REG_EP_TRADDR, (unsigned int) trb_ep[0]);
//				udc_write(USBD_REG_DRBL, USBRD_DRBL(endp_addr & USBRM_EP_NUM, endp_addr & USBRM_EP_DIR));
//				g_start_ep0_out_drbl = 1;
//			}
//		}
//		else{ //ep0_in
//			udc_write(USBD_REG_EP_SEL, (uint32_t) endp_addr);
//			udc_write(USBD_REG_EP_TRADDR, (unsigned int) trb_ep[0]);
//			udc_write(USBD_REG_DRBL, USBRD_DRBL(endp_addr & USBRM_EP_NUM, endp_addr & USBRM_EP_DIR));
//		}
//	}
//#endif	
}
/**
* @brief interface function - initial assigning endpoint with USBD IP
* @param endp_addr the address of endpoint
* @param endp_type the type of endpoint
* @param endp_size the max packet size of endpoint
*/

void snx_epx_init(unsigned char endp_addr,unsigned char endp_type,unsigned short endp_size)
{
	unsigned int volatile  temp ;
	unsigned int    int_flag;
	unsigned int    ep_config;
	unsigned int    endp_buffering;

	volatile uint32_t ep_sts;

//    if(endp_size == 0x1400) endp_size=0x400;
	endp_buffering = ((endp_size & 0x1800)>>11);
	endp_size &= 0x07FF;

	udc_write(USBD_REG_EP_SEL, endp_addr);
	if(get_ep_num(endp_addr) == 0x07)
		ep_config = USBRD_EP_ENABLED | USBRD_EP_TYPE(endp_type)| USBRD_EP_MAXPKSIZE(endp_size)| USBRD_EP_BUFFERING(endp_buffering);
	else
		ep_config = USBRD_EP_ENABLED | USBRD_EP_TYPE(endp_type)| USBRD_EP_MAXPKSIZE(endp_size)| USBRD_EP_BUFFERING(get_ep_buffering(get_ep_num(endp_addr), get_ep_dir(endp_addr)));

	if(get_ep_num(endp_addr) == 0x07 ) {
		ep_config |= USBRD_EP_MULT(0x02);
		temp =  udc_read(USBD_REG_MISC_CTRL);
		temp |=  USBD_MISC_CTRL_BIT_UVC_ISO_EN | USBD_MISC_CTRL_BIT_SCR_PTS_EXIST;
		udc_write(USBD_REG_MISC_CTRL, temp);
	}
	else if(get_ep_num(endp_addr) == 0x03) {
		temp =  udc_read(USBD_REG_MISC_CTRL);
		temp |=  USBD_MISC_CTRL_BIT_UVC_ISO_EN | USBD_MISC_CTRL_BIT_HLENO;
		udc_write(USBD_REG_MISC_CTRL, temp);
	}

	if (endp_addr & USB_DIR_IN) {
		temp = USBD_EP_STS_BIT_TRBERREN | USBD_EP_STS_BIT_DESCMISEN;
	} else {
		temp = USBD_EP_STS_BIT_TRBERREN | USBD_EP_STS_BIT_DESCMISEN | USBD_EP_STS_BIT_OUTSMMEN;
	}

	if(get_ep_num(endp_addr) == 0x07 /*&& (snx_usbd_submode==USBD_SUBMODE_ISO)*/) {
		temp |= USBD_EP_STS_BIT_ISOERREN;
	}

	udc_write(USBD_REG_EP_CFG, ep_config);												//timeout if share bus
	udc_write(USBD_REG_EP_TRADDR, ( unsigned int) trb_ep[ get_ep_num(endp_addr)]);	//dummy dma address
	udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_EPRST);									//reset after ep_enable

	do {
		ep_sts = udc_read(USBD_REG_EP_CMD);
	} while((ep_sts & USBD_EP_CMD_BIT_EPRST) != 0);
	udelay(3);

	udc_write(USBD_REG_EP_TRADDR, ( unsigned int) trb_ep[ get_ep_num(endp_addr)]);	//dummy dma address
	udc_write(USBD_REG_EP_STS_EN, temp);

	// Enable endpoint interrupts
	temp = udc_read(USBD_REG_EP_IEN);
	int_flag = USBRD_EP_INTERRUPT_EN(get_ep_num(endp_addr),  get_ep_dir(endp_addr));
	temp |= int_flag;
	udc_write(USBD_REG_EP_IEN, temp);

	
	udc_write(USBD_REG_CONF, USBD_CONF_BIT_CFGSET); // configure all enpdoints

	if(get_ep_num(endp_addr) == 0x07 || get_ep_num(endp_addr) == 0x03 || get_ep_num(endp_addr) == 0x04) {
		udc_write(USBD_REG_EP_SEL, endp_addr);
		udc_write(USBD_REG_EP_NEW_MAXPKTSIZE, USBD_EP_NEW_MAXPKTSIZE_EN | endp_size);
	}

	udc_write(USBD_REG_EP_SEL, endp_addr);
	udc_write(USBD_REG_EP_TRADDR, ( unsigned int) trb_ep[get_ep_num(endp_addr)]);//dummy dma address
	udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_EPRST);

	do {
		ep_sts = udc_read(USBD_REG_EP_CMD);
	} while((ep_sts & USBD_EP_CMD_BIT_EPRST) != 0);
	udelay(3);

	udc_write(USBD_REG_EP_TRADDR, ( unsigned int) trb_ep[get_ep_num(endp_addr)]);//dummy dma address
	udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CSTALL);
}
/**
* @brief interface function - set halt assigning endpoint with USBD IP
* @param ep the number of endpoint
* @param value the value of status (disable:0/enable:1)
* @return value of error code.
*/
int32_t snx_udc_set_halt(uint32_t ep, uint32_t value)
{
	//struct _sonix_udc_ep   *ep = to_snx_ep(_ep);
	unsigned int    temp;
	volatile uint32_t ep_sts;

	USBD_PRINT("%s-ep = 0x%08X\r\n", __FUNCTION__,ep);

	//udc_write(USBD_REG_EP_SEL, ep->bEndpointAddress);
	udc_write(USBD_REG_EP_SEL, ep);
	if (value) {    //
		udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_SSTALL);
	} else {
		NVIC_DisableIRQ(USBDEV_IRQn);

		temp = udc_read(USBD_REG_EP_TRADDR);
		udc_write(USBD_REG_EP_TRADDR, temp);

		udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_EPRST);
		do {
			ep_sts = udc_read(USBD_REG_EP_CMD);
		} while((ep_sts & USBD_EP_CMD_BIT_EPRST) != 0);
		vTaskDelay(10);

		udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CSTALL);
		udc_write(USBD_REG_EP_TRADDR, temp);

		NVIC_EnableIRQ(USBDEV_IRQn);
	}
	udc_write(USBD_REG_EP_SEL, (unsigned int) 0u);   //resume to EP0
	return 0;
}
/**
* @brief interface function - parsing & execute setup command
* @param setup the structure of control endpoint request data
*/
void parsing_setup_command( struct usb_ctrlrequest *setup)
{
	int                         value;
	uint8_t                     test_cmd = 0;
	struct _sonix_udc           *udc;
	struct usb_composite_dev    *cdev;
	struct usb_request          *req;
	int                         ret;
	uint16_t                    status_value = 0;

	USBD_PRINT("bRequestType = %02X, Request = %02X, value = %04X, index = %04X, length = %04X\n",  setup->bRequestType, setup->bRequest,setup->wValue, setup->wIndex, setup->wLength);

	udc= &snx_udc;
	cdev = udc->gadget.cdev;
	req = cdev->req;
//	if (setup->bRequestType & USB_DIR_IN)
//		udc->ep0state = EP0_IN_DATA_PHASE;
//	else{
//		if(setup_packet->wLength != 0)
//			udc->ep0state = EP0_OUT_DATA_PHASE;
//		else
//			udc->ep0state = EP0_END_XFER_PHASE;
//	}

	udc->ep0state = EP0_END_XFER_PHASE;
	
	if ((setup->bRequestType & USB_DIR_MASK) == USB_DIR_OUT){
		if(setup_packet->wLength != 0)
			udc->ep0state = EP0_OUT_DATA_PHASE;
	}
	
	if ((setup_packet->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (setup_packet->bRequest) {
		/* Set Address  */
		case USB_REQ_SET_ADDRESS:
			// set address rquest is serviced by hardware
			udc_write(USBD_REG_CMD,(((setup->wValue & 0x007F) << 1) | 1) );
			USBD_PRINT(" SET ADDRESS request = %08X(USBD_REG_CMD = %08X)\n",setup->wValue,(((setup->wValue & 0x007F) << 1) | 1) );
			goto setup_done;

		/* Set Feature  */
		case USB_REQ_SET_FEATURE:
			switch (setup->bRequestType & USB_RECIP_MASK) {
			case USB_RECIP_DEVICE:
				if (setup->wValue == USB_DEVICE_TEST_MODE) {
					test_cmd = (uint8_t)(setup->wIndex>>8);
					USBD_PRINT("USB_TEST_MODE enable, test cmd=%x\n", test_cmd);
					switch(test_cmd) {
					case USBD_TEST_J:
						udc_write(USBD_REG_CMD, USBRD_SET_TESTMODE(USBRV_TM_TEST_J));
						break;
					case USBD_TEST_K:
						udc_write(USBD_REG_CMD, USBRD_SET_TESTMODE(USBRV_TM_TEST_K));
						break;
					case USBD_TEST_SE0_NAK:
						udc_write(USBD_REG_CMD, USBRD_SET_TESTMODE(USBRV_TM_SE0_NAK));
						break;
					case USBD_TEST_PACKET:
						udc_write(USBD_REG_CMD, USBRD_SET_TESTMODE(USBRV_TM_TEST_PACKET));
						break;
					case USBD_TEST_FORCE_ENABLE:
						udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_SSTALL);
						break;
					default:
						udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_SSTALL);
						break;
					}
					goto setup_done;
				}
				break;
			case USB_RECIP_ENDPOINT:
				if ((setup->wValue != USB_ENDPOINT_HALT) || (setup->wLength != 0)) {
					break;
				}
				snx_udc_set_halt(setup->wIndex, 1);
				goto setup_done;
			//break;
			default:
				break;
			}
			break;
		case USB_REQ_CLEAR_FEATURE:
			switch (setup->bRequestType & USB_RECIP_MASK) {
			case USB_RECIP_DEVICE:
				if (setup->wValue == USB_DEVICE_TEST_MODE) {
					USBD_PRINT("USB_TEST_MODE disable\n");
				}
				break;
			case USB_RECIP_ENDPOINT:
				if ((setup->wValue != USB_ENDPOINT_HALT) || (setup->wLength != 0)) {
					break;
				}
				//snx_udc_set_halt(setup->wIndex, 0);
				break;
			default:
				break;
			}
			break;
		/* we handle all standard USB descriptors */
		case USB_REQ_GET_DESCRIPTOR:
			USBD_PRINT(" USB_REQ_GET_DESCRIPTOR ->  %d\r\n",setup->wValue >> 8);
			switch (setup->wValue >> 8) {
			case USB_DT_DEVICE:
				cdev->desc.bNumConfigurations = count_configs(cdev, USB_DT_DEVICE);
				req->length = min((uint16_t)setup->wLength, (uint16_t) sizeof cdev->desc);
				memcpy(req->buf, &cdev->desc, req->length);
				snx_udc_ep0_start_TRB(USB_DIR_IN, (uint32_t*)req->buf, req->length);
				goto setup_done;
			//break;
			case USB_DT_DEVICE_QUALIFIER:
				cdev->qualifier->bNumConfigurations = count_configs(cdev, USB_DT_DEVICE);
				req->length = min((uint16_t)setup->wLength, (uint16_t) sizeof *cdev->qualifier);
				memcpy(req->buf, cdev->qualifier, req->length);
				snx_udc_ep0_start_TRB(USB_DIR_IN, (uint32_t*)req->buf, req->length);
				goto setup_done;
			//break;
			case USB_DT_OTHER_SPEED_CONFIG:
			case USB_DT_CONFIG:
				value = config_desc(cdev, setup->wValue);
				if (value >= 0)
					value = min((uint16_t)setup->wLength, (uint16_t)value);
				req->length = value;
				snx_udc_ep0_start_TRB(USB_DIR_IN, (uint32_t*)req->buf, req->length);
				goto setup_done;
			//break;
			case USB_DT_STRING:
				value = get_string(cdev, req->buf,setup->wIndex, setup->wValue & 0xff);
				if (value >= 0)
					value = min((uint16_t)setup->wLength, (uint16_t)value);
				req->length = value;
				snx_udc_ep0_start_TRB(USB_DIR_IN, (uint32_t*)req->buf, req->length);
				goto setup_done;
			//break;
			case USB_DT_BOS:
				/*
				value = bos_desc(cdev);
				if (value >= 0)
					value = min((uint16_t)setup->wLength, (uint16_t)value);
				req->length = value;
				snx_udc_ep0_start_TRB(USB_DIR_IN, (uint32_t*)req->buf, req->length);
				goto setup_done;
				*/
				break;
			}
			break;

		case USB_REQ_GET_STATUS:
			if((setup->bRequestType & USB_RECIP_MASK) == USB_RECIP_DEVICE) {
				req->buf[0]=((cdev->config->bmAttributes & USB_CONFIG_ATT_SELFPOWER) >> 6);
				req->buf[0]|=((cdev->config->bmAttributes  & USB_CONFIG_ATT_WAKEUP) >> 4);
				req->length = setup->wLength;
				snx_udc_ep0_start_TRB(USB_DIR_IN, (uint32_t*)req->buf, req->length);
			}
			else if((setup->bRequestType & USB_RECIP_MASK) == USB_RECIP_ENDPOINT) {
				udc_write(USBD_REG_EP_SEL, udc->ep[setup->wIndex & 0x7f].bEndpointAddress);
				if(udc_read(USBD_REG_EP_STS) & USBD_EP_STS_BIT_STALL) {
					status_value = 1;	//Endpoint halted
				} else {
					status_value = 0;	//Endpoint free
				}
				req->length = setup->wLength;
				*(unsigned short *)req->buf = status_value;
				snx_udc_ep0_start_TRB(USB_DIR_IN, (uint32_t *)req->buf, req->length);
				goto setup_done;
			}
			goto setup_done;
		case USB_REQ_SET_CONFIGURATION:
			ep_rst_ready = 1;
			break;
		default:
			break;
		}
	}
	if (!udc->driver)
		return;
	
	ret = udc->driver->setup(&udc->gadget,setup_packet);
	
	if (ret < 0) {
		if (ret == -EOPNOTSUPP) {
			USBD_PRINT("Operation not supported\r\n");
			//USBD_PRINT("snx_udc_set_halt setup->wIndex = 08x \n",setup->wIndex);
			snx_udc_set_halt(setup->wIndex, 1);
			//udc->ep0state = EP0_SETUP_PHASE;
			goto setup_done;
		}
		else if(ret == -EINVAL) {
			//udc->ep0state = EP0_SETUP_PHASE;
			goto setup_done;
		}
		else {
			USBD_PRINT("dev->driver->setup failed. (%d)\n", ret);
		}
	}	

	if ((setup->bRequestType & USB_TYPE_MASK  ) == USB_TYPE_CLASS && (ret!=0x10000))
		return;
	if ( setup->wIndex>> 8 != 0)
		return;

	if ((setup->bRequestType & USB_DIR_IN  ) == USB_DIR_IN)
		return;
	
setup_done:
	if(udc->ep0state == EP0_END_XFER_PHASE){
		udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_ERDY); //status stage
//		trb_ep0_out->data_buffer_pointer = (uint32_t)setup_packet; //restore buffer
//		trb_ep0_out->length_and_burst = (8) | TRB_BurstLength; //restore trb length
		udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CMPL);		
		udc->ep0state = EP0_SETUP_PHASE;
	}
	
	return;
#if 0
	if (!udc->driver)
		return;
	if (setup->bRequestType & USB_DIR_IN)
		udc->ep0state = EP0_IN_DATA_PHASE;
	else
		udc->ep0state = EP0_OUT_DATA_PHASE;

	ret = udc->driver->setup(&udc->gadget,setup_packet);
	if (ret < 0) {
		if (ret == -EOPNOTSUPP) {
			USBD_PRINT("Operation not supported\r\n");
			//USBD_PRINT("snx_udc_set_halt setup->wIndex = 08x \n",setup->wIndex);
			snx_udc_set_halt(setup->wIndex, 1);
			udc->ep0state = EP0_SETUP_PHASE;
			goto setup_done;
		}
		else if(ret == -EINVAL) {
			udc->ep0state = EP0_SETUP_PHASE;
			goto setup_done;
		}
		else {
			USBD_PRINT("dev->driver->setup failed. (%d)\n", ret);
		}
	}

	udc->ep0state = EP0_SETUP_PHASE;
	if ((setup->bRequestType & USB_TYPE_MASK  ) == USB_TYPE_CLASS && (ret!=0x10000))
		return;
	if ( setup->wIndex>> 8 != 0)
		return;

	if ((setup->bRequestType & USB_DIR_IN  ) == USB_DIR_IN)
		return;

setup_done:
	while (udc_read(USBD_REG_EP_CMD) & USBD_EP_CMD_BIT_ERDY);
	//udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CMPL | USBD_EP_CMD_BIT_ERDY); //status stage
	udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_ERDY); //status stage
	trb_ep0_out->data_buffer_pointer = (uint32_t)setup_packet; //restore buffer
	trb_ep0_out->length_and_burst = (8) | TRB_BurstLength; //restore trb length
	udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CMPL);
	USBD_PRINT("%s end\r\n",__FUNCTION__ );
#endif
	
}


/**
* @brief interface function - initial UDC
*/
void udc_init(void)
{
	unsigned int i;

	USBD_PRINT("%s\r\n",__FUNCTION__ );

	setup_packet = NULL;
	usbd_irq_Queue = NULL;
	usbd_ep0_Queue = NULL;
	usbd_ep0_out_finish = NULL;
	trb_ep0_in = NULL;	
	trb_ep0_out = NULL;	
	
	for(i=0; i<USBD_EP_TOTAL_NUMBER; i++) {
		trb_ep[i] = NULL;
	}

	usbd_ep_function[0]		= usbd_ep0_function;
	usbd_ep_function[1]		= usbd_ep1_function;
	usbd_ep_function[2] 	= usbd_ep2_function;
	usbd_ep_function[3] 	= usbd_ep3_function;
	usbd_ep_function[4] 	= usbd_ep4_function;
	usbd_ep_function[5] 	= usbd_ep5_function;
	usbd_ep_function[6] 	= usbd_ep6_function;
	usbd_ep_function[7] 	= usbd_ep7_function;
	usbd_ep_function[8] 	= usbd_ep8_function;
	usbd_ep_function[9] 	= usbd_ep9_function;
	usbd_ep_function[10]	= usbd_ep10_function;
	usbd_ep_function[11]	= usbd_ep11_function;
	usbd_ep_function[12]	= usbd_ep12_function;
	usbd_ep_function[13]	= usbd_ep13_function;
	usbd_ep_function[14]	= usbd_ep14_function;
	usbd_ep_function[15]	= usbd_ep15_function;
	
	// init snx_ep_ops struct
	snx_ep_ops.alloc_request        = snx_udc_alloc_request;
	snx_ep_ops.free_request         = snx_udc_free_request;
	snx_ep_ops.queue                = snx_udc_queue;
	snx_ep_ops.dequeue				= snx_udc_dequeue;
	snx_ep_ops.reset                = snx_udc_reset;
	snx_ep_ops.enable               = snx_udc_ep_enable;
	snx_ep_ops.disable				= snx_udc_ep_disable;
	snx_ep_ops.set_halt             = snx_udc_set_halt;

	for(i=0; i<USBD_EP_TOTAL_NUMBER; i++) {
		snx_udc.ep[i].num = i;
		snx_udc.ep[i].dev = &snx_udc;
		snx_udc.ep[i].ep = &_usb_ep[i];
		snx_udc.ep[i].ep->ops = &snx_ep_ops;
		snx_udc.ep[i].ep->maxpacket = 0;
		strcpy((char*)snx_udc.ep[i].ep->name,"ep-null");
	}
	snx_udc.ep[0].num = 0;
	snx_udc.ep[0].dev = &snx_udc;
	snx_udc.ep[0].ep = &_usb_ep[0];
	snx_udc.ep[0].ep->ops = &snx_ep_ops;
	snx_udc.ep[0].ep->maxpacket = USBD_EP0_MAXPACKET_SIZE;
	strcpy((char*)snx_udc.ep[0].ep->name,"ep0");

	snx_udc.ep[1].num = 1;
	snx_udc.ep[1].dev = &snx_udc;
	snx_udc.ep[1].ep = &_usb_ep[1];
	snx_udc.ep[1].ep->ops = &snx_ep_ops;
	snx_udc.ep[1].ep->maxpacket = USBD_EP1_MAXPACKET_SIZE;
	strcpy((char*)snx_udc.ep[1].ep->name,"ep1-int");

	snx_udc.ep[3].num = 3;
	snx_udc.ep[3].dev = &snx_udc;
	snx_udc.ep[3].ep = &_usb_ep[3];
	snx_udc.ep[3].ep->ops = &snx_ep_ops;
	snx_udc.ep[3].ep->maxpacket = USBD_EP3_MAXPACKET_SIZE;
	strcpy((char*)snx_udc.ep[3].ep->name,"ep3-iso");

	snx_udc.ep[4].num = 4;
	snx_udc.ep[4].dev = &snx_udc;
	snx_udc.ep[4].ep = &_usb_ep[4];
	snx_udc.ep[4].ep->ops = &snx_ep_ops;
	snx_udc.ep[4].ep->maxpacket = USBD_EP4_MAXPACKET_SIZE;
	strcpy((char*)snx_udc.ep[4].ep->name,"ep4-iso");
	
	snx_udc.ep[5].num = 5;
	snx_udc.ep[5].dev = &snx_udc;
	snx_udc.ep[5].ep = &_usb_ep[5];
	snx_udc.ep[5].ep->ops = &snx_ep_ops;
	snx_udc.ep[5].ep->maxpacket = USBD_EP5_MAXPACKET_SIZE;
	strcpy((char*)snx_udc.ep[5].ep->name,"ep5-int");
	
	snx_udc.ep[7].num = 7;
	snx_udc.ep[7].dev = &snx_udc;
	snx_udc.ep[7].ep = &_usb_ep[7];
	snx_udc.ep[7].ep->ops = &snx_ep_ops;
	snx_udc.ep[7].ep->maxpacket = USBD_EP7_MAXPACKET_SIZE;
	strcpy((char*)snx_udc.ep[7].ep->name,"ep7-iso");

	snx_udc.ep[12].num = 12;
	snx_udc.ep[12].dev = &snx_udc;
	snx_udc.ep[12].ep = &_usb_ep[12];
	snx_udc.ep[12].ep->ops = &snx_ep_ops;
	snx_udc.ep[12].ep->maxpacket = USBD_EP12_MAXPACKET_SIZE;
	strcpy((char*)snx_udc.ep[12].ep->name,"epc-bulk");

	snx_udc.ep[13].num = 13;
	snx_udc.ep[13].dev = &snx_udc;
	snx_udc.ep[13].ep = &_usb_ep[13];
	snx_udc.ep[13].ep->ops = &snx_ep_ops;
	snx_udc.ep[13].ep->maxpacket = USBD_EP13_MAXPACKET_SIZE;
	strcpy((char*)snx_udc.ep[13].ep->name,"epd-bulk");

	snx_udc.ep[14].num = 14;
	snx_udc.ep[14].dev = &snx_udc;
	snx_udc.ep[14].ep = &_usb_ep[14];
	snx_udc.ep[14].ep->ops = &snx_ep_ops;
	snx_udc.ep[14].ep->maxpacket = USBD_EP14_MAXPACKET_SIZE;
	strcpy((char*)snx_udc.ep[14].ep->name,"epe-bulk");
	
	snx_udc.gadget.ep0 = snx_udc.ep[0].ep;

	/* Initialize udc */
	udc_enable();
}

/**
* @brief interface function - Enable UDC
*/
void udc_enable(void)
{
	unsigned int volatile temp;

	// Create USB Device IRQ Queue
	usbd_irq_Queue = xQueueCreate(100, sizeof(struct _usb_irq_status));

	// Create ep0 IRQ Queue
	usbd_ep0_Queue = xQueueCreate(50, sizeof(struct _usb_irq_status));
	
	// Create ep0 Out Semaphore
	usbd_ep0_out_finish = xSemaphoreCreateCounting(1,0);

	setup_packet = (struct usb_ctrlrequest*)usbd_malloc(sizeof(struct usb_ctrlrequest));


	if(!trb_ep0_in)
		trb_ep0_in = (trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*256);
	memset(trb_ep0_in, 0, sizeof(trb_struct_t)*256);
	
	if(!trb_ep0_out)
		trb_ep0_out = (trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*64);	
	memset(trb_ep0_out, 0, sizeof(trb_struct_t)*64);
	
	if(!trb_ep[0])
		trb_ep[0] = (trb_struct_t *)usbd_malloc(sizeof(trb_struct_t));
#if defined( CONFIG_MODULE_USBD_HID_CLASS )   	
	if(!trb_ep[1])
		trb_ep[1]=(trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*64);
#endif	
#if defined( CONFIG_MODULE_USBD_UAC_CLASS )   
	if(!trb_ep[3])
		trb_ep[3]=(trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*128);	//1280
	#if USBD_UAC_ISOC_OUT_SUPPORT
	if(!trb_ep[4])
		//trb_ep[4]=(trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*800*2);	//set as 8 * UAC_OUT_XFR_CNT
		trb_ep[4]=(trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*2000);	//set as 8 * UAC_OUT_XFR_CNT
	#endif
#endif    
#if defined( CONFIG_MODULE_USBD_UVC_CLASS ) 	
	if(!trb_ep[5])
		trb_ep[5]=(trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*64);

	if(!trb_ep[7])
		trb_ep[7]=(trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*1280);
	
	if(!trb_ep[14])
		trb_ep[14]=(trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*128);		
#endif  

#if defined( CONFIG_MODULE_USBD_MSC_CLASS ) 	
	if(!trb_ep[12])
		trb_ep[12]=(trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*128);

	if(!trb_ep[13])
		trb_ep[13]=(trb_struct_t *)usbd_malloc(sizeof(trb_struct_t)*128);
	
#endif
	/* Initialize udc */

	//retry enable
	//temp =  udc_read(USBD_REG_MISC_CTRL);
	//temp |=  USBD_MISC_CTRL_BIT_RETRY_EN;
	//udc_write(USBD_REG_MISC_CTRL, temp);
	//printf("MISC=0X%08x\n",udc_read(USBD_REG_MISC_CTRL));

	//udc_write(USBD_REG_CONF, USBD_CONF_BIT_SWRST);

	udc_write(USBD_REG_CONF, USBD_CONF_BIT_LENDIAN);
	udc_write(USBD_REG_CONF, USBD_CONF_BIT_L1DS);
//	udc_write(USBD_REG_CONF, USBD_CONF_BIT_CLK2OFFDS);
	udc_write(USBD_REG_CONF,USBD_CONF_BIT_CLK2OFFEN);
	udc_write(USBD_REG_ISTS, 0xFFFFFFFF);

	/* Initialize udc enable */
	temp = USBD_IEN_BIT_UHSFSRESIEN | USBD_IEN_BIT_CON2IEN | USBD_IEN_BIT_DIS2IEN | USBD_IEN_BIT_CFGRSESIEN | USBD_IEN_BIT_REGTOIEN;
	temp = temp | USBD_IEN_BIT_L2ENTIEN | USBD_IEN_BIT_L2EXTIEN | USBD_IEN_BIT_L1ENTIEN | USBD_IEN_BIT_L1EXTIEN | USBD_IEN_BIT_WAKEIEN;

	udc_write(USBD_REG_IEN, temp);
	udc_write(USBD_REG_EP_IEN, USBD_EP_IEN_BIT_EP0_IN | USBD_EP_IEN_BIT_EP0_OUT);
#if defined(CONFIG_DMA_SINGLE_MODE_ENABLE)
	udc_write(USBD_REG_CONF, USBD_CONF_BIT_DSING | USBD_CONF_BIT_DEVEN);
#elif defined(CONFIG_DMA_MULTI_MODE_ENABLE)
	udc_write(USBD_REG_CONF,  USBD_CONF_BIT_DEVEN | USBD_CONF_BIT_DMULT);	
#endif	

	snx_udc_power_on();

//	*((uint32_t*)0x900000B8) = 0x0FC77FF8;	//change phy register value to register from e-fuse

//	//retry enable
//	temp =  udc_read(USBD_REG_MISC_CTRL);
//	//temp |=  USBD_MISC_CTRL_BIT_RETRY_EN;
//	udc_write(USBD_REG_MISC_CTRL, temp);
}
/**
* @brief interface function - Disable UDC
*/
void udc_disable(void)
{
	unsigned int i;
#if defined(CONFIG_PLATFORM_SN98660)
	temp = inl(0x98000058);
	temp &= ~(0x4000 );
	outl((0x98000058), temp);
#endif

	USBD_PRINT("%s\r\n", __FUNCTION__);
	udc_write(USBD_REG_IEN, 0x00);
	udc_write(USBD_REG_CONF, 0x00);

	if(setup_packet) {
		vPortFree(setup_packet);
		setup_packet = NULL;
	}


	if(trb_ep0_in)	{
		vPortFree(trb_ep0_in);
		trb_ep0_in = NULL;
	}	

	if(trb_ep0_out)	{
		vPortFree(trb_ep0_out);
		trb_ep0_out= NULL;
	}	


	for(i=0; i<USBD_EP_TOTAL_NUMBER; i++) {
		if(trb_ep[i]) {
			vPortFree(trb_ep[i]);
			trb_ep[i] = NULL;
		}
	}

	// Delete USB Device IRQ Queue
	if (usbd_irq_Queue) {
		vQueueDelete(usbd_irq_Queue);
		usbd_irq_Queue = NULL;
	}
	
	// Delete ep0 IRQ Queue
	if (usbd_ep0_Queue) {
		vQueueDelete(usbd_ep0_Queue);
		usbd_ep0_Queue = NULL;
	}	
}
/**
* @brief interface function - initial endpoint 0
* @param endp_addr the address of enpoint
* @param flags endpoint status flag
*/
void snx_ep0_init(unsigned char  endp_addr, unsigned int flags)
{
	unsigned int  temp;

	// Configure all interrupts
	if((endp_addr & USB_DIR_MASK) == USB_DIR_IN)
		temp = USBD_EP_STS_BIT_SETUPEN | USBD_EP_STS_BIT_TRBERREN | USBD_EP_STS_BIT_STPWAITEN /*| USBD_EP_STS_BIT_OUTSMMEN*/;
	else
		temp = USBD_EP_STS_BIT_SETUPEN | USBD_EP_STS_BIT_DESCMISEN | USBD_EP_STS_BIT_TRBERREN | USBD_EP_STS_BIT_STPWAITEN /*| USBD_EP_STS_BIT_OUTSMMEN*/;
	//temp = USBD_EP_STS_BIT_SETUPEN | USBD_EP_STS_BIT_DESCMISEN | USBD_EP_STS_BIT_TRBERREN | USBD_EP_STS_BIT_STPWAITEN /*| USBD_EP_STS_BIT_OUTSMMEN*/;
	
	//USBD_PRINT( "flags=%08x endp_addr==%08x\r\n" ,flags,endp_addr);
	// Configure endpoint
	udc_write(USBD_REG_EP_SEL, endp_addr);
	udc_write(USBD_REG_EP_CFG, flags);
	udc_write(USBD_REG_EP_STS_EN, temp);
}

/**
* @brief interface function - USB device event: disconnect
*/
void usbd_disconnect_function(void)
{
	USBD_PRINT( "%s\r\n",__FUNCTION__);

	close_all_handlers();

#if defined(CONFIG_PLATFORM_SN98660)
	if (usbd_disconnect_cb != NULL) {
		usbd_disconnect_cb();
	}
#endif
}
/**
* @brief interface function - USB device event: suspend
*/
void usbd_suspend_function(void)
{
	USBD_PRINT( "%s\r\n",__FUNCTION__);

#if defined(CONFIG_PLATFORM_SN98660)
	if (usbd_suspend_cb != NULL) {
		usbd_suspend_cb();
	}
#endif
}
/**
* @brief interface function - USB device event: resume
*/
void usbd_resume_function(void)
{
	USBD_PRINT( "%s\r\n",__FUNCTION__);

#if defined(CONFIG_PLATFORM_SN98660)
	if (usbd_resume_cb != NULL) {
		usbd_resume_cb();
	}
#endif
}
/**
* @brief interface function - USB device event: reset
*/
void usbd_reset_function(void)
{
	struct _sonix_udc       *udc;
	usbd_speed_mode         speed_mode;
	uint32_t volatile       usb_sts;
	uint32_t volatile       ists = 0;
	uint32_t                ctrl_config;

	USBD_PRINT( "%s\r\n",__FUNCTION__);

	/* init USB Device Class Driver */
	//USBDClassDrvInit();

	NVIC_DisableIRQ(USBDEV_IRQn);

	udc = &snx_udc;

	close_all_handlers();

	usb_sts=udc_read(USBD_REG_STS);

	speed_mode = UNDEFINED_MODE;
	speed_mode = (usbd_speed_mode) USBRD_GET_USBSPEED(udc_read(USBD_REG_STS));

	switch(speed_mode) {
	case UNDEFINED_MODE:
		udc->gadget.speed = USB_SPEED_UNKNOWN;
		break;

	case USBD_LS_MODE:
		udc->gadget.speed = USB_SPEED_LOW;
		break;

	case USBD_FS_MODE:
		udc->gadget.speed = USB_SPEED_FULL;
		break;

	case USBD_HS_MODE:
		udc->gadget.speed = USB_SPEED_HIGH;
		break;

	case USBD_SS_MODE:
		udc->gadget.speed = USB_SPEED_SUPER;
		break;
	default:
		break;
	}

	USBD_PRINT("%s %d : speed mode = %d\r\n",__FUNCTION__, __LINE__, speed_mode);

	if ((speed_mode == USBD_HS_MODE) || (speed_mode == USBD_FS_MODE)) {
		ctrl_config = USBRD_EP_ENABLED  | USBRD_EP_CONTROL | USBRD_EP_MULT(0) | USBRD_EP_MAXBURST(0)  | USBRD_EP_MAXPKSIZE(64) | USBRD_EP_BUFFERING(get_ep_buffering(0, 0));
		snx_ep0_init(0 | USB_EP_OUT, ctrl_config);
		
		ctrl_config = USBRD_EP_ENABLED  | USBRD_EP_CONTROL | USBRD_EP_MULT(0) | USBRD_EP_MAXBURST(0) | USBRD_EP_MAXPKSIZE(64) | USBRD_EP_BUFFERING(get_ep_buffering(0, 1));
		snx_ep0_init(0 | USB_EP_IN, ctrl_config);
	} else {
		USBD_PRINT("USB Speed Undefined!!! \r\n");
		speed_mode = UNDEFINED_MODE;
	}

	NVIC_EnableIRQ(USBDEV_IRQn);
}
/**
* @brief interface function - USB device event: connect
*/
void usbd_connect_function(void) {
	USBD_PRINT( "%s\r\n",__FUNCTION__);

	usbd_reset_function();
}
/**
* @brief interface function - USB device event: wakeup
*/
void usbd_wakeup_function(void)
{
	USBD_PRINT( "%s\r\n",__FUNCTION__);
}

/**
* @brief interface function - USB device event: ITP/SOF packet detect
*/
void usbd_itpi_function(void)
{
	USBD_PRINT( "%s\r\n",__FUNCTION__);
}

/**
* @brief interface function - USB device event: Register access Timeout
*/
void usbd_regtoi_function(void)
{
	USBD_PRINT( "%s\r\n",__FUNCTION__);
}
/**
* @brief interface function - USB Device all event Handle function (Freertos Task)
* @param pvParameters the parameter of input
*/
void usbd_process( void *pvParameters )
{
	struct _usb_irq_status usbd_irq_status;


	//INTC_IrqSetup(INTC_USB2DEV_IRQ, INTC_LEVEL_TRIG, udc_irq);
	//INTC_IrqEnable(INTC_USB2DEV_IRQ);

	USBD_PRINT("===== usbd_proces start=====");

	// enable interrupt
	USBD_PRINT("pic_enableInterrupt\r\n");
	while(1) {
		xQueueReceive(usbd_irq_Queue,&usbd_irq_status,portMAX_DELAY);

		//taskENTER_CRITICAL();
		USBD_PRINT("[process]usb_event_flag             = 0x%08X\r\n", usbd_irq_status.usb_event_flag);
		USBD_PRINT("[process]eps_event_flag             = 0x%08X\r\n", usbd_irq_status.eps_event_flag);
		while( usbd_irq_status.usb_event_flag != 0x00 ) {
			/* HS/FS mode connection */
			if (usbd_irq_status.usb_event_flag & USBD_ISTS_BIT_CON2I) {
				usbd_connect_function();
				usbd_irq_status.usb_event_flag &= ~( USBD_ISTS_BIT_CON2I);
				//usbd_irq_status.usb_event_flag = 0;
				//xQueueReset(usbd_irq_Queue);
				continue;
			}

			/* USB reset (HS/FS mode) */
			if (usbd_irq_status.usb_event_flag & USBD_ISTS_BIT_U2RESI) {
				usbd_reset_function();
				usbd_irq_status.usb_event_flag &= ~USBD_ISTS_BIT_U2RESI;
				continue;
			}

			/* HS/FS mode disconnection */
			if (usbd_irq_status.usb_event_flag & USBD_ISTS_BIT_DIS2I) {
				usbd_disconnect_function();
				usbd_irq_status.usb_event_flag &= ~USBD_ISTS_BIT_DIS2I;
				continue;
			}

			/* Wakeup */
			if (usbd_irq_status.usb_event_flag & USBD_ISTS_BIT_WAKEI) {
				usbd_wakeup_function();
				usbd_irq_status.usb_event_flag &= ~USBD_ISTS_BIT_WAKEI;
				continue;
			}

			/* LPM L2 state enter */
			if (usbd_irq_status.usb_event_flag & USBD_ISTS_BIT_L2ENTI) {
				usbd_suspend_function();
				usbd_irq_status.usb_event_flag &= ~USBD_ISTS_BIT_L2ENTI;
				continue;
			}

			/* LPM L2 state exit */
			if (usbd_irq_status.usb_event_flag & USBD_ISTS_BIT_L2EXTI) {
				usbd_resume_function();
				usbd_irq_status.usb_event_flag &= ~USBD_ISTS_BIT_L2EXTI;
				continue;
			}

			/* Register access Timeout */
			if (usbd_irq_status.usb_event_flag & USBD_ISTS_BIT_REGTOI) {
				usbd_regtoi_function();
				usbd_irq_status.usb_event_flag &= ~USBD_ISTS_BIT_REGTOI;
				continue;
			}

			/* ITP/SOF packet */
			if (usbd_irq_status.usb_event_flag & USBD_ISTS_BIT_ITPI) {
				usbd_itpi_function();
				usbd_irq_status.usb_event_flag &= ~USBD_ISTS_BIT_ITPI;
				continue;
			}

			if(usbd_irq_status.usb_event_flag != 0x00) {
				usbd_irq_status.usb_event_flag = 0x00;
			}
		}

		if(usbd_irq_status.eps_event_flag) {
			/*check EPX out status*/
			if((usbd_irq_status.eps_event_flag & USB_DIR_IN)==USB_DIR_OUT) {
				usbd_irq_status.ep_status_flag = usbd_ep_function[usbd_irq_status.eps_event_flag-1](usbd_irq_status.ep_status_flag,USB_DIR_OUT);
			}
			/*check EPX in status*/
			else {
				usbd_irq_status.ep_status_flag = usbd_ep_function[(usbd_irq_status.eps_event_flag & (USB_DIR_IN-1))-1](usbd_irq_status.ep_status_flag,USB_DIR_IN);
			}
		}
		//USBD_PRINT("usbd_process-exit !!!\r\n");

		//taskEXIT_CRITICAL();
	}
	//vTaskDelete(NULL);
}
/**
* @brief interface function - USB Device ep0 event function (Freertos Task)
* @param pvParameters the parameter of input
*/
void usbd_ep0_process( void *pvParameters )
{
	struct _usb_irq_status usbd_irq_status;
	
	USBD_PRINT("===== usbd_ep0_process start=====");

	while(1) {
		xQueueReceive(usbd_ep0_Queue, &usbd_irq_status, portMAX_DELAY);	
		/*check EPX out status*/
		if((usbd_irq_status.eps_event_flag & USB_DIR_IN) == USB_DIR_OUT) {
			usbd_irq_status.ep_status_flag = usbd_ep_function[usbd_irq_status.eps_event_flag-1](usbd_irq_status.ep_status_flag,USB_DIR_OUT);
		}
		/*check EPX in status*/
		else{
			usbd_irq_status.ep_status_flag = usbd_ep_function[(usbd_irq_status.eps_event_flag & (USB_DIR_IN-1))-1](usbd_irq_status.ep_status_flag,USB_DIR_IN);
		}		
	}	
}
/**
* @brief interface function - USB Device ISR
* @param irq the number of IRQ .
*/
//void udc_irq(void)
__irq void USBDEV_IRQHandler(void)
{
	uint32_t volatile temp = 0,temp2 = 0;
	uint32_t volatile shift1 = 0,shift2 = 0;
	uint32_t i = 0;
	uint32_t volatile last_epsel = 0;
	uint32_t volatile ep_sts = 0;
	struct _usb_irq_status usbd_irq_status;
	static BaseType_t xHigherPriorityTaskWoken;

	USBD_PRINT("[ISR]udc_irq\r\n");

	//INTC_IrqClear(INTC_USB2DEV_IRQ);
	/*check USB device event */
	temp = udc_read(USBD_REG_ISTS);
	xHigherPriorityTaskWoken = pdFALSE;

	if (temp) {
		USBD_PRINT("[ISR]usb_ists = 0x%08X\r\n", temp);

		usbd_irq_status.usb_event_flag = temp;
		udc_write(USBD_REG_ISTS_CLR,temp);
		
		usbd_irq_status.eps_event_flag = 0;

		xQueueSendToBackFromISR(usbd_irq_Queue,&usbd_irq_status,&xHigherPriorityTaskWoken);
		//=============add by erick

		if((temp & USBD_ISTS_BIT_CON2I) || (temp & USBD_ISTS_BIT_U2RESI)) {
			while(!(temp & USBD_ISTS_BIT_CFGRESI)) {
				temp = udc_read(USBD_REG_ISTS);
			}
		}

		//=============add by erick
		udc_write(USBD_REG_ISTS_CLR,temp);

//        if(xHigherPriorityTaskWoken){
//           portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
//        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		return;
	}


	/*check USB device EP 0~n  event */
	temp = udc_read(USBD_REG_EP_ISTS);                             /*read only & don't need to clear flag*/
	last_epsel = udc_read(USBD_REG_EP_SEL);

	if (temp) {
		USBD_PRINT("[ISR]ep_ists = 0x%08X\r\n", temp);

		usbd_irq_status.usb_event_flag = 0;
		for (i = 0; i < USBD_EP_TOTAL_NUMBER; i++) {
			shift1 = temp >> 16 ;
			shift2 = (0x00000001 << i);
			if( (temp & shift2)) {
				/*check out status*/
				udc_write(USBD_REG_EP_SEL, (i | USB_DIR_OUT));
				temp2= udc_read(USBD_REG_EP_STS);
				udc_write(USBD_REG_EP_SEL,(i | USB_DIR_OUT));
				udc_write(USBD_REG_EP_STS_CLR,temp2);          /*clear usb int flag*/

				usbd_irq_status.ep_status_flag = temp2;
				usbd_irq_status.eps_event_flag = (i+1 | USB_DIR_OUT);
				USBD_PRINT("[ISR]ep_sts = 0x%08X\r\n", temp2);
#if 1
				if(i == 0){
					ep_sts = usbd_irq_status.ep_status_flag;
					if (!(ep_sts & USBD_EP_STS_BIT_SETUP) && ((ep_sts & USBD_EP_STS_BIT_ISP) || (ep_sts & USBD_EP_STS_BIT_IOC))){
						if(xSemaphoreGiveFromISR(usbd_ep0_out_finish, &xHigherPriorityTaskWoken) == pdFALSE){
							printf("send usbd_ep0_out_finish error\n");
						}
					}
					if(xQueueSendToBackFromISR(usbd_ep0_Queue, &usbd_irq_status, &xHigherPriorityTaskWoken) == pdFALSE){
						printf("send usbd_ep0_Queue error\n");
					}
				}
				else{				
					if(xQueueSendToBackFromISR(usbd_irq_Queue, &usbd_irq_status, &xHigherPriorityTaskWoken) == pdFALSE) {
						printf("send usbd_irq_Queue error\n");
					}
				}
#else				
				if(xQueueSendToBackFromISR(usbd_irq_Queue,&usbd_irq_status,&xHigherPriorityTaskWoken) == pdFALSE) {
					printf("send error\n");
				}
#endif				
				//usbd_ep_function[usbd_irq_status.eps_event_flag-1](usbd_irq_status.ep_status_flag,USB_DIR_OUT);
				//temp2=usbd_irq_status.ep_status_flag;
				/*
				if (!(temp2 & USBD_EP_STS_BIT_DESCMIS) && (temp2 & USBD_EP_STS_BIT_STPWAIT)){
					temp2 &= ~USBD_EP_STS_BIT_STPWAIT;
				}
				*/

			}
			if( shift1 & shift2) {
				/*check in status*/
				udc_write(USBD_REG_EP_SEL,(i | USB_DIR_IN));
				temp2 = udc_read(USBD_REG_EP_STS);
				udc_write(USBD_REG_EP_SEL,(i | USB_DIR_IN));
				udc_write(USBD_REG_EP_STS_CLR,temp2);          /*clear usb int flag*/

				if(temp2) {
					usbd_irq_status.ep_status_flag = temp2;
					usbd_irq_status.eps_event_flag = (i+1 | USB_DIR_IN);
					USBD_PRINT("[ISR]ep_sts = 0x%08X\r\n", temp2);
#if 1
					if(i == 0){
						if(xQueueSendToBackFromISR(usbd_ep0_Queue, &usbd_irq_status, &xHigherPriorityTaskWoken) == pdFALSE){
							printf("send usbd_ep0_Queue error\n");
						}					
					}
					else{
						if(xQueueSendToBackFromISR(usbd_irq_Queue, &usbd_irq_status, &xHigherPriorityTaskWoken) == pdFALSE) {
							printf("send usbd_irq_Queue error\n");
						}
					}
#else					
					if(xQueueSendToBackFromISR(usbd_irq_Queue,&usbd_irq_status,&xHigherPriorityTaskWoken) == pdFALSE) {
						printf("send error\n");
					}
#endif					
					//usbd_ep_function[(usbd_irq_status.eps_event_flag & (USB_DIR_IN-1))-1](usbd_irq_status.ep_status_flag,USB_DIR_IN);
					//temp2=usbd_irq_status.ep_status_flag;

				}
			}
		}
	}

	
	udc_write(USBD_REG_EP_SEL, last_epsel);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	return;
}

/**
* @brief interface function - endpoint 0 process function
* @param ep_sts the value of endpoint 0(control endpoint) status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
static uint32_t STPWAIT_rec=0;
unsigned int usbd_ep0_function(uint32_t ep_sts,uint32_t dir)
{
	//unsigned int ep_sts1;
	uint32_t return_value;
	struct _sonix_udc *udc;

	return_value = true;
	udc = &snx_udc;

	USBD_PRINT("%s-(%08x)ep_sts = 0x%08X\r\n", __FUNCTION__, dir,ep_sts);

	if(!ep_sts) return return_value;
	udc_write(USBD_REG_EP_SEL, 0x00 | dir);

	if(ep_sts & USBD_EP_STS_BIT_TRBERR) {
		USBD_PRINT("TRB ERROR Interrupt !!!\r\n");

		ep_sts &= ~USBD_EP_STS_BIT_TRBERR ;
		//return return_value;
	}

	if (!(ep_sts & USBD_EP_STS_BIT_SETUP) && ((ep_sts & USBD_EP_STS_BIT_ISP) || (ep_sts & USBD_EP_STS_BIT_IOC))){
		USBD_PRINT("IOC or ISP !!!\r\n");
		
		ep_sts &= ~(USBD_EP_STS_BIT_ISP | USBD_EP_STS_BIT_IOC);
		if(udc->ep0state == EP0_OUT_DATA_PHASE){
//			trb_ep0_out->data_buffer_pointer = (uint32_t)setup_packet; //restore buffer address
//			trb_ep0_out->length_and_burst = (8) | TRB_BurstLength; //restore trb length
			udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CMPL); 			
			udc->ep0state = EP0_SETUP_PHASE;	
		}
		//else if(udc->ep0state == EP0_IN_DATA_PHASE){
		//	trb_ep0_out->data_buffer_pointer = (uint32_t)setup_packet; //restore buffer
		//	trb_ep0_out->length_and_burst = (8) | TRB_BurstLength; //restore trb length
		//	udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_CMPL);
		//	udc->ep0state = EP0_SETUP_PHASE;
		//}
				
		return ep_sts;
	}

	// Setup 8 Byte has been written into system memory
	if ((ep_sts & USBD_EP_STS_BIT_SETUP) ) {
		USBD_PRINT("SETUP Interrupt !!!\r\n");

		ep_sts &= ~USBD_EP_STS_BIT_SETUP;
		if((ep_sts & USBD_EP_STS_BIT_SETUP) || (ep_sts & USBD_EP_STS_BIT_IOC)){
			ep_sts &= ~(USBD_EP_STS_BIT_ISP | USBD_EP_STS_BIT_IOC);
			parsing_setup_command(setup_packet);
			return ep_sts;
		}
#if 0		
		ep_sts &= ~(USBD_EP_STS_BIT_SETUP | USBD_EP_STS_BIT_ISP | USBD_EP_STS_BIT_IOC);
		parsing_setup_command(setup_packet);
		return ep_sts;
#endif				
	}

	// Device has not prepared TRB for Data Transfer, USB Bus data will be in On-chip ram temporarily
	if ((ep_sts & USBD_EP_STS_BIT_DESCMIS) && (ep_sts & USBD_EP_STS_BIT_STPWAIT)) {
		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS & USBD_EP_STS_BIT_STPWAIT) ;
		if((udc->ep0state == EP0_OUT_DATA_PHASE)||(udc->ep0state == EP0_IN_DATA_PHASE)) {
			USBD_PRINT("ep0state(%08X)\r\n",udc->ep0state);

		}
		else if  (dir == USB_DIR_OUT ) {
			USBD_PRINT("DESCMIS & STPWAIT Interrupt !!!\r\n");

			//udc_write(USBD_REG_CONF, USBD_CONF_BIT_DSING); //resume DMA single for control transfer
			snx_udc_ep0_start_TRB(USB_DIR_OUT, (uint32_t* ) setup_packet, 8);
			STPWAIT_rec = 0;
		}
		return ep_sts;
	}

	if ((ep_sts & USBD_EP_STS_BIT_DESCMIS)) {
		USBD_PRINT("DESCMIS only !!!\r\n");

		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS) ;
		if(STPWAIT_rec) {
			snx_udc_ep0_start_TRB(USB_DIR_OUT, (uint32_t* ) setup_packet, 8);
			STPWAIT_rec = 0;
		}
		return ep_sts;
	}

	if (!(ep_sts & USBD_EP_STS_BIT_DESCMIS) && (ep_sts & USBD_EP_STS_BIT_STPWAIT) ) {
		USBD_PRINT("STPWAIT only Interrupt !!!\r\n");
		{
			STPWAIT_rec=1;
			/*
			    udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep_out);//because dflush will do dummy trb read/write so firmware need to make sure traddr is correct
			    udc_write(USBD_REG_EP_CMD, USBD_EP_CMD_BIT_DFLUSH); //device then expect ep0 desmis
			    udc_write(USBD_REG_EP_TRADDR, (uint32_t)trb_ep_out);
			    udc_write(USBD_REG_EP_SEL, 0);
			*/
			//ep_sts &= ~(USBD_EP_STS_BIT_STPWAIT) ;
		}
		return return_value;
	}

	if (ep_sts & USBD_ISTS_BIT_REGTOI) {
		USBD_PRINT("USBRF_REGTOI Interrupt !!!\r\n");

		ep_sts &= ~USBD_EP_STS_BIT_TRBERR ;
		return ep_sts;
	}

	switch ( udc->ep0state) {
	case EP0_SETUP_PHASE:
		//usbd_ep0_idle();
		break;

	case EP0_IN_DATA_PHASE:
		//usbd_ep0_write_fifo(ep, req);
		break;

	case EP0_OUT_DATA_PHASE:
		//usbd_ep0_read_fifo(ep,req); DESCMIS & STPWAIT Interrupt
		break;

	case EP0_END_XFER_PHASE:
		udc->ep0state = EP0_SETUP_PHASE;
		break;

	case EP0_STALL:
		udc->ep0state = EP0_SETUP_PHASE;
		break;
	}

	ep_sts = 0x00;
	return ep_sts;
}
/**
* @brief interface function - endpoint 1 process function
* @param ep_sts the value of endpoint 1 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep1_function(unsigned int ep_sts,unsigned int dir)
{
	uint32_t 		        return_value = ep_sts;
	uint32_t 		        ep_num = 1;
	struct _sonix_udc 	    *udc = &snx_udc;
	struct _sonix_udc_ep 	*ep = &udc->ep[ep_num];
	struct snx_request 	    *req = ep->req;

	USBD_PRINT("%s,ep_sts= %08x\n", __FUNCTION__,ep_sts);

	if(!ep_sts) return return_value;

	if(ep_sts & (USBD_EP_STS_BIT_TRBERR)) {
		ep_sts &= ~(USBD_EP_STS_BIT_TRBERR) ;
		USBD_PRINT("USBD_EP_STS_BIT_TRBERR clear\r\n");
	}

	if(ep_sts & (USBD_EP_STS_BIT_DESCMIS)) {
		USBD_PRINT("USBD_EP_STS_BIT_DESCMIS & dir=%d\r\n",dir);
		if( dir == USB_DIR_IN) {
			ep->ep->state |= USBD_EP_STATE_IN;
		}
		else {
			ep->ep->state |= USBD_EP_STATE_OUT;
			snx_udc_epx_prepare_packet(req,ep_num,dir);
		}
		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}

	if(ep_sts & (USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP)) {
		ep->ep->state |= (USBD_EP_STATE_TOC|USBD_EP_STATE_TFOC) ;
		ep->ep->state &= ~USBD_EP_STATE_BUSY;
		ep_sts &= ~(USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}

	return_value = ep_sts & (USBD_EP_STS_BIT_DESCMIS|USBD_EP_STS_BIT_IOC|USBD_EP_STS_BIT_ISP);

	return return_value;
}
/**
* @brief interface function - endpoint 2 process function
* @param ep_sts the value of endpoint 2 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep2_function(unsigned int ep_sts,unsigned int dir)
{
	unsigned int return_value=true;
	USBD_PRINT("%s,ep_sts= %08x\r\n", __FUNCTION__,ep_sts);

	if(!ep_sts) 		return return_value;
	return return_value;
}


/**
* @brief interface function - endpoint 3 process function
* @param ep_sts the value of endpoint 3 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep3_function(unsigned int ep_sts,unsigned int dir)
{
	uint32_t 		        return_value = ep_sts;
	uint32_t 		        ep_num = 3;
	struct _sonix_udc 	    *udc = &snx_udc;
	struct _sonix_udc_ep 	*ep = &udc->ep[ep_num];
	struct snx_request 	    *req = ep->req;


	USBD_PRINT("%s,ep_sts= %08x\r\n", __FUNCTION__,ep_sts);
	
	if(!ep_sts) return return_value;

	if(ep_sts & (USBD_EP_STS_BIT_TRBERR)) {
		ep_sts &= ~(USBD_EP_STS_BIT_TRBERR) ;
		USBD_PRINT("USBD_EP_STS_BIT_TRBERR clear\r\n");
	}
	if(ep_sts & (USBD_EP_STS_BIT_DESCMIS)) {
		USBD_PRINT("USBD_EP_STS_BIT_DESCMIS & dir=%d\r\n",dir);
		if( dir == USB_DIR_IN) {
			ep->ep->state |= USBD_EP_STATE_IN;
		}
		else {
			ep->ep->state |= USBD_EP_STATE_OUT;
			snx_udc_epx_prepare_packet(req,ep_num,dir);
		}
		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}
	if(ep_sts & (USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP)) {
		ep->ep->state |= USBD_EP_STATE_TOC ;
		ep->ep->state &= ~USBD_EP_STATE_BUSY;
		ep_sts &= ~(USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}

	return_value = ep_sts & (USBD_EP_STS_BIT_DESCMIS|USBD_EP_STS_BIT_IOC|USBD_EP_STS_BIT_ISP);
	return return_value;
}
/**
* @brief interface function - endpoint 4 process function
* @param ep_sts the value of endpoint 4 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep4_function(unsigned int ep_sts,unsigned int dir)
{
	unsigned int		return_value = ep_sts;
	unsigned int		ep_num = 4;
	unsigned int		i = 0, j = 0;
	unsigned int		addr = 0;
	unsigned int		tempLength = 0;
	
	struct _sonix_udc    *udc = &snx_udc;
	struct _sonix_udc_ep *ep = &udc->ep[ep_num];
	struct snx_request   *req = ep->req;
	static trb_struct_t	 *tempTRB = NULL;


	USBD_PRINT("%s,ep_sts= %08x\n", __FUNCTION__,ep_sts);

	if(!ep_sts) 		return return_value; 

	if(ep_sts & (USBD_EP_STS_BIT_TRBERR)){
		USBD_PRINT("USBD_EP_STS_BIT_TRBERR clear\n");
		ep_sts &= ~(USBD_EP_STS_BIT_TRBERR);
	}

	if(ep_sts & (USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP)){
		ep->ep->state |= USBD_EP_STATE_TOC;
		ep->ep->state &=	~USBD_EP_STATE_BUSY;
		ep_sts &= ~(USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP);
	
		if(!g_odd)
			tempTRB = ep4_ping_normal_trb;
		else
			tempTRB = ep4_pong_normal_trb;
		g_odd = ((~g_odd) & 0x01);

		while(j < ep_gioc_idx[ep_num]){
			if((tempTRB->length_and_burst & 0x1FFFF) == 0){
				tempTRB->length_and_burst = TRB_SLICE_LENGTH[ep_num] | TRB_BurstLength;
			}			
			else if((tempTRB->length_and_burst & 0x1FFFF) != 0 && (addr != tempTRB->data_buffer_pointer)){
				*(((struct usb_request*)(&req->req))->m_length + i) = tempTRB->length_and_burst & 0x1FFFF;
				//erk req->req.length += tempTRB->length_and_burst & 0x1FFFF;
				tempLength += tempTRB->length_and_burst & 0x1FFFF;
				tempTRB->length_and_burst = TRB_SLICE_LENGTH[ep_num] | TRB_BurstLength;
				addr = tempTRB->data_buffer_pointer;
				i++;
			}	
			
			j++;
			tempTRB++;
		}
		if(tempLength == 0){
			//snx_udc_dequeue((struct usb_ep*)ep->ep, (struct usb_request*)&req->req);			
		}
		else{			
			((struct usb_request*)(&req->req))->m_cnt = i;	 
			udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
		}
	}

	if(ep_sts & (USBD_EP_STS_BIT_DESCMIS)) {                	
		ep->ep->state |= USBD_EP_STATE_OUT;
		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS);
		snx_udc_queue((struct usb_ep*)ep->ep, (struct usb_request*)&req->req);		
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
		g_odd = 0;
	}

	return_value = ep_sts & (USBD_EP_STS_BIT_DESCMIS|USBD_EP_STS_BIT_ISOERR|USBD_EP_STS_BIT_IOC|USBD_EP_STS_BIT_ISP);
	return return_value;
}
/**
* @brief interface function - endpoint 5 process function
* @param ep_sts the value of endpoint 5 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep5_function(unsigned int ep_sts,unsigned int dir)
{
	uint32_t 		        return_value = ep_sts;
	uint32_t 		        ep_num = 5;
	struct _sonix_udc 	    *udc = &snx_udc;
	struct _sonix_udc_ep 	*ep = &udc->ep[ep_num];
	struct snx_request 	    *req = ep->req;

	USBD_PRINT("%s,ep_sts= %08x\n", __FUNCTION__,ep_sts);

	if(!ep_sts) return return_value;

	if(ep_sts & (USBD_EP_STS_BIT_TRBERR)) {
		ep_sts &= ~(USBD_EP_STS_BIT_TRBERR) ;
		USBD_PRINT("USBD_EP_STS_BIT_TRBERR clear\r\n");
	}

	if(ep_sts & (USBD_EP_STS_BIT_DESCMIS)) {
		USBD_PRINT("USBD_EP_STS_BIT_DESCMIS & dir=%d\r\n",dir);
		if( dir == USB_DIR_IN) {
			ep->ep->state |= USBD_EP_STATE_IN;
		}
		else {
			ep->ep->state |= USBD_EP_STATE_OUT;
			snx_udc_epx_prepare_packet(req,ep_num,dir);
		}
		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}

	if(ep_sts & (USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP)) {
		ep->ep->state |= USBD_EP_STATE_TOC ;
		ep->ep->state |= USBD_EP_STATE_TFOC;
		ep->ep->state &= ~USBD_EP_STATE_BUSY;
		ep_sts &= ~(USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}


	return_value = ep_sts & (USBD_EP_STS_BIT_DESCMIS|USBD_EP_STS_BIT_IOC|USBD_EP_STS_BIT_ISP);

	return return_value;
}
/**
* @brief interface function - endpoint 6 process function
* @param ep_sts the value of endpoint 6 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep6_function(unsigned int ep_sts,unsigned int dir)
{
	unsigned int return_value=true;

	USBD_PRINT("%s,ep_sts= %08x\n", __FUNCTION__,ep_sts);
	if(!ep_sts)             return return_value;
	return return_value;
}
/**
* @brief interface function - endpoint 7 process function
* @param ep_sts the value of endpoint 7 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep7_function(unsigned int ep_sts,unsigned int dir)
{
	uint32_t 		        return_value = ep_sts;
	uint32_t 		        ep_num = 7;
	struct _sonix_udc 	    *udc = &snx_udc;
	struct _sonix_udc_ep 	*ep = &udc->ep[ep_num];
	struct snx_request 	    *req = ep->req;


	USBD_PRINT("%s,ep_sts= %08x\r\n", __FUNCTION__,ep_sts);
	
	if(!ep_sts) return return_value;

	if(ep_sts & (USBD_EP_STS_BIT_TRBERR)) {
		USBD_PRINT("USBD_EP_STS_BIT_TRBERR clear\r\n");
		ep_sts &= ~(USBD_EP_STS_BIT_TRBERR);
	}

	if(ep_sts & (USBD_EP_STS_BIT_DESCMIS)) {
		USBD_PRINT("USBD_EP_STS_BIT_DESCMIS & dir=%d\r\n",dir);
		if( dir == USB_DIR_IN) {
			ep->ep->state |= USBD_EP_STATE_IN;
		}
		else {
			ep->ep->state |= USBD_EP_STATE_OUT;
			snx_udc_epx_prepare_packet(req,ep_num,dir);
		}
		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}

	if(ep_sts & (USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP)) {
		ep->ep->state |= USBD_EP_STATE_TOC ;
		ep->ep->state &= ~USBD_EP_STATE_BUSY;
		ep_sts &= ~(USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}

	return_value = ep_sts & (USBD_EP_STS_BIT_DESCMIS|USBD_EP_STS_BIT_IOC|USBD_EP_STS_BIT_ISP);
	return return_value;
}
/**
* @brief interface function - endpoint 8 process function
* @param ep_sts the value of endpoint 8 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep8_function(unsigned int ep_sts,unsigned int dir)
{
	unsigned int return_value=true;

	USBD_PRINT("%s,ep_sts= %08x\r\n", __FUNCTION__,ep_sts);
	if(!ep_sts) 		return return_value;
	return return_value;
}
/**
* @brief interface function - endpoint 9 process function
* @param ep_sts the value of endpoint 9 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep9_function(unsigned int ep_sts,unsigned int dir)
{
	unsigned int return_value=true;

	USBD_PRINT("%s,ep_sts= %08x\r\n", __FUNCTION__,ep_sts);
	if(!ep_sts) 		return return_value;
	return return_value;
}
/**
* @brief interface function - endpoint 10 process function
* @param ep_sts the value of endpoint 10 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep10_function(unsigned int ep_sts,unsigned int dir)
{
	unsigned int return_value=true;

	USBD_PRINT("%s,ep_sts= %08x\r\n", __FUNCTION__,ep_sts);
	if(!ep_sts) 		return return_value;
	return return_value;
}
/**
* @brief interface function - endpoint 11 process function
* @param ep_sts the value of endpoint 11 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep11_function(unsigned int ep_sts,unsigned int dir)
{
	unsigned int return_value=true;

	USBD_PRINT("%s,ep_sts= %08x\r\n", __FUNCTION__,ep_sts);
	if(!ep_sts) 		return return_value;
	return return_value;
}
/**
* @brief interface function - endpoint 12 process function
* @param ep_sts the value of endpoint 12 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep12_function(unsigned int ep_sts,unsigned int dir)
{
	uint32_t 		        return_value = ep_sts;
	uint32_t 		        ep_num = 12;
	struct _sonix_udc 	    *udc = &snx_udc;
	struct _sonix_udc_ep 	*ep = &udc->ep[ep_num];
	struct snx_request 	    *req = ep->req;

	USBD_PRINT("%s,ep_sts= %08x\r\n", __FUNCTION__,ep_sts);

	if(!ep_sts) return return_value;

	if(ep_sts & (USBD_EP_STS_BIT_TRBERR)) {
		USBD_PRINT("USBD_EP_STS_BIT_TRBERR clear\r\n");
		ep_sts &= ~(USBD_EP_STS_BIT_TRBERR);
	}

	if(ep_sts & (USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP)) {
		ep->ep->state |= USBD_EP_STATE_TOC ;
		ep->ep->state &= ~USBD_EP_STATE_BUSY;
		ep_sts &= ~(USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP) ;
#if defined(CONFIG_DMA_MULTI_MODE_ENABLE)		
		ep->ep->state |= USBD_EP_STATE_TFOC;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
#else		
		if(ep_gioc_idx[ep_num] > 0) {
			udc_write(USBD_REG_EP_SEL, (ep_num | dir));
			udc_write(USBD_REG_DRBL, USBRD_DRBL(ep_num, dir));
			ep_gioc_idx[ep_num]--;
		}
		else {
			ep->ep->state |= USBD_EP_STATE_TFOC;
		}
#endif		
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}
	if(ep_sts & (USBD_EP_STS_BIT_DESCMIS)) {
		USBD_PRINT("USBD_EP_STS_BIT_DESCMIS & dir=%d\r\n",dir);
		if( dir == USB_DIR_IN) {
			ep->ep->state |= USBD_EP_STATE_IN;
		}
		else {
			ep->ep->state |= USBD_EP_STATE_OUT;
			snx_udc_epx_prepare_packet(req,ep_num,dir);
		}
		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}

	return_value = ep_sts & (USBD_EP_STS_BIT_DESCMIS|USBD_EP_STS_BIT_IOC|USBD_EP_STS_BIT_ISP);
	return return_value;
}
/**
* @brief interface function - endpoint 13 process function
* @param ep_sts the value of endpoint 13 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep13_function(unsigned int ep_sts,unsigned int dir)
{
	unsigned int            ep_num;
	struct _sonix_udc       *udc;
	struct _sonix_udc_ep    *ep;
	struct snx_request      *req;
	uint32_t                return_value;

	USBD_PRINT("%s,ep_sts= %08x\n", __FUNCTION__,ep_sts);

	ep_num          = 13;
	udc             = &snx_udc;
	ep              =&udc->ep[ep_num];
	req             = ep->req;
	return_value    = ep_sts;

	if(!ep_sts) return return_value;
	udc_write(USBD_REG_EP_SEL, ep_num | dir);

	if(ep_sts & (USBD_EP_STS_BIT_TRBERR)) {
		ep_sts &= ~(USBD_EP_STS_BIT_TRBERR);
		USBD_PRINT("USBD_EP_STS_BIT_TRBERR clear\n");
	}
	if(ep_sts & (USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP)) {
		ep->ep->state |= USBD_EP_STATE_TOC;
		ep->ep->state &= ~USBD_EP_STATE_BUSY;
		ep_sts &= ~(USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP);
		USBD_PRINT("USBD_EP_STATE_TOC, ep_num=%d, \n", ep_num);

		req->req.length = (trb_ep[ep_num]->length_and_burst & 0x1FFFF);
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}
	if(ep_sts & (USBD_EP_STS_BIT_DESCMIS)) {
		if( dir == USB_DIR_IN) {
			ep->ep->state |= USBD_EP_STATE_IN;
		}
		else {
			ep->ep->state |= USBD_EP_STATE_OUT;
			snx_udc_epx_prepare_packet(req,ep_num,dir);
		}
		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS);
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}

	return_value = ep_sts & (USBD_EP_STS_BIT_DESCMIS|USBD_EP_STS_BIT_IOC|USBD_EP_STS_BIT_ISP);
	return return_value;
}
/**
* @brief interface function - endpoint 14 process function
* @param ep_sts the value of endpoint 14 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep14_function(unsigned int ep_sts,unsigned int dir)
{
	uint32_t 		        return_value = ep_sts;
	uint32_t 		        ep_num = 14;
	struct _sonix_udc 	    *udc = &snx_udc;
	struct _sonix_udc_ep 	*ep = &udc->ep[ep_num];
	struct snx_request 	    *req = ep->req;

	USBD_PRINT("%s,ep_sts= %08x\r\n", __FUNCTION__,ep_sts);
	if(!ep_sts) 		return return_value;

	if(ep_sts & (USBD_EP_STS_BIT_TRBERR)) {
		USBD_PRINT("USBD_EP_STS_BIT_TRBERR clear\r\n");
		ep_sts &= ~(USBD_EP_STS_BIT_TRBERR);
	}

	if(ep_sts & (USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP)) {
		ep->ep->state |= USBD_EP_STATE_TOC ;
		ep->ep->state &= ~USBD_EP_STATE_BUSY;
		ep_sts &= ~(USBD_EP_STS_BIT_IOC | USBD_EP_STS_BIT_ISP) ;
#if defined(CONFIG_DMA_MULTI_MODE_ENABLE)		
		ep->ep->state |= USBD_EP_STATE_TFOC;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
#else		
		if(ep_gioc_idx[ep_num] > 0) {
			udc_write(USBD_REG_EP_SEL, (ep_num | dir));
			udc_write(USBD_REG_DRBL, USBRD_DRBL(ep_num, dir));
			ep_gioc_idx[ep_num]--;
		}
		else {
			ep->ep->state |= USBD_EP_STATE_TFOC;
		}
#endif		
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}
	if(ep_sts & (USBD_EP_STS_BIT_DESCMIS)) {
		USBD_PRINT("USBD_EP_STS_BIT_DESCMIS & dir=%d\r\n",dir);
		if( dir == USB_DIR_IN) {
			ep->ep->state |= USBD_EP_STATE_IN;
		}
		else {
			ep->ep->state |= USBD_EP_STATE_OUT;
			snx_udc_epx_prepare_packet(req,ep_num,dir);
		}
		ep_sts &= ~(USBD_EP_STS_BIT_DESCMIS) ;
		udc->driver->ep_callback[ep_num](&udc->gadget, ep->ep);
	}

	return_value = ep_sts & (USBD_EP_STS_BIT_DESCMIS|USBD_EP_STS_BIT_IOC|USBD_EP_STS_BIT_ISP);
	return return_value;
}
/**
* @brief interface function - endpoint 15 process function
* @param ep_sts the value of endpoint 15 status from USBD IP.
* @param dir direction of endpoint data flow.
* @return value of error code.
*/
unsigned int usbd_ep15_function(unsigned int ep_sts,unsigned int dir)
{
	unsigned int return_value;

	USBD_PRINT("%s,ep_sts= %08x\n", __FUNCTION__,ep_sts);
	return_value = true;
	if(!ep_sts) return return_value;

	return return_value;
}
/**
* @brief interface function - register driver
* @param driver the pointer of driver.
* @return value of error code.
*/
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	int                     retval;
	int                     i;
	struct usb_ep           *_ep;
	struct _sonix_udc_ep    *ep;
	struct _sonix_udc       *udc;

	udc = &snx_udc;

	/* Hook the driver */
	udc->driver = driver;
	//udc->gadget.dev.driver = &driver->driver;


	/* Initialize endpoint list */
	for (i = 0; i < USBD_EP_TOTAL_NUMBER; i++) {
		if(!(&udc->ep[i]))  continue;

		ep = &udc->ep[i];
		_ep = ep->ep;
		_ep->parent_ep = ep;
		udc->gadget.ep_list[i] = _ep ;
		ep->dev = udc;
	}

	if ((retval = driver->bind(&udc->gadget)) != 0) { // ref to composite_bind
		goto register_error;
	}

	return 0;
register_error:
	udc->driver = NULL;
	//udc->gadget.dev.driver = NULL;
	return retval;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct _sonix_udc *udc;

	udc = &snx_udc;
	driver->unbind(&udc->gadget); // ref to composite_unbind
	driver = NULL;

	return 0;
}
/**
* @brief interface function - enable usb request memory in assigning endpoint
* @param _ep the pointer of usb ep.
* @param desc the pointer of dependent descriptor .
* @return value of error code.
*/
static int snx_udc_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct _sonix_udc_ep   *ep;

	USBD_PRINT("%s\n",__FUNCTION__);
	ep = to_snx_ep(_ep);
	ep->bEndpointAddress = desc->bEndpointAddress;
	snx_epx_init(desc->bEndpointAddress,desc->bmAttributes & 0x03,desc->wMaxPacketSize);
	snx_udc_reset(_ep);
	_ep->state &= ~USBD_EP_STATE_DISABLE;
	USBD_PRINT( "bEndpointAddress=%08x, bmAttributes=%08x wMaxPacketSize=%d\n",desc->bEndpointAddress,desc->bmAttributes,desc->wMaxPacketSize);

	return 0;
}

static int snx_udc_ep_disable(struct usb_ep *_ep)
{
	//struct _sonix_udc_ep   *ep;
	USBD_PRINT("%s\n",__FUNCTION__);
	snx_udc_reset(_ep);
	_ep->state |= USBD_EP_STATE_DISABLE;
	return 0;
}
void snx_udc_power_on(void)
{
#if defined(CONFIG_PLATFORM_SN98660)
	temp = inl(0x98000058);
	temp = temp|(0x4000 );
	outl((0x98000058), temp);
	temp = inl(0x9800005c);
	temp = (temp & ~(0x00000002));
	outl((0x9800005c), temp);
#elif defined(CONFIG_PLATFORM_ST53510)
	*((uint32_t*)0x90060000) &= 0x7fffffff;
	*((uint32_t*)0x9006001c) = 0x0000009a;
#elif defined(CONFIG_PLATFORM_SN7320)
	outl((0x45000110), 0x0000000<<8 | 0x00000028); //select PHY0
//	outl((0x45000110), 0x0000000<<8 | 0x00000028 | 0x00000200); //select PHY1
#endif

}

void snx_udc_power_off(void)
{
#if defined(CONFIG_PLATFORM_SN98660)
	temp = inl(0x98000058);
	temp &= ~(0x4000);
	outl(0x98000058, temp);
	temp = inl(0x9800005c);
	temp = (temp & ~(0x00000002));
	outl(0x9800005c, temp);
#endif
}

void snx_udc_fw_reconnection(void)
{
	uint32_t temp;

	temp = udc_read(USBD_REG_CONF);
	udc_write(USBD_REG_CONF, temp | USBD_CONF_BIT_USBDIS);  //status stage
	vTaskDelay(200/portTICK_RATE_MS);
	temp = udc_read(USBD_REG_CONF);
	udc_write(USBD_REG_CONF, temp | USBD_CONF_BIT_SWRST);   //status stage
	USBD_PRINT("USB Device Re-Connection By Firmware.\n");
}
/**
* @brief interface function - allocate usb request memory in assigning endpoint
* @param _ep the point of usb ep.
* @return the point of usb ep request.
*/
static struct usb_request *snx_udc_alloc_request(struct usb_ep *_ep)
{
	struct _sonix_udc_ep *ep;
	struct snx_request *req;

	USBD_PRINT( "%s\n",__FUNCTION__);
	if (!_ep)
		return NULL;

	ep = to_snx_ep(_ep);

	req = (struct snx_request*)usbd_malloc(sizeof *req);
	if (!req)
		return NULL;
	memset(req, 0, sizeof *req);

	ep->req = req;
	req->req.parent_req = req;

	return &req->req;
}
/**
* @brief interface function - free usb request memory in assigning endpoint
* @param _ep the pointer of usb ep.
* @param _req the pointer of usb ep request.
*/
static void snx_udc_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct _sonix_udc_ep    *ep;
	struct snx_request      *req;

	ep      = to_snx_ep(_ep);
	req     = to_snx_req(_req);

	if ( !ep ) return;
	vPortFree(req);
	req = NULL ;
}
