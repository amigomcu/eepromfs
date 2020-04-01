/*************************************************************************
  * @�ļ�:EepromFs.c
  * @����:XY
  * @�汾:1.0,Copyright 2019 by XY.
  * @����:2019-09-11
  * @����:
  * @��ע:
        1���̶���С�ļ�Eeprom�ļ�ϵͳ��
        2����������ʽ�������ļ���������������ļ���Ϣ��ʽ��
        3��˫������ʽ�����쳣�ָ��ļ���Ϣ��
*************************************************************************/
#define __EEPROMFS_C__
#include "EepromFs.h"

static fs_meta_t fs_meta={-1};
static eeprom_handle_t *eeprom_handle=NULL;
static unsigned char fs_buff[FS_BUFF_LEN];
#define FS_FORMAT_BUFF_LEN  512
//�ļ���Ϣ����
#define FILE_HANDLE_INFO_LEN   ((int)&(((file_handle_t *)0)->fileCrc))//�ļ���Ϣ���Ȳ�������Crc  
#define FILE_HANDLE_STORE_LEN ((int)&(((file_handle_t *)0)->addr_fileCrc))//�ļ���Ϣ����

//�ڲ���������
static eepromfs_result_t __eepromfs_seekLastFile(file_handle_t *fh);

#if FUN_MATH_CRC16_LH_ENABLE==1
unsigned short Math_CRC16_LH(unsigned char* buf, unsigned short len)
{
    unsigned short i=0;
    unsigned short j=0;
    unsigned short r=0xffff;

    for(i=0; i<len; i++)
    {
        r ^= buf[i];
        for(j=0; j<8; j++)
        {
            if(r&0x01)
                r = (r>>1)^0xa001;
            else
                r = r>>1;
        }
    }
    return r;
}
#endif

/*************************************************************************
*����:eepromfs_init
*����:��ʼ��
*************************************************************************/
//extern u16 Math_CRC16_LH(u8* buf, u16 len);
eepromfs_result_t eepromfs_init(eeprom_handle_t *handle,la_t startAddr)
{
    file_handle_t fh;
    eepromfs_result_t eepromfs_result;
    eeprom_handle = handle;
    //��ȡ�ļ�������Ϣ
    if(eeprom_handle->read(eeprom_handle,startAddr,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        fs_meta.fs_size=0;//��ʶ����δ��ʼ��
        //log������Ϣ��ȡ����
        return EFR_EEPROM_ERR;
    }
    //crcУ��
    if(Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2)!=fs_meta.fsCrc)
    {
        fs_meta.fs_size=0;//��ʶ����δ��ʼ��
        //log crcУ�����   
        return EFR_CRC_INFO_ERR;
    }
    
    if(fs_meta.start_address!=startAddr)
    {
        fs_meta.fs_size=0;//��ʶ����δ��ʼ��
        //log ��ʼ��ַ����
        return EFR_STARTADDR_ERR;
    }

    //�ļ������˶�
    eepromfs_result=__eepromfs_seekLastFile(&fh);
    if((eepromfs_result!=EFR_OK)&&(eepromfs_result!=EFR_NO_FILE))
    {
        return eepromfs_result;
    }
    
    return EFR_OK;
}


/*************************************************************************
*����:eepromfs_format_quick
*����:���ٸ�ʽ��
*����:startAddr ��ʼ��ַ
*����:�������
*˵��:���ٸ�ʽ��,���ı�洢����С,������ļ��洢��Ϣ
*************************************************************************/
eepromfs_result_t eepromfs_format_quick(void)//��ʽ�����ı��С
{
    efs_size_t i=0;
    unsigned char temp=0;
    if((fs_meta.start_address+fs_meta.fs_size) > eeprom_handle->size)
    {
        //log �ռ䲻��
        return EFR_NO_SPACE;
    }
    //���eeprom�ռ�
    for(i=0;i<(fs_meta.fs_size-FS_META_LEN);i++)
    {
        if(eeprom_handle->write(eeprom_handle,fs_meta.start_address+FS_META_LEN+i,&temp,1)!=EEPROM_OK)
        {
            //log ������Ϣд�����
            return EFR_EEPROM_ERR;
        }
    }
    //��ʼ���ļ���Ϣ
    fs_meta.fileCnt=0;
    fs_meta.firstFile=NULLFILE;
    fs_meta.lastFile=NULLFILE;
    fs_meta.fsCrc=Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2);
    
    if(eeprom_handle->write(eeprom_handle,fs_meta.start_address,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        //log ������Ϣд�����

        return EFR_EEPROM_ERR;
    }
    return EFR_OK;
}

