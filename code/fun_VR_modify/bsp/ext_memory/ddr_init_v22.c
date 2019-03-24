/*
 * ddr_init.c
 *	SUC7320 FPGA/ASIC DDR Init
 *
 *  Created on: 2017年6月16日
 *      Author: spark
 */

//#include <stdio.h>
#include "ddr_init.h"
#include "comm.h"
#include <stdint.h>

//Initval[MHz][model]
const initval_t InitVal[][2] =
{
		{	//20MHz
		{DDR_SIZE_64M, 0x40, 0x00},	//	Micron
		{DDR_SIZE_16M, 0x40, 0x00},	//	Etron
		},

		{	//25MHz
		{DDR_SIZE_64M, 0x40, 0x00},	//	Micron
		{DDR_SIZE_16M, 0x40, 0x00},	//	Etron
		},

		{	//50MHz
		{DDR_SIZE_64M, 0x40, 0x00},	//	Micron
		{DDR_SIZE_16M, 0x40, 0x00},	//	Etron
		},

		{	//81MHz
		{DDR_SIZE_64M, 0x90, 0x00},	//	Micron
		{DDR_SIZE_16M, 0x50, 0x00},	//	Etron
		},

		{	//100MHz
		{DDR_SIZE_64M, 0x90, 0x00},	//	Micron
		{DDR_SIZE_16M, 0x50, 0x00},	//	Etron
		},

		{	//133MHz
		{DDR_SIZE_64M, 0xa2, 0x01},	//	Micron
		{DDR_SIZE_16M, 0x63, 0x01},	//	Etron
		},

		{	//166MHz
		{DDR_SIZE_64M, 0xa4, 0x15},	//	Micron
		{DDR_SIZE_16M, 0xa6, 0x15},	//	Etron
		},

		{	//200MHz
		{DDR_SIZE_64M, 0xa6, 0x16},	//	Micron
		{DDR_SIZE_16M, 0xa8, 0x16},	//	Etrondddddddddddddd
		},

};

////[MHz][model]
//setreg_t PHY_DQS_TIMING[][2]={
//
//
//};
//
////[MHz][model]
//setreg_t PHY_GATE_LPBK[][2]={
//
//};


void step0(u32 model, u32 MHz)
{
	initval_t target ;
	u8 SET_DDR_SIZE, BYTE16, BYTE8;
	u32 addr, val;

	target = InitVal[MHz][model];

	SET_DDR_SIZE = target.SET_DDR_SIZE;
	BYTE16 = target.BYTE16;
	BYTE8 = target.BYTE8;

//	;//printf("DDR Size: %d\n", target.SET_DDR_SIZE);
//	;//printf("BYTE16: %x\n", target.BYTE16);
//	;//printf("BYTE8: %x\n", target.BYTE8);

	addr = AHB_SLAVE20_BASE + 0x04;
	val = read32(addr);
	val &= 0xf8e0a8ff;
	val |= SET_DDR_SIZE<<24;
	val |= BYTE16<<16;
	val |= BYTE8<<8;
	val |= 0x00;

	write32(addr, val);

}

void setREF(u32 MHz)
{
  u32 addr, val;
  u32 set_val[] = {
      0x009c,	//20MHz
      0x003c,	//25MHz
      0x0186,	//50MHz
      0x0277,	//82MHz
      0x030c,	//100MHz
      0x040d,	//133MHz
      0x050e,	//166MHz
      0x0618,	//200MHz
  };

  addr = AHB_SLAVE20_BASE + 0x08;
  val = read32(addr);
  val &= ~(0x3fff0000);
  val |= (set_val[MHz]<<24);
	val = 0x4ef0000;
  write32(addr, val);
}


//void set_dqs_phy(u32 model, u32 MHz)
//{
//  u32 addr, val;
//
//}

/********************/

#ifndef   SN_OPM_SYS0_BASE
  #define SN_OPM_SYS0_BASE  0x45000000
#endif



