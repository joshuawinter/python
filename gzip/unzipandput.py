#!/usr/bin/env python

import zipfile, os, sys, subprocess


sdir = '/data/scratchspace/207.115.82.98/RawDataFiles/Teletrac/'
odir = sdir

for infilenames in os.listdir(sdir):
	#print infilenames
	fh = open(sdir+infilenames,'r+b')
	data = fh.read()
	pos = data.find('\x50\x4b\x05\x06') # End of central directory signature  
	if (pos > 0):   
		fh.seek(pos + 22)   # size of 'ZIP end of central directory record' 
		fh.truncate()   
	else:  
		# raise error, file is truncate
	
	#print fh
		z = zipfile.ZipFile(fh)
		for name in z.namelist():
			uncompressed = z.read(name)
			outfile = open(odir+name, 'wb')
			outfile.write(uncompressed)
			outfile.close()
			fh.close()
print os.listdir(odir)


