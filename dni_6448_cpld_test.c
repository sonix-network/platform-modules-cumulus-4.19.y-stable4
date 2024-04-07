#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "dni_6448_cpld.h"

/*
**
** Copyright 2012 Cumulus Networks, Inc.
** All rights reserved.
**
** Crufty throw away test for the DNI 6448 sysfs driver.
**
** Build: ../build/crosstools/bin/powerpc-gcc -Wall -o cpld_test dni_6448_cpld_test.c
**
*/

void static dump( uint32_t val)
{
	uint16_t p;

	printf("Port     Mode   TX en   Present   RX Los\n");
	printf("=====+========+=======+=========+========\n");
	for ( p = 45; p < 53; p++) {
		printf("%d     %6s   %d       %d         %d\n", p,
		       dni6448_get_fiber_mode( val, p) ?  "fiber" : "copper",
		       dni6448_get_tx_enable(val, p),
		       dni6448_get_sfp_present(val, p),
		       dni6448_get_rx_los(val, p));
	}

}

int main (int argc, char* argv[])
{
	uint32_t val;
	FILE* f;
	char* dev = "/sys/devices/e0005000.localbus/fa000000.cpld/port_raw_ctrl";

	f = fopen(dev, "r");
	if ( f == NULL) {
		printf("Failed to open %s\n", dev);
		return -1;
	}

	val = 0x0;

	if ( fread( &val, sizeof(val), 1, f) != 1) {
		printf("Failed to read 1 item from %s\n", dev);
		return -1;
	}
	fclose(f);
	printf("Read 0x%08x\n", val);

	dump(val);

	printf("Inverting val....\n");
	val = ~val;

	f = fopen(dev, "w");
	if ( f == NULL) {
		printf("Failed to open for writing %s\n", dev);
		return -1;
	}

	if ( fwrite( &val, sizeof(val), 1, f) != 1) {
		printf("Failed to write 1 item from %s\n", dev);
		return -1;
	}
	fclose(f);

	f = fopen(dev, "r");
	if ( f == NULL) {
		printf("Failed to open %s\n", dev);
		return -1;
	}

	val = 0x0;

	if ( fread( &val, sizeof(val), 1, f) != 1) {
		printf("Failed to read 1 item from %s\n", dev);
		return -1;
	}
	fclose(f);
	printf("Read 0x%08x\n", val);

	dump(val);

	return 0;
}
