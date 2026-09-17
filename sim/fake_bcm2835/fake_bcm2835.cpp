#include "bcm2835.h"

int bcm2835_init(void) { return 1; }
int bcm2835_close(void) { return 1; }
int bcm2835_spi_begin(void) { return 1; }
void bcm2835_spi_end(void) {}
void bcm2835_gpio_fsel(unsigned char, unsigned char) {}
void bcm2835_gpio_write(unsigned char, unsigned char) {}
void bcm2835_spi_setClockDivider(unsigned short) {}
unsigned char bcm2835_spi_transfer(unsigned char) { return 0; }
void bcm2835_spi_transfernb(char*, char*, unsigned int) {}
