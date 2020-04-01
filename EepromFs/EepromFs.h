/*************************************************************************
  * @文件:EepromFs.h
  * @作者:XY
  * @版本:1.0,Copyright 2019 by XY.
  * @日期:2019-09-12
  * @功能:
  * @备注:
*************************************************************************/
#ifndef __EEPROMFS_H__
#define __EEPROMFS_H__

#include "EepromCommon.h"

#define FUN_MATH_CRC16_LH_ENABLE 1	//函数Crc计算使能配置 

#ifndef NULL
#define NULL ((void *)0)
#endif

//文件名大小
#define FILENAME_LEN    16           
//文件数据缓存区大小
#define FS_BUFF_LEN     128

//地址
typedef short la_t;
//数据
typedef unsigned char fdata_t;
//大小
typedef unsigned short efs_size_t;

#define NULLFILE    (-1)

//返回值定义
typedef enum eepromfs_result_e
{
	EFR_OK=0,
    EFR_META_ERR,           //分区异常
    EFR_NO_FILE,            //不存在文件
    EFR_CRC_INFO_ERR,       //Crc异常
    EFR_CRC_DATA_ERR,
    EFR_NO_SPACE,           //存储空间不足
    EFR_TEST_ERR,           //测试异常
    EFR_STARTADDR_ERR,      //起始地址异常
    EFR_EEPROM_ERR,         //eeprom操作异常
    EFR_EXIST_FILENAME,     //文件名重复    
    EFR_FILENAME_TOOLONG,   //文件名过长
    EFR_FILE_TOOLARGE,      //文件过大
    EFR_PARA_ERR,           //参数异常
    EFR_FILEADDR_ERR,
    EFR_ERR,                //其它异常
}eepromfs_result_t;

//文件系统头信息
typedef struct fs_meta_s
{
	la_t start_address;
	efs_size_t fs_size;
    efs_size_t fileCnt;//文件数量用于辅助判断文件系统是否异常
    la_t firstFile; //双向链表确保中间文件参数损坏导致后续文件不可访问
    la_t lastFile;  //双向链表确保中间文件参数损坏导致后续文件不可访问
    unsigned short fsCrc;
} fs_meta_t;
#define FS_META_LEN (sizeof(fs_meta_t))

//文件句柄
typedef struct file_handle
{        
    la_t cur;    
    la_t prev;//前一个文件              //2字节
    la_t next;//后一个文件              //2字节
    //存储在EEProm内文件信息参数
	char filename[FILENAME_LEN];       //8字节	
	efs_size_t dataSize;//数据大小      //2字节
    unsigned short fileCrc; //文件CRC//2字节
    unsigned short dataCrc; //数据CRC//2字节
    //文件打开后计算量
    la_t addr_fileCrc;
    la_t addr_dataCrc;
	la_t addr_firstData;
	la_t addr_lastData;
    int opWriteCnt;
} file_handle_t;

//操作函数
eepromfs_result_t eepromfs_init(eeprom_handle_t *handle,la_t startAddr);
eepromfs_result_t eepromfs_format_quick(void);//格式化不改变大小
eepromfs_result_t eepromfs_format_full(la_t startAddr,efs_size_t size);//格式化并重新指定大小

eepromfs_result_t eepromfs_new(file_handle_t *fh,const char* filename,efs_size_t fileSize);
eepromfs_result_t eepromfs_open(file_handle_t *fh,const char* filename);
eepromfs_result_t eepromfs_close(file_handle_t* fh);
eepromfs_result_t eepromfs_write(file_handle_t* fh, const fdata_t* data, efs_size_t size);
eepromfs_result_t eepromfs_offsetWrite(file_handle_t* fh,la_t offSet,const fdata_t* data, efs_size_t size);
eepromfs_result_t eepromfs_read(file_handle_t* fh, fdata_t* buf,efs_size_t bufLen,efs_size_t *pFilesize);
eepromfs_result_t eepromfs_delete(const char* filename);
eepromfs_result_t eepromfs_copy(eeprom_handle_t *handle,la_t destAddr,la_t srcAddr,efs_size_t size);

#endif /* __EEPROMFS_H__ */

