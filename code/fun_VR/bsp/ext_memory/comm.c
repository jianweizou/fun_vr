/*
 * comm.c
 *
 *  Created on: 2017/08/17
 *      Author: spark
 */

#include "comm.h"

u32 set_fld(u32 src, u32 m, u32 n, u32 val)
{
	src &= ~MASK(m,n);
	src |= val<<n;

	return src;

}


void setReg(u32 addr, u32 mask, u32 offset24, u32 offset16, u32 offset8, u32 offset0)
{
	u32 val;

	val = read32(addr);
	val &= mask;
	val |= offset24<<24;
	val |= offset16<<16;
	val |= offset8<<8;
	val |= offset0;
	write32(addr, val);
}