u32 ddr_init()
{
	u32 addr, val;
	
//	 //
//	 //Disable OPI mode
//	 //
//	 srdata(`AHB_SLAVE20_BASE + 32'he0, data);
//	 data[0] = 0;
//	 sw(`AHB_SLAVE20_BASE + 32'he0, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xe0;
	val = read32(addr);
	val &= ~(1);
	write32(addr, val);

////set DDR IO drive strength
// srdata(`AHB_SLAVE20_BASE + 32'h60, data);
// data[2:1] = 2;
// sw(`AHB_SLAVE20_BASE + 32'h60, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x60;
	val = read32(addr);
	val = set_fld(val, 2, 1, 0x02);
	write32(addr, val);	

////==============================================================
////
////New add AHB control signal
////

////{4'b0,ch4_async,ch3_async,ch2_async,ch1_async} = 8'h0f; //2d
////srdata(`AHB_SLAVE20_BASE + 32'h0c, data);
////sw(`AHB_SLAVE20_BASE + 32'h0c, data & 32'hfffff0ff | 8'h0<<24 | 8'h0<<16 | 8'h0f<<8 | 8'h0, 2'b10);
	//	addr = AHB_SLAVE20_BASE + 0x0c;
	//	val = read32(addr);
	//	val &= 0xfffff0ff;
	//	val |= 0x0<<24;
	//	val |= 0x0<<16;
	//	val |= 0x0f<<8;
	//	val |= 0x0;
	//	write32(addr, val);


//	 //{ch1_ahb_wrlen}=8'h0f; //11
//	 //{ch1_ahb_rdlen}=8'h0f; //10
//	 //{ch0_ahb_wrlen}=8'h0f; //11
//	 //{ch0_ahb_rdlen}=8'h0f; //10
//   sw(`AHB_SLAVE20_BASE + 32'ha0, (8'h0f<<24 | 8'h0f<<16 | 8'h0f<<8 | 8'h0f), 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xa0, 0x00000000, 0x0f, 0x0f, 0x0f, 0x0f);

//	 //{ch3_ahb_wrlen}=8'h0f; //11
//	 //{ch3_ahb_rdlen}=8'h0f; //10
//	 //{ch2_ahb_wrlen}=8'h0f; //11
//	 //{ch2_ahb_rdlen}=8'h0f; //10
//   sw(`AHB_SLAVE20_BASE + 32'ha4, (8'h0f<<24 | 8'h0f<<16 | 8'h0f<<8 | 8'h0f), 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xa4, 0x00000000, 0x0f, 0x0f, 0x0f, 0x0f);

//	 //{ch8_command_seq_int,ch7_command_seq_int,ch6_command_seq_int,ch5_command_seq_int,
//	 // ch4_command_seq_int,ch3_command_seq_int,ch2_command_seq_int,ch1_command_seq_int}; //c0
//	 srdata(`AHB_SLAVE20_BASE + 32'hc0, data);		 
//	 sw(`AHB_SLAVE20_BASE + 32'hc0, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xc0, 0x00000000, 0x00, 0x00, 0x00, 0x00);

//	 //{1'b0,ch78_command_priority,1'b0,ch56_command_priority}, //c6
//	 //{1'b0,ch34_command_priority,1'b0,ch12_command_priority}  //c5
//	 //{1'b0,refl_command_priority,1'b0,refh_command_priority}  //c4
//	 srdata(`AHB_SLAVE20_BASE + 32'hc4, data);
//	 sw(`AHB_SLAVE20_BASE + 32'hc4, data & 32'hffff8888 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h10, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0xc4, 0xffff8888, 0x00, 0x00, 0x00, 0x10);

////{ch8_command_seq_superb,ch7_command_seq_superb,ch6_command_seq_superb,ch5_command_seq_superb,
//// ch4_command_seq_superb,ch3_command_seq_superb,ch2_command_seq_superb,ch1_command_seq_superb}; //d0
//srdata(`AHB_SLAVE20_BASE + 32'hd0, data);
//sw(`AHB_SLAVE20_BASE + 32'hd0, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xd0;
	val = read32(addr);
	val = 0x00;
	write32(addr, val);
	
////{WR_REQ_DELAY}  //d4
// srdata(`AHB_SLAVE20_BASE + 32'hd4, data);
// data[3:0]=4'hf;
// sw(`AHB_SLAVE20_BASE + 32'hd4, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0xd4;
	val = read32(addr);
	val = set_fld(val, 3, 0, 0); /*  Channel clk > DRAM CLK : 0 
                                        DRAM CLK > Channel clk : 0xF   */
    
    
	write32(addr, val);	

////PHY_DLL_MASTER_CTRL_REG=32'h0012_0004;
// srdata(`AHB_SLAVE20_BASE + 32'h1C, data);
// sw(`AHB_SLAVE20_BASE + 32'h1C, data & 32'h00000000 | 8'h00<<24 | 8'h22<<16 | 8'h00<<8 | 8'h04, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x1c, 0x00000000, 0x00, 0x22, 0x00, 0x04);

////PHY_DLL_SLAVE_CTRL_REG=32'h40404040;
// srdata(`AHB_SLAVE20_BASE + 32'h20, data);
// sw(`AHB_SLAVE20_BASE + 32'h20, data & 32'h00000000 | 8'h40<<24 | 8'h40<<16 | 8'h40<<8 | 8'h40, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x20, 0x00000000, 0x40, 0x40, 0x40, 0x40);
	
#if DDR_freq==DDR_RUN_20MHZ
#if DDR_model==Micron
//	`ifdef RUN_20MHZ
//	`ifdef Micron_model
//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h05<<16 | 8'h14<<8 | 8'h12, 2'b10); 
//	`endif
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x05, 0x14, 0x12);

