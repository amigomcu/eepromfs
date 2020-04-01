#include <stdio.h>
#include "./test/test_EepromFs.h"
#include "./test/test_DEepromFs.h"

int main(int argc, char *argv[])
{
	RunTests_EepromFs();
	RunTests_DEepromFs();
	return 0;
}

