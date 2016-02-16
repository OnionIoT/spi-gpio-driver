#ifndef _MAIN_SPI_TOOL_H_
#define _MAIN_SPI_TOOL_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>

#include <onion-debug.h>

#include <onion-spi.h>


#define SPI_TOOL_COMMAND_READ				"read"
#define SPI_TOOL_COMMAND_WRITE				"write"
#define SPI_TOOL_COMMAND_SETUP_DEVICE		"setup"


// type definitions
typedef enum e_SpiToolMode {
	SPI_TOOL_MODE_NONE 			= 0x00,
	SPI_TOOL_MODE_READ			= 0x01,
	SPI_TOOL_MODE_WRITE			= 0x02,
	SPI_TOOL_MODE_SETUP_DEVICE	= 0x10,
	SPI_TOOL_NUM_MODES			= 4
} eSpiToolMode;

/*
#ifdef __cplusplus
extern "C"{
#endif 







#ifdef __cplusplus
}
#endif */
#endif // _MAIN_SPI_TOOL_H_ 