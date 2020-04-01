/*************************************************************************
  * @文件:DEepromFs.c
  * @作者:XY
  * @版本:1.0,Copyright 2019 by XY.
  * @日期:2019-11-07
  * @功能:
  * @备注:
*************************************************************************/
#include "DEepromFs.h"

typedef struct deepromfs_s{
    eeprom_handle_t *handle;
    
    la_t startAddr1;
    efs_size_t size1;
    
    la_t startAddr2;
    efs_size_t size2;
}deepromfs_t;
static deepromfs_t deepromfs;

eepromfs_result_t deepromfs_init(eeprom_handle_t *handle,la_t startAddr,efs_size_t size)
{
    eepromfs_result_t eepromfs_result1;
    eepromfs_result_t eepromfs_result2;

    //初始化
    deepromfs.handle=handle;
    deepromfs.startAddr1=startAddr;
    deepromfs.size1=size/2;
    deepromfs.startAddr2=deepromfs.startAddr1+deepromfs.size1;
    deepromfs.size2=size/2;


    eepromfs_result1=eepromfs_init(deepromfs.handle,deepromfs.startAddr1);
    eepromfs_result2=eepromfs_init(deepromfs.handle,deepromfs.startAddr2);
    if((eepromfs_result1==EFR_OK)&&(eepromfs_result2==EFR_OK))
    {
        return EFR_OK;
    }
    else if((eepromfs_result1==EFR_OK)&&(eepromfs_result2!=EFR_OK))
    {
        //拷贝分区1数据到分区2
        eepromfs_copy(deepromfs.handle,deepromfs.startAddr2,deepromfs.startAddr1,deepromfs.size1);
        return EFR_OK;
    }
    else if((eepromfs_result1!=EFR_OK)&&(eepromfs_result2==EFR_OK))
    {
        //拷贝分区2数据到分区1
        eepromfs_copy(deepromfs.handle,deepromfs.startAddr1,deepromfs.startAddr2,deepromfs.size2);
        return EFR_OK;
    }
    else if((eepromfs_result1==EFR_CRC_INFO_ERR)&&(eepromfs_result2==EFR_CRC_INFO_ERR))
    {
        return EFR_CRC_INFO_ERR;
    }
    return EFR_ERR;
}
eepromfs_result_t deepromfs_format_quick(void)
{
    eepromfs_result_t eepromfs_result;
    
    eepromfs_result=eepromfs_init(deepromfs.handle,deepromfs.startAddr1);
    if(eepromfs_result!=EFR_OK)
    {
        return eepromfs_result;
    }
    
    if(eepromfs_format_quick()!=EFR_OK)
    {
        return eepromfs_result;
    }

    eepromfs_result=eepromfs_init(deepromfs.handle,deepromfs.startAddr2);
    if(eepromfs_result!=EFR_OK)
    {
        return eepromfs_result;
    }
    
    eepromfs_result=eepromfs_format_quick();
    if(eepromfs_result!=EFR_OK)
    {
        return eepromfs_result;
    }
    
    return EFR_OK;
}


eepromfs_result_t deepromfs_format_full(la_t startAddr,efs_size_t size)
{
    eepromfs_result_t eepromfs_result;

    eepromfs_result=eepromfs_init(deepromfs.handle,deepromfs.startAddr1);
    eepromfs_result=eepromfs_format_full(deepromfs.startAddr1,deepromfs.size1);
    if(eepromfs_result!=EFR_OK)
    {
        return eepromfs_result;
    }
    
    eepromfs_result=eepromfs_init(deepromfs.handle,deepromfs.startAddr2);
    eepromfs_result=eepromfs_format_full(deepromfs.startAddr2,deepromfs.size2);
    if(eepromfs_result!=EFR_OK)
    {
        return eepromfs_result;
    }
    return EFR_OK;
}

//双存储读
eepromfs_result_t deepromfs_write(const char* filename,const fdata_t* buf, efs_size_t size)
{
    file_handle_t fh;
    eepromfs_result_t eepromfs_result;
    //区域1文件
    if(eepromfs_init(deepromfs.handle,deepromfs.startAddr1)!=EFR_OK)
    {
        return EFR_ERR;
    }
    eepromfs_result=eepromfs_open(&fh,filename);
    if(eepromfs_result==EFR_OK)
    {
        if(eepromfs_write(&fh,buf,size)!=EFR_OK)
        {
            return EFR_ERR;
        }
        if(eepromfs_close(&fh)!=EFR_OK)
        {
            return EFR_ERR;
        }
    }
    else if(eepromfs_result==EFR_NO_FILE)
    {
        eepromfs_result=eepromfs_new(&fh,filename,size);
        if(eepromfs_result==EFR_OK)
        {
            if(eepromfs_write(&fh,buf,size)!=EFR_OK)
            {
                return EFR_ERR;
            }
            if(eepromfs_close(&fh)!=EFR_OK)
            {
                return EFR_ERR;
            }
        }
        else
        {
            return eepromfs_result;
        }
    }
    else if(eepromfs_result==EFR_CRC_INFO_ERR)
    {
        if(eepromfs_write(&fh,buf,size)!=EFR_OK)
        {
            return EFR_ERR;
        }
        if(eepromfs_close(&fh)!=EFR_OK)
        {
            return EFR_ERR;
        }
    }
    else
    {
        return eepromfs_result;
    }
    
    //区域2文件
    if(eepromfs_init(deepromfs.handle,deepromfs.startAddr2)!=EFR_OK)
    {
        return EFR_ERR;
    }
    eepromfs_result=eepromfs_open(&fh,filename);
    if(eepromfs_result==EFR_OK)
    {
        if(eepromfs_write(&fh,buf,size)!=EFR_OK)
        {
            return EFR_ERR;
        }
        if(eepromfs_close(&fh)!=EFR_OK)
        {
            return EFR_ERR;
        }
    }
    else if(eepromfs_result==EFR_NO_FILE)
    {
        eepromfs_result=eepromfs_new(&fh,filename,size);
        if(eepromfs_result==EFR_OK)
        {
            if(eepromfs_write(&fh,buf,size)!=EFR_OK)
            {
                return EFR_ERR;
            }
            if(eepromfs_close(&fh)!=EFR_OK)
            {
                return EFR_ERR;
            }
        }
        else
        {
            return eepromfs_result;
        }
    }
    else if(eepromfs_result==EFR_CRC_INFO_ERR)
    {
        if(eepromfs_write(&fh,buf,size)!=EFR_OK)
        {
            return EFR_ERR;
        }
        if(eepromfs_close(&fh)!=EFR_OK)
        {
            return EFR_ERR;
        }
    }
    else
    {
        return eepromfs_result;
    }
    return EFR_OK;
}