//	`ifdef Micron_model
//	 //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//	 srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h20<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
//	`endif
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x20, 0x02, 0x00, 0x61);

#endif //End Micron

#if DDR_model==Etron
//	`ifdef Etron_model
//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h05<<16 | 8'h14<<8 | 8'h12, 2'b10);
//	`endif
	//setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x05, 0x14, 0x12);
	addr = AHB_SLAVE20_BASE + 0x24;
  val = read32(addr);
  val &= 0xfff00000;
	val = 0x051412;
	write32(addr, val);


//	 //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//	 srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h20<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x20, 0x02, 0x00, 0x61);

#endif
#endif

#if DDR_freq==DDR_RUN_25MHZ
#if DDR_model==Micron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24

//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h05<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x05, 0x14, 0x12);

//	  //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//	  srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//	  sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h20<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x20, 0x02, 0x00, 0x61);

#endif //End Micron

#if DDR_model==Etron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h05<<16 | 8'h14<<8 | 8'h12, 2'b10);			
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x05, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//		sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h01<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x01, 0x02, 0x00, 0x61);

#endif //End Etron
#endif //End RUN_25MHZ

#if DDR_freq==DDR_RUN_50MHZ
#if DDR_model==Micron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h05<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x05, 0x14, 0x12);
//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//    sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h20<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x20, 0x02, 0x00, 0x61);

#endif //End Micron

#if DDR_model==Etron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h05<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x05, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//		sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h01<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x01, 0x02, 0x00, 0x61);
	
#endif //End Etron
#endif //End RUN_50MHZ



#if DDR_freq==DDR_RUN_81MHZ
#if DDR_model==Micron
//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h06<<16 | 8'h14<<8 | 8'h12, 2'b10);	
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x06, 0x14, 0x12);

//	 //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//	 srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h20<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x20, 0x02, 0x00, 0x61);

#endif //End Micron

#if DDR_model==Etron

//	 //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//	 //{PHY_DQS_TIMING_REG}=8'h14;    //25
//	 //{PHY_DQ_TIMING_REG}=8'h12;     //24
//	 srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h05<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x05, 0x14, 0x12);

//	 //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//	 srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h01<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x01, 0x02, 0x00, 0x61);
	
#endif //End Etron
#endif //End RUN_81MHZ


#if DDR_freq==DDR_RUN_100MHZ
#if DDR_model==Micron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h06<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x06, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//    sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h20<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x20, 0x02, 0x00, 0x61);

#endif //End Micron

#if DDR_model==Etron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h05<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x05, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//		sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h01<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x01, 0x02, 0x00, 0x61);

#endif //End Etron
#endif //End RUN_100MHZ

#if DDR_freq==DDR_RUN_133MHZ
#if DDR_model==Micron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h06<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x06, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//    sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h20<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x20, 0x02, 0x00, 0x61);

#endif //End Micron

