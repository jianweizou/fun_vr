

#include <stdio.h>
#include <string.h>
#include "avi.h"

#define _FOURCC(a, b, c, d)         (((d) << 24) | ((c) <<16) | ((b) << 8) | (a))

#define AVI_FCC_SAVI        _FOURCC('S', 'A', 'V', 'I')
#define AVI_FCC_RIFF        _FOURCC('R', 'I', 'F', 'F')
#define AVI_FCC_WAVE        _FOURCC('W', 'A', 'V', 'E')
#define AVI_FCC_FMT         _FOURCC('f', 'm', 't', ' ')
#define AVI_FCC_FACT        _FOURCC('f', 'a', 'c', 't')
#define AVI_FCC_DATA        _FOURCC('d', 'a', 't', 'a')
#define AVI_FCC_LIST        _FOURCC('L', 'I', 'S', 'T')


#define stringize(s)    #s
#define to_str(s)       stringize(s)


#define _assert(expression)  \
    ((void)((expression) ? 0 : printf("%s[%u] err '%s'\n", __func__, __LINE__, to_str(expression))))

#define FRAME_CNT_OFFSET	(0x30>>2)
#define AUDIO_EXIST_OFFSET	(0x38>>2)
#define FRAME_RATE_OFFSET	(0x84>>2)
#define JPEG_W_OFFSET		(0x40>>2)
#define JPEG_H_OFFSET		(0x44>>2)
#define ADU_LIST_OFFSET		(0x188>>2)
#define RAW_VID_OFFSET		(0x220>>2)


	
int
avi_parse_header(
    uint32_t        *pBuf,
    uint32_t        buf_size,
    avi_header_t   *pHeader)
{
	 int     rval = 0;

    _assert(pBuf != 0);
    _assert(buf_size >= 512);

    do {
        avi_header_t     *pHdr = (avi_header_t*)pBuf;

        if( pHdr->tag != AVI_FCC_RIFF )
        {
            rval = -1;
            break;
        }

		pHeader->tag = AVI_FCC_RIFF;
		pHeader->frame_max_num = *((uint32_t*)pHeader + FRAME_CNT_OFFSET);
		pHeader->frame_rate = *((uint32_t*)pHeader + FRAME_RATE_OFFSET);
		pHeader->audioexist = *((uint32_t*)pHeader + AUDIO_EXIST_OFFSET);
		pHeader->jpeg_width = *((uint32_t*)pHeader + JPEG_W_OFFSET);
		pHeader->jpeg_height = *((uint32_t*)pHeader + JPEG_H_OFFSET);
		pHeader->raw_data_offset_vid = *((uint32_t*)pHeader + RAW_VID_OFFSET);
		pHeader->adu_list_offset = *((uint32_t*)pHeader + ADU_LIST_OFFSET);
		pHeader->idx_table_offset_vid = 0x22C;
		pHeader->idx_table_size_vid = pHeader->frame_max_num * 8;
		pHeader->is_init_jpeg_header = 0;
        pHeader->frame_max_num -= 2;

    } while(0);

    return rval;	
}


int
avi_demux_video(
    avi_header_t       *pHdr,
    avi_demux_ctrl_t   *pCtrl_info)
{
	
}