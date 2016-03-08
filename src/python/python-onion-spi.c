#include <Python.h>
#include <onion-spi.h>

#if PY_MAJOR_VERSION < 3
#define PyLong_AS_LONG(val) PyInt_AS_LONG(val)
#define PyLong_AsLong(val) PyInt_AsLong(val)
#endif

// Macros needed for Python 3
#ifndef PyInt_Check
#define PyInt_Check			PyLong_Check
#define PyInt_FromLong		PyLong_FromLong
#define PyInt_AsLong		PyLong_AsLong
#define PyInt_Type			PyLong_Type
#endif


// static object variable for error:
static PyObject *PyOnionSpiError;


PyDoc_STRVAR(onionSpi_module_doc,
	"This module defines an object type that allows SPI transactions\n"
	"on hosts running the Linux kernel. The host kernel must have SPI\n"
	"support and SPI device interface support.\n"
	"All of these can be either built-in to the kernel, or loaded from\n"
	"modules.\n"
);

typedef struct {
	PyObject_HEAD

	struct spiParams	params;
} OnionSpiObject;

// required class functions
static PyObject *
onionSpi_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	OnionSpiObject *self;
	if ((self = (OnionSpiObject *)type->tp_alloc(type, 0)) == NULL)
		return NULL;

	// setup the params to the default
	spiParamInit(&(self->params));

	Py_INCREF(self);
	return (PyObject *)self;
}

static int
onionSpi_init(OnionSpiObject *self, PyObject *args, PyObject *kwds)
{
	int bus 	= -1;
	int client 	= -1;
	static char *kwlist[] = {"bus", "client", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ii:__init__", kwlist, &bus, &client) ) {
		return -1;
	}

	if (bus >= 0) {
		self->params.busNum	= bus;
	}

	if (client >= 0) {
		self->params.deviceId	= client;
	}

	return 0;
}


static PyObject *
onionSpi_close(OnionSpiObject *self)
{
	// reset the params
	spiParamInit(&(self->params));

	Py_INCREF(Py_None);
	return Py_None;
}

static void
onionSpi_dealloc(OnionSpiObject *self)
{
	PyObject *ref = onionSpi_close(self);
	Py_XDECREF(ref);

	Py_TYPE(self)->tp_free((PyObject *)self);
}


/*
 * 	Functions being mapped to object functions
 */

static char *wrmsg_list0 	= "Empty argument list.";
static char *wrmsg_spi 		= "SPI transaction failed.";
static char *wrmsg_val 		= "Non-Int/Long value in arguments: %x.";

PyDoc_STRVAR(onionSpi_setVerbosity_doc,
	"setVerbosity(level) -> None\n\n"
	"Set the verbosity for the object (-1 to 2).\n");

static PyObject *
onionSpi_setVerbosity(OnionSpiObject *self, PyObject *args)
{
	int 		verbose;


	// parse the arguments
	if (!PyArg_ParseTuple(args, "i", &verbose) ) {
		return NULL;
	}

	// set the verbosity level
	onionSetVerbosity(verbose);
	

	Py_INCREF(Py_None);
	return Py_None;
}


PyDoc_STRVAR(onionSpi_readBytes_doc,
	"readBytes(addr, numBytes) -> [values]\n\n"
	"Read 'numBytes' bytes from address 'addr' on an SPI device.\n");

static PyObject *
onionSpi_readBytes(OnionSpiObject *self, PyObject *args)
{
	int 		status, addr, bytes, i;
	uint8_t 	*txBuffer;
	uint8_t 	*rxBuffer;
	PyObject	*list;


	// parse the arguments
	if (!PyArg_ParseTuple(args, "ii", &addr, &bytes) ) {
		return NULL;
	}

	// allocate the buffers based on the number of bytes
	txBuffer  	= (uint8_t*)malloc(sizeof(uint8_t) * bytes);
	rxBuffer  	= (uint8_t*)malloc(sizeof(uint8_t) * bytes);

	// populate the address
	*txBuffer 	= (uint8_t)addr;

	// perform the transfer
	status 	= spiTransfer(&(self->params), txBuffer, rxBuffer, bytes);

	if (status != EXIT_SUCCESS) {
		PyErr_SetString(PyExc_IOError, wrmsg_spi);
		return NULL;
	}

	// build the python object to be returned from the rxBuffer
	list = PyList_New(bytes);

	for (i = 0; i < bytes; i++) {
		PyObject *val = Py_BuildValue("l", (long)rxBuffer[i]);
		PyList_SET_ITEM(list, i, val);
	}

	// clean-up
	free(txBuffer);
	free(rxBuffer);


	return list;
}


