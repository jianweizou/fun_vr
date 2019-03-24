
/*********************************************************************************
* sdcad_common.h
*
* Header file of  sccard common define
*
* History:
*    2016/04/21 - [Allen_Chang] created file
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.

*********************************************************************************/
#ifndef __REC_COMMON_H__
#define __REC_COMMON_H__

#include <record.h>
#include <usr_ctrl.h>
#include <core_common.h>
#include <error.h>
#include <ff.h>

/** @defgroup rec_common Record Common Function
 * @ingroup record_func
 * @brief common function for record
 * @{
 */


#define REC_PRINT(level, fmt, args...) printf(level, "[REC]"fmt,##args)


#define LEN_FULLPATH			256							    /**<the max length of absolute path */
#define LEN_FILENAME			128								/**<the max length of filename */

#define REC_FMT_H264			(0x1 << 1)	// 2				/**<the h264 format*/
#define REC_FMT_MJPEG			(0x1 << 2)						/*<the MJPE format*/

#define REC_ROOT          	    "1:sonix"	//vicky	"/sonix"              					/**<memory card root dir */
#define SD_ALARM_PATH       		"md"														/**<useless */
#define REC_RECORD_PATH      	  "record"                     		/**<memory card schedule record folder*/
#define REC_PROTECT_PATH      	"protect"                      	/**<memory card protected record folder*/
#define REC_PICTURE_PATH      	"picture"                     	/**<memory card picture folder*/
#define REC_THUMBNAIL_PATH      "thumbnail"                    	/**<memory card thumbnail folder*/
#define REC_TIMELAPSE_PATH      "timelapse"                   	/**<memory card timelpase recordl folder*/
#define SD_RECORD_FILELIST      "recordfilelist.txt"           	/**<memory card record file index.txt*/
#define SD_PROTECT_FILELIST     "protectfilelist.txt"          	/**<memory card protected record file index.txt*/
#define SD_PICTURE_FILELIST     "picturefilelist.txt"          	/**<memory card picture file index.txt*/
#define SD_THUMBNAIL_FILELIST  	"thumbnailfilelist.txt"        	/**<memory card thumbnail file index.txt*/
#define SD_TIMELAPSE_FILELIST  	"timelapsefilelist.txt"       	/**<memory card timelapse file index.txt*/
#define RECORD_SEED           	"seed"													/**<memory card seed file*/
#define RECORD_TIMESEED       	"timeseed"											/**<memery card timeseed file*/

#define SD_REC_SCHED		      0								/**<schedule record type*/
#define SD_REC_PROTECT		      1								/**<protect type*/
#define SD_REC_PICTURE		      2								/**<picture type*/
#define SD_THUMBNAIL		      3								/**<thumbnail type*/
#define SD_REC_TIMELAPSE		  4								/**<timelapse type*/

#define REC_SCHED_PRENAME       "record_"        				/**<schuedule pre_name for count base  */
#define REC_TIMELAPSE_PRENAME  	"timelapse_"     				/**<timelpase  pre_name for count base */
#define REC_PICTURE_PRENAME     "picture_"       				/**<picture  pre_name for count base*/
#define REC_PROTECT_PRENAME     "protect_"       				/**<protect pre_name for count base*/
#define REC_THUMBNAIL_PRENAME  	"thumbnail_"    				/**<thumnail pre_name for count base*/
#define MEDIA_TYPE_AVI          "avi"							/**<record file AVI format*/
#define MEDIA_TYPE_MP4          "mp4"							/**<record file MP$ format*/
#define COUNT_INDEX             "index"							/**<record file count for Dron config*/


#define SDCARDSIZESMALLLIMIT	(700 << 20)	             		/**< if less than this value , must alart format for app  unit: B  */
#define SD_MAX_RECORD_LENGTH	1800	         				/**< max record length   unit: seconds */

//#define SD_RECORD_UPBD          	       0.75
#define SD_RECORD_UPBD          0.55         					/**<record folder at what percentage */
#define SD_PROTECT_UPBD         0.1          					/**<protected  folder at what percentage */
#define SD_PICTURE_UPBD         0.1          					/**<picture  folder at what percentage */
#define SD_THUMBNAIL_UPBD       0.05        					/**<thumbnail  folder at what percentage */
#define SD_TIMELAPSE_UPBD       0.20          					/**<timelapse  folder at what percentage */


