#ifndef DEBUGMEM_H_
#define DEBUGMEM_H_

#include <stdlib.h>
#include <stdio.h>
#include "Debug.h"
/*
inline static void DebugMem(const char *tag, unsigned char * buf, int len) {
	unsigned char * curaddr;
	int i;
	int left;
	int sector = 0;
	left = len;
	my_printf("\n", tag);
	my_printf("%s", tag);
	char msgbuf[1024];
	curaddr = buf;
	while (left > 0) {

		if (left < 16) {
			memset(msgbuf, 0, sizeof(msgbuf));
			int cnt = sprintf(msgbuf, "\n%08Xh  ", (unsigned int) curaddr);
			for (i = 0; i < left; i++) {
				cnt += sprintf(msgbuf + cnt, "%02X ", *(curaddr + i));
			}
			for (i = left; i < 16; i++)
				cnt += sprintf(msgbuf + cnt, "   ");

			for (i = 0; i < left; i++) {
				if ((*(curaddr + i) >= '!') && (*(curaddr + i) <= '~'))
					cnt += sprintf(msgbuf + cnt, "%c", *(curaddr + i));
				else
					cnt += sprintf(msgbuf + cnt, ".");
			}
			left -= 16;
			my_printf("%s", msgbuf);
		} else {
			memset(msgbuf, 0, sizeof(msgbuf));
			int cnt = sprintf(msgbuf, "\n%08Xh  ", (unsigned int) curaddr);
			for (i = 0; i < 16; i++) {
				cnt += sprintf(msgbuf + cnt, "%02X ", *(curaddr + i));
			}
			for (i = 0; i < 16; i++) {
				if ((*(curaddr + i) >= '!') && (*(curaddr + i) <= '~'))
					cnt += sprintf(msgbuf + cnt, "%c", *(curaddr + i));
				else
					cnt += sprintf(msgbuf + cnt, ".");
			}
			my_printf("%s", msgbuf);

		}

		left -= 16;
		sector += 16;
		curaddr += 16;
		if ((sector % 512) == 0) {
			my_printf(" \n", sector / 512);
		}
	}
	my_printf("\n\n");
}
*/
#endif
