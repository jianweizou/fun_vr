/*
 * opi_phy_scan.c
 *
 *  Created on: 2018年7月5日
 */

#include "type.h"
#include "comm.h"

#define AHB_SLAVE21_BASE	0x30000000

//********************************************************
// Test Function :
// 1. Test OPI DDR DQS scan
//********************************************************
//`include "../vec/testbench/IPSysDef.inc"

//reg [31:0] temp;
//reg [31:0] data;

//reg [7:0] PARAM_DLL_WRDQS_DELAY_S0;
//reg [7:0] PARAM_DLL_WRDQS_DELAY_S1;

//reg [7:0] PARAM_DLL_RDDQS_DELAY_S0;
//reg [7:0] PARAM_DLL_RDDQS_DELAY_S1;

//reg [7:0] PARAM_DLL_WRDQS_DELAY_S0_ORG;

//reg [7:0] WR_DQS0_LOW, WR_DQS0_UPPER;
//reg [7:0] RD_DQS0_LOW, RD_DQS0_UPPER;

//reg [7:0] WR_DQS0_DLY, RD_DQS0_DLY;

//reg [7:0] WR_DQS1_LOW, WR_DQS1_UPPER;
//reg [7:0] RD_DQS1_LOW, RD_DQS1_UPPER;

//reg [7:0] WR_DQS1_DLY, RD_DQS1_DLY;

//reg [31:0] MEM_BASE; 

//reg [7:0] DLY_INCR;

//reg [31:0] test_pattern [0:3];

//reg        ERROR;
//reg        DQS_LOW_FOUND, DQS_UPPER_FOUND;

//reg        TOGGLE;

//reg OPI_RD_LATANCY_FIXED;
//reg OPI_EXTENDED_TEMP;

//parameter SET__DDR_SIZE = 8'h3; //DDR16Mb

//integer i, j, loop;

uint8_t		PARAM_DLL_WRDQS_DELAY_S0;
uint8_t		PARAM_DLL_WRDQS_DELAY_S1;

uint8_t		PARAM_DLL_RDDQS_DELAY_S0;
uint8_t		PARAM_DLL_RDDQS_DELAY_S1;

uint8_t		PARAM_DLL_WRDQS_DELAY_S0_ORG;

uint8_t		WR_DQS0_LOW;
uint8_t		WR_DQS0_UPPER;
uint8_t		RD_DQS0_LOW;
uint8_t		RD_DQS0_UPPER;

uint8_t		WR_DQS0_DLY;
uint8_t		RD_DQS0_DLY;

uint8_t		WR_DQS1_LOW;
uint8_t		WR_DQS1_UPPER;
uint8_t		RD_DQS1_LOW;
uint8_t		RD_DQS1_UPPER;

uint8_t		WR_DQS1_DLY;
uint8_t		RD_DQS1_DLY;

uint32_t	MEM_BASE;

uint8_t		DLY_INCR;

uint32_t	test_pattern[4];

uint8_t		ERROR_RESULT;
uint8_t		DQS_LOW_FOUND;
uint8_t		DQS_UPPER_FOUND;

uint8_t		TOGGLE;

//uint8_t		OPI_RD_LATANCY_FIXED;
uint8_t		OPI_EXTENDED_TEMP;

//********************************************************
//Initialize OPI DDR controller 
//********************************************************
void opi_phy_scan_init()
{
//OPI_RD_LATANCY_FIXED=0;
//OPI_EXTENDED_TEMP=0;
	OPI_EXTENDED_TEMP = 0;
	
//`include "../vec/stimulus/bfm/opi_ddr_initialization_setting.pat"

//set variables
//MEM_BASE = `AHB_SLAVE21_BASE;
//TOGGLE=0;
//DLY_INCR=2;
	MEM_BASE = AHB_SLAVE21_BASE;
	TOGGLE = 0;
	DLY_INCR = 2;
}

