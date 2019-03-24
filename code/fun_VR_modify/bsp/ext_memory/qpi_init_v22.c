/*
 * qpi_init_v2.c
 *
 *  Created on: 2017/09/07
 *      Author: spark
 */

#include "type.h"
#include "comm.h"
#include "ddr_init.h"
#include "qpi_init.h"


void setOPIpagesize(u32 pagesize)
{
  u32 addr, data;

//  //set OPI page size
//  srdata(`AHB_SLAVE20_BASE + 32'hf4, data);
//  `ifdef SQPI_16mb
//  data[1:0]=2'b11;       //SQPI memory page size, 512byte
//  `else
//  data[1:0]=2'b00;       //SQPI memory page size, 1Kbyte
//  `endif
//  sw(`AHB_SLAVE20_BASE+32'hf4, data, 2'b10);

  addr = AHB_SLAVE20_BASE + 0xf4;
  data = read32(addr);
  data = set_fld(data, 1, 0, (pagesize&3));
  write32(addr, data);


}

u32 qpi_init() 
{
	vu32 addr, data;
    *(u32*)0x45000100 = 0xFFFF;

//	 //
//	 //New add AHB control signal
//	 //
//

	*(u32*)0x45000100 = 0xFFFF;

//	 //{4'b0,ch4_async,ch3_async,ch2_async,ch1_async} = 8'h0f; //2d
//	 srdata(`AHB_SLAVE20_BASE + 32'h0c, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h0c, data & 32'hfffff0ff | 8'h0<<24 | 8'h0<<16 | 8'h0f<<8 | 8'h0, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x0c, 0xfffff0ff, 0x00, 0x00, 0x0f, 0x0);


//	 //{ch1_ahb_wrlen}=8'h0f; //11
//	 //{ch1_ahb_rdlen}=8'h0f; //10
//	 //{ch0_ahb_wrlen}=8'h0f; //11
//	 //{ch0_ahb_rdlen}=8'h0f; //10
//	 sw(`AHB_SLAVE20_BASE + 32'ha0, (8'hff<<24 | 8'h3f<<16 | 8'hff<<8 | 8'h3f), 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xa0, 0x00000000, 0, 0, 0, 0);


//	 //{ch3_ahb_wrlen}=8'h0f; //11
//	 //{ch3_ahb_rdlen}=8'h0f; //10
//	 //{ch2_ahb_wrlen}=8'h0f; //11
//	 //{ch2_ahb_rdlen}=8'h0f; //10
//	 sw(`AHB_SLAVE20_BASE + 32'ha4, (8'hff<<24 | 8'h3f<<16 | 8'hff<<8 | 8'h3f), 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xa4, 0x00000000, 0, 0, 0, 0);


//	 //{ch8_command_seq_int,ch7_command_seq_int,ch6_command_seq_int,ch5_command_seq_int,
//	 // ch4_command_seq_int,ch3_command_seq_int,ch2_command_seq_int,ch1_command_seq_int}; //c0
//	 srdata(`AHB_SLAVE20_BASE + 32'hc0, data);
//	 sw(`AHB_SLAVE20_BASE + 32'hc0, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xc0, 0x00000000, 0x00, 0x00, 0x00, 0x00);



//	 //{1'b0,ch78_command_priority,1'b0,ch56_command_priority}, //c6
//	 //{1'b0,ch34_command_priority,1'b0,ch12_command_priority}  //c5
//	 //{1'b0,refl_command_priority,1'b0,refh_command_priority}  //c4
//	 srdata(`AHB_SLAVE20_BASE + 32'hc4, data);
//	 sw(`AHB_SLAVE20_BASE + 32'hc4, data & 32'hffff8888 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xc4, 0xffff8888, 0x00, 0x00, 0x00, 0x0);






//
//	 //PHY_DLL_MASTER_CTRL_REG=32'h0012_0004;
//	 srdata(`AHB_SLAVE20_BASE + 32'h1C, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h1C, data & 32'h00000000 | 8'h00<<24 | 8'h12<<16 | 8'h00<<8 | 8'h04, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x1C, 0x00000000, 0x00, 0x12, 0x00, 0x04);
//
//	 //PHY_DLL_SLAVE_CTRL_REG=32'h80808080;
//	 srdata(`AHB_SLAVE20_BASE + 32'h20, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h20, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x20, 0x00000000, 0x00, 0x00, 0x00, 0x00);
//

#ifdef QPI_RUN_100MHZ
	//	`ifdef RUN_100MHZ
	//
	//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
	//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
	//	 //{PHY_DQ_TIMING_REG}=8'h02;     //24
	//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
	//	 data[31:24] = 8'h00;
	//	 data[23:16] = 8'h06;
	//	 data[15:8]  = 8'h14;
	//	 data[7:0]   = 8'h02;
	//	 sw(`AHB_SLAVE20_BASE + 32'h24, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x24;
	data = read32(addr);
	data = set_fld(data, 31, 24, 0x00);
	data = set_fld(data, 23, 16, 0x06);
	data = set_fld(data, 15, 8, 0x14);
	data = set_fld(data, 7, 0, 0x02);
	write32(addr, data);

	//
	//	 //RD_WIN_SEL
	//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
	//	 data[23:22]=3;
	//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	//
	addr = AHB_SLAVE20_BASE + 0x4;
	data = read32(addr);
	data = set_fld(data, 23, 22, 3);
	write32(addr, data);

	//	`else //RUN_100MHZ
#endif //QPI_RUN_100MHZ

#ifdef QPI_RUN_81MHZ
//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h02;     //24
//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//	 data[31:24] = 8'h00;
//	 data[23:16] = 8'h06;
//	 data[15:8]  = 8'h14;
//	 data[7:0]   = 8'h02;
//	 sw(`AHB_SLAVE20_BASE + 32'h24, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x24;
	data = read32(addr);
	data = set_fld(data, 31, 24, 0x00);
	data = set_fld(data, 23, 16, 0x06);
	data = set_fld(data, 15, 8, 0x14);
	data = set_fld(data, 7, 0, 0x02);
	write32(addr, data);

//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=3;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x4;
	data = read32(addr);
	data = set_fld(data, 23, 22, 3);
	write32(addr, data);

#endif //QPI_RUN_81MHZ

#ifdef QPI_RUN_50MHZ
	//	`ifdef RUN_50MHZ
	//
	//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
	//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
	//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
	//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
	//	 data[31:24] = 8'h00;
	//	 data[23:16] = 8'h05;
	//	 data[15:8]  = 8'h14;
	//	 data[7:0]   = 8'h02;
	//	 sw(`AHB_SLAVE20_BASE + 32'h24, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x24;
	data = read32(addr);
	data = set_fld(data, 31, 24, 0x00);
	data = set_fld(data, 23, 16, 0x05);
	data = set_fld(data, 15, 8, 0x14);
	data = set_fld(data, 7, 0, 0x02);
	write32(addr, data);

	//
	//	 //RD_WIN_SEL
	//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
	//	 data[23:22]=2;
	//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x4;
	data = read32(addr);
	data = set_fld(data, 23, 22, 2);
	write32(addr, data);

	//
	//	`else //RUN_50MHZ
#endif //QPI_RUN_50MHZ

#ifdef QPI_RUN_25MHZ
	//	`ifdef RUN_25MHZ
	//
	//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
	//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
	//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
	//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
	//	 data[31:24] = 8'h00;
	//	 data[23:16] = 8'h05;
	//	 data[15:8]  = 8'h14;
	//	 data[7:0]   = 8'h02;
	//	 sw(`AHB_SLAVE20_BASE + 32'h24, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x24;
	data = read32(addr);
	data = set_fld(data, 31, 24, 0x00);
	data = set_fld(data, 23, 16, 0x05);
	data = set_fld(data, 15, 8, 0x14);
	data = set_fld(data, 7, 0, 0x02);
	write32(addr, data);

	//
	//	 //RD_WIN_SEL
	//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
	//	 data[23:22]=2;
	//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x4;
	data = read32(addr);
	data = set_fld(data, 23, 22, 2);
	write32(addr, data);

	//
	//	`else //RUN_25MHZ
#endif //QPI_RUN_25MHZ

#ifdef QPI_RUN_20MHZ
//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//	 data[31:24] = 8'h00;
//	 data[23:16] = 8'h05;
//	 data[15:8]  = 8'h14;
//	 data[7:0]   = 8'h02;
//	 sw(`AHB_SLAVE20_BASE + 32'h24, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x24;
	data = read32(addr);
	data = set_fld(data, 31, 24, 0x00);
	data = set_fld(data, 23, 16, 0x05);
	data = set_fld(data, 15, 8, 0x14);
	data = set_fld(data, 7, 0, 0x02);
	write32(addr, data);

//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=2;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x4;
	data = read32(addr);
	data = set_fld(data, 23, 22, 2);
	write32(addr, data);

#endif //QPI_RUN_20MHZ

//
//	 //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//	 srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h01<<24 | 8'h01<<16 | 8'h00<<8 | 8'h01, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x01, 0x01, 0x00, 0x01);
//
//	 //PHY_PAD_DRIVE_REG_0=32'h02000000;
//	 //srdata(`AHB_SLAVE20_BASE + 32'h2c, data);
//	 //sw(`AHB_SLAVE20_BASE + 32'h2c, data & 32'hfffffff0 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h0f, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x2c, 0xfffffff0, 0x00, 0x00, 0x00, 0x0f);
//
//	 //PHY_PAD_DRIVE_REG_1=32'h00000000;
//	 //srdata(`AHB_SLAVE20_BASE + 32'h30, data);
//	 //sw(`AHB_SLAVE20_BASE + 32'h30, data & 32'hffffffe0 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h03, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x30, 0xffffffe0, 0x00, 0x00, 0x00, 0x03);
//
//	 //PHY_PAD_DRIVE_REG_2=32'h0000_0000;
//	 srdata(`AHB_SLAVE20_BASE + 32'h34, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h34, data & 32'h00000000 | 8'hff<<24 | 8'hff<<16 | 8'hff<<8 | 8'hff, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x34, 0x00000000, 0xff, 0xff, 0xff, 0xff);
//
//	 //PHY_PAD_TERM_REG_0=32'h0000_f000;
//	 srdata(`AHB_SLAVE20_BASE + 32'h38, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h38, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'hf0<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x38, 0x00000000, 0x00, 0x00, 0xf0, 0x00);
//
//	 //PHY_PAD_TERM_REG_1=32'h00000000;
//	 srdata(`AHB_SLAVE20_BASE + 32'h3c, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h3c, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x3C, 0x00000000, 0x00, 0x00, 0x00, 0x00);
//
//	 //PHY_PAD_TERM_REG_2=32'h00000000;
//	 srdata(`AHB_SLAVE20_BASE + 32'h40, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h40, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x40, 0x00000000, 0x00, 0x00, 0x00, 0x00);
//
//	 //FIT2REF=32'h1;
//	 srdata(`AHB_SLAVE20_BASE + 32'h5c, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h5c, data & 32'hfffffffe | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h01, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x5C, 0xfffffffe, 0x00, 0x00, 0x00, 0x01);
//
//
//	 //reset DDR PHY DLL
//	 //{6'b0,DLL_LOCK} =8'h0; //1b
//	 //{7'b0,N_DLL_RST}=8'h1; //1a
//	 //{ch8_command_seq_int,ch7_command_seq_int,ch6_command_seq_int,ch5_command_seq_int,
//	 // ch4_command_seq_int,ch3_command_seq_int,ch2_command_seq_int,ch1_command_seq_int}=8'h0; //19
//	 //{1'b0,refl_command_priority,1'b0,refh_command_priority}=8'h0;  //18
//	 srdata(`AHB_SLAVE20_BASE + 32'h18, data);
	addr = AHB_SLAVE20_BASE + 0x18;
	data = read32(addr);

//	 //reset DLL
//	 sw(`AHB_SLAVE20_BASE + 32'h18, data & 32'hfffe0088 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x18, 0xfffe0088, 0x00, 0x00, 0x00, 0x00);

//	 //un-reset DLL
//	 sw(`AHB_SLAVE20_BASE + 32'h18, data & 32'hfffe0088 | 8'h00<<24 | 8'h01<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x18, 0xfffe0088, 0x00, 0x01, 0x00, 0x00);

//	 //check if DLL is locked
//	 srdata(`AHB_SLAVE20_BASE + 32'h18, data);
//	 while (data[25:24]!=2'b11) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h18, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x18;
	do {
		data = (read32(addr) >> 24) & 0x3;
	} while (data != 0x3);

//
//
//	//------------------------------------------------------------------------
//	//Setting control registers for SQPI mode
//	//------------------------------------------------------------------------
//
//	//Decide if gated clock is enabled
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[3]=1; //QPI clock gated is enabled when CE# is high
//	data[0]=1; //OPI_DDR_EN
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 3, 3, 1);
	data = set_fld(data, 0, 0, 1);
	write32(addr, data);

