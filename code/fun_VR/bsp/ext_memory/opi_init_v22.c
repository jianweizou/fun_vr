/*
 * opi_init.c
 *
 *  Created on: 2017/08/17
 *      Author: spark
 */

#include "type.h"
#include "comm.h"
#include "ddr_init.h"
#include "opi_init.h"

u32 opi_init()
{
	u32 addr, data;
	//
	//New add AHB control signal
	//
	//{4'b0,ch4_async,ch3_async,ch2_async,ch1_async} = 8'h0f; //2d
	//	 srdata(`AHB_SLAVE20_BASE + 32'h0c, data);
	//	 sw(`AHB_SLAVE20_BASE + 32'h0c, data & 32'hfffff0ff | 8'h0<<24 | 8'h0<<16 | 8'h0f<<8 | 8'h0, 2'b10);
	//setReg(AHB_SLAVE20_BASE + 0x0c, 0xfffff0ff, 0x00, 0x00, 0x0f, 0x00);

    
    
    ///ADD pad control
    
    *(int*)(AHB_SLAVE20_BASE + 0x60) &= (3<<1);
    *(int*)(AHB_SLAVE20_BASE + 0x60) |= (2<<1);
    
    //ADD Write request
    *(int*)(AHB_SLAVE20_BASE + 0xD4) |= 0;  /*  Channel clk > DRAM CLK : 0 
                                                DRAM CLK > Channel clk : 0xF   */
    
    
//	 //{ch1_ahb_wrlen}=8'h0f; //11
//	 //{ch1_ahb_rdlen}=8'h0f; //10
//	 //{ch0_ahb_wrlen}=8'h0f; //11
//	 //{ch0_ahb_rdlen}=8'h0f; //10
//	 sw(`AHB_SLAVE20_BASE + 32'ha0, (8'hff<<24 | 8'hff<<16 | 8'hff<<8 | 8'hff), 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xa0, 0x00000000, 0x0F, 0x0F, 0x0F, 0x0F);

//	 //{ch3_ahb_wrlen}=8'h0f; //11
//	 //{ch3_ahb_rdlen}=8'h0f; //10
//	 //{ch2_ahb_wrlen}=8'h0f; //11
//	 //{ch2_ahb_rdlen}=8'h0f; //10
//	 sw(`AHB_SLAVE20_BASE + 32'ha4, (8'hff<<24 | 8'hff<<16 | 8'hff<<8 | 8'hff), 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xa4, 0x00000000, 0x0F, 0x0F, 0x0F, 0x0F);
//
//	 //{ch8_command_seq_int,ch7_command_seq_int,ch6_command_seq_int,ch5_command_seq_int,
//	 // ch4_command_seq_int,ch3_command_seq_int,ch2_command_seq_int,ch1_command_seq_int}; //c0
//	 srdata(`AHB_SLAVE20_BASE + 32'hc0, data);
//	 sw(`AHB_SLAVE20_BASE + 32'hc0, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xc0, 0x00000000, 0x00, 0x00, 0x00, 0x00);
//
//
//	 //{1'b0,ch78_command_priority,1'b0,ch56_command_priority}, //c6
//	 //{1'b0,ch34_command_priority,1'b0,ch12_command_priority}  //c5
//	 //{1'b0,refl_command_priority,1'b0,refh_command_priority}  //c4
//	 srdata(`AHB_SLAVE20_BASE + 32'hc4, data);
//	 sw(`AHB_SLAVE20_BASE + 32'hc4, data & 32'hffff8888 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xc4, 0xffff8888, 0x00, 0x00, 0x00, 0x00);



//
//	 //PHY_DLL_MASTER_CTRL_REG=32'h0012_0004;
//	 srdata(`AHB_SLAVE20_BASE + 32'h1C, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h1C, data & 32'h00000000 | 8'h00<<24 | 8'h12<<16 | 8'h00<<8 | 8'h04, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x1C, 0x00000000, 0x00, 0x42, 0x00, 0x04);
//
//	 //PHY_DLL_SLAVE_CTRL_REG=32'h40404040;
//	 srdata(`AHB_SLAVE20_BASE + 32'h20, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h20, data & 32'h00000000 | 8'h40<<24 | 8'h40<<16 | 8'h40<<8 | 8'h40, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x20, 0x00000000, 0x40, 0x40, 0x40, 0x40);

	 //==============================================================
#ifdef OPI_RUN_200MHZ
//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
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
	data = set_fld(data, 15,  8, 0x14);
	data = set_fld(data,  7,  0, 0x02);
	write32(addr, data);

//
//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=2;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x04;
	data = read32(addr);
	data = set_fld(data, 23, 22, 2);
	write32(addr, data);
#endif

#ifdef OPI_RUN_166MHZ
//	//{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
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
	data = set_fld(data, 15,  8, 0x14);
	data = set_fld(data,  7,  0, 0x02);
	write32(addr, data);
//
//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=2;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x04;
	data = read32(addr);
	data = set_fld(data, 23, 22, 2);
	write32(addr, data);

#endif


#ifdef OPI_RUN_133MHZ
//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
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
	data = set_fld(data, 15,  8, 0x14);
	data = set_fld(data,  7,  0, 0x02);
	write32(addr, data);

//
//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=2;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x04;
	data = read32(addr);
	data = set_fld(data, 23, 22, 2);
	write32(addr, data);

#endif

#ifdef OPI_RUN_100MHZ
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
	data = set_fld(data, 15,  8, 0x14);
	data = set_fld(data,  7,  0, 0x02);
	write32(addr, data);
//
//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=1;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x04;
	data = read32(addr);
	data = set_fld(data, 23, 22, 1);
	write32(addr, data);
#endif

#ifdef OPI_RUN_81MHZ
//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//	 data[31:24] = 8'h00;
//	 data[23:16] = 8'h05;
//	 //data[15:8]  = 8'h14;
//	 data[15:8]  = 8'h14;
//	 data[7:0]   = 8'h02;
//	 sw(`AHB_SLAVE20_BASE + 32'h24, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x24;
	data = read32(addr);
	data = set_fld(data, 31, 24, 0x00);
	data = set_fld(data, 23, 16, 0x05);
	data = set_fld(data, 15,  8, 0x14);
	data = set_fld(data,  7,  0, 0x02);
	write32(addr, data);

//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=1;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x04;
	data = read32(addr);
	data = set_fld(data, 23, 22, 1);
	write32(addr, data);
#endif

#ifdef OPI_RUN_50MHZ
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
	data = set_fld(data, 15,  8, 0x14);
	data = set_fld(data,  7,  0, 0x02);
	write32(addr, data);
//
//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=1;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x04;
	data = read32(addr);
	data = set_fld(data, 23, 22, 1);
	write32(addr, data);
#endif


#ifdef OPI_RUN_25MHZ
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
	data = set_fld(data, 15,  8, 0x14);
	data = set_fld(data,  7,  0, 0x02);
	write32(addr, data);

//
//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=1;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x04;
	data = read32(addr);
	data = set_fld(data, 23, 22, 1);
	write32(addr, data);
#endif

#ifdef OPI_RUN_20MHZ
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
	data = set_fld(data, 15,  8, 0x14);
	data = set_fld(data,  7,  0, 0x02);
	write32(addr, data);

//	 //RD_WIN_SEL
//	 srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//	 data[23:22]=1;
//	 sw(`AHB_SLAVE20_BASE + 32'h04, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x04;
	data = read32(addr);
	data = set_fld(data, 23, 22, 1);
	write32(addr, data);
#endif

#ifdef OPI_DDR_X16
//	//PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//	 srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h01<<24 | 8'h01<<16 | 8'h00<<8 | 8'h01, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x01, 0x01, 0x00, 0x01);
#else
//	 //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//	 srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h01<<24 | 8'h01<<16 | 8'h00<<8 | 8'h08, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x01, 0x01, 0x00, 0x08);
#endif


//	 //PHY_PAD_DRIVE_REG_0=32'h02000000;
//	 //srdata(`AHB_SLAVE20_BASE + 32'h2c, data);
//	 //sw(`AHB_SLAVE20_BASE + 32'h2c, data & 32'hfffffff0 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h0f, 2'b10);
//	setReg(AHB_SLAVE20_BASE + 0x2c, 0xfffffff0, 0x00, 0x00, 0x00, 0x0f);
//
//	 //PHY_PAD_DRIVE_REG_1=32'h00000000;
//	 //srdata(`AHB_SLAVE20_BASE + 32'h30, data);
//	 //sw(`AHB_SLAVE20_BASE + 32'h30, data & 32'hffffffe0 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h03, 2'b10);
//	setReg(AHB_SLAVE20_BASE + 0x30, 0xffffffe0, 0x00, 0x00, 0x00, 0x03);

//
//	 //PHY_PAD_DRIVE_REG_2=32'h0000_0000;
//	 srdata(`AHB_SLAVE20_BASE + 32'h34, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h34, data & 32'h00000000 | 8'hff<<24 | 8'hff<<16 | 8'hff<<8 | 8'hff, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x34, 0x00000000, 0xff, 0xf5, 0xff, 0xff);
//
//	 //PHY_PAD_TERM_REG_0=32'h0000_f000;
//	 srdata(`AHB_SLAVE20_BASE + 32'h38, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h38, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'hf0<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x38, 0x00000000, 0x00, 0x00, 0xf0, 0x00);
//
//	 //PHY_PAD_TERM_REG_1=32'h00000000;
//	 srdata(`AHB_SLAVE20_BASE + 32'h3c, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h3c, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x3c, 0x00000000, 0x00, 0x00, 0x00, 0x00);
//
//	 //PHY_PAD_TERM_REG_2=32'h00000000;
//	 srdata(`AHB_SLAVE20_BASE + 32'h40, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h40, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x40, 0x00000000, 0x00, 0x00, 0x00, 0x00);
//
//	 //FIT2REF=32'h1;
//	 srdata(`AHB_SLAVE20_BASE + 32'h5c, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h5c, data & 32'hfffffffe | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h01, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x5c, 0xfffffffe, 0x00, 0x00, 0x00, 0x01);

#if 1
    /* SW reset on DRAM ctr */
    *(u32*)(AHB_SLAVE20_BASE + 0x18) |= 1<<8;
    *(u32*)(AHB_SLAVE20_BASE + 0x18) &= ~(1<<8);
#endif

//	 //reset DDR PHY DLL
//	 //{6'b0,DLL_LOCK} =8'h0; //1b
//	 //{7'b0,N_DLL_RST}=8'h1; //1a
//	 //{ch8_command_seq_int,ch7_command_seq_int,ch6_command_seq_int,ch5_command_seq_int,
//	 // ch4_command_seq_int,ch3_command_seq_int,ch2_command_seq_int,ch1_command_seq_int}=8'h0; //19
//	 //{1'b0,refl_command_priority,1'b0,refh_command_priority}=8'h0;  //18
//	 srdata(`AHB_SLAVE20_BASE + 32'h18, data);
	data = read32(AHB_SLAVE20_BASE + 0x18);

//	 //reset DLL
//	 sw(`AHB_SLAVE20_BASE + 32'h18, data & 32'hfffe0088 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	data &= 0xfffe0088;
	write32(AHB_SLAVE20_BASE + 0x18, data);

//	 //un-reset DLL
//	 sw(`AHB_SLAVE20_BASE + 32'h18, data & 32'hfffe0088 | 8'h00<<24 | 8'h01<<16 | 8'h00<<8 | 8'h00, 2'b10);
	data &= 0xfffe0088;
	data |= 0x00<<24;
	data |= 0x01<<16;
	data |= 0x00<<8;
	data |= 0x00;
	write32(AHB_SLAVE20_BASE + 0x18, data);

//	 //check if DLL is locked
//	 srdata(`AHB_SLAVE20_BASE + 32'h18, data);
//	 while (data[25:24]!=2'b11) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h18, data);
//	 end

	addr = AHB_SLAVE20_BASE + 0x18;
	do{
		data = read32(addr);
		data = (data >> 24)&3;
	}while(data != 3);

	//------------------------------------------------------------------------
	//Setting OPI control registers
	//------------------------------------------------------------------------

//	//Decide if gated clock is enabled
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[3]=1; //OPI clock gated is enabled when CE# is high
//	data[0]=1; //OPI_DDR_EN
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 3,  3, 1);
	data = set_fld(data, 0,  0, 1);
	write32(addr, data);

