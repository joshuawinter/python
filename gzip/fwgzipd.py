#!/usr/bin/env python

import tarfile,zlib, os, sys

CHUNKSIZE=1024


for filenames in os.listdir('/home/n0083510/Downloads/quicktest/'):
    # print path to all filenames.
    ##print filenames
    
	d = zlib.decompressobj(16+zlib.MAX_WBITS)
	f=open('/home/n0083510/Downloads/quicktest/'+filenames,'rb')
	buffer=f.read(CHUNKSIZE)
	while buffer:
	  outstr = d.decompress(buffer)
	  #print(outstr)
	  sys.stdout.write(outstr)
	  buffer=f.read(CHUNKSIZE)
	outstr = d.flush()
	#print(outstr)
	f.close()



