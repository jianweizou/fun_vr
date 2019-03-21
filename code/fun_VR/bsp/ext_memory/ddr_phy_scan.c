/*
 * ddr_phy_scan.c
 *
 *  Created on: 2018年6月26日
 */

#include "type.h"
#include "ddr_init.h"
#include "comm.h"

#define AHB_SLAVE21_BASE	0x30000000
		
//reg [31:0] temp;
//reg [31:0] data;

//reg [7:0] PARAM_DLL_WRDQS_DELAY_S0;
//reg [7:0] PARAM_DLL_WRDQS_DELAY_S1;

//reg [7:0] PARAM_DLL_RDDQS_DELAY_S0;
//reg [7:0] PARAM_DLL_RDDQS_DELAY_S1;

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

//parameter SET__DDR_SIZE = 8'h3; //DDR16Mb

//integer i, j, loop;

u8	PARAM_DLL_WRDQS_DELAY_S0;
u8	PARAM_DLL_WRDQS_DELAY_S1;

u8	PARAM_DLL_RDDQS_DELAY_S0;
u8	PARAM_DLL_RDDQS_DELAY_S1;

u8	WR_DQS0_LOW;
u8	WR_DQS0_UPPER;
u8	RD_DQS0_LOW;
u8	RD_DQS0_UPPER;

u8	WR_DQS0_DLY;
u8	RD_DQS0_DLY;

u8	WR_DQS1_LOW;
u8	WR_DQS1_UPPER;
u8	RD_DQS1_LOW;
u8	RD_DQS1_UPPER;

u8	WR_DQS1_DLY;
u8	RD_DQS1_DLY;

u8	DLY_INCR;
u8	ERROR_RESULT;
u8	DQS_LOW_FOUND;
u8	DQS_UPPER_FOUND;
u32	MEM_BASE;
u32	test_pattern[4];

//********************************************************
//Initialize DDR1 controller 
//********************************************************
void ddr_phy_scan_init()
{
//`include "../vec/stimulus/bfm/ddr_initialization_setting.pat"

////set variables
//MEM_BASE = `AHB_SLAVE21_BASE;
//test_pattern[0] = 32'hffff_0000;
//test_pattern[1] = 32'hffff_0000;
//test_pattern[2] = 32'h5a5a_a5a5;
//test_pattern[3] = 32'h5a5a_a5a5;
//DLY_INCR=2;
	MEM_BASE = AHB_SLAVE21_BASE;
	test_pattern[0] = 0xffff0000;
	test_pattern[1] = 0xffff0000;
	test_pattern[2] = 0x5a5aa5a5;
	test_pattern[3] = 0x5a5aa5a5;	
	DLY_INCR = 2;
}

