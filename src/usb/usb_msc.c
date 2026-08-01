#if 0
/*
 * Copyright (c) 2022, Stefan Odermatt
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
//#include "SpiFatFS.h"
#include "usb_msc.h"
#include "McuUtility.h"
#include "McuShell.h"
#include "tusb.h"
#include "McuLog.h"
#include "McuRTT.h"
//#include "diskio.h"
//#include "SpiFlashConfig.h"
//#include "../nvmc/littleFS/McuLittleFS.h"
//#include "../boardAPI/led.h"

// if file system is ejected or not
static bool ejected = true;
static bool copyFinished = false;

static TaskHandle_t MSCTaskHandle = NULL;
static SemaphoreHandle_t semaphore;

//Define how deep directories can be nested at max.
//Bigger number means more task stack will be needed
#define USBMSC_MAX_NOF_NESTED_DIRECTORIES (4)

//Define nof characters for absolute path name that is allowed at max
//E.g. "Year/Month/Day/Measurement..." is allowed at max this amount of characters
#define USBMSC_MAX_NOF_CHAR_ABSOLUTE_PATHNAME (60)

//Prototype
static uint8_t UsbMsc_CopyFS(const char *path);

/*!
 * \brief Whether usb MSC is conneted or not
 * \return true if MSC is not connected
 */
bool UsbMsc_IsEjected(void)
{
	return ejected;
}

/*!
 * \brief Whether copy of littlefs to FatFS has finished
 * \return true if copy has finished
 */
bool UsbMsc_CopyFinished(void)
{
	return copyFinished;
}

/*!
 * \brief Toggles connection of USB MSC
 * Used to Start USB data Transfer to Host PC via ejected flag
 * Only starts transfer if data was copied from littleFS
 */
void UsbMsc_ToggleTransferState(void)
{
	if(ejected)
	{
		//Start disk transfer
		if(copyFinished)
		{
		// Data is avaiable and can be read
		ejected = false;
		}
	} else
	{
		//Stop disk transfer
		ejected = true;
	}
}

/*!
 * \brief Starts copying littlefs filesystem to FatFS
 */
void UsbMsc_StartFSCopy(void)
{
	xSemaphoreGive(semaphore);
}

#if 0
/*!
 * \brief Used to read file content from a littlefs
 * Can only read 1kbyte Data from a file at a time
 * \return ERR_PARAM_SIZE if EOF was reached
 */
static uint8_t UsbMsc_ReadLfsFile(lfs_file_t* file, bool readFromBeginning,size_t* nofBytes, uint8_t * buffer)
{
	static int32_t filePos;
	size_t fileSize;

	if( *nofBytes > 1024) {
		*nofBytes = 1024;
	  }
	if(readFromBeginning) {
		lfs_file_rewind(McuLFS_GetFileSystem(),file);
		filePos = 0;
	} else {
		lfs_file_seek(McuLFS_GetFileSystem(),file, filePos,LFS_SEEK_SET);
	}

	fileSize = lfs_file_size(McuLFS_GetFileSystem(), file);
	filePos = lfs_file_tell(McuLFS_GetFileSystem(), file);
	fileSize = fileSize - filePos;

	if (fileSize < 0) {
		return ERR_FAILED;
	}

	if(fileSize > *nofBytes)  {
		if (lfs_file_read(McuLFS_GetFileSystem(), file, buffer, *nofBytes) < 0) {
	    	return ERR_FAILED;
	    	}
		filePos = filePos + *nofBytes;
		return ERR_OK;
	} else {
		if (lfs_file_read(McuLFS_GetFileSystem(), file, buffer, fileSize) < 0) {
			return ERR_FAILED;
		}
		*nofBytes = fileSize;
		filePos = filePos + fileSize;
		return ERR_PARAM_SIZE; //EOF
	}
}
#endif

#if 0
/*!
 * \brief copies a file from littlefs file format to FatFS file format
 */
