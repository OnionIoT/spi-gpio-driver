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

#define SPI_BUFFER_SIZE				32

#define SPI_DEFAULT_SPEED			100000
#define SPI_DEFAULT_BITS_PER_WORD	0 				// corresponds to 8 bits per word
#define SPI_DEFAULT_MODE			(SPI_MODE_0 | SPI_TX_DUAL | SPI_RX_DUAL)


// type definitions
struct spiParams {
	int 	busNum;
	int 	deviceId;

	int		speedInHz;
	int 	delayInUs;
	int 	bitsPerWord;

	int 	mode;
};

// for debugging
#ifndef __APPLE__
	#define SPI_ENABLED		1
#endif


#ifdef __cplusplus
extern "C"{
#endif 





// spi functions
void 	spiParamInit			(struct spiParams *params);

int 	spiRegisterDevice 		(struct spiParams *params);
int 	spiInitDevice 			(struct spiParams *params);

int 	spiTransfer				(struct spiParams *params, uint8_t *txBuffer, uint8_t *rxBuffer, int bytes);


//int 	spi_readByte 			(int busNum, int devId, int addr, int *val);

	/*
int 	spi_writeBuffer			(int devNum, int devAddr, int addr, uint8_t *buffer, int size);
int 	spi_write	 			(int devNum, int devAddr, int addr, int val);
int 	spi_writeBytes 			(int devNum, int devAddr, int addr, int val, int numBytes);

int 	spi_read 				(int devNum, int devAddr, int addr, uint8_t *buffer, int numBytes);
int 	spi_readByte 			(int devNum, int devAddr, int addr, int *val);
*/

#ifdef __cplusplus
}
#endif 
#endif // _ONION_SPI_H_ 