
#ifndef __TEST_EEPROMFS_H__
#define __TEST_EEPROMFS_H__

#include "../cutest-1.5/CuTest.h"
#include "../EepromFs/EepromCommon.h"
#include "../EepromFs/EepromFs.h"

#define EEPROM_SIMULATE_BUFF_LEN	512
extern char eeprom_simulate_buff[];

eeprom_handle_t *eeprom_simulate_init(void);
void test_EepromFs_task01(void);
void RunTests_EepromFs(void);

#endif