PyDoc_STRVAR(onionSpi_writeBytes_doc,
	"writeBytes(addr, [values]) -> None\n\n"
	"Write bytes from 'value' list to address 'addr' on an SPI device.\n");

static PyObject *
onionSpi_writeBytes(OnionSpiObject *self, PyObject *args)
{
	int 		status, addr, bytes, i;
	uint8_t 	*txBuffer;
	uint8_t 	*rxBuffer;
	PyObject	*list;
	char		wrmsg_text[4096];


	// parse the arguments
	if (!PyArg_ParseTuple(args, "iO", &addr, &list) ) {
		return NULL;
	}

	if (!PyList_Size(list) > 0) {
		PyErr_SetString(PyExc_TypeError, wrmsg_list0);
		return NULL;
	}

	// find size of list
	bytes 	= PyList_GET_SIZE(list);
	bytes++;	// add one for the address

	// allocate the buffers based on the number of bytes
	txBuffer  	= (uint8_t*)malloc(sizeof(uint8_t) * bytes);
	rxBuffer  	= (uint8_t*)malloc(sizeof(uint8_t) * bytes);

	// populate the address
	txBuffer[0] 	= (uint8_t)addr;

	// populate the values (by iterating through the list)
	for (i = 0; i < (bytes-1); i++) {
		PyObject *val = PyList_GET_ITEM(list, i);
#if PY_MAJOR_VERSION < 3
		if (PyInt_Check(val)) {
			txBuffer[i+1] = (uint8_t)PyInt_AS_LONG(val);
		} else
#endif
		{
			if (PyLong_Check(val)) {
				txBuffer[i+1] = (uint8_t)PyLong_AS_LONG(val);
			} else {
				snprintf(wrmsg_text, sizeof (wrmsg_text) - 1, wrmsg_val, val);
				PyErr_SetString(PyExc_TypeError, wrmsg_text);
				return NULL;
			}
		}
	}

	// perform the transfer
	status 	= spiTransfer(&(self->params), txBuffer, rxBuffer, bytes);

	if (status != EXIT_SUCCESS) {
		PyErr_SetString(PyExc_IOError, wrmsg_spi);
		return NULL;
	}

	// clean-up
	free(txBuffer);
	free(rxBuffer);


	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(onionSpi_write_doc,
	"write([values]) -> None\n\n"
	"Write bytes from 'value' list to address 'addr' on an SPI device.\n");

static PyObject *
onionSpi_write(OnionSpiObject *self, PyObject *args)
{
	int 		status, bytes, i;
	uint8_t 	*txBuffer;
	uint8_t 	*rxBuffer;
	PyObject	*list;
	char		wrmsg_text[4096];


	// parse the arguments
	if (!PyArg_ParseTuple(args, "O", &list) ) {
		return NULL;
	}

	if (!PyList_Size(list) > 0) {
		PyErr_SetString(PyExc_TypeError, wrmsg_list0);
		return NULL;
	}

	// find size of list
	bytes 	= PyList_GET_SIZE(list);

	// allocate the buffers based on the number of bytes
	txBuffer  	= (uint8_t*)malloc(sizeof(uint8_t) * bytes);
	rxBuffer  	= (uint8_t*)malloc(sizeof(uint8_t) * bytes);

	// populate the values (by iterating through the list)
	for (i = 0; i < bytes; i++) {
		PyObject *val = PyList_GET_ITEM(list, i);
#if PY_MAJOR_VERSION < 3
		if (PyInt_Check(val)) {
			txBuffer[i] = (uint8_t)PyInt_AS_LONG(val);
		} else
#endif
		{
			if (PyLong_Check(val)) {
				txBuffer[i] = (uint8_t)PyLong_AS_LONG(val);
			} else {
				snprintf(wrmsg_text, sizeof (wrmsg_text) - 1, wrmsg_val, val);
				PyErr_SetString(PyExc_TypeError, wrmsg_text);
				return NULL;
			}
		}
	}

	// perform the transfer
	status 	= spiTransfer(&(self->params), txBuffer, rxBuffer, bytes);

	if (status != EXIT_SUCCESS) {
		PyErr_SetString(PyExc_IOError, wrmsg_spi);
		return NULL;
	}

	// clean-up
	free(txBuffer);
	free(rxBuffer);


	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(onionSpi_checkDevice_doc,
	"checkDevice() -> 0|1\n\n"
	"Check if SPI device has a sysfs interface.\n"
	"Returns 0 if sysfs interface is mapped\n"
	"Returns 1 if not mapped\n");

static PyObject *
onionSpi_checkDevice(OnionSpiObject *self, PyObject *args)
{
	int 		bNoDevice;
	PyObject 	*result;

	// check the device
	bNoDevice 	= spiCheckDevice(self->params.busNum, self->params.deviceId, ONION_SEVERITY_DEBUG_EXTRA);

	// create the python value
	result 		= Py_BuildValue("i", bNoDevice);

	Py_INCREF(result);
	return result;
}

PyDoc_STRVAR(onionSpi_registerDevice_doc,
	"registerDevice() -> 0|1\n\n"
	"If SPI device does not have a sysfs interface,\n"
	"register the SPI device\n"
	"Does nothing if the sysfs interface exists\n");

static PyObject *
onionSpi_registerDevice(OnionSpiObject *self, PyObject *args)
{
	int 		status;
	PyObject 	*result;

	// check the device
	status 		= spiRegisterDevice(&(self->params));

	// create the python value
	result 		= Py_BuildValue("i", status);
	
	Py_INCREF(result);
	return result;
}

PyDoc_STRVAR(onionSpi_setupDevice_doc,
	"setupDevice() -> 0|1\n\n"
	"Set the following SPI parameters on the sysfs interface:\n"
	" mode, bits per word, max speed\n");

static PyObject *
onionSpi_setupDevice(OnionSpiObject *self, PyObject *args)
{
	int 		status;
	PyObject 	*result;

	// check the device
	status 		= spiSetupDevice(&(self->params));

	// create the python value
	result 		= Py_BuildValue("i", status);
	
	Py_INCREF(result);
	return result;
}


/*
 * 	Define the get and set functions for the parameters
 */

// pass in a python value, get a C int back
//	returns -1 if conversion failed for whatever reason
static int
onionSpi_convertPyValToInt (PyObject *val)
{
	int 	cValue;

	// check for a valid value
	if (val == NULL) {
		PyErr_SetString(PyExc_TypeError,
			"Cannot delete attribute");
		return -1;
	}

	// convert the python value
#if PY_MAJOR_VERSION < 3
	if (PyInt_Check(val)) {
		cValue = PyInt_AS_LONG(val);
	} else
#endif
	{
		if (PyLong_Check(val)) {
			cValue = PyLong_AS_LONG(val);
		} else {
			PyErr_SetString(PyExc_TypeError,
				"The attribute must be an integer");
			return -1;
		}
	}

	return 	cValue;
}

// bus 
static PyObject *
onionSpi_get_bus(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.busNum);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_bus(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.busNum = value;
		return 0;
	}
	
	return -1;
}

// device 
static PyObject *
onionSpi_get_device(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.deviceId);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_device(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.deviceId = value;
		return 0;
	}
	
	return -1;
}