/*************************************************************************
*����:eepromfs_format_full
*����:��ʽ��������ָ����С
*����:startAddr ��ʼ��ַ
      size �洢����С
*����:�������
*************************************************************************/
eepromfs_result_t eepromfs_format_full(la_t startAddr,efs_size_t size)
{    
    int i=0;
    unsigned char buff[FS_FORMAT_BUFF_LEN];
    int cnt=0;
    if((startAddr+size) > eeprom_handle->size)
    {
        //log �ռ䲻��
        
        return EFR_NO_SPACE;
    }
    //���eeprom�ռ�
    for(i=0;i<FS_FORMAT_BUFF_LEN;i++)
    {
        buff[i]=0;
    }
    for(i=0;i<size;i++)
    {
        if((i+FS_FORMAT_BUFF_LEN)<=size)
        {
            cnt=FS_FORMAT_BUFF_LEN;
        }
        else
        {
            cnt=size-i;
        }
        if(eeprom_handle->write(eeprom_handle,startAddr+i,buff,cnt)!=EEPROM_OK)
        {
            //log ������Ϣд�����
            return EFR_EEPROM_ERR;
        }
        i+=cnt;
    }
    //��ʼ��
    memset(&fs_meta,0,FS_META_LEN);
    fs_meta.start_address=startAddr;    
    fs_meta.fs_size=size;
    fs_meta.fileCnt=0;
    fs_meta.firstFile=NULLFILE;
    fs_meta.lastFile=NULLFILE;
    fs_meta.fsCrc=Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2);
    
    if(eeprom_handle->write(eeprom_handle,startAddr,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        //log ������Ϣд�����

        return EFR_EEPROM_ERR;
    }
    return EFR_OK;
}

/*************************************************************************
*����:__eepromfs_seekNextFile
*����:��ȡ��һ���ļ������ȡ���ݣ��˶�crc
*************************************************************************/
static eepromfs_result_t __eepromfs_seekNextFile(file_handle_t *curFile,file_handle_t *nextFile)
{
    if(curFile->next==NULLFILE)
    {
        return EFR_NO_FILE;
    }
    if(eeprom_handle->read(eeprom_handle,curFile->next,(unsigned char *)nextFile,FILE_HANDLE_STORE_LEN)!=EEPROM_OK)
    {
        return EFR_EEPROM_ERR;
    }
    return EFR_OK;
}

static eepromfs_result_t __eepromfs_seekLastFile(file_handle_t *fh)
{
    int cnt=0;        
    la_t eeprom_addr=NULLFILE;
    if(fs_meta.firstFile==NULLFILE)
    {
        return EFR_NO_FILE;
    }
    //��ȡ��һ���ļ�    
    eeprom_addr=fs_meta.firstFile;
    if(eeprom_handle->read(eeprom_handle,fs_meta.firstFile,(unsigned char *)fh,FILE_HANDLE_STORE_LEN)!=EEPROM_OK)
    {
        return EFR_EEPROM_ERR;
    }    
    cnt++;
    while((fh->next!=NULLFILE)&&(cnt<fs_meta.fileCnt))
    {        
        eeprom_addr=fh->next;
        if(eeprom_handle->read(eeprom_handle,fh->next,(unsigned char *)fh,FILE_HANDLE_STORE_LEN)!=EEPROM_OK)
        {
            return EFR_EEPROM_ERR;
        }
        cnt++;
    }
    if((fh->next==NULLFILE)&&(cnt==fs_meta.fileCnt))
    {
        if(eeprom_addr==fh->cur)
        {
            return EFR_OK;
        }
        return EFR_FILEADDR_ERR;
    }
    return EFR_META_ERR;
}