#if DDR_model==Etron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h05<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x05, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//    sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h00<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x00, 0x02, 0x00, 0x61);

#endif //End Etron
#endif //End RUN_133MHZ

#if DDR_freq==DDR_RUN_166MHZ
#if DDR_model==Micron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h06<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x06, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//    sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h20<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x20, 0x02, 0x00, 0x61);

#endif //End Micron

#if DDR_model==Etron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h06<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x06, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//    sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h00<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x00, 0x02, 0x00, 0x61);

#endif //End Etron
#endif //End RUN_166MHZ

#if DDR_freq==DDR_RUN_200MHZ
#if DDR_model==Micron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h06<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x06, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//    sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h20<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);			
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x20, 0x02, 0x00, 0x61);

#endif //End Micron

#if DDR_model==Etron
//    //{4'b0,PHY_READ_CTRL_REG}=8'h5; //26
//    //{PHY_DQS_TIMING_REG}=8'h14;    //25
//    //{PHY_DQ_TIMING_REG}=8'h12;     //24
//    srdata(`AHB_SLAVE20_BASE + 32'h24, data);
//    sw(`AHB_SLAVE20_BASE + 32'h24, data & 32'hfff00000 | 8'h00<<24 | 8'h06<<16 | 8'h14<<8 | 8'h12, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x24, 0xfff00000, 0x00, 0x06, 0x14, 0x12);

//    //PHY_GATE_LPBK_CTRL_REG=32'h60010060;
//    srdata(`AHB_SLAVE20_BASE + 32'h28, data);
//    sw(`AHB_SLAVE20_BASE + 32'h28, data & 32'h00000000 | 8'h00<<24 | 8'h02<<16 | 8'h00<<8 | 8'h61, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x28, 0x00000000, 0x00, 0x02, 0x00, 0x61);

#endif //End Etron
#endif //End RUN_200MHZ



// DDR CTRL v2.0 disable
////PHY_PAD_DRIVER_REG_0 = 32'h02000000;
////srdata(`AHB_SLAVE20_BASE + 32'h2c, data);
////sw(`AHB_SLAVE20_BASE + 32'h2c, data & 32'hfffffff0 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h0f, 2'b10);
//	addr = AHB_SLAVE20_BASE + 0x2c;
//	val = read32(addr);
//	val &= 0x00000000;
//	val |= 0x01<<24;
//	val |= 0x00<<16;
//	val |= 0x00<<8;
//	val |= 0x10;
//	write32(addr, val);
//
////PHY_PAD_DRIVER_REG_1 = 32'h00000000;
////srdata(`AHB_SLAVE20_BASE + 32'h30, data);
////sw(`AHB_SLAVE20_BASE + 32'h30, data & 32'hffffffe0 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h03, 2'b10);
//	addr = AHB_SLAVE20_BASE + 0x30;
//	val = read32(addr);
//	val &= 0x00000000;
//	val |= 0x00<<24;
//	val |= 0x00<<16;
//	val |= 0x00<<8;
//	val |= 0x00;
//	write32(addr, val);

//#259
////PHY_PAD_DRIVE_REG_2=32'h0000_0000;
//srdata(`AHB_SLAVE20_BASE + 32'h34, data);
//sw(`AHB_SLAVE20_BASE + 32'h34, data & 32'hffffffff | 8'hff<<24 | 8'hff<<16 | 8'hff<<8 | 8'hff, 2'b10);
//	setReg(AHB_SLAVE20_BASE + 0x34, 0xffffffff, 0xff, 0xff, 0xff, 0xff);
	addr = AHB_SLAVE20_BASE + 0x34;
	val = read32(addr);
	val &= 0x00000000;
	val = 0xFFFFFFFF;
	write32(addr, val);


	//PHY_PAD_TERM_REG_0 = 32'h0000_f000;
//	srdata(`AHB_SLAVE20_BASE + 32'h38, data);
//	sw(`AHB_SLAVE20_BASE + 32'h38, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'hf0<<8 | 8'h00, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x38;
	val = read32(addr);
	val &= 0x00000000;
	val |= 0x00<<24;
	val |= 0x00<<16;
	val |= 0xf0<<8;
	val |= 0x00;
	write32(addr, val);

	//PHY_PAD_TERM_REG_1 = 32'h0000_0000;