//
//	//set OPI mode & DLL update cycle
//	//Decide if DDR 4Byte length is used
//	srdata(`AHB_SLAVE20_BASE + 32'hf0, data);
//	`ifdef OPI_DDR_X16
//	data[2:0]=3'b001; //OPIx16 mode
//	`else
//	data[2:0]=3'b000; //OPI mode
//	`endif
//	data[7]=0;        //4byte length used for ch4
//	data[6]=1;        //4byte length used for ch3
//	data[5]=0;        //4byte length used for ch2
//	data[4]=1;        //4byte length used for ch1
//	data[31:16]=16'h0200;   //dll update cycle
//	sw(`AHB_SLAVE20_BASE+32'hf0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xf0;
	data = read32(addr);

#ifdef OPI_DDR_X16
	data = set_fld(data, 2,  0, 1);	//OPIx16 mode
#else
	data = set_fld(data, 2,  0, 0);	//OPI mode
#endif
	data = set_fld(data, 7,  7, 0);
	data = set_fld(data, 6,  6, 1);
	data = set_fld(data, 5,  5, 0);
	data = set_fld(data, 4,  4, 1);
	data = set_fld(data, 31,  16, 0x200);

    data |= 0xFFFF00F0; //ADD

	write32(addr, data);

#ifdef OPI_RUN_200MHZ
//	//OPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=4;        //tCPH, 20ns
//	data[12:8]=12;      //tRC, 60ns
//	data[15:13]=3;      //tCEM minimum, 3 clock cycles
//	data[31:16]=200;    //tCEM maximum, 1us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0,  0, 1);
	data = set_fld(data, 6,  4, 4);
	data = set_fld(data, 12,  8, 12);
	data = set_fld(data, 15,  13, 3);
	//data = set_fld(data, 31,  16, 200);