static uint8_t UsbMsc_CopyFile(const char* FileName)
{
	//Start to copy File
	if (SpiFatFS_CreateFile((const uint8_t *)FileName, NULL) == ERR_FAILED)
	{
		McuLog_error("Failed creating new file while copying!\r\n");
		return ERR_FAILED;
	}
	FIL fat_fp;
	lfs_file_t lfs_fp;

	// Variables used for copying
	bool readFromBegining = true;
	//Nof Bytes read from littleFS file, start with 1kByte
	size_t nofBytesLfs = 1024;
	//Nof Bytes written to FatFS file
	UINT nofBytesFat = 0;
	uint8_t res = ERR_OK;
	uint8_t buffer[1024];


	if (SpiFatFS_open(&fat_fp, FileName, FA_WRITE) != FR_OK)
	{
		McuLog_error("Failed opening new FAT file while copying!\r\n");
		return ERR_FAILED;
	}

	// Open LittleFS file
	if (McuLFS_openFile(&lfs_fp, (uint8_t*)FileName) == ERR_FAILED)
	{
		McuLog_error("Failed opening littlefs file while copying!\r\n");
		return ERR_FAILED;
	}

	while(res != ERR_PARAM_SIZE)
	{
		res = UsbMsc_ReadLfsFile(&lfs_fp, readFromBegining, &nofBytesLfs, buffer);
		if(readFromBegining == true)
		{
			// Append data afterwards
			readFromBegining = false;
		}
		if(res == ERR_FAILED)
		{
			McuLog_error("Failed reading littlefs file while copying!\r\n");
			return ERR_FAILED;
		}

		if(SpiFatFS_write(&fat_fp, buffer, nofBytesLfs, &nofBytesFat) != FR_OK)
		{
			//Check if FatFS was full
			if(nofBytesFat < nofBytesLfs)
			{
				McuLog_error("FatFS Disk full while copying");
			} else
			{
				McuLog_error("Failed writing FatFS file while copying!\r\n");
			}
			return ERR_FAILED;
		}
	}

	if(SpiFatFS_close(&fat_fp) != FR_OK)
	{
		McuLog_error("Failed closing FatFS file while copying!\r\n");
		return ERR_FAILED;
	}

	if(McuLFS_closeFile(&lfs_fp) == ERR_FAILED)
	{
		McuLog_error("Failed closing littleFS file while copying!\r\n");
		return ERR_FAILED;
	}

	return ERR_OK;
}
#endif

#if 0
/*!
 * \brief used to calculate current numbers of nested directories
 * \return Number of nested directories
 */
static uint8_t UsbMsc_NofNestedDirectories(const char *path)
{
	uint8_t nofNested = 0;
	if(*path == '/')
	{
		//Current directory is made in root directory so we are not nested yet
		return nofNested;
	}

	while(*path != '\0')
	{
		if(*(path++) == '/')
		{
			nofNested++;
		}
	}
	return nofNested;
}
#endif

#if 0
/*!
 * \brief copy a directory from littlefs file format to a FatFS file format
 */
static uint8_t UsbMsc_CopyDirectory(const char *path,const char * DirectoryName)
{
	// FatFS used with relative Paths
	// Littlefs used with absolute Paths

	if(UsbMsc_NofNestedDirectories(path) + 1 > USBMSC_MAX_NOF_NESTED_DIRECTORIES)
	{
		// Not allowed to generate such deep nested directories reduce nof nested directories on littleFS
		// If more directories should be allowed Usbmsc task stack should be set higher else
		// taskmemory overflow could happen
		McuLog_error("Denied, reduce number of nested Directories on littleFS!\r\n");
		return ERR_FAILED;
	}

	if (SpiFatFS_MakeDirectory((const uint8_t *)DirectoryName, NULL) == ERR_FAILED)
	{
		McuLog_error("Failed creating new directory while copying!\r\n");
		return ERR_FAILED;
	}

	//Jump into FAT directory
	if(SpiFatFS_ChangeDirectory((const uint8_t*)DirectoryName, NULL) == ERR_FAILED)
	{
		McuLog_error("Failed cd to new directory while copying!\r\n");
		return ERR_FAILED;
	}

	uint8_t name[USBMSC_MAX_NOF_CHAR_ABSOLUTE_PATHNAME];
	//Make sure strcat appends from beginning of name
	name[0] = '\0';

	//Check if current directory is nested
	if(!(McuUtility_strcmp(path, "/") == 0))
	{
		//We are nested, so path needs to be concatenated
		McuUtility_strcat((uint8_t*)name, sizeof(name), (const uint8_t*)path);
		McuUtility_strcat((uint8_t*)name, sizeof(name), (const uint8_t*)"/");
		McuUtility_strcat((uint8_t*)name, sizeof(name), (const uint8_t*)DirectoryName);
	} else
	{
		McuUtility_strcat((uint8_t*)name, sizeof(name), (const uint8_t*)DirectoryName);
	}

	// Recursively copy filesystem
	if(UsbMsc_CopyFS((const char*)name) == ERR_FAILED)
	{
		McuLog_error("Failed to copy files and directory recursively!\r\n");
		return ERR_FAILED;
	}

	//Jump out of FAT directory
	if (SpiFatFS_ChangeDirectory((const uint8_t*)"..", NULL) == ERR_FAILED)
	{
		McuLog_error("Failed to get out of current directory while copying!\r\n");
		return ERR_FAILED;
	}

	return ERR_OK;
}
#endif

