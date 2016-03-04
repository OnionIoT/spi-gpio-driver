#ifndef _ONION_SPI_H_
#define _ONION_SPI_H_

#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#ifndef __APPLE__
#include <linux/types.h>
#include <linux/spi/spidev.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>


#include <onion-debug.h>


#define SPI_DEV_PATH				"/dev/spidev%d.%d"
#define SPI_PRINT_BANNER			"onion-spi::"

#define SPI_DEV_INSMOD_TEMPLATE 	"insmod spi-gpio-custom bus%d=%d,%d,%d,%d,%d,%d,%d"

#define SPI_BUFFER_SIZE				32

#define SPI_DEFAULT_SPEED			100000
#define SPI_DEFAULT_BITS_PER_WORD	0 				// corresponds to 8 bits per word
#define SPI_DEFAULT_MODE 			SPI_MODE_0
#define SPI_DEFAULT_MODE_BITS		(SPI_MODE_0 | SPI_TX_DUAL | SPI_RX_DUAL)

#define SPI_DEFAULT_GPIO_SCK		6
#define SPI_DEFAULT_GPIO_MOSI		18
#define SPI_DEFAULT_GPIO_MISO		1
#define SPI_DEFAULT_GPIO_CS			7

// type definitions
struct spiParams {
	int 	busNum;
	int 	deviceId;

	int		speedInHz;
	int 	delayInUs;
	int 	bitsPerWord;

	int 	mode;
	int 	modeBits;

	int 	sckGpio;
	int 	mosiGpio;
	int 	misoGpio;
	int 	csGpio;
};

// for debugging
#ifndef __APPLE__
	#define SPI_ENABLED		1
#endif


#ifdef __cplusplus
extern "C"{
#endif 





//// spi functions
// initialize the spiParams structure to default values
void 	spiParamInit			(struct spiParams *params);

// check if an SPI device is mapped sysfs
int 	spiCheckDevice 			(int busNum, int devId, int printSeverity);

// register an SPI device with sysfs
int 	spiRegisterDevice 		(struct spiParams *params);
// setup paramaters of the sysfs SPI interface
int 	spiSetupDevice 			(struct spiParams *params);

// transfer data through the SPI interface
int 	spiTransfer				(struct spiParams *params, uint8_t *txBuffer, uint8_t *rxBuffer, int bytes);


int 	spiWrite				(struct spiParams *params, int addr, uint8_t *wrBuffer, int bytes);
int 	spiRead					(struct spiParams *params, int addr, uint8_t *rdBuffer, int bytes);


#ifdef __cplusplus
}
#endif 
#endif // _ONION_SPI_H_ 