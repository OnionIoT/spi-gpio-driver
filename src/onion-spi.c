#include <onion-spi.h>

// helper function prototypes
int 	_spiGetFd				(int busNum, int devId, int *devHandle);
int 	_spiReleaseFd			(int devHandle);

int 	_spi_setDevice 			(int devHandle, int addr);
int 	_spi_setDevice10bit 	(int devHandle, int addr);

int 	_spi_writeBuffer		(int devNum, int devAddr, uint8_t *buffer, int size);

static void hex_dump(const void *src, size_t length, size_t line_size, char *prefix);


//// spi functions
// initialize the parameter structure
void spiParamInit(struct spiParams *params)
{
	params->busNum			= 0;
	params->deviceId		= 0;

	params->speedInHz		= SPI_DEFAULT_SPEED;	
	params->delayInUs		= 0;
	params->bitsPerWord		= SPI_DEFAULT_BITS_PER_WORD;
}

// perform a transfer
int spiTransfer(struct spiParams *params, uint8_t *txBuffer, uint8_t *rxBuffer, int bytes)
{
	int 	status;
	int 	fd, res;
	struct 	spi_ioc_transfer xfer;

	res 	= EXIT_FAILURE;

	// open the file handle
	status 	= _spiGetFd(params->busNum, params->deviceId, &fd);

	// attempt the SPI transfter
	if (status == EXIT_SUCCESS) {
		
		memset(&xfer, 0, sizeof(xfer));
		xfer.tx_buf 		= (unsigned long)txBuffer;
		xfer.rx_buf 		= (unsigned long)rxBuffer;
		xfer.len 			= bytes;
		xfer.speed_hz 		= params->speedInHz;
        xfer.delay_usecs 	= params->delayInUs;
        xfer.bits_per_word 	= params->bitsPerWord;
        xfer.cs_change 		= 0;

		onionPrint(ONION_SEVERITY_DEBUG, "%s Trasferring 0x%02x, expecting %d bytes\n", SPI_PRINT_BANNER, *txBuffer, xfer.len);

		// make the transfer
		res = ioctl(fd, SPI_IOC_MESSAGE(bytes), &xfer);

		// check the return
		// LAZAR: implement

		onionPrint(ONION_SEVERITY_DEBUG, "   Received: 0x%02x, ioctl status: %d\n", *rxBuffer, res);

		if (onionGetVerbosity() > ONION_SEVERITY_DEBUG) {
			hex_dump(txBuffer, bytes, 32, "TX");
			hex_dump(rxBuffer, bytes, 32, "RX");
		}

		// clean-up
		status 	= _spiReleaseFd(fd);
	}

	return res;
}

/*
int spi_readByte (int busNum, int devId, int addr, int *val)
{
	int 	status;
	uint8_t	addr8, value8;

	addr8	= (uint8_t)addr;


	status 	= spiTransfer(busNum, devId, &addr8, &value8, 1);
	*val 	= (int)value8;

	return 	status;
}
*/

//// helper functions
// get a handle to the device
int _spiGetFd(int busNum, int devId, int *devHandle)
{
	int 	status;
	char 	pathname[255];

	// define the path to open
	status = snprintf(pathname, sizeof(pathname), SPI_DEV_PATH, devId, busNum);

	// check the filename
	if (status < 0 || status >= sizeof(pathname)) {
		// add errno
		return EXIT_FAILURE;
	}

	// create a file descriptor for the I2C bus
	if ( (*devHandle = open(pathname, O_RDWR)) < 0) {
		onionPrint(ONION_SEVERITY_FATAL, "ERROR: could not open sysfs device '%s'\n", pathname);
		return 	EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// release the device file handle
int _spiReleaseFd(int devHandle)
{
	if ( close(devHandle) < 0 ) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static void hex_dump(const void *src, size_t length, size_t line_size, char *prefix)
{
        int i = 0;
        const unsigned char *address = src;
        const unsigned char *line = address;
        unsigned char c;

        printf("%s | ", prefix);
        while (length-- > 0) {
                printf("%02X ", *address++);
                if (!(++i % line_size) || (length == 0 && i % line_size)) {
                        if (length == 0) {
                                while (i++ % line_size)
                                        printf("__ ");
                        }
                        printf(" | ");  /* right close */
                        while (line < address) {
                                c = *line++;
                                printf("%c", (c < 33 || c == 255) ? 0x2E : c);
                        }
                        printf("\n");
                        if (length > 0)
                                printf("%s | ", prefix);
                }
        }
}