//
//	//set OPI MODE & DLL update cycle
//	//enable DDR 4Byte length
//	srdata(`AHB_SLAVE20_BASE + 32'hf0, data);
//	data[2:0]=3'b100;       //SPI mode
//	data[7]=1; //4byte length used for ch4
//	data[6]=1; //4byte length used for ch3
//	data[5]=1; //4byte length used for ch2
//	data[4]=1; //4byte length used for ch1
//	data[31:16]=16'h0200;   //dll update cycle
//	sw(`AHB_SLAVE20_BASE+32'hf0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xf0;
	data = read32(addr);
	data = set_fld(data, 2, 0, 0x4);
	data = set_fld(data, 7, 7, 1);
	data = set_fld(data, 6, 6, 1);
	data = set_fld(data, 5, 5, 1);
	data = set_fld(data, 4, 4, 1);
	data = set_fld(data, 31, 16, 0x200);
	write32(addr, data);

	//set OPI page size
	setOPIpagesize(SQPI_PAGE_SIZE);

#ifdef QPI_RUN_100MHZ
//	//QPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=2;        //tCPH, 18ns : ((OPI_TCPH+0.5)*PERIOD>18ns)
//	data[12:8]=1;       //tRC, no necessary for SQPI
//	data[15:13]=1;      //tCEM minimum, 1 clock cycles
//	data[31:16]=199;    //tCEM maximum, 2us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0, 0, 1);
	data = set_fld(data, 6, 4, 2);
	data = set_fld(data, 12, 8, 1);
	data = set_fld(data, 15, 13, 1);
	data = set_fld(data, 31, 16, 199);
	write32(addr, data);