//********************************************************
//Start to simulate
//********************************************************
void ddr_phy_scan_start()
{
	int	i = 0;
	u32	addr = 0;
	u32	data = 0;
	u32 temp = 0;
	u32 value = 0;
		
	ddr_phy_scan_init();

//            cpu_data = 8'h20; // REF_RATE = 32
//            cpu_write_one_byte(16'h8c17, cpu_data);
//srdata(`AHB_SLAVE20_BASE + 32'h08, data);
//data[29:16]=14'h0020;
//sw(`AHB_SLAVE20_BASE + 32'h08, data, 2'b10);		
	addr = AHB_SLAVE20_BASE + 0x08;
	data = read32(addr);
	data = set_fld(data, 29, 16, 0x20	);
	write32(addr, data);
	
////##########################################################
////find Write DQS 0 delay
////##########################################################
//ERROR=0;
//DQS_LOW_FOUND=0;
//DQS_UPPER_FOUND=0;
//PARAM_DLL_WRDQS_DELAY_S0=0;
	ERROR_RESULT = 0;
	DQS_LOW_FOUND = 0;
	DQS_UPPER_FOUND = 0;
	PARAM_DLL_WRDQS_DELAY_S0 = 0;
	
//srdata(`AHB_SLAVE20_BASE + 32'h20, temp);
//PARAM_DLL_RDDQS_DELAY_S0=temp[7:0];
//PARAM_DLL_RDDQS_DELAY_S1=temp[23:16];
//PARAM_DLL_WRDQS_DELAY_S1=temp[31:24];	
	addr = AHB_SLAVE20_BASE + 0x20;
	temp = read32(addr);
	PARAM_DLL_RDDQS_DELAY_S0 = (temp&0xff);
	PARAM_DLL_RDDQS_DELAY_S1 = (temp>>16)&0xff;	
	PARAM_DLL_WRDQS_DELAY_S1 = (temp>>24)&0xff;
//	printf("Write DQS 0 : temp = 0x%x, RDDQS_DELAY_S0 = 0x%x, RDDQS_DELAY_S1 = 0x%x, WRDQS_DELAY_S1 = 0x%x\r\n",
//	temp, PARAM_DLL_RDDQS_DELAY_S0, PARAM_DLL_RDDQS_DELAY_S1, PARAM_DLL_WRDQS_DELAY_S1);
	
	do
	{	
//while (DQS_LOW_FOUND==0 || DQS_UPPER_FOUND==0) begin
	
//   //set write dqs delay
//   sw(`AHB_SLAVE20_BASE + 32'h20, {PARAM_DLL_WRDQS_DELAY_S1, PARAM_DLL_RDDQS_DELAY_S1, PARAM_DLL_WRDQS_DELAY_S0, PARAM_DLL_RDDQS_DELAY_S0}, `HSIZE_WORD);							
		addr = AHB_SLAVE20_BASE + 0x20;
		value = 0;
		value |= PARAM_DLL_WRDQS_DELAY_S1<<24;
		value |= PARAM_DLL_RDDQS_DELAY_S1<<16;
		value |= PARAM_DLL_WRDQS_DELAY_S0<<8;
		value |= PARAM_DLL_RDDQS_DELAY_S0;
		write32(addr, value);
//		printf("Write DQS 0 : addr = 0x%x, value = 0x%x\r\n", addr, value);
		
//   //wait for refresh cycles
//   for (i=0; i<32; i=i+1) begin
//      srdata(`AHB_SLAVE20_BASE + 32'h00, data); 
//   end
		for( i=0; i<32; i++ )
		{
			addr = AHB_SLAVE20_BASE + 0x00;
			data = read32(addr);
		}

//   //write test patterns
//   bw_i4(`HSIZE_WORD, MEM_BASE, 32'h0000_0000,
//      test_pattern[0], test_pattern[1], test_pattern[2], test_pattern[3]);		
		for( i=0; i<4; i++ )
		{
			addr = MEM_BASE+4*i;
			write32(addr, test_pattern[i]);
		}
		
//   //read and compare test patterns
//   for (i=0; i<4; i=i+1) begin
//       srdata(MEM_BASE+4*i, temp);
//       if (temp!==test_pattern[i]) begin
//          ERROR=1;
//       end
//   end
		for( i=0; i<4; i++)
		{
			addr = MEM_BASE+4*i;
			temp = read32(addr);			
			if( temp != test_pattern[i] )
			{
//				printf("Write DQS 0 : temp = 0x%x, test_pattern[%d] = 0x%x\r\n", temp, i, test_pattern[i]);
				ERROR_RESULT = 1;
			}
		}
				
//   if (DQS_LOW_FOUND==0) begin
//      if (ERROR==0) begin
//         WR_DQS0_LOW = PARAM_DLL_WRDQS_DELAY_S0;      
//         DQS_LOW_FOUND=1;
//      end
//   end
//   else begin
//      if (ERROR==1||PARAM_DLL_WRDQS_DELAY_S0>8'h80) begin
//         WR_DQS0_UPPER = PARAM_DLL_WRDQS_DELAY_S0-DLY_INCR; 
//         DQS_UPPER_FOUND=1;
//      end
//   end
		if( DQS_LOW_FOUND == 0 )
		{
			if( ERROR_RESULT == 0 )
			{
				WR_DQS0_LOW = PARAM_DLL_WRDQS_DELAY_S0;
				DQS_LOW_FOUND = 1;
			}
		}
		else
		{
			if( ERROR_RESULT == 1 || PARAM_DLL_WRDQS_DELAY_S0 > 0x80 )
			{
				WR_DQS0_UPPER = PARAM_DLL_WRDQS_DELAY_S0-DLY_INCR;
				DQS_UPPER_FOUND = 1;
			}
		}
//		printf("Write DQS 0 : DQS_LOW_FOUND = %d, DQS_UPPER_FOUND = %d\r\n", DQS_LOW_FOUND, DQS_UPPER_FOUND);
		
//   PARAM_DLL_WRDQS_DELAY_S0=PARAM_DLL_WRDQS_DELAY_S0+DLY_INCR;
//   ERROR=0;
		PARAM_DLL_WRDQS_DELAY_S0 = PARAM_DLL_WRDQS_DELAY_S0+DLY_INCR;
		ERROR_RESULT = 0;
		
//end	
	}	while( DQS_LOW_FOUND == 0 || DQS_UPPER_FOUND == 0 );
	
	//update new write DQS 0 delay value
	WR_DQS0_DLY = (WR_DQS0_UPPER+WR_DQS0_LOW)/2;
	printf("Write DQS 0 : WR_DQS0_UPPER = 0x%x, WR_DQS0_LOW = 0x%x, WR_DQS0_DLY = 0x%x\r\n", 
				WR_DQS0_UPPER, WR_DQS0_LOW, WR_DQS0_DLY);

//##########################################################
//find Read DQS 0 delay
//##########################################################
//ERROR=0;
//DQS_LOW_FOUND=0;
//DQS_UPPER_FOUND=0;
//PARAM_DLL_RDDQS_DELAY_S0=0;
	ERROR_RESULT = 0;
	DQS_LOW_FOUND = 0;
	DQS_UPPER_FOUND = 0;
	PARAM_DLL_RDDQS_DELAY_S0 = 0;
		
	do
	{
//while (DQS_LOW_FOUND==0 || DQS_UPPER_FOUND==0) begin

//   //set read dqs delay
//   sw(`AHB_SLAVE20_BASE + 32'h20, {PARAM_DLL_WRDQS_DELAY_S1, PARAM_DLL_RDDQS_DELAY_S1, WR_DQS0_DLY, PARAM_DLL_RDDQS_DELAY_S0}, `HSIZE_WORD);
		addr = AHB_SLAVE20_BASE + 0x20;
		value = 0;
		value |= PARAM_DLL_WRDQS_DELAY_S1<<24;
		value |= PARAM_DLL_RDDQS_DELAY_S1<<16;
		value |= WR_DQS0_DLY<<8;
		value |= PARAM_DLL_RDDQS_DELAY_S0;
		write32(addr, value);
//		printf("Read DQS 0 : addr = 0x%x, value = 0x%x\r\n", addr, value);
//   //wait for refresh cycles
//   for (i=0; i<32; i=i+1) begin
//      srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//   end
		for( i=0; i<32; i++ )
		{
			addr = AHB_SLAVE20_BASE + 0x00;
			data = read32(addr);
		}

//   bw_i4(`HSIZE_WORD, MEM_BASE, 32'h0000_0000,
//      test_pattern[0], test_pattern[1], test_pattern[2], test_pattern[3]);
		for( i=0; i<4; i++ )
		{
			addr = MEM_BASE+4*i;
			write32(addr, test_pattern[i]);
		}
		
//   //read and compare test patterns
//   for (i=0; i<4; i=i+1) begin
//       srdata(MEM_BASE+4*i, temp);
//       if (temp!==test_pattern[i]) begin
//          ERROR=1;
//       end
//   end
		for( i=0; i<4; i++)
		{
			addr = MEM_BASE+4*i;
			temp = read32(addr);			
			if( temp != test_pattern[i] )
			{
//				printf("Read DQS 0 : temp = 0x%x, test_pattern[%d] = 0x%x\r\n", temp, i, test_pattern[i]);
				ERROR_RESULT = 1;
			}
		}
		
//		printf("Read DQS 0 : DQS_LOW_FOUND = %d, DQS_UPPER_FOUND = %d\r\n", DQS_LOW_FOUND, DQS_UPPER_FOUND);
		
//   if (DQS_LOW_FOUND==0) begin
//      if (ERROR==0) begin
//         RD_DQS0_LOW = PARAM_DLL_RDDQS_DELAY_S0;      
//         DQS_LOW_FOUND=1;
//      end
//   end
//   else begin
//      if (ERROR==1||PARAM_DLL_RDDQS_DELAY_S0>8'h80) begin
//         RD_DQS0_UPPER = PARAM_DLL_RDDQS_DELAY_S0-DLY_INCR;
//         DQS_UPPER_FOUND=1;
//      end
//   end
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
			if( ERROR_RESULT == 1|| PARAM_DLL_RDDQS_DELAY_S0 > 0x80 )
			{
				RD_DQS0_UPPER = PARAM_DLL_RDDQS_DELAY_S0-DLY_INCR;
				DQS_UPPER_FOUND = 1;
			}
		}

//   PARAM_DLL_RDDQS_DELAY_S0=PARAM_DLL_RDDQS_DELAY_S0+DLY_INCR;
//   ERROR=0;
		PARAM_DLL_RDDQS_DELAY_S0 = PARAM_DLL_RDDQS_DELAY_S0 + DLY_INCR;
		ERROR_RESULT = 0;
//end
	} while( DQS_LOW_FOUND == 0 || DQS_UPPER_FOUND == 0 );

	
