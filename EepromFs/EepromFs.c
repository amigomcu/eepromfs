/*************************************************************************
  * @文件:EepromFs.c
  * @作者:XY
  * @版本:1.0,Copyright 2019 by XY.
  * @日期:2019-09-11
  * @功能:
  * @备注:
        1、固定大小文件Eeprom文件系统。
        2、采用链表方式把所有文件串联起来，替代文件信息表方式。
        3、双向链表方式用于异常恢复文件信息。
*************************************************************************/
#define __EEPROMFS_C__
#include "EepromFs.h"

static fs_meta_t fs_meta={-1};
static eeprom_handle_t *eeprom_handle=NULL;
static unsigned char fs_buff[FS_BUFF_LEN];
#define FS_FORMAT_BUFF_LEN  512
//文件信息长度
#define FILE_HANDLE_INFO_LEN   ((int)&(((file_handle_t *)0)->fileCrc))//文件信息长度不含数据Crc  
#define FILE_HANDLE_STORE_LEN ((int)&(((file_handle_t *)0)->addr_fileCrc))//文件信息长度

//内部函数声明
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
*函数:eepromfs_init
*功能:初始化
*************************************************************************/
//extern u16 Math_CRC16_LH(u8* buf, u16 len);
eepromfs_result_t eepromfs_init(eeprom_handle_t *handle,la_t startAddr)
{
    file_handle_t fh;
    eepromfs_result_t eepromfs_result;
    eeprom_handle = handle;
    //获取文件分区信息
    if(eeprom_handle->read(eeprom_handle,startAddr,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        fs_meta.fs_size=0;//标识分区未初始化
        //log分区信息读取错误
        return EFR_EEPROM_ERR;
    }
    //crc校验
    if(Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2)!=fs_meta.fsCrc)
    {
        fs_meta.fs_size=0;//标识分区未初始化
        //log crc校验错误   
        return EFR_CRC_INFO_ERR;
    }
    
    if(fs_meta.start_address!=startAddr)
    {
        fs_meta.fs_size=0;//标识分区未初始化
        //log 起始地址错误
        return EFR_STARTADDR_ERR;
    }

    //文件索引核对
    eepromfs_result=__eepromfs_seekLastFile(&fh);
    if((eepromfs_result!=EFR_OK)&&(eepromfs_result!=EFR_NO_FILE))
    {
        return eepromfs_result;
    }
    
    return EFR_OK;
}


/*************************************************************************
*函数:eepromfs_format_quick
*功能:快速格式化
*参数:startAddr 起始地址
*返回:操作结果
*说明:快速格式化,不改变存储区大小,仅清除文件存储信息
*************************************************************************/
eepromfs_result_t eepromfs_format_quick(void)//格式化不改变大小
{
    efs_size_t i=0;
    unsigned char temp=0;
    if((fs_meta.start_address+fs_meta.fs_size) > eeprom_handle->size)
    {
        //log 空间不足
        return EFR_NO_SPACE;
    }
    //清空eeprom空间
    for(i=0;i<(fs_meta.fs_size-FS_META_LEN);i++)
    {
        if(eeprom_handle->write(eeprom_handle,fs_meta.start_address+FS_META_LEN+i,&temp,1)!=EEPROM_OK)
        {
            //log 分区信息写入错误
            return EFR_EEPROM_ERR;
        }
    }
    //初始化文件信息
    fs_meta.fileCnt=0;
    fs_meta.firstFile=NULLFILE;
    fs_meta.lastFile=NULLFILE;
    fs_meta.fsCrc=Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2);
    
    if(eeprom_handle->write(eeprom_handle,fs_meta.start_address,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        //log 分区信息写入错误

        return EFR_EEPROM_ERR;
    }
    return EFR_OK;
}

