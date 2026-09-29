// A real, linkable no-op stand-in for libbcm2835's public API -- only the
// handful of functions lib/core/Cube.h actually calls. Lets Cube.h (and
// everything that includes it, unmodified) be built and run on a normal
// dev machine for the simulator, instead of the real Raspberry Pi. Not
// used anywhere near the real hardware build (see Makefile's `sim` vs
// `build` targets).
#pragma once

#define HIGH 1
#define LOW 0
#define RPI_GPIO_P1_11 17
#define RPI_GPIO_P1_15 22
#define BCM2835_GPIO_FSEL_OUTP 1
#define BCM2835_SPI_CLOCK_DIVIDER_16 16

int bcm2835_init(void);
int bcm2835_close(void);
int bcm2835_spi_begin(void);
void bcm2835_spi_end(void);
void bcm2835_gpio_fsel(unsigned char pin, unsigned char mode);
void bcm2835_gpio_write(unsigned char pin, unsigned char on);
void bcm2835_spi_setClockDivider(unsigned short divider);
unsigned char bcm2835_spi_transfer(unsigned char value);
void bcm2835_spi_transfernb(char* tbuf, char* rbuf, unsigned int len);
