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
addr 	= int((0x37 << 1) | 0x80)
print "Reading from addr %02x"%(addr)
rdBytes	= spi.readBytes(addr, size)
print "readBytes return: ", rdBytes


# perform a read 
size 	= 1
addr 	= int((0x01 << 1) | 0x80)
print "Reading from addr %02x"%(addr)
rdBytes	= spi.readBytes(addr, size)
print "readBytes return: ", rdBytes

# perform a write 
addr 	= int(0x01 << 1)
values	= [0x0d]
print "Writing to addr %02x"%(addr)
wrBytes	= spi.writeBytes(addr, values)
print "writeBytes return: ", wrBytes

# perform a read 
size 	= 1
addr 	= int((0x01 << 1) | 0x80)
print "Reading from addr %02x"%(addr)
rdBytes	= spi.readBytes(addr, size)
print "readBytes return: ", rdBytes



print "Done"