#ifdef OPI_EXTENDED_TEMP
	data = set_fld(data, 31,  16, 199);	 //tCEM maximum, 1us
#else
	data = set_fld(data, 31,  16, 799);	 //tCEM maximum, 4us
#endif
	write32(addr, data);

//	//OPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[11:8]=7;    //write latency
//	data[27:24]=14;  //read latency
//	data[28]=OPI_RD_LATANCY_FIXED;      //fixed read latency
//	data[31:30]=1;   //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 11,  8, 7);
	data = set_fld(data, 27,  24, 14);
	data = set_fld(data, 28,  28, OPI_RD_LATANCY_FIXED);
	data = set_fld(data, 31,  30, 1);
	write32(addr, data);


//	//OPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=4;        //read variable end cycle
//	data[23:16]=7+3-5;   //read data start cycle
//	data[31:24]=7+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3,  0, 2);
	data = set_fld(data, 6,  4, 4);
	data = set_fld(data, 23,  16, (7+3-5));
	data = set_fld(data, 31,  24, (7+3-4));
	write32(addr, data);


#endif

#ifdef OPI_RUN_166MHZ

//	//OPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=3;        //tCPH, 20ns
//	data[12:8]=10;      //tRC, 60ns
//	data[15:13]=3;      //tCEM minimum, 3 clock cycles
//	data[31:16]=167;    //tCEM maximum, 1us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0,  0, 1);
	data = set_fld(data, 6,  4, 3);
	data = set_fld(data, 12,  8, 10);
	data = set_fld(data, 15,  13, 3);
	//data = set_fld(data, 31,  16, 167);
#ifdef OPI_EXTENDED_TEMP
	data = set_fld(data, 31,  16, 167);	 //tCEM maximum, 1us
#else
	data = set_fld(data, 31,  16, 662);	 //tCEM maximum, 4us
#endif
	write32(addr, data);

//	//OPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[11:8]=6;    //write latency
//	data[27:24]=12;  //read latency
//	data[28]=OPI_RD_LATANCY_FIXED;      //fixed read latency
//	data[31:30]=1;   //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 11,  8, 6);
	data = set_fld(data, 27,  24, 12);
	data = set_fld(data, 28,  28, OPI_RD_LATANCY_FIXED);
	data = set_fld(data, 31,  30, 1);
	write32(addr, data);

//	//OPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=4;        //read variable end cycle
//	data[23:16]=6+3-5;  //read data start cycle
//	data[31:24]=6+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3,  0, 2);
	data = set_fld(data, 6,  4, 4);
	data = set_fld(data, 23,  16, (6+3-5));
	data = set_fld(data, 31,  24, (6+3-4));
	write32(addr, data);

#endif

#ifdef OPI_RUN_133MHZ
//	//OPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=3;        //tCPH, 20ns
//	data[12:8]=8;      //tRC, 60ns
//	data[15:13]=3;      //tCEM minimum, 3 clock cycles
//	data[31:16]=133;    //tCEM maximum, 1us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0,  0, 1);
	data = set_fld(data, 6,  4, 3);
	data = set_fld(data, 12,  8, 8);
	data = set_fld(data, 15,  13, 3);
	//data = set_fld(data, 31,  16, 133);
#ifdef OPI_EXTENDED_TEMP
	data = set_fld(data, 31,  16, 133);	 //tCEM maximum, 1us
#else
	data = set_fld(data, 31,  16, 532);	 //tCEM maximum, 4us
#endif
	write32(addr, data);

//	//OPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[11:8]=5;    //write latency
//	data[27:24]=10;  //read latency
//	data[28]=OPI_RD_LATANCY_FIXED;      //fixed read latency
//	data[31:30]=1;   //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 11,  8, 5);
	data = set_fld(data, 27,  24, 10);
	data = set_fld(data, 28,  28, OPI_RD_LATANCY_FIXED);
	data = set_fld(data, 31,  30, 1);
	write32(addr, data);

//	//OPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=4;        //read variable end cycle
//	data[23:16]=5+3-5;  //read data start cycle
//	data[31:24]=5+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3,  0, 2);
	data = set_fld(data, 6,  4, 4);
	data = set_fld(data, 23,  16, (5+3-5));
	data = set_fld(data, 31,  24, (5+3-4));
	write32(addr, data);

#endif

#ifdef OPI_RUN_100MHZ
//	//OPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=2;        //tCPH, 20ns
//	data[12:8]=6;      //tRC, 60ns
//	data[15:13]=3;      //tCEM minimum, 3 clock cycles
//	data[31:16]=100;    //tCEM maximum, 1us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0,  0, 1);
	data = set_fld(data, 6,  4, 2);
	data = set_fld(data, 12,  8, 6);
	data = set_fld(data, 15,  13, 3);
	//data = set_fld(data, 31,  16, 100);
#ifdef OPI_EXTENDED_TEMP
	data = set_fld(data, 31,  16, 99);	 //tCEM maximum, 1us
#else
	data = set_fld(data, 31,  16, 399);	 //tCEM maximum, 4us
#endif
	write32(addr, data);


//	//OPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[11:8]=4;    //write latency
//	data[27:24]=8;  //read latency
//	data[28]=OPI_RD_LATANCY_FIXED;      //fixed read latency
//	data[31:30]=1;   //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 11,  8, 4);
	data = set_fld(data, 27,  24, 8);
	data = set_fld(data, 28,  28, OPI_RD_LATANCY_FIXED);
	data = set_fld(data, 31,  30, 1);
	write32(addr, data);

