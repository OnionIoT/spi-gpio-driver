import onionSpi
import time


print 'Starting: onionSpi module testing...'

spi 	= onionSpi.OnionSpi()

# set the verbosity
spi.setVerbosity(1)


# setup the bus number and device id
bus 	= 0
device 	= 1
spi.setDevice(0, 1)
print "setDevice returned"

# perform a read
size 	= 1
addr 	= 0xee
rdBytes	= spi.readBytes(addr, size)
print "readBytes return: ", rdBytes




print "Done"
