
#include "base_types.h"
#include "DataFlashApi.h"

//////////////////////////////////////////
// The API must be provided by caller
//////////////////////////////////////////
#ifndef k_nFlashPageSize
#define k_nFlashPageSize	0x400	// 1024 Bytes
#endif

INT DataFlash_Write(BYTE *dest, BYTE *src, INT nSize)
{
	// following is fake code

	memcpy(dest, src, nSize);
	return 0;
}

INT DataFlash_Erase(BYTE *dest, INT nSize)
{
	// following is fake code
	INT i;
	for (i = 0; i < nSize; i += k_nFlashPageSize)
		memset(dest + i, 0xff, k_nFlashPageSize);
	return 0;
}
