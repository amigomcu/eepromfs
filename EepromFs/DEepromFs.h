/*************************************************************************
  * @�ļ�:DEepromFs.h
  * @����:XY
  * @�汾:1.0,Copyright 2019 by XY.
  * @����:2019-11-08
  * @����:
  * @��ע:
*************************************************************************/
#ifndef __DEEPROMFS_H__
#define __DEEPROMFS_H__

#include "EepromFs.h"

eepromfs_result_t deepromfs_init(eeprom_handle_t *handle,la_t startAddr,efs_size_t size);
eepromfs_result_t deepromfs_format_quick(void);
eepromfs_result_t deepromfs_format_full(la_t startAddr,efs_size_t size);
eepromfs_result_t deepromfs_write(const char* filename,const fdata_t* buf, efs_size_t size);
eepromfs_result_t deepromfs_read(const char* filename,fdata_t* buf,efs_size_t bufLen,efs_size_t *pFilesize);

#endif /* __DEEPROMFS_H__ */

