#include "util.h"

char *format_uid(uint8_t uid[static 7]) {
	static char argbuf[15];
	
	for(int i=0; i < 7; i++) {
		snprintf(argbuf + 2*i, 3, "%02X", uid[i]);
	}

	return argbuf;
}
