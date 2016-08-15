# spi-gpio-driver
Driver to bit-bang SPI protocol through GPIOs

# Default Pin Mapping

| Functionality | GPIO       |
|---------------|------------|
| SPI SCK       | 6          |
| SPI MOSI      | 18         |
| SPI MISO      | 1          |
| SPI CS        | 7          |


# Example Code

## `spiTransfer` Function

Example code showing how to use the `spiTransfer()` function

### Reading with the `spiTransfer` Function

Read a value from register:

```
void readValue(int addr)
{
	int 		status, size;
	uint8_t 	*txBuffer;
	uint8_t 	*rxBuffer;

	struct spiParams	params;
	spiParamInit(&params);

	// set the transmission size and allocate memory for the buffers
	size 		= 1;
	txBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);
	rxBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);

	// assign the register address to the transmission buffer
	*txBuffer 	= (uint8_t)addr;

	// invoke the SPI transfer
	status 	= spiTransfer(&params, txBuffer, rxBuffer, size);

	// rxBuffer now contains the data read through the SPI interface
	printf("> SPI Read from addr 0x%02x: 0x%02x\n", addr, *rxBuffer);

	// clean-up
	free(txBuffer);
	free(rxBuffer);
}
```


### Writing with the `spiTransfer` Function

Write a value to a register:

```
void writeValue(int addr, int value)
{
	int 		status, size;
	uint8_t 	*txBuffer;
	uint8_t 	*rxBuffer;

	struct spiParams	params;
	spiParamInit(&params);

	// set the transmission size and allocate memory for the buffers
	size 		= 2;
	txBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);
	rxBuffer 	= (uint8_t*)malloc(sizeof(uint8_t) * size);

	// assign the register address and data to be written to the transmission buffer
	txBuffer[0] = (uint8_t)addr;
	txBuffer[1] = (uint8_t)value;

	// invoke the SPI transfer
	status 	= spiTransfer(&params, txBuffer, rxBuffer, size);

	// data has been written
	// any response is now in rxBuffer
	printf("> SPI Write to addr 0x%02x: 0x%02x\n", txBuffer[0], txBuffer[1] );

	// clean-up
	free(txBuffer);
	free(rxBuffer);
}
```





## `spiRead` Function

Read a value from register:

```
void readValue2(int addr)
{
	int 		status, size;
	uint8_t 	*rdBuffer;

	struct spiParams	params;
	spiParamInit(&params);

	// set the transmission size and allocate memory for the buffers
	size 		= 1;
	rdBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);

	// invoke the SPI read
	status 		= spiRead(&params, addr, rdBuffer, size);

	// rdBuffer now contains the data read through the SPI interface
	printf("> SPI Read from addr 0x%02x: 0x%02x\n", addr, *rdBuffer);

	// clean-up
	free(rdBuffer);
}
```



## `spiWrite` Function

Write a value to a register:

```
void writeValue2(int addr, int value)
{
	int 		status, size;
	uint8_t 	*wrBuffer;

	struct spiParams	params;
	spiParamInit(&params);

	// set the transmission size and allocate memory for the buffers
	size 		= 1;
	wrBuffer	= (uint8_t*)malloc(sizeof(uint8_t) * size);

	// put the value to be written in the wrBuffer
	*wrBuffer 	= (uint8_t)value;

	// invoke the SPI read
	status 		= spiWrite(&params, addr, wrBuffer, size);

	// data in wrBuffer has been written to the SPI device
	printf("> SPI Write to addr 0x%02x: 0x%02x\n", addr, *wrBuffer );

	// clean-up
	free(wrBuffer);
}
```

