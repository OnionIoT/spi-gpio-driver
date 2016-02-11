#include <onion-spi.h>

// helper function prototypes
int 	_spi_getFd				(int busNum, int devId, int *devHandle);
int 	_spi_releaseFd			(int devHandle);

int 	_spi_setDevice 			(int devHandle, int addr);
int 	_spi_setDevice10bit 	(int devHandle, int addr);

int 	_spi_writeBuffer		(int devNum, int devAddr, uint8_t *buffer, int size);


// spi functions
int spiTransfer(int busNum, int devId, uint8_t *txBuffer, uint8_t *rxBuffer, int bytes)
{
	int 	status;
	int 	fd, res;
	struct 	spi_ioc_transfer xfer;

	res 	= EXIT_FAILURE;

	// open the file handle
	status 	= _spi_getFd(busNum, devId, &fd);

	// attempt the SPI transfter
	if (status == EXIT_SUCCESS) {
		
		memset(&xfer, 0, sizeof(xfer));
		xfer.tx_buf = (unsigned long)*txBuffer;
		xfer.rx_buf = (unsigned long)*rxBuffer;
		xfer.len = bytes;

		onionPrint(ONION_SEVERITY_DEBUG, "%s Trasferring 0x%02x, expecting %d bytes\n", SPI_PRINT_BANNER, xfer.tx_buf, xfer.len);

		res = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);

		onionPrint(ONION_SEVERITY_DEBUG, "   Received: 0x%02x, ioctl status: %d\n", xfer.rx_buf, res);
	}

	return res;
}

int spi_readByte (int busNum, int devId, int addr, int *val)
{
	int 	status;
	uint8_t	addr8, value8;

	addr8	= (uint8_t)addr;


	status 	= spiTransfer(busNum, devId, &addr8, &value8, 1);
	*val 	= (int)value8;

	return 	status;
}


// helper functions
int _spi_getFd(int busNum, int devId, int *devHandle)
{
	int 	status;
	char 	pathname[255];

	// define the path to open
	status = snprintf(pathname, sizeof(pathname), SPI_DEV_PATH, busNum, devId);

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


