#!/usr/bin/env python

import sys
 
from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
 
from hbase import Hbase
from hbase.ttypes import *



# Make socket
transport = TSocket.TSocket('vxkit-phdp0003.lmig.com', 9090)
 
# Buffering is critical. Raw sockets are very slow
transport = TTransport.TBufferedTransport(transport)
 
# Wrap in a protocol
protocol = TBinaryProtocol.TBinaryProtocol(transport)

client = Hbase.Client(protocol)
 
transport.open()

t = "test_tbl"
columns = []
col = ColumnDescriptor()
col.name = 'core:'
col.maxVersions = 10
columns.append(col)



try:
  print "creating table: %s" %(t)
  client.createTable(t, columns)
except ttypes.AlreadyExists, ae:
  print "WARN: " + ae.message


print client.getTableNames()