//	srdata(`AHB_SLAVE20_BASE + 32'h3c, data);
//	sw(`AHB_SLAVE20_BASE + 32'h3c, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x3c;
	val = read32(addr);
	val &= 0x00000000;
	val |= 0x00<<24;
	val |= 0x00<<16;
	val |= 0x00<<8;
	val |= 0x00;
	write32(addr, val);

	//PHY_PAD_TERM_REG_2 = 32'h0000_0000;
//	srdata(`AHB_SLAVE20_BASE + 32'h40, data);
//	sw(`AHB_SLAVE20_BASE + 32'h40, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);	
	addr = AHB_SLAVE20_BASE + 0x40;
	val = read32(addr);
	val &= 0x00000000;
	val |= 0x00<<24;
	val |= 0x00<<16;
	val |= 0x00<<8;
	val |= 0x00;
	write32(addr, val);

	//FIT2REF=32'h1;
//	srdata(`AHB_SLAVE20_BASE + 32'h5c, data);
//	sw(`AHB_SLAVE20_BASE + 32'h5c, data & 32'hfffffffe | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h01, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x5c;
	val = read32(addr);
	val &= 0xfffffffe;
	val |= 0x00<<24;
	val |= 0x00<<16;
	val |= 0x00<<8;
	val |= 0x01;
	write32(addr, val);

//	//Set if burst can be interrupt for channels
//	 //{6'b0,DLL_LOCK} ; //1b
//	 //{7'b0,N_DLL_RST}; //1a
//	 srdata(`AHB_SLAVE20_BASE + 32'h18, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h18, data & 32'h00000000 | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);
//	setReg(AHB_SLAVE20_BASE + 0x18, 0x00000000, 0x00, 0x00, 0x00, 0x00);
//	addr = AHB_SLAVE20_BASE + 0x18;
//	val = read32(addr);
//	val &= 0;
//	val = 0x0;
//	write32(addr, val);

#if 1
    /* SW reset on DRAM ctr */
    *(u32*)(AHB_SLAVE20_BASE + 0x18) |= 1<<8;
    *(u32*)(AHB_SLAVE20_BASE + 0x18) &= ~(1<<8);
#endif

//reset DDR PHY DLL
	//{6'b0,DLL_LOCK} = 8'h0; //1b
	//{7'b0,N_DLL_RST} = 8'h1; //1a
	//srdata(`AHB_SLAVE20_BASE + 32'h18, data);
	addr = AHB_SLAVE20_BASE + 0x18;
	val = read32(addr);

//reset DLL
	//sw(`AHB_SLAVE20_BASE + 32'h18, data & 32'hfffeffff | 8'h00<<24 | 8'h00<<16 | 8'h00<<8 | 8'h00, 2'b10);	
	val &= 0xfffeffff;
	val |= 0x00<<24;
	val |= 0x00<<16;
	val |= 0x00<<8;
	val |= 0x00;
	write32(addr, val);

	//un-reset DLL
	//sw(`AHB_SLAVE20_BASE + 32'h18, data & 32'hfffeffff | 8'h00<<24 | 8'h01<<16 | 8'h00<<8 | 8'h00, 2'b10);
	val &= 0xfffeffff;
	val |= 0x00<<24;
	val |= 0x01<<16;
	val |= 0x00<<8;
	val |= 0x00;
	write32(addr, val);

	//check if DLL is locked
//	srdata(`AHB_SLAVE20_BASE + 32'h18, data);
//	while (data[25:24]!==2'b11) begin
//    srdata(`AHB_SLAVE20_BASE + 32'h18, data);
//	end
	addr = AHB_SLAVE20_BASE + 0x18;
	do{
		val = read32(addr);
		val = (val>>24)&0x3;
	}while(val != 0x3);

//#386 => X
	