//	//OPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=4;        //read variable end cycle
//	data[23:16]=4+3-5;  //read data start cycle
//	data[31:24]=4+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3,  0, 2);
	data = set_fld(data, 6,  4, 4);
	data = set_fld(data, 23,  16, (4+3-5));
	data = set_fld(data, 31,  24, (4+3-4));
	write32(addr, data);

#endif


#ifdef OPI_RUN_81MHZ
//	//OPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=2;        //tCPH, 20ns
//	data[12:8]=5;       //tRC, 60ns
//	data[15:13]=3;      //tCEM minimum, 3 clock cycles
//	if (OPI_EXTENDED_TEMP)
//	   data[31:16]=80;    //tCEM maximum, 1us
//	else
//	   data[31:16]=320;    //tCEM maximum, 4us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0,  0, 1);
	data = set_fld(data, 6,  4, 2);
	data = set_fld(data, 12,  8, 5);
	data = set_fld(data, 15,  13, 3);

#ifdef OPI_EXTENDED_TEMP
	data = set_fld(data, 31,  16, 80);	 //tCEM maximum, 1us
#else
	data = set_fld(data, 31,  16, 320);	 //tCEM maximum, 4us
#endif
	write32(addr, data);


//	//OPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[11:8]=4;    //write latency
//	data[27:24]=8;  //read latency
//	data[28]=OPI_RD_LATANCY_FIXED;      //fixed read latency
//	data[31:30]=1;   //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 11,  8, 4);
	data = set_fld(data, 27,  24, 8);
	data = set_fld(data, 28,  28, OPI_RD_LATANCY_FIXED);
	data = set_fld(data, 31,  30, 1);
	write32(addr, data);

//	//OPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=4;        //read variable end cycle
//	data[23:16]=4+3-5;  //read data start cycle
//	data[31:24]=4+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3,  0, 2);
	data = set_fld(data, 6,  4, 4);
	data = set_fld(data, 23,  16, (4+3-5));
	data = set_fld(data, 31,  24, (4+3-4));
	write32(addr, data);

#endif

#ifdef OPI_RUN_50MHZ
//	//OPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=1;        //tCPH, 20ns
//	data[12:8]=3;      //tRC, 60ns
//	data[15:13]=3;      //tCEM minimum, 3 clock cycles
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0,  0, 1);
	data = set_fld(data, 6,  4, 1);
	data = set_fld(data, 12,  8, 3);
	data = set_fld(data, 15,  13, 3);

#ifdef OPI_EXTENDED_TEMP
	data = set_fld(data, 31,  16, 49);	 //tCEM maximum, 1us
#else
	data = set_fld(data, 31,  16, 199);	 //tCEM maximum, 4us
#endif

	write32(addr, data);

//	//OPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[11:8]=3;    //write latency
//	data[27:24]=6;  //read latency
//	data[28]=OPI_RD_LATANCY_FIXED;      //fixed read latency
//	data[31:30]=1;   //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 11,  8, 3);
	data = set_fld(data, 27,  24, 6);
	data = set_fld(data, 28,  28, OPI_RD_LATANCY_FIXED);
	data = set_fld(data, 31,  30, 1);
	write32(addr, data);

//	//OPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=4;        //read variable end cycle
//	data[23:16]=3+3-5;  //read data start cycle
//	data[31:24]=3+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3,  0, 2);
	data = set_fld(data, 6,  4, 4);
	data = set_fld(data, 23,  16, (3+3-5));
	data = set_fld(data, 31,  24, (3+3-4));
	write32(addr, data);

#endif

#ifdef OPI_RUN_25MHZ
//	//OPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=1;        //tCPH, 20ns
//	data[12:8]=2;      //tRC, 60ns
//	data[15:13]=3;      //tCEM minimum, 3 clock cycles
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0,  0, 1);
	data = set_fld(data, 6,  4, 1);
	data = set_fld(data, 12,  8, 2);
	data = set_fld(data, 15,  13, 3);
	//data = set_fld(data, 31,  16, 25);
#ifdef OPI_EXTENDED_TEMP
	data = set_fld(data, 31,  16, 24);	 //tCEM maximum, 1us
#else
	data = set_fld(data, 31,  16, 99);	 //tCEM maximum, 4us
#endif

	write32(addr, data);

//	//OPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[11:8]=3;    //write latency
//	data[27:24]=6;  //read latency
//	data[28]=OPI_RD_LATANCY_FIXED;      //fixed read latency
//	data[31:30]=1;   //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 11,  8, 3);
	data = set_fld(data, 27,  24, 6);
	data = set_fld(data, 28,  28, OPI_RD_LATANCY_FIXED);
	data = set_fld(data, 31,  30, 1);
	write32(addr, data);

//	//OPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=4;        //read variable end cycle
//	data[23:16]=3+3-5;  //read data start cycle
//	data[31:24]=3+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3,  0, 2);
	data = set_fld(data, 6,  4, 4);
	data = set_fld(data, 23,  16, (3+3-5));
	data = set_fld(data, 31,  24, (3+3-4));
	write32(addr, data);

#endif

#ifdef OPI_RUN_20MHZ
//	//OPI DDR register 0xE0 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	data[0]=1;          //OPI_DDR_EN
//	data[6:4]=1;        //tCPH, 20ns
//	data[12:8]=2;       //tRC, 60ns
//	data[15:13]=3;      //tCEM minimum, 3 clock cycles
//	if (OPI_EXTENDED_TEMP)
//	   data[31:16]=19;     //tCEM maximum, 1us
//	else
//	   data[31:16]=79;     //tCEM maximum, 4us
//	sw(`AHB_SLAVE20_BASE+32'he0, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 0,  0, 1);
	data = set_fld(data, 6,  4, 1);
	data = set_fld(data, 12,  8, 2);
	data = set_fld(data, 15,  13, 3);

#ifdef OPI_EXTENDED_TEMP
	data = set_fld(data, 31,  16, 19);	 //tCEM maximum, 1us
#else
	data = set_fld(data, 31,  16, 79);	 //tCEM maximum, 4us
#endif

	write32(addr, data);


//	//OPI DDR register 0xE4 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he4, data);
//	data[11:8]=3;    //write latency
//	data[27:24]=6;  //read latency
//	data[28]=OPI_RD_LATANCY_FIXED;      //fixed read latency
//	data[31:30]=1;   //read FIFO reset delay cycle select
//	sw(`AHB_SLAVE20_BASE+32'he4, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe4;
	data = read32(addr);
	data = set_fld(data, 11,  8, 3);
	data = set_fld(data, 27,  24, 6);
	data = set_fld(data, 28,  28, OPI_RD_LATANCY_FIXED);
	data = set_fld(data, 31,  30, 1);
	write32(addr, data);