//update new read/write DQS 0 delay value
//RD_DQS0_DLY = (RD_DQS0_UPPER+RD_DQS0_LOW)/2;
	RD_DQS0_DLY = (RD_DQS0_UPPER+RD_DQS0_LOW)/2;
	printf("Read DQS 0 : RD_DQS0_UPPER = 0x%x, RD_DQS0_LOW = 0x%x, RD_DQS0_DLY = 0x%x\r\n", 
				RD_DQS0_UPPER, RD_DQS0_LOW, RD_DQS0_DLY);

//##########################################################
//find Write DQS 1 delay
//##########################################################
//ERROR=0;
//DQS_LOW_FOUND=0;
//DQS_UPPER_FOUND=0;
//PARAM_DLL_WRDQS_DELAY_S1=0;
	ERROR_RESULT = 0;
	DQS_LOW_FOUND = 0;
	DQS_UPPER_FOUND = 0;
	PARAM_DLL_WRDQS_DELAY_S1 = 0;
	
	do
	{
//while (DQS_LOW_FOUND==0 || DQS_UPPER_FOUND==0) begin
//   //set write dqs delay
//   sw(`AHB_SLAVE20_BASE + 32'h20, {PARAM_DLL_WRDQS_DELAY_S1, PARAM_DLL_RDDQS_DELAY_S1, WR_DQS0_DLY, RD_DQS0_DLY}, `HSIZE_WORD);
		addr = AHB_SLAVE20_BASE + 0x20;
		value = 0;
		value |= PARAM_DLL_WRDQS_DELAY_S1<<24;
		value |= PARAM_DLL_RDDQS_DELAY_S1<<16;
		value |= WR_DQS0_DLY<<8;
		value |= RD_DQS0_DLY;
		write32(addr, value);
//		printf("Write DQS 1 : addr = 0x%x, value = 0x%08x\r\n", addr, value);
		
//   //wait for refresh cycles
//   for (i=0; i<32; i=i+1) begin
//      srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//   end
		for( i=0; i<32; i++ )
		{
			addr = AHB_SLAVE20_BASE + 0x00;
			data = read32(addr);
		}

//   //write test patterns
//   bw_i4(`HSIZE_WORD, MEM_BASE, 32'h0000_0000,
//      test_pattern[0], test_pattern[1], test_pattern[2], test_pattern[3]);
		for( i=0; i<4; i++ )
		{
			addr = MEM_BASE+4*i;
			write32(addr, test_pattern[i]);
		}
		
//   //read and compare test patterns
//   for (i=0; i<4; i=i+1) begin
//       srdata(MEM_BASE+4*i, temp);
//       if (temp!==test_pattern[i]) begin
//          ERROR=1;
//       end
//   end
		for( i=0; i<4; i++)
		{
			addr = MEM_BASE+4*i;
			temp = read32(addr);
			if( temp != test_pattern[i] )
			{
//				printf("Write DQS 1 : temp = 0x%x, test_pattern[%d] = 0x%x\r\n", temp, i, test_pattern[i]);
				ERROR_RESULT = 1;
			}
		}
		
//   if (DQS_LOW_FOUND==0) begin
//      if (ERROR==0) begin
//         WR_DQS1_LOW = PARAM_DLL_WRDQS_DELAY_S1;      
//         DQS_LOW_FOUND=1;
//      end
//   end
//   else begin
//      if (ERROR==1||PARAM_DLL_WRDQS_DELAY_S1>8'h80) begin
//         WR_DQS1_UPPER = PARAM_DLL_WRDQS_DELAY_S1-DLY_INCR; 
//         DQS_UPPER_FOUND=1;
//      end
//   end
		if( DQS_LOW_FOUND == 0 )
		{
			if( ERROR_RESULT == 0 )
			{
				WR_DQS1_LOW = PARAM_DLL_WRDQS_DELAY_S1;
				DQS_LOW_FOUND = 1;
			}
		}
		else
		{
			if( ERROR_RESULT == 1|| PARAM_DLL_WRDQS_DELAY_S1 > 0x80 )
			{
				WR_DQS1_UPPER = PARAM_DLL_WRDQS_DELAY_S1-DLY_INCR;
				DQS_UPPER_FOUND = 1;
			}
		}
		
//   PARAM_DLL_WRDQS_DELAY_S1=PARAM_DLL_WRDQS_DELAY_S1+DLY_INCR;
//   ERROR=0;
		PARAM_DLL_WRDQS_DELAY_S1 = PARAM_DLL_WRDQS_DELAY_S1 + DLY_INCR;
		ERROR_RESULT = 0;
//end
	}	while( DQS_LOW_FOUND == 0 || DQS_UPPER_FOUND == 0 );

