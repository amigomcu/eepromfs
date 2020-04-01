/*************************************************************************
  * @�ļ�:EepromFs.h
  * @����:XY
  * @�汾:1.0,Copyright 2019 by XY.
  * @����:2019-09-12
  * @����:
  * @��ע:
*************************************************************************/
#ifndef __EEPROMFS_H__
#define __EEPROMFS_H__

#include "EepromCommon.h"

#define FUN_MATH_CRC16_LH_ENABLE 1	//����Crc����ʹ������ 

#ifndef NULL
#define NULL ((void *)0)
#endif

//�ļ�����С
#define FILENAME_LEN    16           
//�ļ����ݻ�������С
#define FS_BUFF_LEN     128

//��ַ
typedef short la_t;
//����
typedef unsigned char fdata_t;
//��С
typedef unsigned short efs_size_t;

#define NULLFILE    (-1)

//����ֵ����
typedef enum eepromfs_result_e
{
	EFR_OK=0,
    EFR_META_ERR,           //�����쳣
    EFR_NO_FILE,            //�������ļ�
    EFR_CRC_INFO_ERR,       //Crc�쳣
    EFR_CRC_DATA_ERR,
    EFR_NO_SPACE,           //�洢�ռ䲻��
    EFR_TEST_ERR,           //�����쳣
    EFR_STARTADDR_ERR,      //��ʼ��ַ�쳣
    EFR_EEPROM_ERR,         //eeprom�����쳣
    EFR_EXIST_FILENAME,     //�ļ����ظ�    
    EFR_FILENAME_TOOLONG,   //�ļ�������
    EFR_FILE_TOOLARGE,      //�ļ�����
    EFR_PARA_ERR,           //�����쳣
    EFR_FILEADDR_ERR,
    EFR_ERR,                //�����쳣
}eepromfs_result_t;

//�ļ�ϵͳͷ��Ϣ
typedef struct fs_meta_s
{
	la_t start_address;
	efs_size_t fs_size;
    efs_size_t fileCnt;//�ļ��������ڸ����ж��ļ�ϵͳ�Ƿ��쳣
    la_t firstFile; //˫������ȷ���м��ļ������𻵵��º����ļ����ɷ���
    la_t lastFile;  //˫������ȷ���м��ļ������𻵵��º����ļ����ɷ���
    unsigned short fsCrc;
} fs_meta_t;
#define FS_META_LEN (sizeof(fs_meta_t))

//�ļ����
typedef struct file_handle
{        
    la_t cur;    
    la_t prev;//ǰһ���ļ�              //2�ֽ�
    la_t next;//��һ���ļ�              //2�ֽ�
    //�洢��EEProm���ļ���Ϣ����
	char filename[FILENAME_LEN];       //8�ֽ�	
	efs_size_t dataSize;//���ݴ�С      //2�ֽ�
    unsigned short fileCrc; //�ļ�CRC//2�ֽ�
    unsigned short dataCrc; //����CRC//2�ֽ�
    //�ļ��򿪺������
    la_t addr_fileCrc;
    la_t addr_dataCrc;
	la_t addr_firstData;
	la_t addr_lastData;
    int opWriteCnt;
} file_handle_t;

//��������
eepromfs_result_t eepromfs_init(eeprom_handle_t *handle,la_t startAddr);
eepromfs_result_t eepromfs_format_quick(void);//��ʽ�����ı��С
eepromfs_result_t eepromfs_format_full(la_t startAddr,efs_size_t size);//��ʽ��������ָ����С

eepromfs_result_t eepromfs_new(file_handle_t *fh,const char* filename,efs_size_t fileSize);
eepromfs_result_t eepromfs_open(file_handle_t *fh,const char* filename);
eepromfs_result_t eepromfs_close(file_handle_t* fh);
eepromfs_result_t eepromfs_write(file_handle_t* fh, const fdata_t* data, efs_size_t size);
eepromfs_result_t eepromfs_offsetWrite(file_handle_t* fh,la_t offSet,const fdata_t* data, efs_size_t size);
eepromfs_result_t eepromfs_read(file_handle_t* fh, fdata_t* buf,efs_size_t bufLen,efs_size_t *pFilesize);
eepromfs_result_t eepromfs_delete(const char* filename);
eepromfs_result_t eepromfs_copy(eeprom_handle_t *handle,la_t destAddr,la_t srcAddr,efs_size_t size);

#endif /* __EEPROMFS_H__ */