//	//OPI DDR register 0xE8 setting
//	srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	data[3:0]=2;        //command cycle
//	data[6:4]=4;        //read variable end cycle
//	data[23:16]=3+3-5;  //read data start cycle
//	data[31:24]=3+3-4;   //write data start cycle
//	sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xe8;
	data = read32(addr);
	data = set_fld(data, 3,  0, 2);
	data = set_fld(data, 6,  4, 4);
	data = set_fld(data, 23,  16, (3+3-5));
	data = set_fld(data, 31,  24, (3+3-4));
	write32(addr, data);

#endif

	//------------------------------------------------------------------------
	//Wait 150us for device initialization
	//------------------------------------------------------------------------
	//idle(10);
    DELAY_US(162,150);

//	for(data=0;data<1000;data++){

//	}

//	//------------------------------------------------------------------------
//
//	//Setting OPI DLL is locked
//	//------------------------------------------------------------------------
//	 //wait OPI DDR DLL locked
//	 srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 while (data[1]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 end

	addr = AHB_SLAVE20_BASE + 0xe0;
	do{
		data = (read32(addr)>>1)&1;
	}while(data!=1);

//	 //Test DLL updated register clear
//	 srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 data[1]=0;
//	 sw(`AHB_SLAVE20_BASE + 32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	data = read32(addr);
	data = set_fld(data, 1,  1, 0);
	write32(addr, data);

//	 //wait OPI DDR DLL locked
//	 srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 while (data[1]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0xe0;

	do{
		data = (read32(addr)>>1)&1;
	}while(data!=1);



//	//------------------------------------------------------------------------
//	// OPI initialization & Mode register write
//	//------------------------------------------------------------------------
//
//	//Global reset command
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hff; //Global reset command
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7,  0, 0xff);
	write32(addr, data);


//	//trigger clobal reset command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16,  16, 1);
	write32(addr, data);

DELAY_US(162,2);

//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end

	addr = AHB_SLAVE20_BASE 
    ;
	do{
		data = (read32(addr)>>17)&1;
	}while(data!=1);

	for(data=0;data<1000;data++){

	}

#ifdef OPI_RUN_200MHZ
//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8'h30;    //mode register OP code
//	else                         data[23:16]=8'h10;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7,  0, 0xc0);
	data = set_fld(data, 15,  8, 0x00);

	if(OPI_RD_LATANCY_FIXED==1){
		data = set_fld(data, 23,  16, 0x30);
	}
	else{
		data = set_fld(data, 23,  16, 0x10);
	}
	write32(addr, data);


#endif

#ifdef OPI_RUN_166MHZ

//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8'h2c;    //mode register OP code
//	else                         data[23:16]=8'h0c;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7,  0, 0xc0);
	data = set_fld(data, 15,  8, 0x00);

	if(OPI_RD_LATANCY_FIXED==1){
		data = set_fld(data, 23,  16, 0x2c);
	}
	else{
		data = set_fld(data, 23,  16, 0x0c);
	}
	write32(addr, data);


#endif

#ifdef OPI_RUN_133MHZ

//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8'h28;    //mode register OP code
//	else                         data[23:16]=8'h08;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7,  0, 0xc0);
	data = set_fld(data, 15,  8, 0x00);

	if(OPI_RD_LATANCY_FIXED==1){
		data = set_fld(data, 23,  16, 0x28);
	}
	else{
		data = set_fld(data, 23,  16, 0x08);
	}
	write32(addr, data);

#endif

#ifdef OPI_RUN_100MHZ

//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8'h24;    //mode register OP code
//	else                         data[23:16]=8'h04;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7,  0, 0xc0);
	data = set_fld(data, 15,  8, 0x00);

	if(OPI_RD_LATANCY_FIXED==1){
		data = set_fld(data, 23,  16, 0x24);
	}
	else{
		data = set_fld(data, 23,  16, 0x04);
	}
	write32(addr, data);

#endif

#ifdef OPI_RUN_81MHZ

//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8'h24;    //mode register OP code
//	else                         data[23:16]=8'h04;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7,  0, 0xc0);
	data = set_fld(data, 15,  8, 0x00);

	if(OPI_RD_LATANCY_FIXED==1){
		data = set_fld(data, 23,  16, 0x24);
	}
	else{
		data = set_fld(data, 23,  16, 0x04);
	}
	write32(addr, data);

#endif

#ifdef OPI_RUN_50MHZ

//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8'h20;    //mode register OP code
//	else                         data[23:16]=8'h00;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7,  0, 0xc0);
	data = set_fld(data, 15,  8, 0x00);

	if(OPI_RD_LATANCY_FIXED==1){
		data = set_fld(data, 23,  16, 0x20);
	}
	else{
		data = set_fld(data, 23,  16, 0x00);
	}
	write32(addr, data);

#endif

#ifdef OPI_RUN_25MHZ

//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8'h20;    //mode register OP code
//	else                         data[23:16]=8'h00;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7,  0, 0xc0);
	data = set_fld(data, 15,  8, 0x00);

	if(OPI_RD_LATANCY_FIXED==1){
		data = set_fld(data, 23,  16, 0x20);
	}
	else{
		data = set_fld(data, 23,  16, 0x00);
	}
	write32(addr, data);

#endif

#ifdef OPI_RUN_20MHZ

//	//Mode register write address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h00;     //mode register address
//	if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8'h20;    //mode register OP code
//	else                         data[23:16]=8'h00;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7,  0, 0xc0);
	data = set_fld(data, 15,  8, 0x00);

	if(OPI_RD_LATANCY_FIXED==1){
		data = set_fld(data, 23,  16, 0x20);
	}
	else{
		data = set_fld(data, 23,  16, 0x00);
	}
	write32(addr, data);

#endif

//	//trigger mode register write command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);


//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;

	do{
		data = (read32(addr)>>17)&1;
	}while(data!=1);



#ifdef OPI_RUN_200MHZ
//	//Mode register write address 4
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h04;     //mode register address
//	data[23:16]=8'h20;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xc0);
	data = set_fld(data, 15, 8, 0x04);
	data = set_fld(data, 23, 16, 0x20);
	write32(addr, data);

#endif

#ifdef OPI_RUN_166MHZ
//	//Mode register write address 4
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h04;     //mode register address
//	data[23:16]=8'hC0;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xc0);
	data = set_fld(data, 15, 8, 0x04);
	data = set_fld(data, 23, 16, 0xc0);
	write32(addr, data);

