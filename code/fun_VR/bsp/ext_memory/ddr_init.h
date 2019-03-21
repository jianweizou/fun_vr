/*
 * ddr_init.h
 *
 *  Created on: 2017年5月18日
 *      Author: spark
 */

#ifndef DDR_INIT_H_
#define DDR_INIT_H_

#include "type.h"

#define DDR_SIZE_64M	0
#define DDR_SIZE_16M	3

#if 0
enum
{
	RUN_25MHZ = 0,
	RUN_50MHZ,
	RUN_100MHZ,
	RUN_133MHZ,
	RUN_166MHZ,
	RUN_200MHZ
};

enum
{
	Micron = 0,
	Etron,
};
#endif

#define Micron  0
#define Etron   1

#define  DDR_RUN_20MHZ      0
#define  DDR_RUN_25MHZ      1
#define  DDR_RUN_50MHZ      2
#define  DDR_RUN_81MHZ      3
#define  DDR_RUN_100MHZ     4
#define  DDR_RUN_133MHZ     5
#define  DDR_RUN_166MHZ     6
#define  DDR_RUN_200MHZ     7

typedef struct initval_
{
	u32 SET_DDR_SIZE;
	u8	BYTE16;
	u8 	BYTE8;
}initval_t;


typedef struct
{
	u32	MASK;
	u8	BYTE24;
	u8	BYTE16;
	u8 	BYTE8;
	u8 	BYTE0;
}setreg_t;
 
//#define AHB_SLAVE20_BASE	0x4005A000
#define AHB_SLAVE20_BASE	0x40040000

/********* DDR Setting *********/
#define DDR_model	Etron
#define DDR_freq	DDR_RUN_81MHZ

/*******************************/

u32 ddr_init(void);

#endif /* DDR_INIT_H_ */
