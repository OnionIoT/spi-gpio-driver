#include <main-spi-tool.h>

int 	verbose;

void usage(const char* progName) 
{
	printf("spi-tool: interface devices using the SPI protocol\n");
}

int parseOptions(int argc, char** argv, struct spiParams *params)
{
	const char 	*progname;
	int 	status	= EXIT_SUCCESS;
	int 	ch;

	int 	option_index = 0;

	static const struct option lopts[] = {
		{ "verbose",	no_argument, 		0, 'v' },
		{ "quiet",		no_argument, 		0, 'q' },

		{ "bus",		required_argument, 	0, 'b' },
		{ "device",		required_argument, 	0, 'd' },
		{ "frequency",	required_argument, 	0, 's' },
		{ "delay",		required_argument, 	0, 'D' },
		{ "bpw",		required_argument, 	0, 'B' },

		{ "3wire",		no_argument, 		0, '3' },
		{ "no-cs",		no_argument, 		0, 'N' },
		
		{ "sck",		required_argument, 	0, 'S' },
		{ "mosi",		required_argument, 	0, 'O' },
		{ "miso",		required_argument, 	0, 'I' },
		{ "cs",			required_argument, 	0, 'C' },

/*		{ "loop",		0, 0, 'l' },
		{ "cpha",		0, 0, 'H' },
		{ "cpol",		0, 0, 'O' },
		{ "lsb",		0, 0, 'L' },
		{ "cs-high",	0, 0, 'C' },
		{ "ready",		0, 0, 'R' },
		{ "dual",		0, 0, '2' },
		{ "quad",		0, 0, '4' },*/

		{ NULL, 0, 0, 0 },
	};

	// save the program name
	progname 		= argv[0];

	// parse the option arguments
	while( (ch = getopt_long (argc, argv, "vqhb:d:s:D:B:S:O:I:C", lopts, &option_index)) != -1) {
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
				params->busNum 		= atoi(optarg);
				break;
			case 'd':
				// set the device id
				params->deviceId	= atoi(optarg);
				break;
			case 's':
				// set the transmission speed
				params->speedInHz	= atoi(optarg);
				break;
			case 'D':
				// set the delay
				params->delayInUs	= atoi(optarg);
				break;
			case 'B':
				// set the bits per word
				params->bitsPerWord	= atoi(optarg);
				break;

			case '3':
				// set the mode to 3-wire
				params->modeBits	|= SPI_3WIRE;
				break;
			case 'N':
				// set the mode to no CS pin
				params->modeBits	|= SPI_NO_CS;
				break;

			case 'S':
				// set the SCK gpio
				params->sckGpio		= atoi(optarg);
				break;
			case 'O':
				// set the MOSI gpio
				params->mosiGpio	= atoi(optarg);
				break;
			case 'I':
				// set the MISO gpio
				params->misoGpio	= atoi(optarg);
				break;
			case 'C':
				// set the CS gpio
				params->csGpio		= atoi(optarg);
				break;

			default:
				usage(progname);
				return 0;
		}
	}

	return 	status;
}

int main(int argc, char** argv)
{
	const char 	*progname;
	int 		status;
	int 		ch;
	int 		debug, mode;

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
	addr 			= -1;
	value 			= -1;

	spiParamInit(&params);

	// save the program name
	progname 		= argv[0];	


	// parse the option arguments
	parseOptions(argc, argv, &params);

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
	}

	// check the arguments
	if 	(	addr < 0 &&
			(mode == SPI_TOOL_MODE_READ ||
			 mode == SPI_TOOL_MODE_WRITE)
		)
	{
		onionPrint(ONION_SEVERITY_FATAL, "> ERROR: address argument required!\n\n");
		usage(progname);
		return 0;
	}
	if 	(	value < 0 &&
			mode == SPI_TOOL_MODE_WRITE
		)
	{
		onionPrint(ONION_SEVERITY_FATAL, "> ERROR: value argument required!\n\n");
		usage(progname);
		return 0;
	}


	///////////////////
	//* initialization *//
	// set verbosity
	onionSetVerbosity(verbose);


	//* program *//
	if (mode & SPI_TOOL_MODE_SETUP_DEVICE) {
		status 		= spiRegisterDevice(&params);
		if (status == EXIT_SUCCESS) {
			status		= spiInitDevice(&params);
		}
		else {
			onionPrint(ONION_SEVERITY_FATAL, "> ERROR: could not register SPI sysfs device!\n");
		}
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


		/*size 		= 1;
		rxBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);

		status 	= spiRead(&params, addr, rxBuffer, size);
		onionPrint(ONION_SEVERITY_INFO, 	"> SPI Read from addr 0x%02x: 0x%02x\n", addr, *rxBuffer);
		onionPrint(ONION_SEVERITY_DEBUG, 	"    spiRead status is: %d\n", status);

		// clean-up
		free(rxBuffer);*/
	}
	else if (mode & SPI_TOOL_MODE_WRITE) {
		// make a call
		size 		= 2;
		txBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);
		rxBuffer 	= (uint8_t*)malloc(sizeof(uint8_t) * size);

		txBuffer[0] = (uint8_t)addr;
		txBuffer[1] = (uint8_t)value;

		onionPrint(ONION_SEVERITY_INFO, 	"> SPI Write to addr 0x%02x: 0x%02x\n", txBuffer[0], txBuffer[1] );
		status 	= spiTransfer(&params, txBuffer, rxBuffer, size);
		onionPrint(ONION_SEVERITY_DEBUG, 	"    spiTransfer status is: %d\n", status);

		// clean-up
		free(txBuffer);
		free(rxBuffer);

		/*size 		= 1;
		txBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);

		*txBuffer 	= (uint8_t)value;

		onionPrint(ONION_SEVERITY_INFO, 	"> SPI Write to addr 0x%02x: 0x%02x\n", addr, *txBuffer );
		status 	= spiWrite(&params, addr, txBuffer, size);
		onionPrint(ONION_SEVERITY_DEBUG, 	"    spiTransfer status is: %d\n", status);

		// clean-up
		free(txBuffer);*/
	}
	else {
		onionPrint(ONION_SEVERITY_FATAL, 	"ERROR: Invalid command!\n");
	}
	//status 	= spi_readByte (busNum, devId, addr, &value);
	//onionPrint(ONION_SEVERITY_INFO, "spi_readByte status is: %d, read from addr 0x%02x: 0x%02x\n", status, addr, value);

	//* clean-up *//
	
	
	return 0;
}