//	//SPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //SPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'h9f; //SPI read ID
//	data[27:24]=8;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0x9f);
	data = set_fld(data, 27, 24, 8);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//SPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=8;            //command cycle
//	data[6:4]=3;            //read variable end cycle
//	data[23:16]=8+30+3-6;   //read data start cycle
//	data[31:24]=0+30+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 8);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (8+30+3-6));
	data = set_fld(data, 31, 24, (0+30+3-4));
	write32(addr, data);

#endif //End QPI_RUN_100MHZ

#ifdef QPI_RUN_81MHZ
//	//QPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=1;        //tCPH, 9.6ns
//	data[12:8]=1;       //tRC, no necessary for SQPI
//	data[15:13]=1;      //tCEM minimum, 1 clock cycles
//	data[31:16]=161;    //tCEM maximum, 2us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0, 0, 1);
	data = set_fld(data, 6, 4, 1);
	data = set_fld(data, 12, 8, 1);
	data = set_fld(data, 15, 13, 1);
	data = set_fld(data, 31, 16, 161);
	write32(addr, data);

//	//SPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //SPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'h9f; //SPI read ID
//	data[27:24]=8;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0x9f);
	data = set_fld(data, 27, 24, 8);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//SPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=8;            //command cycle
//	data[6:4]=3;            //read variable end cycle
//	data[23:16]=8+30+3-6;   //read data start cycle
//	data[31:24]=0+30+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 8);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (8+30+3-6));
	data = set_fld(data, 31, 24, (0+30+3-4));
	write32(addr, data);