#define SD_RECORD_RESERVED     	400 << 20         				/**<reserver size for record , if less than this value , cycling or stop record  unit: B*/
#define SD_PROTECT_RESERVED     100 << 20         				/**<reserver size for protected record , if less than this value , cycling or stop record unit: B */
#define SD_PICTURE_RESERVED    	10 << 20         				/**<reserver size for picture , if less than this value , stop picture  unit: B*/
#define SD_THUMBNAIL_RESERVED  	1 << 20        					/**<reserver size for thumbnail , if less than this value , stop thumbnail  unit: B*/
#define SD_TIMELAPSE_RESERVED 	100 << 20         				/**<reserver size for timelpase record , if less than this value , cycling or stop record  unit: B*/

#define Frame_PER_SEC                 30						/**<fps */
#define MAXFILESIZE     3900      //mb							/**<max file size */
void saferFree(void **pp);
#define safeFree(p) saferFree((void**)&(p));

#define NVRAM_SPACE       	  	"SPACE"							/**<  NVRAM config name */
#define NVRAM_RECORD_UPBD      	"RECORDUPBDSPACE"				/**<  NVRAM config name */
#define NVRAM_PROTECT_UPBD     	"PROTECTUPBDSPACE"				/**<  NVRAM config name */
#define NVRAM_PICTURE_UPBD   	"PICTUREUPBDSPACE"				/**<  NVRAM config name */
#define NVRAM_THUMBNAIL_UPBD   	"THUMBNAILUPBDSPACE"			/**<  NVRAM config name */
#define NVRAM_TIMELAPSE_UPBD   	"TIMELAPSEUPBDSPACE"			/**<  NVRAM config name */
//55 10 10 5 20

#define RECORD_SCHED_ACTIVE     0x80 																/**< record stae */ 
#define RECORD_LAPSE_ACTIVE     0x40																/**< record state */
#define RECORD_START_RUNNING    0x20 // for controling wb mem to be released/created				/**< record state */
#define RECORD_RESERVE2         0x01																/**< reserve */						

#define MAX_PRE_TIME            1

#define NVRAM_PATH_INFO			"SD_PATH_INFO"					/**<  NVRAM config name */
#define NVRAM_SD_ROOT			"SD_ROOT_DIR"					/**<  NVRAM config name */
#define NVRAM_SD_RECORD			"SD_RECORD_DIR"					/**<  NVRAM config name */
#define NVRAM_SD_PROTECT		"SD_PROTECT_DIR"				/**<  NVRAM config name */
#define NVRAM_SD_PICTURE		"SD_PICTURE_DIR"				/**<  NVRAM config name */
#define NVRAM_SD_THUMBNAIL		"SD_THUMBNAIL_DIR"				/**<  NVRAM config name */
#define NVRAM_SD_TIMELAPSE		"SD_TIMELAPSE_DIR"				/**<  NVRAM config name */


#define NVRAM_RECORD       	                   "RECORD"			/**<  NVRAM config name */

#define NVRAM_RECORD_TIME_0_BEG_HOUR           "RECORDTIME0BEGHOUR"			/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_0_BEG_MINUTE         "RECORDTIME0BEGMINUTE"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_0_BEG_SECOND         "RECORDTIME0BEGSECOND"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_0_DURATION           "RECORDTIME0DURATION"		/**<  NVRAM config name */

#define NVRAM_RECORD_TIME_1_BEG_HOUR           "RECORDTIME1BEGHOUR"			/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_1_BEG_MINUTE         "RECORDTIME1BEGMINUTE"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_1_BEG_SECOND         "RECORDTIME1BEGSECOND"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_1_DURATION           "RECORDTIME1DURATION"		/**<  NVRAM config name */

