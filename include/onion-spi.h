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


#define SPI_DEV_PATH		"/dev/spidev%d.%d"
#define SPI_PRINT_BANNER	"onion-spi::"

#define SPI_BUFFER_SIZE		32


// for debugging
#ifndef __APPLE__
	#define SPI_ENABLED		1
#endif


#ifdef __cplusplus
extern "C"{
#endif 





// spi functions
int 	spiTransfer				(int busNum, int devId, uint8_t *txBuffer, uint8_t *rxBuffer, int bytes);


int 	spi_readByte 			(int busNum, int devId, int addr, int *val);

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