#endif //End QPI_RUN_81MHZ

#ifdef QPI_RUN_50MHZ
	//	`ifdef RUN_50MHZ
	//
//	//QPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=1;        //tCPH, 9.6ns
//	data[12:8]=1;       //tRC, no necessary for SQPI
//	data[15:13]=1;      //tCEM minimum, 1 clock cycles
//	data[31:16]=99;    //tCEM maximum, 2us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0, 0, 1);
	data = set_fld(data, 6, 4, 1);
	data = set_fld(data, 12, 8, 1);
	data = set_fld(data, 15, 13, 1);
	data = set_fld(data, 31, 16, 99);
	write32(addr, data);

//	//SPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //SPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'h9f; //SPI read ID
//	data[27:24]=8;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0x9f);
	data = set_fld(data, 27, 24, 8);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//SPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=8;            //command cycle
//	data[6:4]=3;            //read variable end cycle
//	data[23:16]=8+30+3-6;   //read data start cycle
//	data[31:24]=0+30+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 8);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (8+30+3-6));
	data = set_fld(data, 31, 24, (0+30+3-4));
	write32(addr, data);

	//
	//	`else //RUN_50MHZ
#endif //End QPI_RUN_50MHZ

#ifdef QPI_RUN_25MHZ
//	//QPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=1;        //tCPH, 9.6ns
//	data[12:8]=1;       //tRC, no necessary for SQPI
//	data[15:13]=1;      //tCEM minimum, 1 clock cycles
//	data[31:16]=49;    //tCEM maximum, 2us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0, 0, 1);
	data = set_fld(data, 6, 4, 1);
	data = set_fld(data, 12, 8, 1);
	data = set_fld(data, 15, 13, 1);
	data = set_fld(data, 31, 16, 49);
	write32(addr, data);