// speed 
static PyObject *
onionSpi_get_speed(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.speedInHz);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_speed(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.speedInHz = value;
		return 0;
	}
	
	return -1;
}

// delay
static PyObject *
onionSpi_get_delay(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.delayInUs);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_delay(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.delayInUs = value;
		return 0;
	}
	
	return -1;
}

// bpw
static PyObject *
onionSpi_get_bpw(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.bitsPerWord);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_bpw(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.bitsPerWord = value;
		return 0;
	}
	
	return -1;
}


// mode
static PyObject *
onionSpi_get_mode(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.mode);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_mode(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.mode = value;
		return 0;
	}
	
	return -1;
}


// modeBits
static PyObject *
onionSpi_get_modeBits(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.modeBits);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_modeBits(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.modeBits = value;
		return 0;
	}
	
	return -1;
}

// modeBits: 3-wire
static PyObject *
onionSpi_get_modeBits_3wire(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", ((self->params.modeBits & SPI_3WIRE) > 0 ? 1 : 0) );
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_modeBits_3wire(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		if (value > 0)
			self->params.modeBits |= SPI_3WIRE;
		return 0;
	}
	
	return -1;
}

// modeBits: lsbFirst
static PyObject *
onionSpi_get_modeBits_lsbfirst(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", ((self->params.modeBits & SPI_LSB_FIRST) > 0 ? 1 : 0) );
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_modeBits_lsbfirst(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		if (value > 0)
			self->params.modeBits |= SPI_LSB_FIRST;
		return 0;
	}
	
	return -1;
}

