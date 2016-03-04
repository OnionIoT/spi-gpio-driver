import onionSpi
import time


def printSettings(obj):
	print "SPI Device Settings:"
	print "  bus:    %d"%(obj.bus)
	print "  device: %d"%(obj.device)
	print "  speed:    %d Hz (%d kHz)"%(obj.speed, obj.speed/1000)
	print "  delay:    %d us"%(obj.delay)
	print "  bpw:      %d"%(obj.bitsPerWord)
	print "  mode:     %d (0x%02x)"%(obj.mode, obj.modeBits)
	print "     3wire:    %d"%(obj.threewire)
	print "     lsb:      %d"%(obj.lsbfirst)
	print "     loop:     %d"%(obj.loop)
	print "     no-cs:    %d"%(obj.noCs)
	print "     cs-high:  %d"%(obj.csHigh)
	print ""
	print "GPIO Settings:"
	print "  sck:      %d"%(obj.sck)
	print "  mosi:     %d"%(obj.mosi)
	print "  miso:     %d"%(obj.miso)
	print "  cs:       %d"%(obj.cs)
	print ""

print 'Starting: onionSpi module testing...'

spi 	= onionSpi.OnionSpi(0, 1)

# set the verbosity
spi.setVerbosity(1)

# read the device settings
printSettings(spi)



print ""
ret = raw_input('Ready to register the device?')

# check the device
print 'Checking if device exists...'
ret = spi.checkDevice()
print '   Device does not exist: %d'%(ret)

# register the device
print 'Registering the device...'
ret = spi.registerDevice()
print '   registerDevice returned: %d'%(ret)

# initialize the device parameters
print 'Initializing the device parameters...'
ret = spi.setupDevice()
print '   setupDevice returned: %d'%(ret)

# check the device again
print '\nChecking if device exists...'
ret = spi.checkDevice()
print '   Device does not exist: %d'%(ret)


print ""
ret = raw_input('Ready to test reading and writing?')

# perform a read
size 	= 1
addr 	= int((0x37 << 1) | 0x80)
print "Reading from addr %02x"%(addr)
rdBytes	= spi.readBytes(addr, size)
print "readBytes return: ", rdBytes, " first element: ", rdBytes[0]


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


print ""
ret = raw_input('Ready to test write() function?')

# perform a write using the other write function
values 	= [0x02, 0x0c]
print "Writing 2 bytes"
wrBytes = spi.write(values)
print "write return: ", wrBytes

# perform a read 
size 	= 1
addr 	= int((0x01 << 1) | 0x80)
print "Reading from addr %02x"%(addr)
rdBytes	= spi.readBytes(addr, size)
print "readBytes return: ", rdBytes


print ""
ret = raw_input('Ready to change settings?')

# change some settings and print again
spi.cs = 20
spi.threewire = 1
spi.csHigh = 1
printSettings(spi)


print "Done"