#if 0
/*!
 * \brief Copy entire littlefs filesystem to the FatFS filesystem
 * Only copies data until a defined number of nested directories
 */
static uint8_t UsbMsc_CopyFS(const char *path)
{
	int res;
	lfs_dir_t dir;
	struct lfs_info info;


	if(path == NULL)
	{
		/* default Path */
		path = "/";
	}

	if (!McuLFS_IsMounted())
	{
	    McuLog_error("File system is not mounted, mount it first.\r\n");
	    return ERR_FAILED;
	}

	res = lfs_dir_open(McuLFS_GetFileSystem(), &dir, path);

	if(res != LFS_ERR_OK)
	{
		McuLog_error("Failed lfs_dir_open()!\r\n");
		return ERR_FAILED;
	}

	for(;;)
	{
	    res = lfs_dir_read(McuLFS_GetFileSystem(), &dir, &info);

	    if (res < 0)
	    {
	    	McuLog_error("Failed lfs_dir_read()!\r\n");
	    	return ERR_FAILED;
	    }

	    if (res == 0)
	    {
	    	/* no more files */
	    	break;
	    }

	    //Skip . and .. file will be created automatically when making a directory
	    if(!(McuUtility_strcmp(info.name,".") == 0 || McuUtility_strcmp(info.name,"..") == 0 ))
	    {
	    	switch (info.type)
	    	{
	    	case LFS_TYPE_REG:

	    		// Found a file to copy
	    		if(UsbMsc_CopyFile(info.name) == ERR_FAILED)
	    		{
	    			McuLog_error("Failed copying File!\r\n");
	    			return ERR_FAILED;
	    		}

	    		break;
	    	case LFS_TYPE_DIR:

	    		// Found a directory to copy
	    		if(UsbMsc_CopyDirectory(path, info.name) == ERR_FAILED)
	    		{
	    			McuLog_error("Failed copying Directory!\r\n");
	    			return ERR_FAILED;
	    		}

	    		break;
	    	default:

	    		break;
	    	}
	    }

	} /* for */

	res = lfs_dir_close(McuLFS_GetFileSystem(), &dir);
	if (res != LFS_ERR_OK)
	{
		McuLog_error("Failed lfs_dir_close()!\r\n");
		return ERR_FAILED;
	}

	return ERR_OK;
}
#endif

#if 0
/*!
 * \brief FreeRTOS Mass Storage Class Task, is notified suspended until data from littlefs
 * needs to be copied to FatFS in order to be transfered to Host PC afterwards
 */
static void msc_task(void* params)
{
	BaseType_t res;
	bool lfs_wasMounted = false;
	bool fat_wasMounted = false;
	uint8_t resOfCopy = false;
	for(;;)
	{
		res = xSemaphoreTake(semaphore, portMAX_DELAY);
		if(res == pdTRUE) {
			//Check if USB is disconnected
			if(ejected)
			{
				//Start to copy data from littleFS to FatFS
				if(!SpiFatFS_IsMounted())
				{
					//Format FatFS
					if(SpiFatFS_FormatFileSystem((TCHAR *)SpiFatFS_CONFIG_DEFAULT_DRIVE_STRING, NULL) == ERR_FAILED)
					{
						McuLog_error("Formating failed for FatFS.\r\n");
						continue;
					}
					// Disk formated so there is no data available anymore
					copyFinished = false;

					// Mount FatFS
					if(SpiFatFS_MountFileSystem((unsigned char *)SpiFatFS_CONFIG_DEFAULT_DRIVE_STRING, NULL) == ERR_FAILED)
					{
						McuLog_error("Mounting failed for FatFS.\r\n");
						continue;
					}
				} else
				{
					fat_wasMounted = true;
				}

				if(!McuLFS_IsMounted())
				{
					// Mount littleFS
					if(McuLFS_Mount(NULL) == ERR_FAILED)
					{
						McuLog_error("Mounting failed for littleFS.\r\n");
						continue;
					}
				} else
				{
					lfs_wasMounted = true;
				}

				//Copy File system starting with default directory
				resOfCopy = UsbMsc_CopyFS(NULL);

				//Correctly unmount file systems
				if(!lfs_wasMounted)
				{
					// Unmount littleFS
					if(McuLFS_Unmount(NULL) == ERR_FAILED)
					{
						McuLog_error("Unmounting failed for littleFS.\r\n");
						continue;
					}
				} else
				{
					// Reset to default state
					lfs_wasMounted = false;
				}

				if(!fat_wasMounted)
				{
					// Unmount FatFS
					if(SpiFatFS_UnMountFileSystem((unsigned char *)SpiFatFS_CONFIG_DEFAULT_DRIVE_STRING, NULL) == ERR_FAILED)
					{
						McuLog_error("Unmounting failed for FatFS.\r\n");
						continue;
					}
				} else
				{
					// Reset to default state
					fat_wasMounted = false;
				}

			} else
			{
				McuLog_error("Eject USB first!\r\n");
				continue;
			}
			if(resOfCopy != ERR_FAILED)
			{
			// Copy was successful, USB can now transfer data
			copyFinished = true;
			resOfCopy = false;
			}
		}
	}
}
#endif