#endif

#ifdef OPI_RUN_133MHZ
//	//Mode register write address 4
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h04;     //mode register address
//	data[23:16]=8'h40;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xc0);
	data = set_fld(data, 15, 8, 0x04);
	data = set_fld(data, 23, 16, 0x40);
	write32(addr, data);

#endif

#ifdef OPI_RUN_100MHZ
//	//Mode register write address 4
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h04;     //mode register address
//	data[23:16]=8'h80;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xc0);
	data = set_fld(data, 15, 8, 0x04);
	data = set_fld(data, 23, 16, 0x80);
	write32(addr, data);

#endif

#ifdef OPI_RUN_81MHZ
//	//Mode register write address 4
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h04;     //mode register address
//	data[23:16]=8'h80;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xc0);
	data = set_fld(data, 15, 8, 0x04);
	data = set_fld(data, 23, 16, 0x80);
	write32(addr, data);

#endif

#ifdef OPI_RUN_50MHZ

//	//Mode register write address 4
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h04;     //mode register address
//	data[23:16]=8'h00;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xc0);
	data = set_fld(data, 15, 8, 0x04);
	data = set_fld(data, 23, 16, 0x00);
	write32(addr, data);

#endif

#ifdef OPI_RUN_25MHZ
//	//Mode register write address 4
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h04;     //mode register address
//	data[23:16]=8'h00;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xc0);
	data = set_fld(data, 15, 8, 0x04);
	data = set_fld(data, 23, 16, 0x00);
	write32(addr, data);
#endif

#ifdef OPI_RUN_20MHZ
//	//Mode register write address 4
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h04;     //mode register address
//	data[23:16]=8'h00;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);

	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xc0);
	data = set_fld(data, 15, 8, 0x04);
	data = set_fld(data, 23, 16, 0x00);
	write32(addr, data);
#endif


//	//trigger mode register write command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);


//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;

	do{
		data = (read32(addr)>>17)&1;
	}while(data!=1);

//	//Mode register write address 8
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'hC0;      //mode register write command
//	data[15:8]=8'h08;     //mode register address
//	data[23:16]=8'h03;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0xc0);
	data = set_fld(data, 15, 8, 0x08);
	data = set_fld(data, 23, 16, 0x03);
	write32(addr, data);

//	//trigger mode register write command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);


//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;

	do{
		data = (read32(addr)>>17)&1;
	}while(data!=1);



//	//------------------------------------------------------------------------
//	// Mode register read test
//	//------------------------------------------------------------------------

//	//Check OPI memory Good Die
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'h40;      //mode register read command
//	data[15:8]=8'h02;     //mode register address
//	data[23:16]=8'h00;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x40);
	data = set_fld(data, 15, 8, 0x02);
	data = set_fld(data, 23, 16, 0x00);
	write32(addr, data);


//	//trigger mode register read command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);

//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;
	do{
		data = (read32(addr)>>17)&1;
	}while(data!=1);

//	 //check Mode register 2
//	 srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	 if (data[31]!=1)
//	    $display("\n ERROR : OPI memory is Fail Die ");
//	 else
//	    $display("\n PASS : OPI memory is Good Die ");
	addr = AHB_SLAVE20_BASE + 0xec;
	data = (read32(addr)>>31)&1;
	if(data!=1){
		;//;//printf("\n ERROR : OPI memory is Fail Die ");
	}
	else
	{
		;//;//printf("\n PASS : OPI memory is Good Die ");
	}

//	//Mode register read address 0
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'h40;      //mode register read command
//	data[15:8]=8'h00;     //mode register address
//	data[23:16]=8'h00;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x40);
	data = set_fld(data, 15, 8, 0);
	data = set_fld(data, 23, 16, 0);
	write32(addr, data);


//	//trigger mode register read command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);


//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;
	do{
		data = (read32(addr)>>17)&1;
	}while(data!=1);


//	 //check Mode register 0
//	 srdata(`AHB_SLAVE20_BASE + 32'hec, data);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);