////update new write DQS 1 delay value
//WR_DQS1_DLY = (WR_DQS1_UPPER+WR_DQS1_LOW)/2;
	WR_DQS1_DLY = (WR_DQS1_UPPER+WR_DQS1_LOW)/2;
	printf("Write DQS 1 : WR_DQS1_UPPER = 0x%x, WR_DQS1_LOW = 0x%x, WR_DQS1_DLY = 0x%x\r\n", 
				WR_DQS1_UPPER, WR_DQS1_LOW, WR_DQS1_DLY);
	
//##########################################################
//find Read DQS 1 delay
//##########################################################
//ERROR=0;
//DQS_LOW_FOUND=0;
//DQS_UPPER_FOUND=0;
//PARAM_DLL_RDDQS_DELAY_S1=0;
	ERROR_RESULT = 0;
	DQS_LOW_FOUND = 0;
	DQS_UPPER_FOUND = 0;
	PARAM_DLL_RDDQS_DELAY_S1 = 0;

	do
	{
//while (DQS_LOW_FOUND==0 || DQS_UPPER_FOUND==0) begin

//   //set read dqs delay
//   sw(`AHB_SLAVE20_BASE + 32'h20, {WR_DQS1_DLY, PARAM_DLL_RDDQS_DELAY_S1, WR_DQS0_DLY, RD_DQS0_DLY}, `HSIZE_WORD);
		addr = AHB_SLAVE20_BASE + 0x20;
		value = 0;
		value |= WR_DQS1_DLY<<24;
		value |= PARAM_DLL_RDDQS_DELAY_S1<<16;
		value |= WR_DQS0_DLY<<8;
		value |= RD_DQS0_DLY;
		write32(addr, value);
//		printf("Read DQS 1 : addr = 0x%x, value = 0x%x\r\n", addr, value);
		
//   //wait for refresh cycles
//   for (i=0; i<32; i=i+1) begin
//      srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//   end
		for( i=0; i<32; i++ )
		{
			addr = AHB_SLAVE20_BASE + 0x00;
			data = read32(addr);
		}
		
//   bw_i4(`HSIZE_WORD, MEM_BASE, 32'h0000_0000,
//      test_pattern[0], test_pattern[1], test_pattern[2], test_pattern[3]);
		for( i=0; i<4; i++ )
		{
			addr = MEM_BASE+4*i;
			write32(addr, test_pattern[i]);
		}
		
//   //read and compare test patterns
//   for (i=0; i<4; i=i+1) begin
//       srdata(MEM_BASE+4*i, temp);
//       if (temp!==test_pattern[i]) begin
//          ERROR=1;
//       end
//   end
		for( i=0; i<4; i++)
		{
			addr = MEM_BASE+4*i;
			temp = read32(addr);
			if( temp != test_pattern[i] )
			{
//				printf("Read DQS 1 : temp = 0x%x, test_pattern[%d] = 0x%x\r\n", temp, i, test_pattern[i]);
				ERROR_RESULT = 1;
			}
		}
		
//   if (DQS_LOW_FOUND==0) begin
//      if (ERROR==0) begin
//         RD_DQS1_LOW = PARAM_DLL_RDDQS_DELAY_S1;      
//         DQS_LOW_FOUND=1;
//      end
//   end
//   else begin
//      if (ERROR==1||PARAM_DLL_RDDQS_DELAY_S1>8'h80) begin
//         RD_DQS1_UPPER = PARAM_DLL_RDDQS_DELAY_S1-DLY_INCR;
//         DQS_UPPER_FOUND=1;
//      end
//   end
		if( DQS_LOW_FOUND == 0 )
		{
			if( ERROR_RESULT == 0 )
			{
				RD_DQS1_LOW = PARAM_DLL_RDDQS_DELAY_S1;
				DQS_LOW_FOUND = 1;
			}
		}
		else
		{
			if( ERROR_RESULT == 1|| PARAM_DLL_RDDQS_DELAY_S1 > 0x80 )
			{
				RD_DQS1_UPPER = PARAM_DLL_RDDQS_DELAY_S1-DLY_INCR;
				DQS_UPPER_FOUND = 1;
			}
		}

//   PARAM_DLL_RDDQS_DELAY_S1=PARAM_DLL_RDDQS_DELAY_S1+DLY_INCR;
//   ERROR=0;
		PARAM_DLL_RDDQS_DELAY_S1 = PARAM_DLL_RDDQS_DELAY_S1+DLY_INCR;
		ERROR_RESULT = 0;
//end
	}	while( DQS_LOW_FOUND == 0 || DQS_UPPER_FOUND == 0 );