//#434
//`elsif RUN_81MHZ
//    // @DDR81
//    // 0x5h = 8'h55; // TRAS=4dck, TRP=2dck, TRCD=2dck, TRRD=1dck
//    // 0x6h = 8'h02; // TWR=2dck(setting 6dck), TRFC=6/7dck
//    // 0x7h = SET__DDR_SIZE; //0:64Mbits, 3:16Mbits
//    $display("\n  ******Memory model RUN_81MHZ******");
//    srdata(`AHB_SLAVE20_BASE + 32'h04, data);
//    `ifdef Micron_model
//       // TRFC=8dck(min), 70ns for micron model / 80ns for Etron_model
//       sw(`AHB_SLAVE20_BASE + 32'h04, data & 32'hf8e0a8ff | SET__DDR_SIZE<<24 | 8'h90<<16 | 8'h00<<8 | 8'h0, 2'b10);
//    `endif 
//    `ifdef Etron_model
//       // TRFC=8dck(min), 70ns for micron model / 80ns for Etron_model
//       sw(`AHB_SLAVE20_BASE + 32'h04, data & 32'hf8e0a8ff | SET__DDR_SIZE<<24 | 8'h50<<16 | 8'h00<<8 | 8'h0, 2'b10);
//    `endif
#if DDR_freq==DDR_RUN_81MHZ
//	printf("\r\n  ******Memory model RUN_81MHZ******\r\n");
	addr = AHB_SLAVE20_BASE + 0x04;
#if DDR_model==Micron
	val &= 0xf8e0a8ff;
	val |= 0x00<<24;
	val |= 0x90<<16;
	val |= 0x00<<8;
	val |= 0x00;
	write32(addr, val);
#endif //End Micron

#if DDR_model==Etron
	val &= 0xf8e0a8ff;
	val |= 0x00<<24;
	val |= 0x50<<16;
	val |= 0x00<<8;
	val |= 0x00;
	write32(addr, val);
#endif //End Etron
#endif //End DDR_RUN_81MHZ

	//==============================
	//	cpu_data = 8'h3c;
//	addr = AHB_SLAVE20_BASE + 0x08;
//	val = read32(addr);
//	val &= 0x00ffffff;
//	val |= 0x3c<<24;
//	val |= 0x0<<16;
//	val |= 0x0<<8;
//	val |= 0x0;
//	write32(addr, val);
	setREF(DDR_freq);
	//==============================


	//==============================================================
	step0(DDR_model, DDR_freq);

	//CL=3
//	//            cpu_data = 8'h08; // SET DDR_CKE_EN = 1
//  //            cpu_write_one_byte(16'h8c01, cpu_data);
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//  sw(`AHB_SLAVE20_BASE + 32'h00, data & 32'hfffff1ff | 8'h00<<24 | 8'h0<<16 | 8'h0c<<8 | 8'h0, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x00, 0xfffff1ff, 0x00, 0x00, 0x0c, 0x00);

	//wait DDR clocking stable

//	// cpu_mode = "ddr init2";
//	//            cpu_data = 8'h18; // SET DDR_CKE_HIGH = 1
//	//            cpu_write_one_byte(16'h8c01, cpu_data);
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);	
//	 sw(`AHB_SLAVE20_BASE + 32'h00, data & 32'hffffe7ff | 8'h00<<24 | 8'h0<<16 | 8'h18<<8 | 8'h0, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x00, 0xffffe7ff, 0x00, 0x00, 0x18, 0x00);


	//=== step 1 ===
	//POLL_INI_RDY;
//	srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	while (data[17]!==1'b1) begin
//    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	end
	addr = AHB_SLAVE20_BASE + 0x00;
	while(regBIT(addr, 17)!=1);

//------------------------------------------------------------------------
//Wait 200us for device initialization  
//------------------------------------------------------------------------
	DELAY_US(162, 300);

////            cpu_data = 8'h38; // SET DDR_INT_MODE = PALL
////            cpu_write_one_byte(16'h8c01, cpu_data);
////            cpu_data = 8'h01; // SET DDR_INT_TRIG = 1
////            cpu_write_one_byte(16'h8c02, cpu_data);
// srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// sw(`AHB_SLAVE20_BASE + 32'h00, data & 32'hfffec7ff | 8'h00<<24 | 8'h01<<16 | 8'h38<<8 | 8'h0, 2'b10);
	setReg(AHB_SLAVE20_BASE + 0x00, 0xfffec7ff, 0x00, 0x01, 0x38, 0x00);

