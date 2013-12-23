/*
 * Project Name JPEG DRIVER IN WINCE
 * Copyright  2007 Samsung Electronics Co, Ltd. All Rights Reserved. 
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").   
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics 
 *
 * This file implements JPEG Test Application.
 *
 * @name JPEG Test Application Module (jpg_app.c)
 * @author Jiyoung Shin (idon.shin@samsung.com)
 * @date 28-05-07
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>

#include "JPGApi.h"
#include "performance.h"


//#define DEBUG		1

#define TEST_DECODE					0  // enable decoder test
#define TEST_DECODE_OUTPUT_YUV422	0  // output file format is YUV422(non-interleaved)
#define TEST_DECODE_OUTPUT_YCBYCR	0  // output file format is YCBYCR(interleaved)

#define TEST_ENCODE					1 // enable encoder test
#define TEST_ENCODE_WITH_EXIF		1 // encoded jpg file will include Exif info
#define TEST_ENCODE_WITH_THUMBNAIL	0 // enable thumbnail encoding


#if (TEST_DECODE == 1)
	#define CTRL_FILE_NAME	"fname_dec.txt"
#elif (TEST_ENCODE == 1)
	#define CTRL_FILE_NAME	"fname_enc.txt"
#endif


void TestDecoder();
void TestEncoder();
void DecodeFileOutYCBYCR(char *OutBuf, UINT32 streamSize, char *filename);
void DecodeFileOutYUV422(char *OutBuf, UINT32 streamSize, char *filename);
void makeExifParam(ExifFileInfo *exifFileInfo);
void printD(char* fmt, ...);

/*
*******************************************************************************
Name            : main
Description     : Main function
Parameter       :
Return Value    : SUCCESS or FAILURE
*******************************************************************************
*/

int main()
{
#if (TEST_DECODE == 1)
	TestDecoder();
#elif (TEST_ENCODE == 1)
	TestEncoder();
#endif

	return 1;
}

