/*
 * opi_init.h
 *
 *  Created on: 2017年8月17日
 *      Author: spark
 */

#ifndef OPI_INIT_H_
#define OPI_INIT_H_

//#include "reg_util.h"

//#define OPI_RUN_200MHZ
//#define OPI_RUN_166MHZ
//#define OPI_RUN_133MHZ
//#define OPI_RUN_100MHZ
#define OPI_RUN_81MHZ
//#define OPI_RUN_50MHZ
//#define OPI_RUN_25MHZ
//#define OPI_RUN_20MHZ


//#define OPI_DDR_X16	//OPI 16bit mode; default: 8 bit mode

//check
#define OPI_RD_LATANCY_FIXED	0



u32 opi_init(void);


#endif /* OPI_INIT_H_ */