#define NVRAM_RECORD_TIME_2_BEG_HOUR           "RECORDTIME2BEGHOUR"			/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_2_BEG_MINUTE         "RECORDTIME2BEGMINUTE"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_2_BEG_SECOND         "RECORDTIME2BEGSECOND"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_2_DURATION           "RECORDTIME2DURATION"		/**<  NVRAM config name */

#define NVRAM_RECORD_TIME_3_BEG_HOUR           "RECORDTIME3BEGHOUR"			/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_3_BEG_MINUTE         "RECORDTIME3BEGMINUTE"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_3_BEG_SECOND         "RECORDTIME3BEGSECOND"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_3_DURATION           "RECORDTIME3DURATION"		/**<  NVRAM config name */

#define NVRAM_RECORD_TIME_4_BEG_HOUR           "RECORDTIME4BEGHOUR"			/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_4_BEG_MINUTE         "RECORDTIME4BEGMINUTE"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_4_BEG_SECOND         "RECORDTIME4BEGSECOND"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_4_DURATION           "RECORDTIME4DURATION"		/**<  NVRAM config name */

#define NVRAM_RECORD_TIME_5_BEG_HOUR           "RECORDTIME5BEGHOUR"			/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_5_BEG_MINUTE         "RECORDTIME5BEGMINUTE"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_5_BEG_SECOND         "RECORDTIME5BEGSECOND"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_5_DURATION           "RECORDTIME5DURATION"		/**<  NVRAM config name */

#define NVRAM_RECORD_TIME_6_BEG_HOUR           "RECORDTIME6BEGHOUR"			/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_6_BEG_MINUTE         "RECORDTIME6BEGMINUTE"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_6_BEG_SECOND         "RECORDTIME6BEGSECOND"		/**<  NVRAM config name */
#define NVRAM_RECORD_TIME_6_DURATION           "RECORDTIME6DURATION"		/**<  NVRAM config name */

#define NVRAM_RECORD_SCHED_INTERVAL            "RECORDSCHEDINTERVAL"       	/**<  NVRAM config name */
#define NVRAM_RECORD_MD_INTERVAL               "RECORDMDINTERVAL"			/**<  NVRAM config name */

#define NVRAM_RECORD_SCHED_ENABLE              "RECORDSCHEDENABLE"          /**<  NVRAM config name */
#define NVRAM_RECORD_MD_ENABLE                 "RECORDMDENABLE"				/**<  NVRAM config name */

#define NVRAM_RECORD_SCHED_CYCLE               "RECORDSCHEDCYCLE"           /**<  NVRAM config name */

#define NVRAM_TIMELAPSE      	               "TIMELAPSE"					/**<  NVRAM config name */
#define NVRAM_TIMELAPSE_ENABLE                 "TIMELAPSEENABLE"            /**< timepase record enable 0:diable 1:enable*/
#define NVRAM_TIMELAPSE_CYCLE                  "TIMELAPSECYCLE"             /**< timepase record cycle is or not  0:diable 1:enable*/
#define NVRAM_TIMELAPSE_INTERVAL               "TIMELAPSEINTERVAL"          /**< timepase record file length*/
#define NVRAM_TIMELAPSE_IFRAME_INTERVAL        "TIMELAPSEIFRAMEINTERVAL"    /**< timepase record i-frame interval */

#define NVRAM_RECORD_FILE_FORMAT               "RECORDFILEFORMAT"           /**<0:timebase  1:countbase */
#define NVRAM_PROTECTRECORD_LENGTH             "PROTECTFILELENGTH"          /**<protect file length  */
#define NVRAM_RECORD_TYPE                      "RECORDTYPE"                 /**< 0:avi 1:mp4 2:stream file */

/**
* @struct DayTime
* @brief structure for time info
*/
struct DayTime {
	int hour;
	int minute;
	int second;
	int duration;
};

/**
* @struct SchedTime
* @brief structure for all status about schedule record
*/
struct SchedTime {
	struct DayTime day[7];
	int sd_alarm_record_en;
	int sd_sched_record_en;          /**<schuedule record enable 0:diable 1:enable*/
	int sd_sched_internal_en;  	     /**<schuedule internal enable 0:diable 1:enable for playback/preview change  */
	int sd_sched_record_cycle;       /**< schuedule record cycle is or not  0:diable 1:enable*/
	int sd_sched_record_interval;    /**< schuedule record file length*/
	int sd_alarm_record_interval;
};