// modeBits: loopback
static PyObject *
onionSpi_get_modeBits_loop(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", ((self->params.modeBits & SPI_LOOP) > 0 ? 1 : 0) );
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_modeBits_loop(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		if (value > 0)
			self->params.modeBits |= SPI_LOOP;
		return 0;
	}
	
	return -1;
}

// modeBits: no-cs
static PyObject *
onionSpi_get_modeBits_noCs(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", ((self->params.modeBits & SPI_NO_CS) > 0 ? 1 : 0) );
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_modeBits_noCs(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		if (value > 0)
			self->params.modeBits |= SPI_NO_CS;
		return 0;
	}
	
	return -1;
}

// modeBits: no-cs
static PyObject *
onionSpi_get_modeBits_csHigh(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", ((self->params.modeBits & SPI_CS_HIGH) > 0 ? 1 : 0) );
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_modeBits_csHigh(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		if (value > 0)
			self->params.modeBits |= SPI_CS_HIGH;
		return 0;
	}
	
	return -1;
}


// sckGpio
static PyObject *
onionSpi_get_sckGpio(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.sckGpio);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_sckGpio(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.sckGpio = value;
		return 0;
	}
	
	return -1;
}

// mosiGpio
static PyObject *
onionSpi_get_mosiGpio(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.mosiGpio);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_mosiGpio(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.mosiGpio = value;
		return 0;
	}
	
	return -1;
}

// misoGpio
static PyObject *
onionSpi_get_misoGpio(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.misoGpio);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_misoGpio(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.misoGpio = value;
		return 0;
	}
	
	return -1;
}

// csGpio
static PyObject *
onionSpi_get_csGpio(OnionSpiObject *self, void *closure)
{
	// create a python value from the integer
	PyObject *result = Py_BuildValue("i", self->params.csGpio);
	Py_INCREF(result);
	return result;
}

static int
onionSpi_set_csGpio(OnionSpiObject *self, PyObject *val, void *closure)
{
	uint32_t value;

	// convert the python value
	value 	= onionSpi_convertPyValToInt(val);

	if (value != -1) {
		self->params.csGpio = value;
		return 0;
	}
	
	return -1;
}

static PyGetSetDef onionSpi_getset[] = {
	{"bus", (getter)onionSpi_get_bus, (setter)onionSpi_set_bus,
			"SPI bus number\n"},
	{"device", (getter)onionSpi_get_device, (setter)onionSpi_set_device,
			"SPI device ID\n"},

	{"speed", (getter)onionSpi_get_speed, (setter)onionSpi_set_speed,
			"Maximum speed in Hz\n"},
	{"delay", (getter)onionSpi_get_delay, (setter)onionSpi_set_delay,
			"How long to delay in us after last bit transfer\n"
			"before optionally deselecting the device before next transfer"},
	{"bitsPerWord", (getter)onionSpi_get_bpw, (setter)onionSpi_get_bpw,
			"Bits per word\n"},

	{"mode", (getter)onionSpi_get_mode, (setter)onionSpi_set_mode,
			"SPI mode as two bit pattern of \n"
			"Clock Polarity  and Phase [CPOL|CPHA]\n"
			"min: 0b00 = 0 max: 0b11 = 3\n"},

	{"modeBits", (getter)onionSpi_get_modeBits, (setter)onionSpi_set_modeBits,
			"Additional SPI mode bits to define additional options, like:\n"
			"sending LSB first, set CS to active-high, no CS pin, \n"
			"3 wire interface (SI/SO signals shared), loopback configuration\n"},

	{"threewire", (getter)onionSpi_get_modeBits_3wire, (setter)onionSpi_set_modeBits_3wire,
			"SI/SO signals shared\n"},
	{"lsbfirst", (getter)onionSpi_get_modeBits_lsbfirst, (setter)onionSpi_set_modeBits_lsbfirst,
			"LSB first\n"},
	{"loop", (getter)onionSpi_get_modeBits_loop, (setter)onionSpi_set_modeBits_loop,
			"loopback configuration\n"},
	{"noCs", (getter)onionSpi_get_modeBits_noCs, (setter)onionSpi_set_modeBits_noCs,
			"no CS pin\n"},
	{"csHigh", (getter)onionSpi_get_modeBits_csHigh, (setter)onionSpi_set_modeBits_csHigh,
			"CS is active-high\n"},


	{"sck", (getter)onionSpi_get_sckGpio, (setter)onionSpi_set_sckGpio,
			"GPIO for SCK signal\n"},
	{"mosi", (getter)onionSpi_get_mosiGpio, (setter)onionSpi_set_mosiGpio,
			"GPIO for MOSI signal\n"},
	{"miso", (getter)onionSpi_get_misoGpio, (setter)onionSpi_set_misoGpio,
			"GPIO for MISO signal\n"},
	{"cs", (getter)onionSpi_get_csGpio, (setter)onionSpi_set_csGpio,
			"GPIO for CS signal\n"},

	
	{NULL} 		/* Sentinel */
};

