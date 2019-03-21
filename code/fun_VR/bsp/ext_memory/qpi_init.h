/*
 * qpi_init_v2.h
 *
 *  Created on: 2017年9月7日
 *      Author: spark
 */

#ifndef QPI_INIT_H_
#define QPI_INIT_H_

#include "type.h"

//#define QPI_RUN_200MHZ
//#define QPI_RUN_166MHZ
//#define QPI_RUN_133MHZ
//#define QPI_RUN_100MHZ
#define QPI_RUN_81MHZ
//#define QPI_RUN_50MHZ
//#define QPI_RUN_25MHZ
//#define QPI_RUN_20MHZ

//SQPI_16mb
//SQPI memory page size, 512byte : 3
#define SQPI_PAGE_SIZE_512B	(3)
//SQPI memory page size, 1Kbyte : 0
#define SQPI_PAGE_SIZE_1KB	(0)
 
//============================================
#define Lyonteck_model
#define SQPI_PAGE_SIZE	(SQPI_PAGE_SIZE_1KB)
//============================================


u32 qpi_init(void);

#endif /* QPI_INIT_H_ */