//            cpu_mode = "ddr init3";

	//=== step 2 ===
////            POLL_INI_RDY;
// srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// while (data[17]!==1'b1) begin
//    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// end 
	addr = AHB_SLAVE20_BASE + 0x00;
	while(regBIT(addr, 17)!=1);


//            cpu_data = 8'h98; // SET DDR_INT_MODE = REF = 04
//            cpu_write_one_byte(16'h8c01, cpu_data);
//            cpu_data = 8'h01; // SET DDR_INT_TRIG = 1
//            cpu_write_one_byte(16'h8c02, cpu_data);
// srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// sw(`AHB_SLAVE20_BASE + 32'h00, data & 32'hfffe16ff | 8'h00<<24 | 8'h01<<16 | 8'h98<<8 | 8'h0, 2'b10);	
	setReg(AHB_SLAVE20_BASE + 0x00, 0xfffe16ff, 0x00, 0x01, 0x98, 0x00);
	
//            cpu_mode = "ddr init7";
	//=== step 6 ===
//            POLL_INI_RDY;
// srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// while (data[17]!==1'b1) begin
//    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// end	
	addr = AHB_SLAVE20_BASE + 0x00;
	while(regBIT(addr, 17)!=1);

	//	cpu_data = 8'h98;
//            cpu_data = 8'h98; // SET DDR_INT_MODE = REF = 04
//            cpu_write_one_byte(16'h8c01, cpu_data);
//            cpu_data = 8'h01; // SET DDR_INT_TRIG = 1
//            cpu_write_one_byte(16'h8c02, cpu_data);
// srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// sw(`AHB_SLAVE20_BASE + 32'h00, data & 32'hfffe16ff | 8'h00<<24 | 8'h01<<16 | 8'h98<<8 | 8'h0, 2'b10);	
	setReg(AHB_SLAVE20_BASE + 0x00, 0xfffe16ff, 0x00, 0x01, 0x98, 0x00);

//            cpu_mode = "ddr init8";

	//=== step 7 ===
//            POLL_INI_RDY;
// srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// while (data[17]!==1'b1) begin
//    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// end	
	addr = AHB_SLAVE20_BASE + 0x00;
	while(regBIT(addr, 17)!=1);

//            cpu_data = 8'hb8; // SET DDR_INT_MODE = MRS = 05
//            cpu_write_one_byte(16'h8c01, cpu_data);
//            cpu_data = 8'h01; // SET DDR_INT_TRIG = 1
//            cpu_write_one_byte(16'h8c02, cpu_data);
// srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// sw(`AHB_SLAVE20_BASE + 32'h00, data & 32'hfffe16ff | 8'h00<<24 | 8'h01<<16 | 8'hb8<<8 | 8'h0, 2'b10);	
	addr = AHB_SLAVE20_BASE + 0x00;
	val = read32(addr);
    val |= (1<<16);
    val &= ~(7<<13);
    val |= 5<<13;
	write32(addr, val);

	//=== step 7 ===
//            POLL_INI_RDY;
// srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// while (data[17]!==1'b1) begin
//    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// end	
	addr = AHB_SLAVE20_BASE + 0x00;
	while(regBIT(addr, 17)!=1);

//vicky updated
//------------------------------------------------------------------------
//Wait 200 clock cycles for SDRAM DLL locked  
//------------------------------------------------------------------------
//	DELAY_US(162, 100);


