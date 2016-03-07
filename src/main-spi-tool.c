#include <main-spi-tool.h>

int 	verbose;

void usage(const char* progName) 
{
	onionPrint(ONION_SEVERITY_FATAL, "\n");
	onionPrint(ONION_SEVERITY_FATAL, "spi-tool: interface devices using the SPI protocol\n");
	onionPrint(ONION_SEVERITY_FATAL, "\n");

	onionPrint(ONION_SEVERITY_FATAL, "Usage: spi-tool -b <bus number> -d <device ID> read <address>\n");
	onionPrint(ONION_SEVERITY_FATAL, "  Perform a read through the SPI protocol\n");
	onionPrint(ONION_SEVERITY_FATAL, "\n");
	
	onionPrint(ONION_SEVERITY_FATAL, "Usage: spi-tool -b <bus number> -d <device ID> write <address> <value>\n");
	onionPrint(ONION_SEVERITY_FATAL, "  Perform a write through the SPI protocol\n");
	onionPrint(ONION_SEVERITY_FATAL, "\n");

	onionPrint(ONION_SEVERITY_FATAL, "Usage: spi-tool -b <bus number> -d <device ID> [options] setup\n");
	onionPrint(ONION_SEVERITY_FATAL, "  Setup a sysfs SPI handle, initialize SPI parameters \n");
	onionPrint(ONION_SEVERITY_FATAL, "\n");
	onionPrint(ONION_SEVERITY_FATAL, "Options:\n");
	onionPrint(ONION_SEVERITY_FATAL, "  --frequency <Hz>         Set max SPI frequency\n");
	onionPrint(ONION_SEVERITY_FATAL, "  --delay <us>             Set delay after the last bit transfered before optionally deselecting the device before the next transfer.\n");
	onionPrint(ONION_SEVERITY_FATAL, "  --bpw <number>           Set number of bits per word\n");
	
	onionPrint(ONION_SEVERITY_FATAL, "  --sck <gpio>             Set GPIO for SPI SCK signal\n");
	onionPrint(ONION_SEVERITY_FATAL, "  --mosi <gpio>            Set GPIO for SPI MOSI signal\n");
	onionPrint(ONION_SEVERITY_FATAL, "  --miso <gpio>            Set GPIO for SPI MISO signal\n");
	onionPrint(ONION_SEVERITY_FATAL, "  --cs <gpio>              Set GPIO for SPI CS signal\n");

	onionPrint(ONION_SEVERITY_FATAL, "  --3wire                  SI/SO signals shared\n");
	onionPrint(ONION_SEVERITY_FATAL, "  --no-cs                  No chip select signal\n");
	onionPrint(ONION_SEVERITY_FATAL, "  --cs-high                Set chip select to active high\n");
	onionPrint(ONION_SEVERITY_FATAL, "  --lsb                    Transmit Least Significant Bit first\n");

	onionPrint(ONION_SEVERITY_FATAL, "\n");
}

int parseOptions(int argc, char** argv, struct spiParams *params)
{
	const char 	*progname;
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
		{ "mode",		required_argument, 	0, 'm' },

		{ "3wire",		no_argument, 		0, '3' },
		{ "no-cs",		no_argument, 		0, 'N' },
		{ "cs-high",	no_argument, 		0, 'H' },
		{ "lsb",		no_argument, 		0, 'L' },
		{ "loop",		no_argument, 		0, 'l' },
		
		{ "sck",		required_argument, 	0, 'S' },
		{ "mosi",		required_argument, 	0, 'O' },
		{ "miso",		required_argument, 	0, 'I' },
		{ "cs",			required_argument, 	0, 'C' },

		{ NULL, 0, 0, 0 },	// sentinel
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
			case 'm':
				// set the SPI Mode
				params->mode		= atoi(optarg);
				break;

			case '3':
				// set the mode to 3-wire
				params->modeBits	|= SPI_3WIRE;
				break;
			case 'N':
				// set the mode to no CS pin
				params->modeBits	|= SPI_NO_CS;
				break;
			case 'H':
				// set the mode to CS active-high
				params->modeBits	|= SPI_CS_HIGH;
				break;
			case 'L':
				// set the mode to LSB first
				params->modeBits	|= SPI_LSB_FIRST;
				break;
			case 'l':
				// set the mode to loopback
				params->modeBits	|= SPI_LOOP;
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
				return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
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
	if( parseOptions(argc, argv, &params) == EXIT_FAILURE) {
		return 0;
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
	}

	// check the arguments
	if (mode == SPI_TOOL_MODE_NONE) {
		onionPrint(ONION_SEVERITY_FATAL, "> ERROR: invalid command!\n\n");
	}
	if 	(	addr < 0 &&
			(mode == SPI_TOOL_MODE_READ ||
			 mode == SPI_TOOL_MODE_WRITE)
		)
	{
		onionPrint(ONION_SEVERITY_FATAL, "> ERROR: address argument required!\n\n");
		mode 	= SPI_TOOL_MODE_NONE;
	}
	if 	(	value < 0 &&
			mode == SPI_TOOL_MODE_WRITE
		)
	{
		onionPrint(ONION_SEVERITY_FATAL, "> ERROR: value argument required!\n\n");
		mode 	= SPI_TOOL_MODE_NONE;
	}

	if (mode == SPI_TOOL_MODE_NONE) {
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
			status		= spiSetupDevice(&params);
		}
		else {
			onionPrint(ONION_SEVERITY_FATAL, "> ERROR: could not register SPI sysfs device!\n");
		}
	}
	else if (mode & SPI_TOOL_MODE_READ) {
		// make a transfer
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
	else if (mode & SPI_TOOL_MODE_WRITE) {
		// make a transfer
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
	}
	else {
		onionPrint(ONION_SEVERITY_FATAL, 	"ERROR: Invalid command!\n");
	}
	

	//* clean-up *//
	
	
	return 0;
}