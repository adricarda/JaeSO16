#include <debug.h>

/* function used in debug phase as breakpoint */
void debugBK () {};
void d() {};
void debugBKtwo (unsigned int b) {
	unsigned int *ramValue = (unsigned int*) 0x00011114;
	*ramValue = b;
};
void debugBK3 (unsigned int b) {
	unsigned int *ramValue = (unsigned int*) 0x00011110;
	*ramValue = b;
};