#ifdef OPI_RUN_200MHZ
//	   if (OPI_RD_LATANCY_FIXED==1) begin
//	      if (data[31:24]!=8'h30)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
//	   else begin
//	      if(data[31:24]!=8'h10)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end

	data = (data>>24)&0xff;
	if (OPI_RD_LATANCY_FIXED==1) {
		if((data!=0x30)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}
	else
	{
		if((data!=0x10)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}

#endif

#ifdef OPI_RUN_166MHZ
//	   if (OPI_RD_LATANCY_FIXED==1) begin
//	      if (data[31:24]!=8'h2c)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
//	   else begin
//	      if(data[31:24]!=8'h0c)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
	data = (data>>24)&0xff;
	if (OPI_RD_LATANCY_FIXED==1) {
		if((data!=0x2c)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}
	else
	{
		if((data!=0x0c)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}

#endif

#ifdef OPI_RUN_133MHZ
//	   if (OPI_RD_LATANCY_FIXED==1) begin
//	      if (data[31:24]!=8'h28)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
//	   else begin
//	      if(data[31:24]!=8'h08)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
	data = (data>>24)&0xff;
	if (OPI_RD_LATANCY_FIXED==1) {
		if((data!=0x28)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}
	else
	{
		if((data!=0x08)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}


#endif

#ifdef OPI_RUN_100MHZ
//	   if (OPI_RD_LATANCY_FIXED==1) begin
//	      if (data[31:24]!=8'h24)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
//	   else begin
//	      if(data[31:24]!=8'h04)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
	data = (data>>24)&0xff;
	if (OPI_RD_LATANCY_FIXED==1) {
		if((data!=0x24)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}
	else
	{
		if((data!=0x04)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}

#endif

#ifdef OPI_RUN_81MHZ
//	   if (OPI_RD_LATANCY_FIXED==1) begin
//	      if (data[31:24]!=8'h24)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
//	   else begin
//	      if(data[31:24]!=8'h04)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
	data = (data>>24)&0xff;
	if (OPI_RD_LATANCY_FIXED==1) {
		if((data!=0x24)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
//			return FAIL;
		}
	}
	else
	{
		if((data!=0x04)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
//			return FAIL;
		}
	}

#endif

#ifdef OPI_RUN_50MHZ
//	   if (OPI_RD_LATANCY_FIXED==1) begin
//	      if (data[31:24]!=8'h20)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
//	   else begin
//	      if(data[31:24]!=8'h00)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
	data = (data>>24)&0xff;
	if (OPI_RD_LATANCY_FIXED==1) {
		if((data!=0x20)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}
	else
	{
		if((data!=0x00)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}

#endif

#ifdef OPI_RUN_25MHZ
//	   if (OPI_RD_LATANCY_FIXED==1) begin
//	      if (data[31:24]!=8'h20)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
//	   else begin
//	      if(data[31:24]!=8'h00)
//	         $display("\n Mode register setting ERROR for MA0 ");
	data = (data>>24)&0xff;
	if (OPI_RD_LATANCY_FIXED==1) {
		if((data!=0x20)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}
	else
	{
		if((data!=0x00)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}
#endif

#ifdef OPI_RUN_20MHZ
//	   if (OPI_RD_LATANCY_FIXED==1) begin
//	      if (data[31:24]!=8'h20)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
//	   else begin
//	      if(data[31:24]!=8'h00)
//	         $display("\n Mode register setting ERROR for MA0 ");
//	   end
	data = (data>>24)&0xff;
	if (OPI_RD_LATANCY_FIXED==1) {
		if((data!=0x20)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}
	else
	{
		if((data!=0x00)){
			;//;//printf("\n Mode register setting ERROR for MA0 ");
			return FAIL;
		}
	}
#endif

//	//Mode register read address 4
//	srdata(`AHB_SLAVE20_BASE + 32'hec, data);
//	data[7:0]=8'h40;      //mode register write command
//	data[15:8]=8'h04;     //mode register address
//	data[23:16]=8'h00;    //mode register OP code
//	sw(`AHB_SLAVE20_BASE + 32'hec, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = read32(addr);
	data = set_fld(data, 7, 0, 0x40);
	data = set_fld(data, 15, 8, 0x04);
	data = set_fld(data, 23, 16, 0x00);
	write32(addr, data);


//	//trigger mode register read command
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	data[16]=1;
//	sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	data = read32(addr);
	data = set_fld(data, 16, 16, 1);
	write32(addr, data);


//	//            POLL_INI_RDY;
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 while (data[17]!=1'b1) begin
//	    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 end
	addr = AHB_SLAVE20_BASE + 0x00;
	do{
		data = (read32(addr)>>17)&1;
	}while(data!=1);



//	 //check Mode register 4
//	 srdata(`AHB_SLAVE20_BASE + 32'hec, data);
	addr = AHB_SLAVE20_BASE + 0xec;
	data = (read32(addr)>>24)&0xff;

#ifdef OPI_RUN_200MHZ
//	  if(data[31:24]!=8'h20)
//	     $display("\n Mode register setting ERROR for MA4 ");
	  if(data!=0x20){
	     ;//;//printf("\n Mode register setting ERROR for MA4 ");
	     return FAIL;
	  }

#endif

#ifdef OPI_RUN_166MHZ

//	  if(data[31:24]!=8'hc0)
//	     $display("\n Mode register setting ERROR for MA4 ");
	  if(data!=0xc0){
	     ;//;//printf("\n Mode register setting ERROR for MA4 ");
	     return FAIL;
	  }

#endif

#ifdef OPI_RUN_133MHZ
//	  if(data[31:24]!=8'h40)
//	     $display("\n Mode register setting ERROR for MA4 ");
	  if(data!=0x40){
	     ;//;//printf("\n Mode register setting ERROR for MA4 ");
	     return FAIL;
	  }
#endif

#ifdef OPI_RUN_100MHZ

//	  if(data[31:24]!=8'h80)
//	     $display("\n Mode register setting ERROR for MA4 ");

	  if(data!=0x80){
	     ;//;//printf("\n Mode register setting ERROR for MA4 ");
	     return FAIL;
	  }

#endif

#ifdef OPI_RUN_81MHZ

//	  if(data[31:24]!=8'h80)
//	     $display("\n Mode register setting ERROR for MA4 ");

	  if(data!=0x80){
	     ;//;//printf("\n Mode register setting ERROR for MA4 ");
	     return FAIL;
	  }

#endif

#ifdef OPI_RUN_50MHZ
//	  if(data[31:24]!=8'h00)
//	     $display("\n Mode register setting ERROR for MA4 ");
	  if(data!=0x00){
	     ;//;//printf("\n Mode register setting ERROR for MA4 ");
	     return FAIL;
	  }

#endif

#ifdef OPI_RUN_25MHZ
//	  if(data[31:24]!=8'h00)
//	     $display("\n Mode register setting ERROR for MA4 ");
	  if(data!=0x00){
	     ;//;//printf("\n Mode register setting ERROR for MA4 ");
	     return FAIL;
	  }

#endif

#ifdef OPI_RUN_20MHZ
//	  if(data[31:24]!=8'h00)
//	     $display("\n Mode register setting ERROR for MA4 ");
	  if(data!=0x00){
	     ;//;//printf("\n Mode register setting ERROR for MA4 ");
	     return FAIL;
	  }

#endif


	  //------------------------------------------------------------------------
	  // OPI DDR read start cycle setting
	  //------------------------------------------------------------------------

#ifdef OPI_RUN_200MHZ
//	  //OPI DDR register 0xE8 setting
//	  srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//	  data[3:0]=2;        //command cycle
//	  data[6:4]=4;        //read variable end cycle
//	  if (OPI_RD_LATANCY_FIXED==1) data[23:16]=14+3-5;  //read data start cycle
//	  else                         data[23:16]=7+3-5;  //read data start cycle
//	  data[31:24]=7+3-4;   //write data start cycle
//	  sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

		addr = AHB_SLAVE20_BASE + 0xe8;
		data = read32(addr);
		data = set_fld(data, 3,  0, 2);
		data = set_fld(data, 6,  4, 4);

		if(OPI_RD_LATANCY_FIXED==1){
			data = set_fld(data, 23,  16, (14+3-5));
		}
		else{
			data = set_fld(data, 23,  16, (7+3-5));
		}

		data = set_fld(data, 31,  24, (7+3-4));

		write32(addr, data);
#endif

#ifdef OPI_RUN_166MHZ
//		//OPI DDR register 0xE8 setting
//		srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//		data[3:0]=2;        //command cycle
//		data[6:4]=4;        //read variable end cycle
//		if (OPI_RD_LATANCY_FIXED==1) data[23:16]=12+3-5;  //read data start cycle
//		else                         data[23:16]=6+3-5;  //read data start cycle
//		data[31:24]=6+3-4;   //write data start cycle
//		sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

		addr = AHB_SLAVE20_BASE + 0xe8;
		data = read32(addr);
		data = set_fld(data, 3,  0, 2);
		data = set_fld(data, 6,  4, 4);

		if(OPI_RD_LATANCY_FIXED==1){
			data = set_fld(data, 23,  16, (12+3-5));
		}
		else{
			data = set_fld(data, 23,  16, (6+3-5));
		}

		data = set_fld(data, 31,  24, (6+3-4));

		write32(addr, data);

#endif

#ifdef OPI_RUN_133MHZ
//		//OPI DDR register 0xE8 setting
//		srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//		data[3:0]=2;        //command cycle
//		data[6:4]=4;        //read variable end cycle
//		if (OPI_RD_LATANCY_FIXED==1) data[23:16]=10+3-5;  //read data start cycle
//		else                         data[23:16]=5+3-5;  //read data start cycle
//		data[31:24]=5+3-4;   //write data start cycle
//		sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
		addr = AHB_SLAVE20_BASE + 0xe8;
		data = read32(addr);
		data = set_fld(data, 3,  0, 2);
		data = set_fld(data, 6,  4, 4);

		if(OPI_RD_LATANCY_FIXED==1){
			data = set_fld(data, 23,  16, (10+3-5));
		}
		else{
			data = set_fld(data, 23,  16, (5+3-5));
		}

		data = set_fld(data, 31,  24, (5+3-4));

		write32(addr, data);


#endif

#ifdef OPI_RUN_100MHZ
//		//OPI DDR register 0xE8 setting
//		srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//		data[3:0]=2;        //command cycle
//		data[6:4]=4;        //read variable end cycle
//		if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8+3-5;  //read data start cycle
//		else                         data[23:16]=4+3-5;  //read data start cycle
//		data[31:24]=4+3-4;   //write data start cycle
//		sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

		addr = AHB_SLAVE20_BASE + 0xe8;
		data = read32(addr);
		data = set_fld(data, 3,  0, 2);
		data = set_fld(data, 6,  4, 4);

		if(OPI_RD_LATANCY_FIXED==1){
			data = set_fld(data, 23,  16, (8+3-5));
		}
		else{
			data = set_fld(data, 23,  16, (4+3-5));
		}

		data = set_fld(data, 31,  24, (4+3-4));

		write32(addr, data);
#endif

#ifdef OPI_RUN_81MHZ
//		//OPI DDR register 0xE8 setting
//		srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//		data[3:0]=2;        //command cycle
//		data[6:4]=4;        //read variable end cycle
//		if (OPI_RD_LATANCY_FIXED==1) data[23:16]=8+3-5;  //read data start cycle
//		else                         data[23:16]=4+3-5;  //read data start cycle
//		data[31:24]=4+3-4;   //write data start cycle
//		sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);

		addr = AHB_SLAVE20_BASE + 0xe8;
		data = read32(addr);
		data = set_fld(data, 3,  0, 2);
		data = set_fld(data, 6,  4, 4);

		if(OPI_RD_LATANCY_FIXED==1){
			data = set_fld(data, 23,  16, (8+3-5));
		}
		else{
			data = set_fld(data, 23,  16, (4+3-5));
		}

		data = set_fld(data, 31,  24, (4+3-4));

		write32(addr, data);
#endif

#ifdef OPI_RUN_50MHZ
//		//OPI DDR register 0xE8 setting
//		srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//		data[3:0]=2;        //command cycle
//		data[6:4]=4;        //read variable end cycle
//		if (OPI_RD_LATANCY_FIXED==1) data[23:16]=6+3-5;  //read data start cycle
//		else                         data[23:16]=3+3-5;  //read data start cycle
//		data[31:24]=3+3-4;   //write data start cycle
//		sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
		addr = AHB_SLAVE20_BASE + 0xe8;
		data = read32(addr);
		data = set_fld(data, 3,  0, 2);
		data = set_fld(data, 6,  4, 4);

		if(OPI_RD_LATANCY_FIXED==1){
			data = set_fld(data, 23,  16, (6+3-5));
		}
		else{
			data = set_fld(data, 23,  16, (3+3-5));
		}

		data = set_fld(data, 31,  24, (3+3-4));

		write32(addr, data);
#endif

#ifdef OPI_RUN_25MHZ
//		//OPI DDR register 0xE8 setting
//		srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//		data[3:0]=2;        //command cycle
//		data[6:4]=4;        //read variable end cycle
//		if (OPI_RD_LATANCY_FIXED==1) data[23:16]=6+3-5;  //read data start cycle
//		else                         data[23:16]=3+3-5;  //read data start cycle
//		data[31:24]=3+3-4;   //write data start cycle
//		sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
		addr = AHB_SLAVE20_BASE + 0xe8;
		data = read32(addr);
		data = set_fld(data, 3,  0, 2);
		data = set_fld(data, 6,  4, 4);

		if(OPI_RD_LATANCY_FIXED==1){
			data = set_fld(data, 23,  16, (6+3-5));
		}
		else{
			data = set_fld(data, 23,  16, (3+3-5));
		}

		data = set_fld(data, 31,  24, (3+3-4));

		write32(addr, data);

#endif

#ifdef OPI_RUN_20MHZ
//		//OPI DDR register 0xE8 setting
//		srdata(`AHB_SLAVE20_BASE + 32'he8, data);
//		data[3:0]=2;        //command cycle
//		data[6:4]=4;        //read variable end cycle
//		if (OPI_RD_LATANCY_FIXED==1) data[23:16]=6+3-5;  //read data start cycle
//		else                         data[23:16]=3+3-5;  //read data start cycle
//		data[31:24]=3+3-4;   //write data start cycle
//		sw(`AHB_SLAVE20_BASE+32'he8, data, 2'b10);
		addr = AHB_SLAVE20_BASE + 0xe8;
		data = read32(addr);
		data = set_fld(data, 3,  0, 2);
		data = set_fld(data, 6,  4, 4);

		if(OPI_RD_LATANCY_FIXED==1){
			data = set_fld(data, 23,  16, (6+3-5));
		}
		else{
			data = set_fld(data, 23,  16, (3+3-5));
		}

		data = set_fld(data, 31,  24, (3+3-4));

		write32(addr, data);

#endif

//	  //------------------------------------------------------------------------
//	  // OPI DDR initial done
//	  //------------------------------------------------------------------------
//
//
//	   //{7'h0,initial_ok} = 8'h01; //0c
//	   srdata(`AHB_SLAVE20_BASE + 32'h0c, data);
//	   sw(`AHB_SLAVE20_BASE + 32'h0c, data & 32'hfffffffe | 8'h0<<24 | 8'h0<<16 | 8'h0<<8 | 8'h01, 2'b10);
	   setReg(AHB_SLAVE20_BASE + 0x0c, 0xfffffffe, 0x00, 0x00, 0x00, 0x01);



	return PASS;
}
