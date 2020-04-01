//测试 DEepromFs.c
#include "../EepromFs/EepromCommon.h"
#include "../EepromFs/EepromFs.h"
#include "../EepromFs/DEepromFs.h"
#include "../cutest-1.5/CuTest.h"
#include "test_DEepromFs.h"



void Test_DEepromFs(CuTest* tc)
{
	int i=0;
	eeprom_handle_t *p_eeprom_simulate;
	eepromfs_result_t  eepromfs_result;
	char strBuff1[]="1234567890abcdefghijklmnopqrstuvwxyz";
	char strBuff2[]="112233445566778899001122334455667788";
    #define STRBUFFREAD_LEN 128
	char strBuffRead[STRBUFFREAD_LEN];
	efs_size_t fileSize=0;
	p_eeprom_simulate=eeprom_simulate_init();
	if(deepromfs_init(p_eeprom_simulate,0,EEPROM_SIMULATE_BUFF_LEN)!=EFR_OK)
	{
        CuAssert(tc, "01 deepromfs format_full", EFR_OK == deepromfs_format_full(0,EEPROM_SIMULATE_BUFF_LEN));
		CuAssert(tc, "02 deepromfs init", EFR_OK == deepromfs_init(p_eeprom_simulate,0,EEPROM_SIMULATE_BUFF_LEN));
	}
	//文件1读写测试 
	CuAssert(tc, "03 deepromfs write data", EFR_OK == deepromfs_write("file01",strBuff1,20));
	CuAssert(tc, "04 deepromfs read", EFR_OK == deepromfs_read("file01",strBuffRead,20,&fileSize));
	CuAssert(tc, "05 fileSize is 20", fileSize == 20);
	for(i=0;i<20;i++)
	{
		CuAssert(tc, "06 data is ok", strBuff1[i] == strBuffRead[i]);
	}
	//文件2读写测试 
	CuAssert(tc, "07 deepromfs write data", EFR_OK == deepromfs_write("file02",strBuff2,20));
	CuAssert(tc, "08 deepromfs read", EFR_OK == deepromfs_read("file02",strBuffRead,20,&fileSize));
	CuAssert(tc, "09 fileSize is 20", fileSize == 20);
	for(i=0;i<20;i++)
	{
		CuAssert(tc, "10 data is ok", strBuff2[i] == strBuffRead[i]);
	}
	//文件1读测试
	CuAssert(tc, "11 deepromfs read", EFR_OK == deepromfs_read("file01",strBuffRead,20,&fileSize));
	CuAssert(tc, "12 fileSize is 20", fileSize == 20);
	for(i=0;i<20;i++)
	{
		CuAssert(tc, "13 data is ok", strBuff1[i] == strBuffRead[i]);
	}
	//文件2读测试 
	CuAssert(tc, "14 deepromfs read", EFR_OK == deepromfs_read("file02",strBuffRead,20,&fileSize));
	CuAssert(tc, "15 fileSize is 20", fileSize == 20);
	for(i=0;i<20;i++)
	{
		CuAssert(tc, "16 data is ok", strBuff2[i] == strBuffRead[i]);
	}
	//文件1修改测试
	CuAssert(tc, "17 deepromfs write data", EFR_OK == deepromfs_write("file01",strBuff2,20));
	CuAssert(tc, "18 deepromfs read", EFR_OK == deepromfs_read("file01",strBuffRead,20,&fileSize));
	CuAssert(tc, "19 fileSize is 20", fileSize == 20);
	for(i=0;i<20;i++)
	{
		CuAssert(tc, "20 data is ok", strBuff2[i] == strBuffRead[i]);
	}
	//测试数据破坏后读1
	CuAssert(tc, "21 eepromfs init",EFR_OK==eepromfs_init(p_eeprom_simulate,0));
	CuAssert(tc, "22 eepromfs format_quick",EFR_OK==eepromfs_format_quick());
	//修改后文件1读测试 
	CuAssert(tc, "23 deepromfs read", EFR_OK == deepromfs_read("file01",strBuffRead,20,&fileSize));
	CuAssert(tc, "24 fileSize is 20", fileSize == 20);
	for(i=0;i<20;i++)
	{
		CuAssert(tc, "25 data is ok", strBuff2[i] == strBuffRead[i]);
	}
	//文件2读测试 
	CuAssert(tc, "26 deepromfs read", EFR_OK == deepromfs_read("file02",strBuffRead,20,&fileSize));
	CuAssert(tc, "27 fileSize is 20", fileSize == 20);
	for(i=0;i<20;i++)
	{
		CuAssert(tc, "28 data is ok", strBuff2[i] == strBuffRead[i]);
	}

    //测试数据破坏后读2
    CuAssert(tc, "29 eepromfs init",EFR_OK==eepromfs_init(p_eeprom_simulate,EEPROM_SIMULATE_BUFF_LEN/2));
    CuAssert(tc, "30 eepromfs format_quick",EFR_OK==eepromfs_format_quick());
    //修改后文件1读测试 
    CuAssert(tc, "31 deepromfs read", EFR_OK == deepromfs_read("file01",strBuffRead,20,&fileSize));
    CuAssert(tc, "32 fileSize is 20", fileSize == 20);
    for(i=0;i<20;i++)
    {
        CuAssert(tc, "33 data is ok", strBuff2[i] == strBuffRead[i]);
    }
    //文件2读测试 
    CuAssert(tc, "34 deepromfs read", EFR_OK == deepromfs_read("file02",strBuffRead,20,&fileSize));
    CuAssert(tc, "35 fileSize is 20", fileSize == 20);
    for(i=0;i<20;i++)
    {
        CuAssert(tc, "36 data is ok", strBuff2[i] == strBuffRead[i]);
    }
}

CuSuite* Tests_DEepromFs_GetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, Test_DEepromFs);
	return suite;
}

void RunTests_DEepromFs(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, Tests_DEepromFs_GetSuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("DEepromFs %s\n", output->buffer);
}