//********************************************************
//Start to simulate
//********************************************************
void opi_phy_scan_start()
{
	int				i = 0;
	uint32_t	addr = 0;
	uint32_t	temp = 0;
	uint32_t	data = 0;
	uint32_t	value = 0;
	
	opi_phy_scan_init();
	
////set DLL update cycle
////Decide if DDR 4Byte length is used
//srdata(`AHB_SLAVE20_BASE + 32'hf0, data);
//data[31:16]=16'h0020;   //dll update cycle
//sw(`AHB_SLAVE20_BASE+32'hf0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xf0;
	data = read32(addr);
	data = set_fld(data, 31, 16, 0x20	);
	write32(addr, data);
	
//##########################################################
//find Read DQS 0 delay
//##########################################################
	ERROR_RESULT = 0;
	DQS_LOW_FOUND = 0;
	DQS_UPPER_FOUND = 0;
	PARAM_DLL_RDDQS_DELAY_S0 = 0;	
	
//srdata(`AHB_SLAVE20_BASE + 32'h20, temp);
//PARAM_DLL_WRDQS_DELAY_S0=temp[15:8];
//PARAM_DLL_WRDQS_DELAY_S0_ORG=temp[15:8];
//PARAM_DLL_RDDQS_DELAY_S1=temp[23:16];
//PARAM_DLL_WRDQS_DELAY_S1=temp[31:24];
	addr = AHB_SLAVE20_BASE + 0x20;
	temp = read32(addr);
	PARAM_DLL_WRDQS_DELAY_S0 = (temp>>8)&0xff;
	PARAM_DLL_WRDQS_DELAY_S0_ORG = (temp>>8)&0xff;
	PARAM_DLL_RDDQS_DELAY_S1 = (temp>>16)&0xff;	
	PARAM_DLL_WRDQS_DELAY_S1 = (temp>>24)&0xff;
	
//while (DQS_LOW_FOUND==0 || DQS_UPPER_FOUND==0) begin	
	while (DQS_LOW_FOUND==0 || DQS_UPPER_FOUND==0)
	{
		//set read dqs delay
//	sw(`AHB_SLAVE20_BASE + 32'h20, {PARAM_DLL_WRDQS_DELAY_S1, PARAM_DLL_RDDQS_DELAY_S1, PARAM_DLL_WRDQS_DELAY_S0, PARAM_DLL_RDDQS_DELAY_S0}, `HSIZE_WORD);
		addr = AHB_SLAVE20_BASE + 0x20;
//		setReg(addr, 0x00000000, PARAM_DLL_WRDQS_DELAY_S1, PARAM_DLL_RDDQS_DELAY_S1, PARAM_DLL_WRDQS_DELAY_S0, PARAM_DLL_RDDQS_DELAY_S0);
		value = 0;
		value |= PARAM_DLL_WRDQS_DELAY_S1<<24;
		value |= PARAM_DLL_RDDQS_DELAY_S1<<16;
		value |= PARAM_DLL_WRDQS_DELAY_S0<<8;
		value |= PARAM_DLL_RDDQS_DELAY_S0;
		write32(addr, value);
//		printf("Read DQS 0 : addr = 0x%x, value = 0x%x\r\n", addr, value);
		
		//wait for DLL update cycles
//  for (i=0; i<32; i=i+1) begin
//  	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//  end
		for( i=0; i<32; i++ )
		{
			addr = AHB_SLAVE20_BASE + 0x00;
			data = read32(addr);
		}
		
//	if (TOGGLE) begin
//      test_pattern[0] = 32'hffff_0000;
//      test_pattern[1] = 32'hffff_0000;
//      test_pattern[2] = 32'h5a5a_a5a5;
//      test_pattern[3] = 32'h5a5a_a5a5;
//   end
//   else begin
//      test_pattern[0] = 32'h5a5a_a5a5;
//      test_pattern[1] = 32'h5a5a_a5a5;
//      test_pattern[2] = 32'hffff_0000;
//      test_pattern[3] = 32'hffff_0000;
//   end		
//end
		if (TOGGLE)
		{
			test_pattern[0] = 0xffff0000;
			test_pattern[1] = 0xffff0000;
			test_pattern[2] = 0x5a5aa5a5;
			test_pattern[3] = 0x5a5aa5a5;	
		}
		else
		{
			test_pattern[0] = 0x5a5aa5a5;
			test_pattern[1] = 0x5a5aa5a5;
			test_pattern[2] = 0xffff0000;
			test_pattern[3] = 0xffff0000;	
		}
		
//	bw_i4(`HSIZE_WORD, MEM_BASE, 32'h0000_0000, test_pattern[0], test_pattern[1], test_pattern[2], test_pattern[3]);
		for( i=0; i<4; i++ )
		{
			addr = MEM_BASE+4*i;
			write32(addr, test_pattern[i]);
		}
		
		//read and compare test patterns
//   for (i=0; i<4; i=i+1) begin
//       srdata(MEM_BASE+4*i, temp);
//       if (temp!==test_pattern[i]) begin
//          ERROR=1;
//       end
//   end	
		for( i=0; i<4; i++ )
		{
			addr = MEM_BASE+4*i;
			temp = 0;
			temp = read32(addr);			
			if( temp != test_pattern[i] )
			{
				ERROR_RESULT = 1;
			}
		}
		
		TOGGLE = ~TOGGLE;
		
//	if (DQS_LOW_FOUND==0) begin
//      if (ERROR==0) begin
//         RD_DQS0_LOW = PARAM_DLL_RDDQS_DELAY_S0;      
//         DQS_LOW_FOUND=1;
//      end
//  end
//  else begin
//      if (ERROR==1||PARAM_DLL_RDDQS_DELAY_S0>8'h80) begin
//         RD_DQS0_UPPER = PARAM_DLL_RDDQS_DELAY_S0-DLY_INCR;
//         DQS_UPPER_FOUND=1;
//      end
//  end
		if( DQS_LOW_FOUND == 0 )
		{
			if( ERROR_RESULT == 0 )
			{
				RD_DQS0_LOW = PARAM_DLL_RDDQS_DELAY_S0;
				DQS_LOW_FOUND = 1;
			}
		}
		else
		{
			if( ERROR_RESULT == 1 || PARAM_DLL_RDDQS_DELAY_S0 > 0x80 )
			{
				RD_DQS0_UPPER = PARAM_DLL_RDDQS_DELAY_S0-DLY_INCR;
				DQS_UPPER_FOUND = 1;
			}
		}
		
		PARAM_DLL_RDDQS_DELAY_S0 = PARAM_DLL_RDDQS_DELAY_S0+DLY_INCR;
		ERROR_RESULT = 0;		
	}	//end of while
	
	//update new read/write DQS 0 delay value
	RD_DQS0_DLY = (RD_DQS0_UPPER+RD_DQS0_LOW)/2;
	printf("Read  DQS 0 : RD_DQS0_UPPER = 0x%02x, RD_DQS0_LOW = 0x%02x, RD_DQS0_DLY = 0x%02x\r\n", 
				RD_DQS0_UPPER, RD_DQS0_LOW, RD_DQS0_DLY);
	
//##########################################################
//find Write DQS 0 delay
//##########################################################
	ERROR_RESULT = 0;
	DQS_LOW_FOUND = 0;
	DQS_UPPER_FOUND = 0;
	
//	while (DQS_LOW_FOUND==0 || DQS_UPPER_FOUND==0) begin
	while (DQS_LOW_FOUND==0 || DQS_UPPER_FOUND==0)
	{
		//set write dqs delay
//  sw(`AHB_SLAVE20_BASE + 32'h20, {PARAM_DLL_WRDQS_DELAY_S1, PARAM_DLL_RDDQS_DELAY_S1, PARAM_DLL_WRDQS_DELAY_S0, RD_DQS0_DLY}, `HSIZE_WORD);
		addr = AHB_SLAVE20_BASE + 0x20;
//		setReg(addr, 0x00000000, PARAM_DLL_WRDQS_DELAY_S1, PARAM_DLL_RDDQS_DELAY_S1, PARAM_DLL_WRDQS_DELAY_S0, RD_DQS0_DLY);
		value = 0;
		value |= PARAM_DLL_WRDQS_DELAY_S1<<24;
		value |= PARAM_DLL_RDDQS_DELAY_S1<<16;
		value |= PARAM_DLL_WRDQS_DELAY_S0<<8;
		value |= RD_DQS0_DLY;
		write32(addr, value);
//		printf("Write DQS 0 : addr = 0x%x, value = 0x%x\r\n", addr, value);
		
		//wait for DLL updae cycles
//   for (i=0; i<32; i=i+1) begin
//      srdata(`AHB_SLAVE20_BASE + 32'h00, data); 
//   end
		for( i=0; i<32; i++ )
		{
			addr = AHB_SLAVE20_BASE + 0x00;
			data = read32(addr);
		}
		
//	if (TOGGLE) begin
//      test_pattern[0] = 32'hffff_0000;
//      test_pattern[1] = 32'hffff_0000;
//      test_pattern[2] = 32'h5a5a_a5a5;
//      test_pattern[3] = 32'h5a5a_a5a5;
//  end
//  else begin
//      test_pattern[0] = 32'h5a5a_a5a5;
//      test_pattern[1] = 32'h5a5a_a5a5;
//      test_pattern[2] = 32'hffff_0000;
//      test_pattern[3] = 32'hffff_0000;
//  end
		if (TOGGLE)
		{
			test_pattern[0] = 0xffff0000;
			test_pattern[1] = 0xffff0000;
			test_pattern[2] = 0x5a5aa5a5;
			test_pattern[3] = 0x5a5aa5a5;	
		}
		else
		{
			test_pattern[0] = 0x5a5aa5a5;
			test_pattern[1] = 0x5a5aa5a5;
			test_pattern[2] = 0xffff0000;
			test_pattern[3] = 0xffff0000;	
		}
		
		TOGGLE = ~TOGGLE;
		
		//write test patterns
//  bw_i4(`HSIZE_WORD, MEM_BASE, 32'h0000_0000, test_pattern[0], test_pattern[1], test_pattern[2], test_pattern[3]);
		for( i=0; i<4; i++ )
		{
			addr = MEM_BASE+4*i;
			write32(addr, test_pattern[i]);
		}
		
		//read and compare test patterns
//  for (i=0; i<4; i=i+1) begin
//       srdata(MEM_BASE+4*i, temp);
//       if (temp!==test_pattern[i]) begin
//          ERROR=1;
//       end
//  end
		for( i=0; i<4; i++ )
		{
			addr = MEM_BASE+4*i;
			temp = read32(addr);			
			if( temp != test_pattern[i] )
			{
				ERROR_RESULT = 1;
			}
		}
		
//	if (DQS_LOW_FOUND==0) begin
//      if (ERROR==1||PARAM_DLL_WRDQS_DELAY_S0==0) begin
//         WR_DQS0_LOW = PARAM_DLL_WRDQS_DELAY_S0;      
//         DQS_LOW_FOUND=1;
//         PARAM_DLL_WRDQS_DELAY_S0=PARAM_DLL_WRDQS_DELAY_S0_ORG;
//      end
//      else begin 
//         PARAM_DLL_WRDQS_DELAY_S0=(PARAM_DLL_WRDQS_DELAY_S0<=DLY_INCR) ? 8'b0 : (PARAM_DLL_WRDQS_DELAY_S0-DLY_INCR);
//      end
//  end
//  else begin
//      if (ERROR==1||PARAM_DLL_WRDQS_DELAY_S0>8'h80) begin
//         WR_DQS0_UPPER = PARAM_DLL_WRDQS_DELAY_S0-DLY_INCR; 
//         DQS_UPPER_FOUND=1;
//      end
//      else begin
//         PARAM_DLL_WRDQS_DELAY_S0=(PARAM_DLL_WRDQS_DELAY_S0+DLY_INCR);
//      end
//  end		
//	ERROR=0;
		if( DQS_LOW_FOUND == 0 )
		{
			if( ERROR_RESULT == 1 || PARAM_DLL_WRDQS_DELAY_S0 == 0 )
			{
				WR_DQS0_LOW = PARAM_DLL_WRDQS_DELAY_S0;
				DQS_LOW_FOUND = 1;
				PARAM_DLL_WRDQS_DELAY_S0 = PARAM_DLL_WRDQS_DELAY_S0_ORG;
			}
			else
			{
				PARAM_DLL_WRDQS_DELAY_S0 = (PARAM_DLL_WRDQS_DELAY_S0 <= DLY_INCR) ? 0x00 : (PARAM_DLL_WRDQS_DELAY_S0-DLY_INCR);
			}
		}
		else
		{
			if( ERROR_RESULT == 1 || PARAM_DLL_WRDQS_DELAY_S0 > 0x80 )
			{
				WR_DQS0_UPPER = PARAM_DLL_WRDQS_DELAY_S0-DLY_INCR;
				DQS_UPPER_FOUND = 1;
			}
			else
			{
				PARAM_DLL_WRDQS_DELAY_S0 = (PARAM_DLL_WRDQS_DELAY_S0+DLY_INCR);
			}
		}	
		ERROR_RESULT = 0;
	}	//end of while
	
	//update new write DQS 0 delay value
	WR_DQS0_DLY = (WR_DQS0_UPPER+WR_DQS0_LOW)/2;
	printf("Write DQS 0 : WR_DQS0_UPPER = 0x%02x, WR_DQS0_LOW = 0x%02x, WR_DQS0_DLY = 0x%02x\r\n", 
				WR_DQS0_UPPER, WR_DQS0_LOW, WR_DQS0_DLY);
	
	//update new Read/write DQS 1 delay value
//RD_DQS1_DLY = (RD_DQS1_UPPER+RD_DQS1_LOW)/2;
//sw(`AHB_SLAVE20_BASE + 32'h20, {PARAM_DLL_WRDQS_DELAY_S1, PARAM_DLL_RDDQS_DELAY_S1, WR_DQS0_DLY, RD_DQS0_DLY}, `HSIZE_WORD);
	RD_DQS1_DLY = (RD_DQS1_UPPER+RD_DQS1_LOW)/2;
	addr = AHB_SLAVE20_BASE + 0x20;
//	setReg(addr, 0x00000000, PARAM_DLL_WRDQS_DELAY_S1, PARAM_DLL_RDDQS_DELAY_S1, WR_DQS0_DLY, RD_DQS0_DLY);
	value = 0;
	value |= PARAM_DLL_WRDQS_DELAY_S1<<24;
	value |= PARAM_DLL_RDDQS_DELAY_S1<<16;
	value |= WR_DQS0_DLY<<8;
	value |= RD_DQS0_DLY;
	write32(addr, value);
	
	//wait for DLL update cycles
//for (i=0; i<32; i=i+1) begin
//   srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//end
	for( i=0; i<32; i++ )
	{
		addr = AHB_SLAVE20_BASE + 0x00;
		data = read32(addr);
	}
	
	//write test patterns
//bw_i4(`HSIZE_WORD, MEM_BASE+8, 32'h0000_0000, test_pattern[0], test_pattern[1], test_pattern[2], test_pattern[3]);
	for( i=0; i<4; i++ )
	{
		addr = MEM_BASE+8+4*i;
		write32(addr, test_pattern[i]);
	}
	
	//read and compare test patterns
//for (i=0; i<4; i=i+1) begin
//       srdata(MEM_BASE+8+4*i, temp);
//       if (temp!==test_pattern[i]) begin
//          ERROR=1;
//       end
//end
	for( i=0; i<4; i++ )
	{
		addr = MEM_BASE+8+4*i;
		temp = read32(addr);			
		if( temp != test_pattern[i] )
		{
			ERROR_RESULT = 1;
		}
	}
		
	printf("The Final Read  DQS 0 DELAY = 0x%x\r\n", RD_DQS0_DLY);
	printf("The Final Write DQS 0 DELAY = 0x%x\r\n", WR_DQS0_DLY);
}