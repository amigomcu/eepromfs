//测试 EepromFs.c
#include "test_EepromFs.h"
//模拟eeprom 
char eeprom_simulate_buff[EEPROM_SIMULATE_BUFF_LEN];

int eeprom_simulate_read(eeprom_handle_t *handle,int start_addr,unsigned char *p_buf,int len)
{
	int i=0;
	if((start_addr+len)>handle->size)
	{
		return EEPROM_ERROR;
	}
	for(i=0;i<len;i++)
	{
		p_buf[i]=eeprom_simulate_buff[start_addr+i];
	}
	return EEPROM_OK;
}
int eeprom_simulate_write(eeprom_handle_t *handle,int start_addr,unsigned char *p_buf,int len)
{
	int i=0;
	if((start_addr+len)>EEPROM_SIMULATE_BUFF_LEN)
	{
		return EEPROM_ERROR;
	}
	for(i=0;i<len;i++)
	{
		eeprom_simulate_buff[start_addr+i]=p_buf[i];
	}
	return EEPROM_OK;	
}
eeprom_handle_t *eeprom_simulate_init(void)
{
	int i=0;
	static eeprom_handle_t eeprom_simulate;
	for(i=0;i<EEPROM_SIMULATE_BUFF_LEN;i++)
	{
		eeprom_simulate_buff[i]=0;
	}
	eeprom_simulate.size=EEPROM_SIMULATE_BUFF_LEN;
	eeprom_simulate.read=eeprom_simulate_read;
	eeprom_simulate.write=eeprom_simulate_write;
	return &eeprom_simulate;
}
file_handle_t file1,file2,file3,file4,file5,file6;
void test_EepromFs_task01(void)
{
	int i=0;
	eeprom_handle_t *p_eeprom_simulate;
	eepromfs_result_t  eepromfs_result;
	char strBuff[]="1234567890abcdefghijklmnopqrstuvwxyz";
	#define STRBUFFREAD_LEN	128
	char strBuffRead[STRBUFFREAD_LEN];
	efs_size_t fileSize=0;
	p_eeprom_simulate=eeprom_simulate_init();
	if(eepromfs_init(p_eeprom_simulate,0)!=EFR_OK)
	{
		eepromfs_format_full(0,EEPROM_SIMULATE_BUFF_LEN);
		eepromfs_init(p_eeprom_simulate,0);
	}
	eepromfs_new(&file1,"test01",20);
	eepromfs_write(&file1,strBuff,20);
	eepromfs_close(&file1);
	eepromfs_new(&file2,"test02",10);
	eepromfs_write(&file2,strBuff,10);
	eepromfs_close(&file2);
	eepromfs_new(&file3,"test03",30);
	eepromfs_write(&file3,strBuff,30);
	eepromfs_close(&file3);
	eepromfs_new(&file4,"test04",10);
	eepromfs_write(&file4,strBuff,10);
	eepromfs_close(&file4);
	eepromfs_new(&file5,"test05",50);
	eepromfs_write(&file5,strBuff,5);
	eepromfs_close(&file5);
	
	if(eepromfs_new(&file6,"test01",20)==EFR_OK)
	{
		eepromfs_write(&file6,strBuff,20);
		eepromfs_close(&file6);	
	}
	
	eepromfs_open(&file1,"test01");
	eepromfs_read(&file1,strBuffRead,STRBUFFREAD_LEN,&fileSize);
	if(fileSize!=20)
	{
		printf("file1 read len err!\n");
	}
	for(i=0;i<fileSize;i++)
	{
		if(strBuffRead[i]!=strBuff[i])
		{
			printf("file1 read data[%d]=%d err!\n",i,strBuffRead[i]);
		}
	}
	for(i=0;i<STRBUFFREAD_LEN;i++)
	{
		strBuffRead[i]=0;		
	}
	
	//file2 
	eepromfs_open(&file2,"test02");
	eepromfs_read(&file2,strBuffRead,STRBUFFREAD_LEN,&fileSize);
	if(fileSize!=10)
	{
		printf("file1 read len err!\n");
	}
	for(i=0;i<fileSize;i++)
	{
		if(strBuffRead[i]!=strBuff[i])
		{
			printf("file1 read data[%d]=%d err!\n",i,strBuffRead[i]);
		}
	}
	for(i=0;i<STRBUFFREAD_LEN;i++)
	{
		strBuffRead[i]=0;		
	}
	
	//file3 
	eepromfs_open(&file3,"test03");
	eepromfs_read(&file3,strBuffRead,STRBUFFREAD_LEN,&fileSize);
	if(fileSize!=30)
	{
		printf("file1 read len err!\n");
	}
	for(i=0;i<fileSize;i++)
	{
		if(strBuffRead[i]!=strBuff[i])
		{
			printf("file1 read data[%d]=%d err!\n",i,strBuffRead[i]);
		}
	}
	for(i=0;i<STRBUFFREAD_LEN;i++)
	{
		strBuffRead[i]=0;		
	}
	
	//file4 
	eepromfs_open(&file4,"test04");
	eepromfs_read(&file4,strBuffRead,STRBUFFREAD_LEN,&fileSize);
	if(fileSize!=10)
	{
		printf("file1 read len err!\n");
	}
	for(i=0;i<fileSize;i++)
	{
		if(strBuffRead[i]!=strBuff[i])
		{
			printf("file1 read data[%d]=%d err!\n",i,strBuffRead[i]);
		}
	}
	for(i=0;i<STRBUFFREAD_LEN;i++)
	{
		strBuffRead[i]=0;		
	}
}

