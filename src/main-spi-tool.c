#include <main-spi-tool.h>

void usage(const char* progName) 
{
	printf("device-client: interface with Onion cloud device-server\n");
}
/*
int parseOptions(int argc, char** argv)
{
	int 	status	= EXIT_SUCCESS;
	int 	ch;

	static const struct option lopts[] = {
		{ "bus",		1, 0, 'b' },
		{ "device",		1, 0, 'b' },
		{ "speed",		1, 0, 's' },
		{ "delay",		1, 0, 'd' },
		{ "bpw",		1, 0, 'B' },
		{ "loop",		0, 0, 'l' },
		{ "cpha",		0, 0, 'H' },
		{ "cpol",		0, 0, 'O' },
		{ "lsb",		0, 0, 'L' },
		{ "cs-high",	0, 0, 'C' },
		{ "3wire",		0, 0, '3' },
		{ "no-cs",		0, 0, 'N' },
		{ "ready",		0, 0, 'R' },
		{ "dual",		0, 0, '2' },
		{ "verbose",	0, 0, 'v' },
		{ "quad",		0, 0, '4' },
		{ NULL, 0, 0, 0 },
	};

	return 	status;
}*/

int main(int argc, char** argv)
{
	const char 	*progname;
	int 		status;
	int 		ch;
	int 		verbose, debug, mode;

	int 		addr;
	int 		value;
	int 		size;
	uint8_t 	*txBuffer;
	uint8_t 	*rxBuffer;

	struct spiParams	params;


	// set defaults
	verbose 		= ONION_VERBOSITY_NORMAL;
	debug 			= 0;
	mode 			= SPI_TOOL_MODE_NONE;

	spiParamInit(&params);

	// save the program name
	progname 		= argv[0];	


	//// parse the option arguments
	while ((ch = getopt(argc, argv, "vqhb:d:")) != -1) {
		switch (ch) {
			case 'v':
				// verbose output
				verbose++;
				break;
			case 'q':
				// quiet output
				verbose = ONION_SEVERITY_FATAL;
				break;
			case 'b':
				// set the bus number
				params.busNum 	= atoi(optarg);
			case 'd':
				// set the device id
				params.deviceId	= atoi(optarg);
				break;
			default:
				usage(progname);
				return 0;
		}
	}

	// advance past the option arguments
	argc 	-= optind;
	argv	+= optind;

	//// parse the real arguments
	if (argc >= 1 ) {
		// argument1 - find the mode
		if (strcmp(argv[0], SPI_TOOL_COMMAND_READ) == 0) {
			mode 	= SPI_TOOL_MODE_READ;
		}
		else if (strcmp(argv[0], SPI_TOOL_COMMAND_WRITE) == 0) {
			mode 	= SPI_TOOL_MODE_WRITE;
		}
		else if (strcmp(argv[0], SPI_TOOL_COMMAND_SETUP_DEVICE) == 0) {
			mode 	= SPI_TOOL_MODE_SETUP_DEVICE;
		}

		// read the address
		if 	(	argc >= 2 &&
				(mode == SPI_TOOL_MODE_READ ||
				 mode == SPI_TOOL_MODE_WRITE)
			)
		{
			sscanf (argv[1],"0x%02x", &addr);
		}

		// write mode: read the value to write
		if 	(	argc >= 3 &&
				mode == SPI_TOOL_MODE_WRITE
			)
		{
			sscanf (argv[2],"0x%02x", &value);
		}

		// read device setup arguments
		// LAZAR: implement this
	}



	///////////////////
	//* initialization *//
	// set verbosity
	onionSetVerbosity(verbose);


	//* program *//
	if (mode & SPI_TOOL_MODE_SETUP_DEVICE) {
		status 		= spiRegisterDevice(&params);
		status		= spiInitDevice(&params);
	}
	else if (mode & SPI_TOOL_MODE_READ) {
		// make a call
		size 		= 1;
		txBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);
		rxBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);

		*txBuffer 	= (uint8_t)addr;

		status 	= spiTransfer(&params, txBuffer, rxBuffer, size);
		onionPrint(ONION_SEVERITY_INFO, 	"> SPI Read from addr 0x%02x: 0x%02x\n", *txBuffer, *rxBuffer);
		onionPrint(ONION_SEVERITY_DEBUG, 	"    spiTransfer status is: %d\n", status);

		// clean-up
		free(txBuffer);
		free(rxBuffer);
	}
	else {
		onionPrint(ONION_SEVERITY_FATAL, 	"ERROR: Invalid command!\n");
	}
	//status 	= spi_readByte (busNum, devId, addr, &value);
	//onionPrint(ONION_SEVERITY_INFO, "spi_readByte status is: %d, read from addr 0x%02x: 0x%02x\n", status, addr, value);

	//* clean-up *//
	
	
	return 0;
}