static eepromfs_result_t __eepromfs_seekFile(file_handle_t *fh,const char* filename)
{
    int cnt=1;
    la_t eeprom_addr=NULLFILE;
    if(fs_meta.firstFile==NULLFILE)
    {
        return EFR_NO_FILE;
    }
    //��ȡ��һ���ļ�
    eeprom_addr=fs_meta.firstFile;
    if(eeprom_handle->read(eeprom_handle,fs_meta.firstFile,(unsigned char *)fh,FILE_HANDLE_STORE_LEN)!=EEPROM_OK)
    {
        return EFR_EEPROM_ERR;
    }    
    while((strcmp(filename,fh->filename)!=0)&&(fh->next!=NULLFILE)&&(cnt<fs_meta.fileCnt))
    {
        eeprom_addr=fh->next;
        if(eeprom_handle->read(eeprom_handle,fh->next,(unsigned char *)fh,FILE_HANDLE_STORE_LEN)!=EEPROM_OK)
        {
            return EFR_EEPROM_ERR;
        }
        cnt++;
    }
    if(strcmp(filename,fh->filename)==0)
    {
        if(eeprom_addr==fh->cur)
        {
            return EFR_OK;
        }
        else
        {
            return EFR_FILEADDR_ERR;
        }
    }
    return EFR_NO_FILE;
}



/*************************************************************************
*����:eepromfs_new
*����:
*************************************************************************/
eepromfs_result_t eepromfs_new(file_handle_t *fh,const char* filename,efs_size_t fileSize)
{
    file_handle_t fileTemp;
    eepromfs_result_t ret;
    la_t prevAddr=0;
    la_t addr=0;
    int i=0;
    if(fs_meta.fs_size==0)
    {
        return EFR_META_ERR;
    }
    //�����ļ����Ƿ����
    ret=__eepromfs_seekFile(&fileTemp,filename);
    if(ret!=EFR_NO_FILE)
    {
        if(ret==EFR_OK)
        {
            //�ļ�������
            return EFR_EXIST_FILENAME;
        }
        if(ret==EFR_EEPROM_ERR)
        {
            //Eeprom����
            return EFR_EEPROM_ERR;
        }
        if(ret==EFR_FILEADDR_ERR)
        {
            return EFR_FILEADDR_ERR;
        }
    }
    if(strlen(filename)>(FILENAME_LEN-1))
    {
        //�ļ�������
        
        return EFR_FILENAME_TOOLONG;
    }

    if(fileSize>=FS_BUFF_LEN)
    {
        //�ļ�����
        
        return EFR_FILE_TOOLARGE;
    }
    
    //ʣ��洢�ռ��ж�
    ret=__eepromfs_seekLastFile(&fileTemp);
    if(ret==EFR_OK)
    {
        //ʣ��ռ��ж�,��׷����ĩβ
        addr=fileTemp.cur+FILE_HANDLE_STORE_LEN+fileTemp.dataSize;
        prevAddr=fileTemp.cur;
    }
    else if(ret==EFR_NO_FILE)//û���ļ�
    {
        addr=fs_meta.start_address+FS_META_LEN;
        prevAddr=NULLFILE;
    }
    else
    {
        return ret;
    }
    if((addr+FILE_HANDLE_STORE_LEN+fileSize)>(fs_meta.start_address+fs_meta.fs_size))
    {
        //�ռ䲻��

        return EFR_NO_SPACE;
    }
    //��ʼ���ļ����
    memset(fh,0,sizeof(file_handle_t));    
    fh->cur=addr;    
    fh->prev=prevAddr;
    fh->next=NULLFILE;
    strncpy(fh->filename,filename,FILENAME_LEN);
    fh->filename[FILENAME_LEN-1]='\0';
    fh->dataSize=fileSize;
    fh->fileCrc=Math_CRC16_LH((unsigned char *)fh,FILE_HANDLE_INFO_LEN);//�ֶμ���CRC  
    //��ַ��Ϣ����    
    fh->addr_fileCrc=fh->cur+FILE_HANDLE_INFO_LEN;
    fh->addr_dataCrc=fh->addr_fileCrc+2;
    fh->addr_firstData=fh->addr_dataCrc+2;
    fh->addr_lastData=fh->addr_firstData+fh->dataSize-1;
    fh->opWriteCnt=0;
    //������ղ�����Crc
    for(i=0;i<fh->dataSize;i++)
    {
        fs_buff[i]=0;
    }
    fh->dataCrc=Math_CRC16_LH(fs_buff,fh->dataSize);//�ֶμ���CRC  
    //�ļ���Ϣ����ϢCrc������Crc
    if(eeprom_handle->write(eeprom_handle,fh->cur,(unsigned char *)fh,FILE_HANDLE_STORE_LEN)!=EEPROM_OK)
    {
        //log ������Ϣд�����
        return EFR_EEPROM_ERR;
    }
    //����
    if(eeprom_handle->write(eeprom_handle,fh->addr_firstData,fs_buff,fh->dataSize)!=EEPROM_OK)
    {
        //log ������Ϣд�����
        return EFR_EEPROM_ERR;
    }
    //�������һ���ļ�����
    if(prevAddr!=NULLFILE)
    {
        fileTemp.next=addr;
        fileTemp.fileCrc=Math_CRC16_LH((unsigned char *)&fileTemp,FILE_HANDLE_INFO_LEN);    
        if(eeprom_handle->write(eeprom_handle,fileTemp.cur,(unsigned char *)&fileTemp,FILE_HANDLE_STORE_LEN)!=EEPROM_OK)
        {
            //���һ���ļ���Ϣ�����쳣
            return EFR_EEPROM_ERR;
        }
    }

    //����������Ϣ
    if(prevAddr==NULLFILE)//��һ���ļ�
    {
        fs_meta.firstFile=addr;
    }
    fs_meta.lastFile=fh->cur;
    fs_meta.fileCnt++;
    fs_meta.fsCrc=Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2);
    
    if(eeprom_handle->write(eeprom_handle,fs_meta.start_address,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        //���һ���ļ���Ϣ�����쳣
        return EFR_EEPROM_ERR;
    }
	return EFR_OK;
}
    