/*
*******************************************************************************
Name            : TestDecoder
Description     : To test Decoder
Parameter       : imageType - JPG_YCBYCR or JPG_RGB16
Return Value    : void
*******************************************************************************
*/
void TestDecoder()
{
	char 	*InBuf = NULL;
	char 	*OutBuf = NULL;
	FILE 	*fp;
	FILE 	*CTRfp;
	UINT32 	fileSize;
	long 	streamSize;
	int		handle;
	INT32 	width, height, samplemode;
	JPEG_ERRORTYPE ret;
	char 	outFilename[128];
	char 	inFilename[128];
	BOOL	result = TRUE;
#ifdef FPS
	struct timeval start;
	struct timeval stop;
	unsigned int	time = 0;
#endif


	printf("------------------------Decoder Test Start ---------------------\n");

	//////////////////////////////////////////////////////////////
	// 0. Get input/output file name                            //
	//////////////////////////////////////////////////////////////
	CTRfp = fopen(CTRL_FILE_NAME, "rb");
	if(CTRfp == NULL){
		printf("file open error : %s\n", CTRL_FILE_NAME);
		return;
	}

	mkdir("./testVectors/testOut", 0644);

	do{
		memset(outFilename, 0x00, sizeof(outFilename));
		memset(inFilename, 0x00, sizeof(inFilename));

		fscanf(CTRfp, "%s", inFilename);

		if(inFilename[0] == '#'){
			printf("------------------------Decoder Test Done ---------------------\n");
			fclose(CTRfp);
			return;
		}

		fscanf(CTRfp, "%s", outFilename);

		if(inFilename == NULL || outFilename == NULL){
			printf("read file error\n");
			printf("------------------------Decoder Test Done ---------------------\n");
			fclose(CTRfp);
			return;
		}
	
		printf("inFilename : %s \noutFilename : %s\n", inFilename, outFilename);
		//////////////////////////////////////////////////////////////
		// 1. handle Init                                           //
		//////////////////////////////////////////////////////////////
		#ifdef FPS
			gettimeofday(&start, NULL);
		#endif
		
		handle = SsbSipJPEGDecodeInit();
		if(handle < 0)
			break;

		#ifdef FPS
			gettimeofday(&stop, NULL);
			time += measureTime(&start, &stop);
		#endif

		//////////////////////////////////////////////////////////////
		// 2. open JPEG file to decode                              //
		//////////////////////////////////////////////////////////////
		fp = fopen(inFilename, "rb");
		if(fp == NULL){
			result = FALSE;
			printf("file open error : %s\n", inFilename);
			break;
		}
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		printD("filesize : %d\n", fileSize);

		//////////////////////////////////////////////////////////////
		// 3. get Input buffer address                              //
		//////////////////////////////////////////////////////////////
		InBuf = SsbSipJPEGGetDecodeInBuf(handle, fileSize);
		if(InBuf == NULL){
			printf("Input buffer is NULL\n");
			result = FALSE;
			break;
		}
		printD("inBuf : 0x%x\n", InBuf);

		//////////////////////////////////////////////////////////////
		// 4. put JPEG frame to Input buffer                        //
		//////////////////////////////////////////////////////////////
		fread(InBuf, 1, fileSize, fp);
		fclose(fp);


		//////////////////////////////////////////////////////////////
		// 5. Decode JPEG frame                                     //
		//////////////////////////////////////////////////////////////
		#ifdef FPS
			gettimeofday(&start, NULL);
		#endif
		
		ret = SsbSipJPEGDecodeExe(handle);
		
		#ifdef FPS
			gettimeofday(&stop, NULL);
			time += measureTime(&start, &stop);
			printf("[JPEG Decoding Performance] Elapsed time : %u\n", time);
			time = 0;
		#endif

		if(ret != JPEG_OK){
			printf("Decoding failed\n");
			result = FALSE;
			break;
		}
		
		//////////////////////////////////////////////////////////////
		// 6. get Output buffer address                             //
		//////////////////////////////////////////////////////////////
		OutBuf = SsbSipJPEGGetDecodeOutBuf(handle, &streamSize);
		if(OutBuf == NULL){
			printf("Output buffer is NULL\n");
			result = FALSE;
			break;
		}
		printD("OutBuf : 0x%x streamsize : %d\n", OutBuf, streamSize);

		//////////////////////////////////////////////////////////////
		// 7. get decode config.                                    //
		//////////////////////////////////////////////////////////////
		SsbSipJPEGGetConfig(JPEG_GET_DECODE_WIDTH, &width);
		SsbSipJPEGGetConfig(JPEG_GET_DECODE_HEIGHT, &height);
		SsbSipJPEGGetConfig(JPEG_GET_SAMPING_MODE, &samplemode);

		printD("width : %d height : %d samplemode : %d\n", width, height, samplemode);
  
		//////////////////////////////////////////////////////////////
		// 8. wirte output file & dispaly to LCD                    //
		//////////////////////////////////////////////////////////////
	#if (TEST_DECODE_OUTPUT_YCBYCR == 1)
		DecodeFileOutYCBYCR(OutBuf, streamSize, outFilename); // YCBYCR interleaved
	#elif (TEST_DECODE_OUTPUT_YUV422 == 1)
		DecodeFileOutYUV422(OutBuf, streamSize, outFilename);  // yuv422 non-interleaved
	#endif
		//////////////////////////////////////////////////////////////
		// 9. finalize handle                                      //
		//////////////////////////////////////////////////////////////
		SsbSipJPEGDecodeDeInit(handle);
		sleep(1);
	}while(1);

	if(result == FALSE){
		SsbSipJPEGDecodeDeInit(handle);
	}
	
	fclose(CTRfp);
	printf("------------------------Decoder Test Done ---------------------\n");
}

