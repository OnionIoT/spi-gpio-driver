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


// LAZAR: CHANGE THIS TEXT
PyDoc_STRVAR(onionSpi_module_doc,
	"This module defines an object type that allows SPI transactions\n"
	"on hosts running the Linux kernel. The host kernel must have SPI\n"
	"support and SPI device interface support.\n"
	"All of these can be either built-in to the kernel, or loaded from\n"
	"modules.\n"
	"\n"
	"Because the SPI device interface is opened R/W, users of this\n"
	"module usually must have root permissions.\n");

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


PyDoc_STRVAR(onionSpi_setDevice_doc,
	"setDevice(bus, device) -> None\n\n"
	"Set the bus number and device ID for the object.\n");

static PyObject *
onionSpi_setDevice(OnionSpiObject *self, PyObject *args)
{
	int 		bus, device;


	// parse the arguments
	if (!PyArg_ParseTuple(args, "ii", &bus, &device) ) {
		return NULL;
	}

	// update the parameters
	self->params.busNum	= bus;
	self->params.deviceId 	= device;
	

	Py_INCREF(Py_None);
	return Py_None;
}


PyDoc_STRVAR(onionSpi_readBytes_doc,
	"readBytes(addr, bytes) -> [values]\n\n"
	"Read 'bytes' bytes from address 'addr' on an SPI device.\n");

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

	if (status < 0) {
		// LAZAR: ERROR MESSAGE
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
	"writeBytes(addr, value) -> None\n\n"
	"Write bytes from 'value' list to address 'addr' on an SPI device.\n");

static PyObject *
onionSpi_writeBytes(OnionSpiObject *self, PyObject *args)
{
	int 		status, addr, bytes, i;
	uint8_t 	*txBuffer;
	uint8_t 	*rxBuffer;
	PyObject	*list;


	// parse the arguments
	if (!PyArg_ParseTuple(args, "iO", &addr, &list) ) {
		return NULL;
	}

	if (!PyList_Size(list) > 0) {
		// LAZAR: ERROR MESSAGE
		//PyErr_SetString(PyExc_TypeError, wrmsg_list0);
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
				// LAZAR: ERROR MESSAGE
				//snprintf(wrmsg_text, sizeof (wrmsg_text) - 1, wrmsg_val, val);
				//PyErr_SetString(PyExc_TypeError, wrmsg_text);
				return NULL;
			}
		}
	}

	// perform the transfer
	status 	= spiTransfer(&(self->params), txBuffer, rxBuffer, bytes);

	if (status < 0) {
		// LAZAR: ERROR MESSAGE
	}

	// clean-up
	free(txBuffer);
	free(rxBuffer);


	Py_INCREF(Py_None);
	return Py_None;
}



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
	{"setDevice", 		(PyCFunction)onionSpi_setDevice, 		METH_VARARGS, 		onionSpi_setDevice_doc},
	{"readBytes", 		(PyCFunction)onionSpi_readBytes, 		METH_VARARGS, 		onionSpi_readBytes_doc},
	{"writeBytes", 		(PyCFunction)onionSpi_writeBytes, 		METH_VARARGS, 		onionSpi_writeBytes_doc},
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
	0,	// LAZAR: implement this
	//SpiDev_getset,			/* tp_getset */
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