////update new Read/write DQS 1 delay value
//RD_DQS1_DLY = (RD_DQS1_UPPER+RD_DQS1_LOW)/2;
	RD_DQS1_DLY = (RD_DQS1_UPPER+RD_DQS1_LOW)/2;
	printf("Read DQS 1 : RD_DQS1_UPPER = 0x%x, RD_DQS1_LOW = 0x%x, RD_DQS1_DLY = 0x%x\r\n", 
				RD_DQS1_UPPER, RD_DQS1_LOW, RD_DQS1_DLY);
	
//sw(`AHB_SLAVE20_BASE + 32'h20, {WR_DQS1_DLY, RD_DQS1_DLY, WR_DQS0_DLY, RD_DQS0_DLY}, `HSIZE_WORD);	
	addr = AHB_SLAVE20_BASE + 0x20;
	temp = 0;//read32(addr);
	temp |= WR_DQS1_DLY<<24;
	temp |= RD_DQS1_DLY<<16;
	temp |= WR_DQS0_DLY<<8;
	temp |= RD_DQS0_DLY;
	write32(addr, temp);
	
////wait for refresh cycles
//for (i=0; i<32; i=i+1) begin
//   srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//end
	for( i=0; i<32; i++ )
	{
		addr = AHB_SLAVE20_BASE + 0x00;
		data = read32(addr);
	}
	
	printf("The Final Write DQS 0 DELAY = 0x%x\r\n", WR_DQS0_DLY);
	printf("The Final Read  DQS 0 DELAY = 0x%x\r\n", RD_DQS0_DLY);
	printf("The Final Write DQS 1 DELAY = 0x%x\r\n", WR_DQS1_DLY);
	printf("The Final Read  DQS 1 DELAY = 0x%x\r\n", RD_DQS1_DLY);		
}

