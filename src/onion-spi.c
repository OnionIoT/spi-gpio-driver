#include <onion-spi.h>

// helper function prototypes
int 	_spiGetFd				(int busNum, int devId, int *devHandle, int printSeverity);
int 	_spiReleaseFd			(int devHandle);

int 	_spiRegisterDevice 		(int printSeverity, struct spiParams *params);

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

	params->mode 		 	= SPI_DEFAULT_MODE;
	params->modeBits 		= SPI_DEFAULT_MODE_BITS;

	params->sckGpio			= SPI_DEFAULT_GPIO_SCK;
	params->mosiGpio		= SPI_DEFAULT_GPIO_MOSI;
	params->misoGpio		= SPI_DEFAULT_GPIO_MISO;
	params->csGpio			= SPI_DEFAULT_GPIO_CS;
}

// check if a device file handle is available
// returns:
//	EXIT_SUCCESS 	if device file handle is available
//	EXIT_FAILURE 	device file handle is not available
int spiCheckDevice(int busNum, int devId, int printSeverity)
{
	int 	status, fd;

	// open the file
	status	= _spiGetFd(busNum, devId, &fd, printSeverity);

	// close the file
	if (status == EXIT_SUCCESS) {
		_spiReleaseFd(fd);
	}

	return status;
}

// check if a specific SPI device exists.
//	if not, register it with sysfs
// 	if it exists, do nothing
int spiRegisterDevice (struct spiParams *params)
{
	int 	status;

	// check if device file is available
	status	= spiCheckDevice(params->busNum, params->deviceId, ONION_SEVERITY_DEBUG_EXTRA);

	if (status == EXIT_FAILURE) {
		// device file does not exist - register the spi device
		status	= _spiRegisterDevice(ONION_SEVERITY_INFO, params);

		// wait for device to be registered
		usleep(500*1000);

		// check that device file exists
		status	= spiCheckDevice(params->busNum, params->deviceId, ONION_SEVERITY_DEBUG_EXTRA);
	}
	else if (status == EXIT_SUCCESS) {
		// device file exists - all good
		onionPrint(ONION_SEVERITY_INFO, "> SPI device already available\n");
	}

	return 	status;
}

// using ioctl, setup parameters of the SPI device interface
int spiSetupDevice (struct spiParams *params)
{
	int 	status, ret, fd;

	// open the file handle
	status 	= _spiGetFd(params->busNum, params->deviceId, &fd, ONION_SEVERITY_DEBUG_EXTRA);

	if (status == EXIT_SUCCESS) {
		onionPrint(ONION_SEVERITY_INFO, "> Initializing SPI parameters...\n");
		onionPrint(ONION_SEVERITY_INFO, "  > Set SPI mode:       0x%x\n", params->modeBits);
		onionPrint(ONION_SEVERITY_INFO, "  > Set bits per word:  %d\n", params->bitsPerWord);
		onionPrint(ONION_SEVERITY_INFO, "  > Set max speed:      %d Hz (%d KHz)\n", params->speedInHz, (params->speedInHz)/1000);


		// set the SPI mode
		ret = ioctl(fd, SPI_IOC_WR_MODE32, &(params->modeBits) );
		if (ret == -1) {
			onionPrint(ONION_SEVERITY_FATAL, "ERROR: Cannot set SPI mode 0x%02x\n", params->modeBits);
			return EXIT_FAILURE;
		}
		
		ret = ioctl(fd, SPI_IOC_RD_MODE32, &(params->modeBits) );
		if (ret == -1){
			onionPrint(ONION_SEVERITY_FATAL, "ERROR: Cannot set SPI mode 0x%02x\n", params->modeBits);
			return EXIT_FAILURE;
		}

		// set the bits per word
		ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &(params->bitsPerWord) );
		if (ret == -1) {
			onionPrint(ONION_SEVERITY_FATAL, "ERROR: Cannot set %d bits per word\n", params->bitsPerWord);
			return EXIT_FAILURE;
		}
		
		ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &(params->bitsPerWord) );
		if (ret == -1) {
			onionPrint(ONION_SEVERITY_FATAL, "ERROR: Cannot set %d bits per word\n", params->bitsPerWord);
			return EXIT_FAILURE;
		}
		
		// set max speed in Hz
		ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &(params->speedInHz) );
		if (ret == -1) {
			onionPrint(ONION_SEVERITY_FATAL, "ERROR: Cannot set max speed %d Hz\n", params->speedInHz);
			return EXIT_FAILURE;
		}
		
		ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &(params->speedInHz) );
		if (ret == -1) {
			onionPrint(ONION_SEVERITY_FATAL, "ERROR: Cannot set max speed %d Hz\n", params->speedInHz);
			return EXIT_FAILURE;
		}

		// clean-up
		status 	|= _spiReleaseFd(fd);
	}

	return 	status;
}

