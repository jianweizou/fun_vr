/*
 * comm.h
 *
 *  Created on: 2017年8月17日
 *      Author: spark
 */

#ifndef COMM_H_
#define COMM_H_

#include "type.h"

u32 set_fld(u32 src, u32 m, u32 n, u32 val);
void setReg(u32 addr, u32 mask, u32 offset24, u32 offset16, u32 offset8, u32 offset0)  __attribute__((always_inline));;


//#define setReg(ADDR, MASK, OFFSET24, OFFSET16, OFFSET8, OFFSET0)  {         \
//  u32 val;                                                                  \
//  val = read32(ADDR);                                                       \
//  val &= MASK;                                                              \
//	val |= OFFSET24<<24;                                                      \
//	val |= OFFSET16<<16;                                                      \
//	val |= OFFSET8<<8;                                                        \
//	val |= OFFSET0;                                                           \
//	write32(ADDR, val);                                                       \
//}  

#endif /* COMM_H_ */