////// Python Module Setup //////
/*
 * 	Bind Python function names to the C functions
 */
PyDoc_STRVAR(OnionSpiObjectType_doc,
	"onionSpi([bus],[client]) -> SPI\n\n"
	"Return an object that can be used to communicate through a\n"
	"specified SPI device interface.\n");

static PyMethodDef onionSpi_methods[] = {
	{"setVerbosity", 	(PyCFunction)onionSpi_setVerbosity, 	METH_VARARGS, 		onionSpi_setVerbosity_doc},
	
	{"checkDevice", 	(PyCFunction)onionSpi_checkDevice, 		METH_VARARGS, 		onionSpi_checkDevice_doc},
	{"registerDevice", 	(PyCFunction)onionSpi_registerDevice, 	METH_VARARGS, 		onionSpi_registerDevice_doc},
	{"setupDevice", 	(PyCFunction)onionSpi_setupDevice, 		METH_VARARGS, 		onionSpi_setupDevice_doc},

	{"readBytes", 		(PyCFunction)onionSpi_readBytes, 		METH_VARARGS, 		onionSpi_readBytes_doc},
	{"writeBytes", 		(PyCFunction)onionSpi_writeBytes, 		METH_VARARGS, 		onionSpi_writeBytes_doc},

	{"write", 			(PyCFunction)onionSpi_write, 			METH_VARARGS, 		onionSpi_write_doc},

	{NULL, NULL}	/* Sentinel */
};


/*
 * 	Python object bindings
 */
static PyTypeObject OnionSpiObjectType = {
#if PY_MAJOR_VERSION >= 3
	PyVarObject_HEAD_INIT(NULL, 0)
#else
	PyObject_HEAD_INIT(NULL)
	0,				/* ob_size */
#endif
	"OnionSpi",			/* tp_name */
	sizeof(OnionSpiObject),		/* tp_basicsize */
	0,				/* tp_itemsize */
	(destructor)onionSpi_dealloc,	/* tp_dealloc */
	0,				/* tp_print */
	0,				/* tp_getattr */
	0,				/* tp_setattr */
	0,				/* tp_compare */
	0,				/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	0,				/* tp_as_mapping */
	0,				/* tp_hash */
	0,				/* tp_call */
	0,				/* tp_str */
	0,				/* tp_getattro */
	0,				/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	OnionSpiObjectType_doc,		/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	0,				/* tp_iter */
	0,				/* tp_iternext */
	onionSpi_methods,			/* tp_methods */
	0,				/* tp_members */
	onionSpi_getset,			/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	(initproc)onionSpi_init,		/* tp_init */
	0,				/* tp_alloc */
	onionSpi_new,			/* tp_new */
};

static PyMethodDef onionSpi_module_methods[] = {
	{NULL}	/* Sentinel */
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"onionSpi",
	onionSpi_module_doc,
	-1,
	onionSpi_module_methods,
	NULL,
	NULL,
	NULL,
	NULL,
};
#else
#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
#endif


/*
 * 	Python calls this to initialize this module
 */
#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC
PyInit_onionSpi(void)
#else
void initonionSpi(void)
#endif
{
	PyObject *m;

	if (PyType_Ready(&OnionSpiObjectType) < 0)
#if PY_MAJOR_VERSION >= 3
		return NULL;
#else
		return;
#endif

#if PY_MAJOR_VERSION >= 3
	m = PyModule_Create(&moduledef);
#else
	m = Py_InitModule3("onionSpi", onionSpi_methods, onionSpi_module_doc);
#endif
    if (m == NULL)
#if PY_MAJOR_VERSION >= 3
		return NULL;
#else
		return;
#endif

    Py_INCREF(&OnionSpiObjectType);
	PyModule_AddObject(m, "OnionSpi", (PyObject *)&OnionSpiObjectType);


    PyOnionSpiError = PyErr_NewException("onionSpi.error", NULL, NULL);
    Py_INCREF(PyOnionSpiError);
    PyModule_AddObject(m, "error", PyOnionSpiError);

#if PY_MAJOR_VERSION >= 3
	return m;
#endif
}