/*
*******************************************************************************
Name            : TestEncoder
Description     : To test Encoder
Parameter       : imageType - JPG_YCBYCR or JPG_RGB16
Return Value    : void
*******************************************************************************
*/
void TestEncoder()
{
	char 			*InBuf = NULL;
	char 			*OutBuf = NULL;
	FILE 			*fp;
	FILE 			*CTRfp;
	JPEG_ERRORTYPE 	ret;
	UINT32 			fileSize;
	long 			frameSize;
	int				handle;
	ExifFileInfo 	ExifInfo;
	char 			outFilename[128];
	char 			inFilename[128];
	char 			widthstr[8], heightstr[8];
	INT32 			width, height;
	BOOL			result = TRUE;
#ifdef FPS
	struct timeval start;
	struct timeval stop;
	unsigned int	time = 0;
#endif


	printf("------------------------Encoder Test Start---------------------\n");
	//////////////////////////////////////////////////////////////
	// 0. Get input/output file name                            //
	//////////////////////////////////////////////////////////////
	CTRfp = fopen(CTRL_FILE_NAME, "rb");
	if(CTRfp == NULL){
		printf("file open error : %s\n", CTRL_FILE_NAME);
		return;
	}

	mkdir("./testVectors/testOut", 0644);

	do{
		memset(outFilename, 0x00, sizeof(outFilename));
		memset(inFilename, 0x00, sizeof(inFilename));
		memset(widthstr, 0x00, sizeof(widthstr));
		memset(heightstr, 0x00, sizeof(heightstr));

		fscanf(CTRfp, "%s", inFilename);
		if(inFilename[0] == '#'){
			printf("------------------------Encoder Test Done---------------------\n");
			fclose(CTRfp);
			return;
		}

		fscanf(CTRfp, "%s", outFilename);
		fscanf(CTRfp, "%s", widthstr);
		fscanf(CTRfp, "%s", heightstr);
		width = (INT32)atoi(widthstr);
		height = (INT32)atoi(heightstr);

		if(inFilename == NULL || outFilename == NULL){
			printf("read file error\n");
			printf("------------------------Encoder Test Done---------------------\n");
			fclose(CTRfp);
			return;
		}

		printf("inFilename : %s \noutFilename : %s width : %d height : %d\n", 
				inFilename, outFilename, width, height);
		//////////////////////////////////////////////////////////////
		// 1. handle Init                                           //
		//////////////////////////////////////////////////////////////
		#ifdef FPS
			gettimeofday(&start, NULL);
		#endif
		
		handle = SsbSipJPEGEncodeInit();

		#ifdef FPS
			gettimeofday(&stop, NULL);
			time += measureTime(&start, &stop);
		#endif
		
		if(handle < 0)
			break;

		//////////////////////////////////////////////////////////////
		// 2. set decode config.                                    //
		//////////////////////////////////////////////////////////////
		if((ret = SsbSipJPEGSetConfig(JPEG_SET_SAMPING_MODE, JPG_422)) != JPEG_OK){
			result = FALSE;
			break;
		}
		if((ret = SsbSipJPEGSetConfig(JPEG_SET_ENCODE_WIDTH, width)) != JPEG_OK){
			result = FALSE;
			break;
		}
		if((ret = SsbSipJPEGSetConfig(JPEG_SET_ENCODE_HEIGHT, height)) != JPEG_OK){
			result = FALSE;
			break;
		}
		if((ret = SsbSipJPEGSetConfig(JPEG_SET_ENCODE_QUALITY, JPG_QUALITY_LEVEL_2)) != JPEG_OK){
			result = FALSE;
			break;
		}
#if (TEST_ENCODE_WITH_THUMBNAIL == 1)
		if((ret = SsbSipJPEGSetConfig(JPEG_SET_ENCODE_THUMBNAIL, TRUE)) != JPEG_OK){
			result = FALSE;
			break;
		}
		if((ret = SsbSipJPEGSetConfig(JPEG_SET_THUMBNAIL_WIDTH, 160)) != JPEG_OK){
			result = FALSE;
			break;
		}
		if((ret = SsbSipJPEGSetConfig(JPEG_SET_THUMBNAIL_HEIGHT, 120)) != JPEG_OK){
			result = FALSE;
			break;
		}
#endif

		//////////////////////////////////////////////////////////////
		// 3. open JPEG file to decode                              //
		//////////////////////////////////////////////////////////////
		fp = fopen(inFilename, "rb");
		if(fp == NULL){
			printf("file open error : %s\n", inFilename);
			result = FALSE;
			break;
		}
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		//////////////////////////////////////////////////////////////
		// 4. get Input buffer address                              //
		//////////////////////////////////////////////////////////////
		printD("filesize : %d\n", fileSize);
		InBuf = SsbSipJPEGGetEncodeInBuf(handle, fileSize);
		if(InBuf == NULL){
			result = FALSE;
			break;
		}
		printD("inBuf : 0x%x\n", InBuf);

		//////////////////////////////////////////////////////////////
		// 5. put YUV stream to Input buffer                        //
		//////////////////////////////////////////////////////////////
		fread(InBuf, 1, fileSize, fp);
		fclose(fp);

		//////////////////////////////////////////////////////////////
		// 6. Make Exif info parameters                             //
		//////////////////////////////////////////////////////////////
		memset(&ExifInfo, 0x00, sizeof(ExifFileInfo));
		makeExifParam(&ExifInfo);

		//////////////////////////////////////////////////////////////
		// 7. Encode YUV stream                                     //
		//////////////////////////////////////////////////////////////
		#ifdef FPS
			gettimeofday(&start, NULL);
		#endif

		#if (TEST_ENCODE_WITH_EXIF == 1)
		ret = SsbSipJPEGEncodeExe(handle, &ExifInfo, JPEG_USE_HW_SCALER);    //with Exif
		#else
		ret = SsbSipJPEGEncodeExe(handle, NULL, 0); 		//No Exif
		#endif

		#ifdef FPS
			gettimeofday(&stop, NULL);
			time += measureTime(&start, &stop);
			printf("[JPEG Encoding Performance] Elapsed time : %u\n", time);
			time = 0;
		#endif

		if(ret != JPEG_OK){
			result = FALSE;
			break;
		}

		//////////////////////////////////////////////////////////////
		// 8. get output buffer address                             //
		//////////////////////////////////////////////////////////////
		OutBuf = SsbSipJPEGGetEncodeOutBuf(handle, &frameSize);
		if(OutBuf == NULL){
			result = FALSE;
			break;
		}
		
		printD("OutBuf : 0x%x freamsize : %d\n", OutBuf, frameSize);

		//////////////////////////////////////////////////////////////
		// 9. write JPEG result file                                //
		//////////////////////////////////////////////////////////////
		
		fp = fopen(outFilename, "wb");
		if(fp == NULL) {
			printf("output file open error\n");
			break;
		}
	
		fwrite(OutBuf, 1, frameSize, fp);
		fclose(fp);
		
		//////////////////////////////////////////////////////////////
		// 10. finalize handle                                      //
		//////////////////////////////////////////////////////////////
		SsbSipJPEGEncodeDeInit(handle);
		
		sleep(1);
	}while(1);

	if(result == FALSE){
		SsbSipJPEGEncodeDeInit(handle);
	}
	fclose(CTRfp);
	printf("------------------------Encoder Test Done---------------------\n");
}