//	//            cpu_data = 8'h59; // SET DDR_INT_MODE = EMRS_EN_DLL = 02, DDR_MA12_1 = 0 (driving strengh=weak)
//	//            cpu_write_one_byte(16'h8c01, cpu_data);
//	//            cpu_data = 8'h01; // SET DDR_INT_TRIG = 1
//	//            cpu_write_one_byte(16'h8c02, cpu_data);
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 sw(`AHB_SLAVE20_BASE + 32'h00, data & 32'h000e16ff | 8'h00<<24 | 8'h01<<16 | 8'h59<<8 | 8'h0, 2'b10);
//	setReg(AHB_SLAVE20_BASE + 0x00, 0x000e16ff, 0x00, 0x01, 0x59, 0x0);
	addr = AHB_SLAVE20_BASE + 0x00;
	val = read32(addr);
    val |= 1<<16;
    
    val &= ~(7<<13);
    val |= 2<<13;
    
    val &= ~(0xFFF<<20);
    val |= 0x010<<20;

	write32(addr, val);

	//=== step 7 ===
//            POLL_INI_RDY;
// srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// while (data[17]!==1'b1) begin
//    srdata(`AHB_SLAVE20_BASE + 32'h00, data);
// end	
	addr = AHB_SLAVE20_BASE + 0x00;
	while(regBIT(addr, 17)!=1);

//	//SRF_EN=0
//	//REF_EN=1
//	//DDR_TREFC=2'h1
//	            //cpu_data = 8'h9b; // SET REF_EN = 1, after DDR init is finished , the REF_EN can be set to 1
//	//            cpu_data = 8'h1b; // SET REF_EN = 1, after DDR init is finished , the REF_EN can be set to 1
//	//            cpu_write_one_byte(16'h8c00, cpu_data);
//	 srdata(`AHB_SLAVE20_BASE + 32'h00, data);
//	 data[3]=1;
//	 data[6:4]=2;
//	 sw(`AHB_SLAVE20_BASE + 32'h00, data, 2'b10);
	addr = AHB_SLAVE20_BASE + 0x00;
	val = read32(addr);
	val &= ~(0x78);
	val |= (1<<3);
	val |= (2<<4);
	write32(addr, val);




	//==================================================

	//{7'h0, initial_ok} = 8'h01; //0c
//srdata(`AHB_SLAVE20_BASE + 32'h0c, data);
//sw(`AHB_SLAVE20_BASE + 32'h0c, data & 32'hfffffffe | 8'h0<<24 | 8'h0<<16 | 8'h0<<8 | 8'h01, 2'b10);	
	addr = AHB_SLAVE20_BASE + 0x0c;
	val = read32(addr);
	val &= 0xfffffffe;
	val |= 0x00<<24;
	val |= 0x00<<16;
	val |= 0x00<<8;
	val |= 0x01;
	write32(addr, val);

#if 0
    /* SW reset on DRAM ctr */
    *(u32*)(AHB_SLAVE20_BASE + 0x18) |= 1<<8;
    *(u32*)(AHB_SLAVE20_BASE + 0x18) &= ~(1<<8);

    //check if DLL is locked
//	srdata(`AHB_SLAVE20_BASE + 32'h18, data);
//	while (data[25:24]!==2'b11) begin
//    srdata(`AHB_SLAVE20_BASE + 32'h18, data);
//	end
    addr = AHB_SLAVE20_BASE + 0x18;
    do{
        val = read32(addr);
        val = (val>>24)&0x3;
    }while(val != 0x3);
#endif

  return 0;
}

void get_ddr_reg_table()
{
	int	i = 0;
	u32 addr = 0;
	u32 val = 0;
	
//	printf("\r\n");
	for( i=0; i<64; i++ )
	{
		addr = AHB_SLAVE20_BASE+(i*4);
		val = read32(addr);
		printf("DDR REG %08x : %08x\r\n", addr, val);
	}
}

 void ddr_powerdown()
{
    uint32_t data = 0;
        
    *(uint32_t*)0x40040000 |= (1<<2); /* enable Self refresh */
    *(uint32_t*)0x40040000 &= ~(1<<3); /* disable auto refresh */
    
    for(int i=0; i<5; i++);
    
    *(uint32_t*)0x40040000 &= ~(1<<12); /* CKE low */

    for(int i=0; i<5; i++);
    
    *(uint32_t*)0x40040000 &= ~(1<<11); /* disable clk */
    
    for(int i=0; i<5; i++);
}


void ddr_powerup()
{
    uint32_t data = 0;
    *(uint32_t*)0x40040000 |= (1<<11); /* enable clk */
    
    for(int i=0; i<5; i++);    
    *(uint32_t*)0x40040000 |= (1<<12); /* CKE high */
    
    for(int i=0; i<5; i++);
    
    *(uint32_t*)0x40040000 |= (1<<3); /* enable auto refresh */
    *(uint32_t*)0x40040000 &= ~(1<<2); /* disable Self refresh */
        
    for(int i=0; i<5; i++);
}