//	//SPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //SPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'h9f; //SPI read ID
//	data[27:24]=8;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0x9f);
	data = set_fld(data, 27, 24, 8);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//SPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=8;            //command cycle
//	data[6:4]=3;            //read variable end cycle
//	data[23:16]=8+30+3-6;   //read data start cycle
//	data[31:24]=0+30+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 8);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (8 + 30 + 3 - 6));
	data = set_fld(data, 31, 24, (0 + 30 + 3 - 4));
	write32(addr, data);

	//
	//	`else //RUN_25MHZ
#endif //End QPI_RUN_25MHZ

#ifdef QPI_RUN_20MHZ
//	//QPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=1;        //tCPH, 9.6ns
//	data[12:8]=1;       //tRC, no necessary for SQPI
//	data[15:13]=1;      //tCEM minimum, 1 clock cycles
//	data[31:16]=39;     //tCEM maximum, 2us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0, 0, 1);
	data = set_fld(data, 6, 4, 1);
	data = set_fld(data, 12, 8, 1);
	data = set_fld(data, 15, 13, 1);
	data = set_fld(data, 31, 16, 39);
	write32(addr, data);

//	//SPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //SPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'h9f; //SPI read ID
//	data[27:24]=8;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0x9f);
	data = set_fld(data, 27, 24, 8);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//SPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=8;            //command cycle
//	data[6:4]=3;            //read variable end cycle
//	data[23:16]=8+30+3-6;   //read data start cycle
//	data[31:24]=0+30+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 8);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (8 + 30 + 3 - 6));
	data = set_fld(data, 31, 24, (0 + 30 + 3 - 4));
	write32(addr, data);

#endif //End QPI_RUN_20MHZ

//
//	`endif //RUN_25MHZ
//	`endif //RUN_50MHZ
//	`endif //RUN_100MHZ
//
//

//	//------------------------------------------------------------------------
//	//Wait 150us for device initialization
//	//------------------------------------------------------------------------
//	idle(10);
//
//	//------------------------------------------------------------------------
//	//Wait until DLL is locked
//	//------------------------------------------------------------------------
//	 //wait DDR DLL locked
//	 srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 while (data[1]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0xe0;
	do {
		data = (read32(addr) >> 1) & 1;
	} while (data != 1);

//
//	 //Test DLL updated register clear
//	 srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 data[1]=0;
//	 sw(`AHB_SLAVE20_BASE + 32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 1, 1, 0);
	write32(addr, data);
//
//	 //wait DDR DLL locked
//	 srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 while (data[1]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0xe0;
	do {
		data = (read32(addr) >> 1) & 1;
	} while (data != 1);

//
//	//------------------------------------------------------------------------
//	// QPI initialization
//	//------------------------------------------------------------------------
//
//	//----------------------
//	//reset enable command
//	//----------------------
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'h66; //reset enable command
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x66);
	write32(addr, data);

