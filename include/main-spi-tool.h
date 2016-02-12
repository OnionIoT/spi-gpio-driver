#ifndef _MAIN_SPI_TOOL_H_
#define _MAIN_SPI_TOOL_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <onion-debug.h>

#include <onion-spi.h>


#define SPI_TOOL_COMMAND_READ				"read"
#define SPI_TOOL_COMMAND_WRITE				"write"
#define SPI_TOOL_COMMAND_SETUP_DEVICE		"setup"


// type definitions
typedef enum e_SpiToolMode {
	SPI_TOOL_MODE_NONE 		= 0,
	SPI_TOOL_MODE_READ,
	SPI_TOOL_MODE_WRITE,
	SPI_TOOL_MODE_SETUP_DEVICE,
	SPI_TOOL_NUM_MODES
} eSpiToolMode;

/*
#ifdef __cplusplus
extern "C"{
#endif 







#ifdef __cplusplus
}
#endif */
#endif // _MAIN_SPI_TOOL_H_ 