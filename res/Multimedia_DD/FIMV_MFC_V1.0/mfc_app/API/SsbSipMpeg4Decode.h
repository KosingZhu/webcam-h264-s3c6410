#ifndef __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPMPEG4DECODE_H__
#define __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPMPEG4DECODE_H__



typedef struct
{
	int width;
	int height;
} SSBSIP_MPEG4_STREAM_INFO;



typedef unsigned int	MPEG4_DEC_CONF;

#define MPEG4_DEC_GETCONF_STREAMINFO		0x00001001
#define MPEG4_DEC_GETCONF_PHYADDR_FRAM_BUF	0x00001002
#define MPEG4_DEC_GETCONF_FRAM_NEED_COUNT	0x00001003
#define MPEG4_DEC_GETCONF_MPEG4_MV_ADDR		0x00001004
#define MPEG4_DEC_GETCONF_MPEG4_MBTYPE_ADDR	0x00001005

#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
#define MPEG4_DEC_GETCONF_MPEG4_FCODE				0x00001011
#define MPEG4_DEC_GETCONF_MPEG4_VOP_TIME_RES		0x00001012
#define MPEG4_DEC_GETCONF_MPEG4_TIME_BASE_LAST		0x00001013
#define MPEG4_DEC_GETCONF_MPEG4_NONB_TIME_LAST		0x00001014
#define MPEG4_DEC_GETCONF_MPEG4_TRD					0x00001015
#define MPEG4_DEC_GETCONF_PHYADDR_B_FRAM_BUF		0x00001016
#define MPEG4_DEC_GETCONF_BYTE_CONSUMED				0x00001017
#endif

#define MPEG4_DEC_SETCONF_POST_ROTATE				0x00002001
#define MPEG4_DEC_SETCONF_CACHE_CLEAN				0x00002002
#define MPEG4_DEC_SETCONF_CACHE_INVALIDATE			0x00002003
#define MPEG4_DEC_SETCONF_CACHE_CLEAN_INVALIDATE	0x00002004
#define MPEG4_DEC_SETCONF_PADDING_SIZE				0x00002005



#ifdef __cplusplus
extern "C" {
#endif


void *SsbSipMPEG4DecodeInit();
int   SsbSipMPEG4DecodeExe(void *openHandle, long lengthBufFill);
int   SsbSipMPEG4DecodeDeInit(void *openHandle);

int   SsbSipMPEG4DecodeSetConfig(void *openHandle, MPEG4_DEC_CONF conf_type, void *value);
int   SsbSipMPEG4DecodeGetConfig(void *openHandle, MPEG4_DEC_CONF conf_type, void *value);


void *SsbSipMPEG4DecodeGetInBuf(void *openHandle, long size);
void *SsbSipMPEG4DecodeGetOutBuf(void *openHandle, long *size);



#ifdef __cplusplus
}
#endif


// Error codes
#define SSBSIP_MPEG4_DEC_RET_OK						(0)
#define SSBSIP_MPEG4_DEC_RET_ERR_INVALID_HANDLE		(-1)
#define SSBSIP_MPEG4_DEC_RET_ERR_INVALID_PARAM		(-2)

#define SSBSIP_MPEG4_DEC_RET_ERR_CONFIG_FAIL		(-100)
#define SSBSIP_MPEG4_DEC_RET_ERR_DECODE_FAIL		(-101)
#define SSBSIP_MPEG4_DEC_RET_ERR_GETCONF_FAIL		(-102)
#define SSBSIP_MPEG4_DEC_RET_ERR_SETCONF_FAIL		(-103)


#endif /* __SAMSUNG_SYSLSI_APDEV_MFCLIB_SSBSIPMPEG4DECODE_H__ */