/*
*******************************************************************************
Name            : makeExifParam
Description     : To make exif input parameter
Parameter       : 
Return Value    : exifFileInfo - exif input parameter
*******************************************************************************
*/
void makeExifParam(ExifFileInfo *exifFileInfo)
{
	strcpy(exifFileInfo->Make,"Samsung SYS.LSI make");;
	strcpy(exifFileInfo->Model,"Samsung 2007 model");
	strcpy(exifFileInfo->Version,"version 1.0.2.0");
	strcpy(exifFileInfo->DateTime,"2007:05:16 12:32:54");
	strcpy(exifFileInfo->CopyRight,"Samsung Electronics@2007:All rights reserved");

	exifFileInfo->Height					= 320;
	exifFileInfo->Width						= 240;
	exifFileInfo->Orientation				= 1; // top-left
	exifFileInfo->ColorSpace				= 1;
	exifFileInfo->Process					= 1;
	exifFileInfo->Flash						= 0;
	exifFileInfo->FocalLengthNum			= 1;
	exifFileInfo->FocalLengthDen			= 4;
	exifFileInfo->ExposureTimeNum			= 1;
	exifFileInfo->ExposureTimeDen			= 20;
	exifFileInfo->FNumberNum				= 1;
	exifFileInfo->FNumberDen				= 35;
	exifFileInfo->ApertureFNumber			= 1;
	exifFileInfo->SubjectDistanceNum		= -20;
	exifFileInfo->SubjectDistanceDen		= -7;
	exifFileInfo->CCDWidth					= 1;
	exifFileInfo->ExposureBiasNum			= -16;
	exifFileInfo->ExposureBiasDen			= -2;
	exifFileInfo->WhiteBalance				= 6;
	exifFileInfo->MeteringMode				= 3;
	exifFileInfo->ExposureProgram			= 1;
	exifFileInfo->ISOSpeedRatings[0]		= 1;
	exifFileInfo->ISOSpeedRatings[1]		= 2;
	exifFileInfo->FocalPlaneXResolutionNum	= 65;
	exifFileInfo->FocalPlaneXResolutionDen	= 66;
	exifFileInfo->FocalPlaneYResolutionNum	= 70;
	exifFileInfo->FocalPlaneYResolutionDen	= 71;
	exifFileInfo->FocalPlaneResolutionUnit	= 3;
	exifFileInfo->XResolutionNum			= 48;
	exifFileInfo->XResolutionDen			= 20;
	exifFileInfo->YResolutionNum			= 48;
	exifFileInfo->YResolutionDen			= 20;
	exifFileInfo->RUnit						= 2;
	exifFileInfo->BrightnessNum				= -7;
	exifFileInfo->BrightnessDen				= 1;

	strcpy(exifFileInfo->UserComments,"Usercomments");
}
/*
*******************************************************************************
Name            : DecodeFileOutYUV422
Description     : To change YCBYCR to YUV422, and write result file.
Parameter       :
Return Value    : void
*******************************************************************************
*/
void DecodeFileOutYUV422(char *OutBuf, UINT32 streamSize, char *filename)
{
	UINT32	i;
	UINT32  indexY, indexCB, indexCR;
	char *Buf;
	FILE *fp;

	Buf = (char *)malloc(MAX_YUV_SIZE);
	memset(Buf, 0x00, MAX_YUV_SIZE);

	printD("convertyuvformat\n");
	indexY = 0;
	indexCB = streamSize >> 1;
	indexCR = indexCB+(streamSize >> 2);

	printD("indexY(%ld), indexCB(%ld), indexCR(%ld)\n", indexY, indexCB, indexCR);

	for(i = 0; i < streamSize; i++)
	{
		if((i%2) == 0)
			Buf[indexY++] = OutBuf[i];

		if((i%4) == 1) 
			Buf[indexCB++] = OutBuf[i];

		if((i%4) == 3) 
			Buf[indexCR++] = OutBuf[i];
	}

	fp = fopen(filename, "wb");
	fwrite(Buf, 1, streamSize, fp);
	fclose(fp);
	free(Buf);

}
/*
*******************************************************************************
Name            : DecodeFileOutYCBYCR
Description     : To write result YCBYCR file.
Parameter       :
Return Value    : void
*******************************************************************************
*/
void DecodeFileOutYCBYCR(char *OutBuf, UINT32 streamSize, char *filename)
{
	FILE *fp;

	fp = fopen(filename, "wb");
	fwrite(OutBuf, 1, streamSize, fp);
	fclose(fp);

}

void printD(char* fmt, ...) 
{
#ifdef DEBUG
    char str[512];

	vsprintf(str, fmt, (char *)(&fmt+1));
	printf(str);
#endif

}

