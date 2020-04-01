/*************************************************************************
  * @文件:EepromCommon.h
  * @作者:XY
  * @版本:1.0,Copyright 2019 by XY.
  * @日期:2019-09-21
  * @功能:
  * @备注:
*************************************************************************/
#ifndef __EEPROMCOMMON_H__
#define __EEPROMCOMMON_H__

#define EEPROM_OK               0         //OK            
#define EEPROM_ERROR          (-1)        //一般错误      

//eeprom操作接口
typedef struct eeprom_handle_s eeprom_handle_t;
struct eeprom_handle_s
{
    int size;
    int (*read)(eeprom_handle_t *handle,int start_addr,unsigned char *p_buf,int len);
    int (*write)(eeprom_handle_t *handle,int start_addr,unsigned char *p_buf,int len);
};

#endif /* __EEPROMCOMMON_H__ */