#if 0
//--------------------------------------------------------------------+
// Shell functions
//--------------------------------------------------------------------+
/*!
 * \brief run a benchmark, to test performance of filesystem copy procedure
 */
uint8_t UsbMsc_RunBenchmark(const McuShell_ConstStdIOType *io)
{
	lfs_file_t file;
	uint32_t i;
	TIMEREC time, startTime;
	int32_t start_mseconds, mseconds;

	if(McuLFS_IsMounted())
	{
		if(io != NULL)
		{
			McuShell_SendStr((const unsigned char*)"ERROR: LittleFS is mounted.\r\n", io->stdErr);
		}
		return ERR_FAILED;
	}

	if(SpiFatFS_IsMounted())
	{
		if(io != NULL)
		{
			McuShell_SendStr((const unsigned char*)"ERROR: FatFS is mounted.\r\n", io->stdErr);
		}
		return ERR_FAILED;
	}

	McuShell_SendStr((const unsigned char*)"Starting to format LittleFS\r\n", io->stdOut);
	if(McuLFS_Format(io) == ERR_FAILED)
	{
		return ERR_FAILED;
	}

	McuShell_SendStr((const unsigned char*)"Starting to mount LittleFS\r\n", io->stdOut);
	if(McuLFS_Mount(io) == ERR_FAILED)
	{
		return ERR_FAILED;
	}

	McuShell_SendStr((const unsigned char*)"Create benchmark file on LittleFS ... ", io->stdOut);

	if (lfs_file_open(McuLFS_GetFileSystem(), &file, "./bench.txt", LFS_O_WRONLY | LFS_O_CREAT)<0)
	{
		McuShell_SendStr((const unsigned char*)"*** Failed creating benchmark file!\r\n", io->stdErr);
	    return ERR_FAILED;
	}
	for(i=0;i<10240;i++)
	{
		if (lfs_file_write(McuLFS_GetFileSystem(), &file, "benchmark ", sizeof("benchmark ")-1)<0)
		{
			McuShell_SendStr((const unsigned char*)"*** Failed writing file!\r\n", io->stdErr);
			(void)lfs_file_close(McuLFS_GetFileSystem(), &file);
			return ERR_FAILED;
		}
	}
	(void)lfs_file_close(McuLFS_GetFileSystem(), &file);

	McuShell_SendStr((const unsigned char*)"done\r\n", io->stdOut);

	/* write benchmark */
	McuShell_SendStr((const unsigned char*)"Benchmark: copy a 100kB file from littleFS to FatFS\r\n", io->stdOut);
	//Start benchmark
	(void)McuTimeDate_GetTime(&startTime);

	//First thing that follows copy process is formatting of FatFS

	if(SpiFatFS_FormatFileSystem((TCHAR *)SpiFatFS_CONFIG_DEFAULT_DRIVE_STRING, NULL) == ERR_FAILED)
	{
		McuLog_error("ERROR: Formating failed for FatFS.\r\n");
		return ERR_FAILED;
	}

	// Mount FatFS
	if(SpiFatFS_MountFileSystem((unsigned char *)SpiFatFS_CONFIG_DEFAULT_DRIVE_STRING, NULL) == ERR_FAILED)
	{
		McuLog_error("ERROR: Mounting failed for FatFS.\r\n");
		return ERR_FAILED;
	}

	// Copy littleFS
	if(UsbMsc_CopyFS(NULL) == ERR_FAILED)
	{
		McuLog_error("ERROR: Copy LittleFS to FatFS failed.\r\n");
		return ERR_FAILED;
	}

	// Unmount littleFS
	if(McuLFS_Unmount(NULL) == ERR_FAILED)
	{
		McuLog_error("ERROR: unmounting failed for littleFS.\r\n");
		return ERR_FAILED;
	}

		// Unmount FatFS
	if(SpiFatFS_UnMountFileSystem((unsigned char *)SpiFatFS_CONFIG_DEFAULT_DRIVE_STRING, NULL) == ERR_FAILED)
	{
		McuLog_error("ERROR: unmounting failed for FatFS.\r\n");
		return ERR_FAILED;
	}

	(void)McuTimeDate_GetTime(&time);
	  start_mseconds = startTime.Hour*60*60*1000 + startTime.Min*60*1000 + startTime.Sec*1000
	#if TmDt1_HAS_SEC100_IN_TIMEREC
	  + startTime.Sec100*10
	#endif
	  ;
	  mseconds = time.Hour*60*60*1000 + time.Min*60*1000 + time.Sec*1000
	#if TmDt1_HAS_SEC100_IN_TIMEREC
	  + time.Sec100*10
	#endif
	  - start_mseconds;
	  McuShell_SendNum32s(mseconds, io->stdOut);
	  McuShell_SendStr((const unsigned char*)" ms for copy (", io->stdOut);
	  McuShell_SendNum32s((100*1000)/mseconds, io->stdOut);
	  McuShell_SendStr((const unsigned char*)" kB/s)\r\n", io->stdOut);
	  McuShell_SendStr((const unsigned char*)"done!\r\n", io->stdOut);

	return ERR_OK;
}
#endif