//
//	//trigger command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);

//
//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;
	do {
		data = (read32(addr) >> 17) & 1;
	} while (data != 1);

//
//	//----------------------
//	//reset command
//	//----------------------
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'h99; //reset command
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x99);
	write32(addr, data);

//
//	//trigger command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);

//
//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;
	do {
		data = (read32(addr) >> 17) & 1;
	} while (data != 1);

//	//------------------------------------------------------------------------
//	// SPI KGD check
//	//------------------------------------------------------------------------
//	//set SPI read ID command
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[23:16]=8'h9f; //SPI read  ID command
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 23, 16, 0x9f);
	write32(addr, data);

//	srdata(`AHB_SLAVE21_BASE + 32'h00, data); //read data QPI memory base address
//	if (data[7:0]==8'b0101_1101)
//	   $display("\n  ******PASS : QPI meory is Good Die******  data=%h",data);
//	else
//	   $display("\n  ******ERROR : QPI meory is Fail Die******  data=%h",data);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = (read32(addr) & 0xff);
	if (data == 0x5d) {
		;//printf("\n  ******PASS : QPI meory is Good Die******  %d\n", data);
	} else {
		;//printf("\n  ******ERROR : QPI meory is Fail Die******  %d\n", data);
	}

//	//set SPI read command
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[23:16]=8'h0b; //SPI fast read command
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 23, 16, 0x0b);
	write32(addr, data);

//
//	//-------------------------
//	//Quad mode enable command
//	//-------------------------
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'h35; //reset command
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x35);
	write32(addr, data);

//
//	//trigger command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);

//
//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;
	do {
		data = (read32(addr) >> 17) & 1;
	} while (data != 1);

//
//	//set OPI MODE
//	srdata(`AHB_SLAVE20_BASE + 32'hf0, data);
//	data[2:0]=3'b101;       //QPI mode
//	sw(`AHB_SLAVE20_BASE+32'hf0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xf0;
	data = read32(addr);
	data = set_fld(data, 2, 0, 0x5);
	write32(addr, data);

//	//------------------------------------------------------------------------
//	//Setting QPI read write timing registers
//	//------------------------------------------------------------------------
#ifdef Lyonteck_model
#ifdef QPI_RUN_100MHZ
//	//QPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //QPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'heb; //QPI fast read
//	data[27:24]=6;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0xeb);
	data = set_fld(data, 27, 24, 6);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//QPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=3;        //read variable end cycle
//	data[23:16]=6+6+3-6;  //read data start cycle
//	data[31:24]=0+6+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 2);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (6+6+3-6));
	data = set_fld(data, 31, 24, (0+6+3-4));
	write32(addr, data);

#endif

#ifdef QPI_RUN_81MHZ
//	//QPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //QPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'h0b; //QPI fast read
//	data[27:24]=4;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0x0b);
	data = set_fld(data, 27, 24, 4);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//QPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=3;        //read variable end cycle
//	data[23:16]=4+6+3-6;  //read data start cycle
//	data[31:24]=0+6+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 2);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (4+6+3-6));
	data = set_fld(data, 31, 24, (0+6+3-4));
	write32(addr, data);

#endif


#ifdef QPI_RUN_50MHZ
//	//QPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //QPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'h0b; //QPI fast read
//	data[27:24]=4;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0x0b);
	data = set_fld(data, 27, 24, 4);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//QPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=3;        //read variable end cycle
//	data[23:16]=4+6+3-6;  //read data start cycle
//	data[31:24]=0+6+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 2);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (4+6+3-6));
	data = set_fld(data, 31, 24, (0+6+3-4));
	write32(addr, data);

#endif

#ifdef QPI_RUN_25MHZ
//	//QPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //QPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'h0b; //QPI fast read
//	data[27:24]=4;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0x0b);
	data = set_fld(data, 27, 24, 4);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//QPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=3;        //read variable end cycle
//	data[23:16]=4+6+3-6;  //read data start cycle
//	data[31:24]=0+6+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 2);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (4+6+3-6));
	data = set_fld(data, 31, 24, (0+6+3-4));
	write32(addr, data);

#endif