eepromfs_result_t eepromfs_open(file_handle_t *fh,const char* filename)
{
    eepromfs_result_t ret;    
    if(fs_meta.fs_size==0)
    {
        return EFR_META_ERR;
    }    
    memset(fh,0,sizeof(file_handle_t));    
    //�����ļ����Ƿ����
    ret=__eepromfs_seekFile(fh,filename);
    if(ret==EFR_OK)
    {
        //�ļ�������
        //��ȡ�ļ�������ϢУ��˶�
        if(fh->fileCrc!=Math_CRC16_LH((unsigned char *)fh,FILE_HANDLE_INFO_LEN))
        {
            return EFR_CRC_INFO_ERR;
        }
        
        //���µ�ַ��Ϣ
        fh->addr_fileCrc=fh->cur+FILE_HANDLE_INFO_LEN;
        fh->addr_dataCrc=fh->addr_fileCrc+2;
        fh->addr_firstData=fh->addr_dataCrc+2;
        fh->addr_lastData=fh->addr_firstData+fh->dataSize-1;
        
        //����
        if(eeprom_handle->read(eeprom_handle,fh->addr_firstData,fs_buff,fh->dataSize)!=EEPROM_OK)
        {
            return EFR_EEPROM_ERR;
        }
        
        //crcУ��        
        if(fh->dataCrc!=Math_CRC16_LH(fs_buff,fh->dataSize))
        {
            return EFR_CRC_DATA_ERR;
        }
        
        return EFR_OK;
    }
    return ret;
}

eepromfs_result_t eepromfs_close(file_handle_t* fh)
{    
    unsigned short crcTemp;
    if(fh==NULL)
    {
        return EFR_PARA_ERR;
    }
    
    //crcУ��        
    crcTemp=Math_CRC16_LH(fs_buff,fh->dataSize);//�ֶμ���CRC
    if((fh->dataCrc!=crcTemp)||(fh->opWriteCnt>0))
    {
        fh->dataCrc=crcTemp;
        
        //����
        if(eeprom_handle->write(eeprom_handle,fh->addr_firstData,fs_buff,fh->dataSize)!=EEPROM_OK)
        {
            //д�����
            return EFR_EEPROM_ERR;
        }
        //Crc
        if(eeprom_handle->write(eeprom_handle,fh->addr_dataCrc,(unsigned char *)&fh->dataCrc,2)!=EEPROM_OK)
        {
            //д�����
            return EFR_EEPROM_ERR;
        }
    }
    return EFR_OK;
}

