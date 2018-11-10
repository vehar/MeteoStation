#include "stdio.h"

#define DEBUG 1

#define DEBUGMSG(f_, ...) do{				\
		if(DEBUG)												\
		{																\
			printf((f_),##__VA_ARGS__);		\
		}																\
	}while(0)

#define DEBUGFMSG(text) DEBUGMSG("%s() >> %s\r\n", __FUNCTION__, text)
#define DEBUGFMSG_START() DEBUGFMSG("Start")
