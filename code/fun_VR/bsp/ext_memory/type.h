/*
 * type.h
 *
 *  Created on: 2017/05/16
 *      Author: spark
 */

#ifndef TYPE_H_
#define TYPE_H_

typedef unsigned int 	u32;
typedef unsigned short 	u16;
typedef unsigned char 	u8;

typedef signed int 		s32;
typedef signed short 	s16;
typedef signed char 	s8;

typedef volatile unsigned int 		vu32;
typedef volatile unsigned short 	vu16;
typedef volatile unsigned char 		vu8;

typedef volatile signed int 		vs32;
typedef volatile signed short 		vs16;
typedef volatile signed char 		vs8;

#define ADDR(addr)			(*((volatile u32 *)(addr)))

#define write32(addr,val)	ADDR((addr)) = ((val)&0xFFFFFFFF)
#define read32(addr)		(ADDR((addr))&0xFFFFFFFF)

#define MASK(H,L)       	((u32)((s32)0x80000000>>((H)-(L)))>>(31-(H)))
#define BIT(offset)			(1<<(offset))
#define regBIT(addr, bit)	((read32((addr))>>(bit))&1)

#define ITEM_SIZE(array)		(sizeof(array)/sizeof(array[0]))

#define PASS	1
#define FAIL	0
#define ERROR	0


#endif /* TYPE_H_ */