eepromfs_result_t eepromfs_write(file_handle_t* fh, const fdata_t* data, efs_size_t size)
{
    int i=0;
    if((fh==NULL)||(data==NULL))
    {
        return EFR_PARA_ERR;
    }
    
    if(size>fh->dataSize)
    {
        return EFR_NO_SPACE;
    }
    //���»���������
    for(i=0;(i<size)&&(i<fh->dataSize);i++)
    {
        fs_buff[i]=data[i];
    }
    fh->opWriteCnt++;
    return EFR_OK;
}

eepromfs_result_t eepromfs_offsetWrite(file_handle_t* fh,la_t offSet,const fdata_t* data, efs_size_t size)
{
    int i=0;
    if((fh==NULL)||(data==NULL))
    {
        return EFR_PARA_ERR;
    }
    
    if(size>fh->dataSize)
    {
        return EFR_NO_SPACE;
    }
    //���»���������
    for(i=0;(i<size)&&((offSet+i)<fh->dataSize);i++)
    {
        fs_buff[i+offSet]=data[i];
    }
    fh->opWriteCnt++;
    return EFR_OK;
}

eepromfs_result_t eepromfs_read(file_handle_t* fh, fdata_t* buf,efs_size_t bufLen,efs_size_t *pFilesize)
{
    int i=0;
    if((fh==NULL)||(buf==NULL)||(bufLen==0))
    {
        return EFR_PARA_ERR;
    }
    //���»���������
    for(i=0;(i<bufLen)&&(i<fh->dataSize);i++)
    {
        buf[i]=fs_buff[i];
    }
    if(pFilesize!=NULL)
    {
        *pFilesize=fh->dataSize;
    }
    return EFR_OK;
}

eepromfs_result_t eepromfs_delete(const char* filename)
{
    return EFR_ERR;
}

/*************************************************************************
*����:eepromfs_copy
*����:��������
*************************************************************************/
eepromfs_result_t eepromfs_copy(eeprom_handle_t *handle,la_t destAddr,la_t srcAddr,efs_size_t size)
{    
    int i=0;
    unsigned char buff[FS_FORMAT_BUFF_LEN];
    int cnt=0;    
    //��ȡ�ļ�������Ϣ,���˶Է�����Сָ���Ƿ���ȷ
    if(eepromfs_init(handle,srcAddr)!=EEPROM_OK)
    {
        return EFR_ERR;
    }
    //ָ��������С�쳣
    if(fs_meta.fs_size!=size)
    {
        return EFR_ERR;
    }
    //������������
    for(i=0;i<fs_meta.fs_size;i++)
    {
        if((i+FS_FORMAT_BUFF_LEN)<=fs_meta.fs_size)
        {
            cnt=FS_FORMAT_BUFF_LEN;
        }
        else
        {
            cnt=fs_meta.fs_size-i;
        }
        if(handle->read(handle,srcAddr+i,buff,cnt)==EEPROM_ERROR)
        {
            return EFR_ERR;
        }
        if(handle->write(handle,destAddr+i,buff,cnt)==EEPROM_ERROR)
        {
            return EFR_ERR;
        }
        i+=cnt;
    }  

    //��ȡ�ļ�������Ϣ
    if(eeprom_handle->read(eeprom_handle,destAddr,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        fs_meta.fs_size=0;//��ʶ����δ��ʼ��
        //log������Ϣ��ȡ����
        return EFR_EEPROM_ERR;
    }
    //�ؽ�������Ϣ
    fs_meta.start_address=destAddr;    
    fs_meta.fsCrc=Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2);
    if(eeprom_handle->write(eeprom_handle,destAddr,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        //log ������Ϣд�����
        return EFR_EEPROM_ERR;
    }
    return EFR_OK;
}

