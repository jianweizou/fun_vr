#include "wwd_constants.h"
#include "wwd_bus_protocol.h"
#include "string.h"
#include "stdio.h"
#include "sdv2_sd.h"
#include "sdv2_sdio.h"

wwd_result_t sdio_cmd52( wwd_bus_transfer_direction_t direction, uint32_t function, uint32_t address, uint8_t value, sdio_response_needed_t response_expected, uint8_t* response )
{
	struct sd_m2_command m2_cmd_info;
	struct sdio_command sdio_cmd_info;

	sd_resetm2cmd(&m2_cmd_info);
	memset(&sdio_cmd_info, 0, sizeof(struct sdio_command));

	if(direction == BUS_WRITE)
	{
		sdio_cmd_info.addr = address;
		sdio_cmd_info.function = function;
		sdio_cmd_info.value = value;
		sdio_cmd_info.write = BUS_WRITE;
		sd_sdio_cmd52(&sdio_cmd_info, &m2_cmd_info);
		return WWD_SUCCESS;
	}
	else 
	{
		uint8_t rc=0;

		sdio_cmd_info.addr = address;
		sdio_cmd_info.function = function;
		sdio_cmd_info.write = BUS_READ;
		sd_sdio_cmd52(&sdio_cmd_info, &m2_cmd_info);

		*response = (m2_cmd_info.resp[1] & 0x000000ff);
		return (wwd_result_t)rc;
	}
}

uint8_t cmd53_dmabuf[4096];
wwd_result_t sdio_cmd53( wwd_bus_transfer_direction_t direction, uint32_t function, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t address, uint16_t data_size, /*@in@*/ /*@out@*/ uint8_t* data, sdio_response_needed_t response_expected, /*@null@*/ uint32_t* response )
{
	struct sdio_command sdio_cmd_info;
	struct sd_m2_command m2_cmd_info;
	uint8_t result;
	
	if(block_size != 64)
		printf("block_size is %d\r\n", block_size);

	if(data_size > 4096){
		printf("sdio cmd53 data_size > 4096\n");
		while(1);
	}

	sd_resetm2cmd(&m2_cmd_info);
	memset(&sdio_cmd_info, 0, sizeof(struct sdio_command));
	sdio_cmd_info.addr = address;
	sdio_cmd_info.function = function;
	sdio_cmd_info.blk_size = block_size;
	sdio_cmd_info.dma_addr = (uint32_t)cmd53_dmabuf;
	sdio_cmd_info.data_len = data_size;
	sdio_cmd_info.block = (sdio_cmd_info.data_len >= sdio_cmd_info.blk_size)?SNX_SDIO_BLOCK_MODE:SNX_SDIO_BYTE_MODE;
	memset(cmd53_dmabuf, 0, (data_size));
	

	if ( direction == BUS_READ )
	{
		sdio_cmd_info.write = 1;
		result = sd_sdio_cmd53(&sdio_cmd_info, &m2_cmd_info);
		memcpy(data, cmd53_dmabuf, data_size);
	}
	else 
	{
		memcpy(cmd53_dmabuf, data, data_size);
		sdio_cmd_info.write = 0;
		result = sd_sdio_cmd53(&sdio_cmd_info, &m2_cmd_info); 
	}

   return (wwd_result_t)result;
}