//读单个存储区文件
static eepromfs_result_t deepromfs_read_s(la_t startAddr,const char* filename, fdata_t* buf,efs_size_t bufLen,efs_size_t *pFilesize)
{
    file_handle_t fh;

    if(eepromfs_init(deepromfs.handle,startAddr)!=EFR_OK)
    {
        return EFR_ERR;
    }
    if(eepromfs_open(&fh,filename)!=EFR_OK)
    {
        return EFR_ERR;
    }
    if(eepromfs_read(&fh,buf,bufLen,pFilesize)!=EFR_OK)
    {
        return EFR_ERR;
    }
    return EFR_OK;
}

//双存储写
eepromfs_result_t deepromfs_read(const char* filename,fdata_t* buf,efs_size_t bufLen,efs_size_t *pFilesize)
{    
    static unsigned char fs_buff1[FS_BUFF_LEN];
    static unsigned char fs_buff2[FS_BUFF_LEN];
    efs_size_t filesize1;
    efs_size_t filesize2;
    eepromfs_result_t eepromfs_result1;
    eepromfs_result_t eepromfs_result2;
    int i=0;

    eepromfs_result1=deepromfs_read_s(deepromfs.startAddr1,filename,fs_buff1,FS_BUFF_LEN,&filesize1);
    eepromfs_result2=deepromfs_read_s(deepromfs.startAddr2,filename,fs_buff2,FS_BUFF_LEN,&filesize2);

    if((eepromfs_result1==EFR_OK)&&(eepromfs_result2!=EFR_OK))
    {        
        file_handle_t fh;
        eepromfs_result_t eepromfs_result;
        do{
                
            //区域2文件
            eepromfs_result=eepromfs_init(deepromfs.handle,deepromfs.startAddr2);
            if(eepromfs_result!=EFR_OK)
            {
                break;
            }
            eepromfs_result=eepromfs_open(&fh,filename);
            if(eepromfs_result==EFR_OK)
            {
                if(eepromfs_write(&fh,fs_buff1,filesize1)!=EFR_OK)
                {
                    break;
                }
                if(eepromfs_close(&fh)!=EFR_OK)
                {
                    break;
                }
            }
            else if(eepromfs_result==EFR_NO_FILE)
            {
                eepromfs_result=eepromfs_new(&fh,filename,filesize1);
                if(eepromfs_result==EFR_OK)
                {
                    if(eepromfs_write(&fh,fs_buff1,filesize1)!=EFR_OK)
                    {
                        break;
                    }
                    if(eepromfs_close(&fh)!=EFR_OK)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            
            for(i=0;(i<filesize1) && (i<bufLen);i++)
            {
                buf[i]=fs_buff1[i];
            }
            *pFilesize=filesize1;

            eepromfs_result=EFR_OK;
        }while(0);
        return eepromfs_result;
    }
    else if((eepromfs_result1!=EFR_OK)&&(eepromfs_result2==EFR_OK))
    {        
        file_handle_t fh;
        eepromfs_result_t eepromfs_result;
        do{
            //区域1文件
            eepromfs_result=eepromfs_init(deepromfs.handle,deepromfs.startAddr1);
            if(eepromfs_result!=EFR_OK)
            {
                break;
            }
            eepromfs_result=eepromfs_open(&fh,filename);
            if(eepromfs_result==EFR_OK)
            {
                if(eepromfs_write(&fh,fs_buff2,filesize2)!=EFR_OK)
                {
                    break;
                }
                if(eepromfs_close(&fh)!=EFR_OK)
                {
                    break;
                }
            }
            else if(eepromfs_result==EFR_NO_FILE)
            {
                eepromfs_result=eepromfs_new(&fh,filename,filesize2);
                if(eepromfs_result==EFR_OK)
                {
                    if(eepromfs_write(&fh,fs_buff2,filesize2)!=EFR_OK)
                    {
                        break;
                    }
                    if(eepromfs_close(&fh)!=EFR_OK)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            
            for(i=0;(i<filesize2) && (i<bufLen);i++)
            {
                buf[i]=fs_buff2[i];
            }
            *pFilesize=filesize2;
            
            eepromfs_result=EFR_OK;
        }while(0);
        return eepromfs_result;
    }
    else if((eepromfs_result1!=EFR_OK)&&(eepromfs_result2!=EFR_OK))
    {
        return EFR_ERR;
    }
    else if((eepromfs_result1==EFR_OK)&&(eepromfs_result2==EFR_OK))
    {
        for(i=0;(i<filesize1) && (i<bufLen);i++)
        {
            buf[i]=fs_buff1[i];
        }
        *pFilesize=filesize1;
        return EFR_OK;
    }
    return EFR_ERR;
}