// perform a transfer
int spiTransfer(struct spiParams *params, uint8_t *txBuffer, uint8_t *rxBuffer, int bytes)
{
	int 	status;
	int 	fd, res;
	struct 	spi_ioc_transfer xfer;

	res 	= EXIT_FAILURE;

	// open the file handle
	status 	= _spiGetFd(params->busNum, params->deviceId, &fd, ONION_SEVERITY_FATAL);

	// attempt the SPI transfter
	if (status == EXIT_SUCCESS) {
		
		memset(&xfer, 0, sizeof(xfer));
		xfer.tx_buf 			= (unsigned long)txBuffer;
		xfer.rx_buf 			= (unsigned long)rxBuffer;
		xfer.len 			= bytes;
		xfer.speed_hz 		= params->speedInHz;
        xfer.delay_usecs 	= params->delayInUs;
        xfer.bits_per_word 	= params->bitsPerWord;
        xfer.cs_change 		= 0;

		/*if (params->mode & SPI_TX_QUAD)
			xfer.tx_nbits = 4;
		else if (params->mode & SPI_TX_DUAL)
			xfer.tx_nbits = 2;
		if (params->mode & SPI_RX_QUAD)
			xfer.rx_nbits = 4;
		else if (params->mode & SPI_RX_DUAL)
			xfer.rx_nbits = 2;
		if (!(params->mode & SPI_LOOP)) {
			if (params->mode & (SPI_TX_QUAD | SPI_TX_DUAL))
				xfer.rx_buf = 0;
			else if (params->mode & (SPI_RX_QUAD | SPI_RX_DUAL))
				xfer.tx_buf = 0;
		}*/

		onionPrint(ONION_SEVERITY_DEBUG, "%s Trasferring 0x%02x, %d byte%s\n", SPI_PRINT_BANNER, *txBuffer, bytes, (bytes > 1 ? "s" : "") );

		// make the transfer
		res = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);

		// check the return
		if (res < 1) {
			// send failed
			onionPrint(ONION_SEVERITY_FATAL, "ERROR: SPI transfer failed\n");
			*rxBuffer 	= 0;
			status	= EXIT_FAILURE;
		}

		onionPrint(ONION_SEVERITY_DEBUG, "   Received: 0x%02x, ioctl status: %d\n", *rxBuffer, res);

		if (status == EXIT_SUCCESS && onionGetVerbosity() > ONION_SEVERITY_DEBUG ) {
			hex_dump(txBuffer, bytes, 32, "TX");
			hex_dump(rxBuffer, bytes, 32, "RX");
		}

		// clean-up
		status 	|= _spiReleaseFd(fd);
	}

	return status;
}

int spiWrite(struct spiParams *params, int addr, uint8_t *wrBuffer, int bytes)
{
	int 		status, i;
	uint8_t 	*txBuffer;
	uint8_t 	*rxBuffer;

	// generate the transmission buffer
	txBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * (1+bytes) );
	rxBuffer 	= (uint8_t*)malloc(sizeof(uint8_t) * (1+bytes) );

	// load the address
	txBuffer[0] = (uint8_t)addr;

	// load the data to write
	for (i = 0; i < bytes; i++) {
		txBuffer[i+1] 	= wrBuffer[i];
	}

	// call the transfer
	status 	= spiTransfer(params, txBuffer, rxBuffer, bytes+1);

	// clean-up
	free(txBuffer);
	free(rxBuffer);

	return 	status;
}

int spiRead(struct spiParams *params, int addr, uint8_t *rdBuffer, int bytes)
{
	int 		status, i;
	uint8_t 	*txBuffer;
	uint8_t 	*rxBuffer;

	// generate the transmission buffer
	txBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * 1 );

	// load the address
	*txBuffer 	= (uint8_t)addr;

	// call the transfer
	status 	= spiTransfer(params, txBuffer, rdBuffer, bytes);

	// clean-up
	free(txBuffer);

	return 	status;
}


//// helper functions ////
// get a handle to the device
int _spiGetFd(int busNum, int devId, int *devHandle, int printSeverity)
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
		onionPrint(printSeverity, "ERROR: could not open sysfs device '%s'\n", pathname);
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

// register an SPI device
int _spiRegisterDevice (int printSeverity, struct spiParams *params)
{
	int 	status;
	char 	cmd[256];

	onionPrint(printSeverity, "> Registering SPI device:\n");
	onionPrint(printSeverity, "  > bus%d, device id: %d\n", params->busNum, params->deviceId);
	onionPrint(printSeverity, "   > SCK:  GPIO%d\n", params->sckGpio);
	onionPrint(printSeverity, "   > MOSI: GPIO%d\n", params->mosiGpio);
	onionPrint(printSeverity, "   > MISO: GPIO%d\n", params->misoGpio);
	onionPrint(printSeverity, "   > CS:   GPIO%d\n", params->csGpio);
	onionPrint(printSeverity, "  > SPI Mode:      %d\n", params->mode);
	onionPrint(printSeverity, "  > Max Frequency: %d Hz (%d kHz)\n", params->speedInHz, (params->speedInHz)/1000);

	// generate the insmod command
	sprintf(cmd, SPI_DEV_INSMOD_TEMPLATE, 	params->busNum, params->deviceId,
											params->sckGpio,
											params->mosiGpio,
											params->misoGpio,
											params->mode,
											params->speedInHz,
											params->csGpio
			);
	onionPrint(printSeverity+1, ">> Command:\n  %s\n", cmd);  

	// call the insmod command to register an SPI device
	status 	= system(cmd);
	if (status < 0) {

		return EXIT_FAILURE;
	}

	return 	EXIT_SUCCESS;
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
