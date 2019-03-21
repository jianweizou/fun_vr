#ifndef __TYPES_H__
#define __TYPES_H__

typedef int						INT;
typedef short 					INT16;
typedef int 					INT32;
typedef long 					LONG;

typedef unsigned char 			UINT8;
typedef unsigned short 			UINT16;
typedef unsigned int 			UINT32;
//typedef long long 			INT64;
typedef signed char 			CHAR;
typedef unsigned char 			UCHAR;
typedef unsigned short 			USHORT;
typedef unsigned int 			UINT;
typedef unsigned long 			ULONG;
typedef short					SHORT;

typedef unsigned char *			PUINT8;
typedef unsigned short *		PUINT16;
typedef unsigned int *			PUINT32;
typedef unsigned long long *	PUINT64;
typedef int *PINT32;
//typedef long long *			PINT64;
//typedef char 					STRING;
typedef char *					PSTRING;
typedef void *					PVOID;
typedef CHAR *					PCHAR;
typedef UCHAR *					PUCHAR;
typedef USHORT *				PUSHORT;
typedef LONG *					PLONG;
typedef ULONG *					PULONG;
typedef UINT *					PUINT;
typedef unsigned long long 		UINT64;


//#define __inline				static inline
#define IN
#define OUT
#define INOUT

#define NDIS_STATUS				INT
#define NTSTATUS				INT
#define BOOLEAN					UCHAR
#define PIRP					PVOID
#define PURB					PVOID
#define ra_dma_addr_t			UINT32
#define PTX_BUFFER				PUCHAR
#define	PNDIS_PACKET			PUCHAR
#define	NDIS_PHYSICAL_ADDRESS	ULONG
#define VOID 					void
#define U32 					UINT
#define U16 					USHORT
#define U8  					UCHAR
#define STATIC 					static

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#endif /* __TYPES_H__ */