/*************************************************************************
*函数:eepromfs_format_full
*功能:格式化并重新指定大小
*参数:startAddr 起始地址
      size 存储区大小
*返回:操作结果
*************************************************************************/
eepromfs_result_t eepromfs_format_full(la_t startAddr,efs_size_t size)
{    
    int i=0;
    unsigned char buff[FS_FORMAT_BUFF_LEN];
    int cnt=0;
    if((startAddr+size) > eeprom_handle->size)
    {
        //log 空间不足
        
        return EFR_NO_SPACE;
    }
    //清空eeprom空间
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
            //log 分区信息写入错误
            return EFR_EEPROM_ERR;
        }
        i+=cnt;
    }
    //初始化
    memset(&fs_meta,0,FS_META_LEN);
    fs_meta.start_address=startAddr;    
    fs_meta.fs_size=size;
    fs_meta.fileCnt=0;
    fs_meta.firstFile=NULLFILE;
    fs_meta.lastFile=NULLFILE;
    fs_meta.fsCrc=Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2);
    
    if(eeprom_handle->write(eeprom_handle,startAddr,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        //log 分区信息写入错误

        return EFR_EEPROM_ERR;
    }
    return EFR_OK;
}

/*************************************************************************
*函数:__eepromfs_seekNextFile
*功能:获取下一个文件不会读取数据，核对crc
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
    //读取第一个文件    
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
    //读取第一个文件
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
*函数:eepromfs_new
*功能:
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
    //查找文件名是否存在
    ret=__eepromfs_seekFile(&fileTemp,filename);
    if(ret!=EFR_NO_FILE)
    {
        if(ret==EFR_OK)
        {
            //文件名存在
            return EFR_EXIST_FILENAME;
        }
        if(ret==EFR_EEPROM_ERR)
        {
            //Eeprom错误
            return EFR_EEPROM_ERR;
        }
        if(ret==EFR_FILEADDR_ERR)
        {
            return EFR_FILEADDR_ERR;
        }
    }
    if(strlen(filename)>(FILENAME_LEN-1))
    {
        //文件名过长
        
        return EFR_FILENAME_TOOLONG;
    }

    if(fileSize>=FS_BUFF_LEN)
    {
        //文件过大
        
        return EFR_FILE_TOOLARGE;
    }
    
    //剩余存储空间判断
    ret=__eepromfs_seekLastFile(&fileTemp);
    if(ret==EFR_OK)
    {
        //剩余空间判断,简单追加在末尾
        addr=fileTemp.cur+FILE_HANDLE_STORE_LEN+fileTemp.dataSize;
        prevAddr=fileTemp.cur;
    }
    else if(ret==EFR_NO_FILE)//没有文件
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
        //空间不足

        return EFR_NO_SPACE;
    }
    //初始化文件句柄
    memset(fh,0,sizeof(file_handle_t));    
    fh->cur=addr;    
    fh->prev=prevAddr;
    fh->next=NULLFILE;
    strncpy(fh->filename,filename,FILENAME_LEN);
    fh->filename[FILENAME_LEN-1]='\0';
    fh->dataSize=fileSize;
    fh->fileCrc=Math_CRC16_LH((unsigned char *)fh,FILE_HANDLE_INFO_LEN);//分段计算CRC  
    //地址信息计算    
    fh->addr_fileCrc=fh->cur+FILE_HANDLE_INFO_LEN;
    fh->addr_dataCrc=fh->addr_fileCrc+2;
    fh->addr_firstData=fh->addr_dataCrc+2;
    fh->addr_lastData=fh->addr_firstData+fh->dataSize-1;
    fh->opWriteCnt=0;
    //数据清空并更新Crc
    for(i=0;i<fh->dataSize;i++)
    {
        fs_buff[i]=0;
    }
    fh->dataCrc=Math_CRC16_LH(fs_buff,fh->dataSize);//分段计算CRC  
    //文件信息及信息Crc及数据Crc
    if(eeprom_handle->write(eeprom_handle,fh->cur,(unsigned char *)fh,FILE_HANDLE_STORE_LEN)!=EEPROM_OK)
    {
        //log 分区信息写入错误
        return EFR_EEPROM_ERR;
    }
    //数据
    if(eeprom_handle->write(eeprom_handle,fh->addr_firstData,fs_buff,fh->dataSize)!=EEPROM_OK)
    {
        //log 分区信息写入错误
        return EFR_EEPROM_ERR;
    }
    //更新最后一个文件链接
    if(prevAddr!=NULLFILE)
    {
        fileTemp.next=addr;
        fileTemp.fileCrc=Math_CRC16_LH((unsigned char *)&fileTemp,FILE_HANDLE_INFO_LEN);    
        if(eeprom_handle->write(eeprom_handle,fileTemp.cur,(unsigned char *)&fileTemp,FILE_HANDLE_STORE_LEN)!=EEPROM_OK)
        {
            //最后一个文件信息更新异常
            return EFR_EEPROM_ERR;
        }
    }

    //更新区域信息
    if(prevAddr==NULLFILE)//第一个文件
    {
        fs_meta.firstFile=addr;
    }
    fs_meta.lastFile=fh->cur;
    fs_meta.fileCnt++;
    fs_meta.fsCrc=Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2);
    
    if(eeprom_handle->write(eeprom_handle,fs_meta.start_address,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        //最后一个文件信息更新异常
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
    //查找文件名是否存在
    ret=__eepromfs_seekFile(fh,filename);
    if(ret==EFR_OK)
    {
        //文件名存在
        //获取文件分区信息校验核对
        if(fh->fileCrc!=Math_CRC16_LH((unsigned char *)fh,FILE_HANDLE_INFO_LEN))
        {
            return EFR_CRC_INFO_ERR;
        }
        
        //更新地址信息
        fh->addr_fileCrc=fh->cur+FILE_HANDLE_INFO_LEN;
        fh->addr_dataCrc=fh->addr_fileCrc+2;
        fh->addr_firstData=fh->addr_dataCrc+2;
        fh->addr_lastData=fh->addr_firstData+fh->dataSize-1;
        
        //数据
        if(eeprom_handle->read(eeprom_handle,fh->addr_firstData,fs_buff,fh->dataSize)!=EEPROM_OK)
        {
            return EFR_EEPROM_ERR;
        }
        
        //crc校验        
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
    
    //crc校验        
    crcTemp=Math_CRC16_LH(fs_buff,fh->dataSize);//分段计算CRC
    if((fh->dataCrc!=crcTemp)||(fh->opWriteCnt>0))
    {
        fh->dataCrc=crcTemp;
        
        //数据
        if(eeprom_handle->write(eeprom_handle,fh->addr_firstData,fs_buff,fh->dataSize)!=EEPROM_OK)
        {
            //写入错误
            return EFR_EEPROM_ERR;
        }
        //Crc
        if(eeprom_handle->write(eeprom_handle,fh->addr_dataCrc,(unsigned char *)&fh->dataCrc,2)!=EEPROM_OK)
        {
            //写入错误
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
    //更新缓存区数据
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
    //更新缓存区数据
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
    //更新缓存区数据
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
*函数:eepromfs_copy
*功能:分区拷贝
*************************************************************************/
eepromfs_result_t eepromfs_copy(eeprom_handle_t *handle,la_t destAddr,la_t srcAddr,efs_size_t size)
{    
    int i=0;
    unsigned char buff[FS_FORMAT_BUFF_LEN];
    int cnt=0;    
    //获取文件分区信息,并核对分区大小指定是否正确
    if(eepromfs_init(handle,srcAddr)!=EEPROM_OK)
    {
        return EFR_ERR;
    }
    //指定分区大小异常
    if(fs_meta.fs_size!=size)
    {
        return EFR_ERR;
    }
    //拷贝分区数据
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

    //获取文件分区信息
    if(eeprom_handle->read(eeprom_handle,destAddr,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        fs_meta.fs_size=0;//标识分区未初始化
        //log分区信息读取错误
        return EFR_EEPROM_ERR;
    }
    //重建分区信息
    fs_meta.start_address=destAddr;    
    fs_meta.fsCrc=Math_CRC16_LH((unsigned char *)&fs_meta,FS_META_LEN-2);
    if(eeprom_handle->write(eeprom_handle,destAddr,(unsigned char *)&fs_meta,FS_META_LEN)!=EEPROM_OK)
    {
        //log 分区信息写入错误
        return EFR_EEPROM_ERR;
    }
    return EFR_OK;
}