#if 0
uint8_t UsbMsc_PrintStatus(const McuShell_StdIOType *io)
{
	McuShell_SendStatusStr((const unsigned char*) "UsbMsc", (const unsigned char*) "UsbMsc status\r\n", io->stdOut);
	McuShell_SendStatusStr((const unsigned char*) " disk ejected", ejected ? (const uint8_t *)"yes\r\n" : (const uint8_t *)"no\r\n", io->stdOut);
	McuShell_SendStatusStr((const unsigned char*) " data ready", copyFinished ? (const uint8_t *)"yes\r\n" : (const uint8_t *)"no\r\n", io->stdOut);
	return ERR_OK;
}

uint8_t UsbMsc_PrintHelp(const McuShell_StdIOType *io)
{
	McuShell_SendHelpStr((unsigned char*) "UsbMsc", (const unsigned char*) "Group of USB mass storage class commands\r\n", io->stdOut);
	McuShell_SendHelpStr((unsigned char*) "  help|status",  (const unsigned char*) "Print help or status information\r\n",  io->stdOut);
	McuShell_SendHelpStr((unsigned char*) "  benchmark",(const unsigned char*) "Run a benchmark to measure performance\r\n",io->stdOut);
	return ERR_OK;
}

/*!
 * \brief Parse command table for shell
 */
uint8_t UsbMsc_ParseCommand(const unsigned char* cmd, bool *handled,const McuShell_StdIOType *io)
{
	if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP) == 0|| McuUtility_strcmp((char*)cmd, "UsbMsc help") == 0)
	{
		*handled = true;
		return UsbMsc_PrintHelp(io);
	} else if (McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS) == 0|| McuUtility_strcmp((char*)cmd, "UsbMsc status") == 0)
	{
		*handled = true;
		return UsbMsc_PrintStatus(io);
	} else if (McuUtility_strcmp((char*)cmd, "UsbMsc benchmark") == 0)
	{
		*handled = true;
		return UsbMsc_RunBenchmark(io);
	}

	return ERR_OK;
}
#endif

//--------------------------------------------------------------------+
// Application Callbacks
//--------------------------------------------------------------------+

// Invoked when received SCSI READ10 command
// - Address = lba * BLOCK_SIZE + offset
//   - offset is only needed if CFG_TUD_MSC_EP_BUFSIZE is smaller than BLOCK_SIZE.
//
// - Application fill the buffer (up to bufsize) with address contents and return number of read byte. If
//   - read < bufsize : These bytes are transferred first and callback invoked again for remaining data.
//
//   - read == 0      : Indicate application is not ready yet e.g disk I/O busy.
//                      Callback invoked again with the same parameters later on.
//
//   - read < 0       : Indicate application error e.g invalid address. This request will be STALLed
//                      and return failed status in command status wrapper phase.
int32_t tud_msc_read10_cb (uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize){
	(void) lun;
	#if 0
	//Forward read command to FatFS
	if(disk_read(0, buffer, lba, bufsize/4096) == RES_OK)
	{
		return bufsize;
	}
	#endif
	return -1;
}

