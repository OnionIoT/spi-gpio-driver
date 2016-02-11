#include <onion-spi.h>

void usage(const char* progName) 
{
	printf("device-client: interface with Onion cloud device-server\n");
}

int main(int argc, char** argv)
{
	const char 	*progname;
	int 		status;
	int 		ch;
	int 		verbose, debug;

	int			busNum, devId;
	int 		addr, value;


	// set defaults
	verbose 		= ONION_VERBOSITY_NORMAL;
	debug 			= 0;

	// save the program name
	progname 		= argv[0];	


	//// parse the option arguments
	while ((ch = getopt(argc, argv, "vqdh")) != -1) {
		switch (ch) {
		case 'v':
			// verbose output
			verbose++;
			break;
		case 'q':
			// quiet output
			verbose = ONION_SEVERITY_FATAL;
			break;
		case 'd':
			// debug mode
			debug 	= 1;
			break;
		default:
			usage(progname);
			return 0;
		}
	}

	// advance past the option arguments
	//argc 	-= optind;
	//argv	+= optind;



	///////////////////
	//* initialization *//
	// set verbosity
	onionSetVerbosity(verbose);


	//* program *//
	// make a call
	busNum 	= 0;
	devId 	= 0;

	addr 	= 0x37;

	status 	= spi_readByte (busNum, devId, addr, &value);
	onionPrint(ONION_SEVERITY_INFO, "spi_readByte status is: %d, read from addr 0x%02x: 0x%02x", status, addr, value);

	//* clean-up *//
	
	
	return 0;
}