#ifdef QPI_RUN_20MHZ
//	//QPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //QPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'h0b; //QPI fast read
//	data[27:24]=4;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0x0b);
	data = set_fld(data, 27, 24, 4);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);

//	//QPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=3;        //read variable end cycle
//	data[23:16]=4+6+3-6;  //read data start cycle
//	data[31:24]=0+6+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 2);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (4+6+3-6));
	data = set_fld(data, 31, 24, (0+6+3-4));
	write32(addr, data);

#endif

#else	//Lyonteck_model

//	//QPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[7:0]=8'h02;   //QPI write command
//	data[11:8]=0;      //write latency
//	data[23:16]=8'heb; //QPI fast read
//	data[27:24]=6;     //read latency
//	data[28]=1;        //fixed read latency
//	data[31:30]=0;     //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x02);
	data = set_fld(data, 11, 8, 0);
	data = set_fld(data, 23, 16, 0xeb);
	data = set_fld(data, 27, 24, 6);
	data = set_fld(data, 28, 28, 1);
	data = set_fld(data, 31, 30, 0);
	write32(addr, data);


//	//QPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=3;        //read variable end cycle
//	data[23:16]=6+6+3-6;  //read data start cycle
//	data[31:24]=0+6+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3, 0, 2);
	data = set_fld(data, 6, 4, 3);
	data = set_fld(data, 23, 16, (6+6+3-6));
	data = set_fld(data, 31, 24, (0+6+3-4));
	write32(addr, data);

#endif	//Lyonteck_model

#if 0	//new version QPI chip
#ifdef Lyonteck_model
#else 	//Lyonteck_model
#if SQPI_PAGE_SIZE==SQPI_PAGE_SIZE_512B
//	//------------------------------
//	//QPI mode register read/write
//	//------------------------------
//	//set mode register read/write decode
//	srdata(`AHB_SLAVE20_BASE + 32'hf4, data);
//	data[15:8]=8'hb1;
//	data[23:16]=8'hb5;
//	sw(`AHB_SLAVE20_BASE+32'hf4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xf4;
	data = read32(addr);
	data = set_fld(data, 15, 8, 0xb1);
	data = set_fld(data, 23, 16, 0xb5);
	write32(addr, data);

//	//---------------------------------------------------
//	//QPI mode register write
//	//---------------------------------------------------
//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hB1;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	data[23:16]=8'h71;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xb1);
	data = set_fld(data, 15, 8, 0x00);
	data = set_fld(data, 23, 16, 0x72);
	write32(addr, data);
//
//	//trigger command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);


//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!==1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;
	do{
	    data = (read32(addr)>>17)&1;
	}while(data!=1);
//
//	//------------------------
//	//QPI mode register read
//	//------------------------
//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hB5;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xb5);
	data = set_fld(data, 15, 8, 0x00);
	write32(addr, data);

//	//trigger command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);


//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!==1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;
	do{
	    data = (read32(addr)>>17)&1;
	}while(data!=1);
//
//	//check Mode register 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//
//	if (data[31:24]!==8'h71)
//	   $display("\n Mode register setting ERROR for MA0 ");
//	else
//	   $display("\n Mode register setting PASS for MA0 ");
	addr = AHB_SLAVE20_BASE + 0x00;
	data = (read32(addr)>>24)&0xff;
	if(data!=0x71){
	    ;//printf("\n Mode register setting ERROR for MA0 ");
	}
	else{
	    ;//printf("\n Mode register setting PASS for MA0 ");
	}

#endif	//SQPI_16mb
#endif	//Lyonteck_model
#endif //new verison QPI chip


//
//
//	//------------------------------------------------------------------------
//	// QPI DDR initial done
//	//------------------------------------------------------------------------
//
//	 //{7'h0,initial_ok} = 8'h01; //0c
//	 srdata(`AHB_SLAVE20_BASE + 32'h0c, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h0c, data & 32'hfffffffe | 8'h0<<24 | 8'h0<<16 | 8'h0<<8 | 8'h01, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x0c, 0xfffffffe, 0x0, 0x0, 0x0, 0x01);
    
    /* QPI mask issue: limited CE# length 4Byte */
    *(u32*)0x400400E0 &= ~0xFFFF0000;
    *(u32*)0x400400E0 |= 0xF << 16;
    
  return 0;
}
