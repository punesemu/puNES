#include <stdio.h>
#include <stdlib.h>

/* test if 7-zip uses 86 filter on this machine */


#include "CpuArch.h"

int main(int argc,char *argv[])
{
	int ret = 0;

#ifdef MY_CPU_X86_OR_AMD64
	ret = 1;
#endif


	return ret;
}