//测试 
void Test_EepromFs_Nor(CuTest* tc)
{
	int i=0;
	eeprom_handle_t *p_eeprom_simulate;
	eepromfs_result_t  eepromfs_result;
	char strBuff[]="1234567890abcdefghijklmnopqrstuvwxyz";
    #define STRBUFFREAD_LEN 128
	char strBuffRead[STRBUFFREAD_LEN];
	efs_size_t fileSize=0;
	p_eeprom_simulate=eeprom_simulate_init();
	if(eepromfs_init(p_eeprom_simulate,0)!=EFR_OK)
	{
        CuAssert(tc, "eepromfs_format_full ok", EFR_OK == eepromfs_format_full(0,EEPROM_SIMULATE_BUFF_LEN));
		CuAssert(tc, "eepromfs_init ok", EFR_OK == eepromfs_init(p_eeprom_simulate,0));
	}
	CuAssert(tc, "file new test01", EFR_OK == eepromfs_new(&file1,"test01",20));
	CuAssert(tc, "test01 write data", EFR_OK == eepromfs_write(&file1,strBuff,20));
	CuAssert(tc, "test01 close", EFR_OK == eepromfs_close(&file1));
    
	CuAssert(tc, "new file test02", EFR_OK == eepromfs_new(&file2,"test02",10));
	CuAssert(tc, "test02 write data", EFR_OK == eepromfs_write(&file2,strBuff,10));
	CuAssert(tc, "test02 close", EFR_OK == eepromfs_close(&file2));
    
	CuAssert(tc, "new file test03", EFR_OK == eepromfs_new(&file3,"test03",30));
	CuAssert(tc, "test03 write data", EFR_OK == eepromfs_write(&file3,strBuff,30));
	CuAssert(tc, "test03 close", EFR_OK == eepromfs_close(&file3));
    
	CuAssert(tc, "new file test04", EFR_OK == eepromfs_new(&file4,"test04",10));
	CuAssert(tc, "test04 write data", EFR_OK == eepromfs_write(&file4,strBuff,10));
	CuAssert(tc, "test04 close", EFR_OK == eepromfs_close(&file4));
    
	CuAssert(tc, "new file test05", EFR_OK == eepromfs_new(&file5,"test05",50));
	CuAssert(tc, "test05 write data", EFR_OK == eepromfs_write(&file5,strBuff,5));
	CuAssert(tc, "test05 close", EFR_OK == eepromfs_close(&file5));
	
	CuAssert(tc, "rename file new is not allowed", EFR_EXIST_FILENAME == eepromfs_new(&file6,"test01",20));

    //核对文件1数据
	CuAssert(tc, "open file test01", EFR_OK == eepromfs_open(&file1,"test01"));
	CuAssert(tc, "read file test01", EFR_OK == eepromfs_read(&file1,strBuffRead,STRBUFFREAD_LEN,&fileSize));
    CuAssert(tc, "fileSize check", fileSize==20);
	for(i=0;i<fileSize;i++)
	{
        CuAssert(tc, "test01 data check", strBuffRead[i]==strBuff[i]);
	}
	for(i=0;i<STRBUFFREAD_LEN;i++)
	{
		strBuffRead[i]=0;		
	}
	
    //核对文件2数据
    CuAssert(tc, "open file test02", EFR_OK == eepromfs_open(&file2,"test02"));
    CuAssert(tc, "read file test02", EFR_OK == eepromfs_read(&file2,strBuffRead,STRBUFFREAD_LEN,&fileSize));
    CuAssert(tc, "fileSize check", fileSize==10);
    for(i=0;i<fileSize;i++)
    {
        CuAssert(tc, "test02 data check", strBuffRead[i]==strBuff[i]);
    }
    for(i=0;i<STRBUFFREAD_LEN;i++)
    {
        strBuffRead[i]=0;       
    }

    //核对文件3数据
    CuAssert(tc, "open file test03", EFR_OK == eepromfs_open(&file3,"test03"));
    CuAssert(tc, "read file test03", EFR_OK == eepromfs_read(&file3,strBuffRead,STRBUFFREAD_LEN,&fileSize));
    CuAssert(tc, "fileSize check", fileSize==30);
    for(i=0;i<fileSize;i++)
    {
        CuAssert(tc, "test03 data check", strBuffRead[i]==strBuff[i]);
    }
    for(i=0;i<STRBUFFREAD_LEN;i++)
    {
        strBuffRead[i]=0;       
    }

    //核对文件4数据
    CuAssert(tc, "open file test04", EFR_OK == eepromfs_open(&file4,"test04"));
    CuAssert(tc, "read file test04", EFR_OK == eepromfs_read(&file4,strBuffRead,STRBUFFREAD_LEN,&fileSize));
    CuAssert(tc, "fileSize check", fileSize==10);
    for(i=0;i<fileSize;i++)
    {
        CuAssert(tc, "test04 data check", strBuffRead[i]==strBuff[i]);
    }
    for(i=0;i<STRBUFFREAD_LEN;i++)
    {
        strBuffRead[i]=0;       
    }

    //核对文件5数据
    CuAssert(tc, "open file test05", EFR_OK == eepromfs_open(&file5,"test05"));
    CuAssert(tc, "read file test05", EFR_OK == eepromfs_read(&file5,strBuffRead,STRBUFFREAD_LEN,&fileSize));
    CuAssert(tc, "fileSize check", fileSize==50);
    for(i=0;i<5;i++)
    {
        CuAssert(tc, "test05 data check", strBuffRead[i]==strBuff[i]);
    }
    for(i=0;i<STRBUFFREAD_LEN;i++)
    {
        strBuffRead[i]=0;       
    }

}

CuSuite* Tests_EepromFs_GetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, Test_EepromFs_Nor);
	return suite;
}

void RunTests_EepromFs(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, Tests_EepromFs_GetSuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("EepromFs %s\n", output->buffer);
}