// Invoked when received SCSI WRITE10 command
// - Address = lba * BLOCK_SIZE + offset
//   - offset is only needed if CFG_TUD_MSC_EP_BUFSIZE is smaller than BLOCK_SIZE.
//
// - Application write data from buffer to address contents (up to bufsize) and return number of written byte. If
//   - write < bufsize : callback invoked again with remaining data later on.
//
//   - write == 0      : Indicate application is not ready yet e.g disk I/O busy.
//                       Callback invoked again with the same parameters later on.
//
//   - write < 0       : Indicate application error e.g invalid address. This request will be STALLed
//                       and return failed status in command status wrapper phase.
//
int32_t tud_msc_write10_cb (uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
	(void) lun;
	//Forward write to FatFS
	#if 0
	if(disk_write(0, buffer, lba, bufsize/4096) == RES_OK)
	{
		return bufsize;
	}
	#endif
	return -1;
}

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
	(void) lun;

	const char vid[] = "TinyUSB";
	const char pid[] = "Mass Storage";
	const char rev[] = "1.0";

	memcpy(vendor_id  , vid, strlen(vid));
	memcpy(product_id , pid, strlen(pid));
	memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun) {
	(void) lun;

	// RAM disk is ready until ejected
	if (ejected) {
		// Additional Sense 3A-00 is NOT_FOUND
		tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
		return false;
	}

	return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size){
	(void) lun;
	#if 0
	*block_count = SPI_FLASH_BLOCK_COUNT;
	*block_size  = SPI_FLASH_BLOCK_SIZE;
	#else
		*block_count = 128;
		*block_size  = 4096;
	#endif
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
  (void) lun;
  (void) power_condition;

  if (load_eject)
  {
    if (start)
    {
      // load disk storage
    }else
    {
      // unload disk storage
      ejected = true;
    }
  }

  return true;
}

/**
 * Invoked when received an SCSI command not in built-in list below.
 * - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, TEST_UNIT_READY, START_STOP_UNIT, MODE_SENSE6, REQUEST_SENSE
 * - READ10 and WRITE10 has their own callbacks
 *
 * \param[in]   lun         Logical unit number
 * \param[in]   scsi_cmd    SCSI command contents which application must examine to response accordingly
 * \param[out]  buffer      Buffer for SCSI Data Stage.
 *                            - For INPUT: application must fill this with response.
 *                            - For OUTPUT it holds the Data from host
 * \param[in]   bufsize     Buffer's length.
 *
 * \return      Actual bytes processed, can be zero for no-data command.
 * \retval      negative    Indicate error e.g unsupported command, tinyusb will \b STALL the corresponding
 *                          endpoint and return failed status in command status wrapper phase.
 */
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
  (void) lun;
  // read10 & write10 has their own callback and MUST not be handled here

  void const* response = NULL;
  int32_t resplen = 0;

  // most scsi handled is input
  bool in_xfer = true;

  switch (scsi_cmd[0])
  {
    default:
      // Set Sense = Invalid Command Operation
      tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

      // negative means error -> tinyusb could stall and/or response with failed status
      resplen = -1;
    break;
  }

  // return resplen must not larger than bufsize
  if ( resplen > bufsize ) resplen = bufsize;

  if ( response && (resplen > 0) )
  {
    if(in_xfer)
    {
      memcpy(buffer, response, (size_t) resplen);
    }else
    {
      // SCSI output
    }
  }

  return (int32_t) resplen;
}

/*!
 * \brief module initialization
 */
void UsbMsc_Init(void) {
	BaseType_t res;

	#if 0
	res = xTaskCreate(msc_task, "msc", 16384/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+2, &MSCTaskHandle);

	if(res !=pdPASS)
	{
		/* Error */
		for(;;) {}
	}
	#endif

	semaphore = xSemaphoreCreateBinary();

	if(semaphore == NULL)
	{
		for(;;) { /* Not enought Memory ? */}
	}
	vQueueAddToRegistry(semaphore, "Usb msc Semaphore");
}

/*!
 * \brief module deinitialization
 */
void UsbMsc_Deinit(void){
	/* Nonething to do yet */
}


#endif