typedef struct SchedTime SchedTime_t;

/**
* @struct TimeLapseTime
* @brief structure for all status about timelapse
*/
struct TimeLapseTime {
	int sd_timelapse_record_en;  	   /**< timepase record enable 0:diable 1:enable*/
	int sd_timelapse_record_cycle;     /**< timepase record cycle is or not  0:diable 1:enable*/
	int sd_timelapse_record_interval;  /**< timepase record file length*/
	int sd_timelapse_iframe_interval;  /**< timepase record i-frame interval */
};


typedef struct TimeLapseTime TimeLapseTime_t;

/**
* @brief enum for create filelist state
*/
enum {
	LIST_FILE_OK 			= 0,
	LIST_FILE_IS_NONE 		= 1,
	LIST_FILE_OPEN_FAIL 	= 2,
	LIST_FILE_TYPE_ERROR 	= 3,
	LIST_FILE_DOWNLOAD 		= 4,
	LIST_FILE_CONFIG_ERR	= 5,
	LIST_FILE_UNKNOWN		= 6
};



/**
* @brief enum for record type
*/
enum RECORDKIND {
	SCHED_RECORD,
	TIMELAPSE_RECORD,
	PROTECT_RECORD,
	SNAPSHOT_RECORD
};


/* App query media items definition */
#define QUERY_RECORD_FILE 				0			/**< query file type */
#define QUERY_PROTECT_FILE				1			/**< query file type */
#define QUERY_PIC_FILE					2			/**< query file type */
#define QUERY_TIMELAPSE_FILE			3			/**< query file type */
#define QUERY_RECORD_THUMBNAIL_FILE		4			/**< query file type */
#define QUERY_PROTECT_THUMBNAIL_FILE	5			/**< query file type */
#define QUERY_TIMELAPSE_THUMBNAIL_FILE	6			/**< query file type */
#define QUERY_FILELIST					7			/**< query file type */



/**
* @struct chk_card_file_info_t
* @brief structure for check card task info
*/
typedef struct _chk_card_file_info{
	int task_state;
	xQueueHandle chk_card_queue;
	xTaskHandle chk_card_task;
}chk_card_file_info_t;
#define IMPORT_CARD_INACTIVE 0			/**< check card task state*/
#define IMPORT_CARD_ACTIVE 1			/**< check card task state*/
#define IMPORT_CARD_FINISHED 2			/**< check card task state*/



void rec_filemanage_init(void);
void rec_filemanage_uninit(void);
int rec_fileformat_get(void);
void rec_thumbnail_del(enum FILE_TYPE type, const char *filename);
int rec_thumbnail_add(enum FILE_TYPE type, const char *filename);
void rec_finishedfile_addlen(enum FILE_TYPE type, char *finishedfile, char *renameoffile);
void rec_snapshot_query(enum RECORDKIND type, const char *name);
void rec_size_del(const char *filename, long long *size);
int rec_recordtype_get(unsigned int type);
int rec_filesize_get(const char *name);
void rec_filenode_init(enum FILE_TYPE type, bool bUseThumbnail, void *cb);
void rec_filenode_update(enum FILE_TYPE type);
int rec_filenode_add(enum FILE_TYPE type, const char *filename, long long *used_size);
int rec_filenode_del(enum FILE_TYPE type, const char *filename);
int rec_import_files(enum FILE_TYPE type);
void rec_filelist_clear(enum FILE_TYPE type);
char ** rec_filelist_CreateBuf(enum FILE_TYPE type, int *listsize);
void rec_filelist_ReleaseBuf(enum FILE_TYPE type, char **listbuffer, int listsize);
int rec_filelist_Createfile(enum FILE_TYPE type, const char *target_path, int len);
void rename_recfile_addlen(const char *oldname, char *newname, int filelen);
int chkdir_writetofile(int type, char *path);
int rec_query_abspath(int filetype, char *output, int output_size, const char *filename);
int get_usr_config(struct usr_config **pusr_config);

#endif
/** @} */
