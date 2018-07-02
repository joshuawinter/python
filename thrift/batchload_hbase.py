#!/usr/bin/env python


from thrift.transport.TSocket import TSocket
from thrift.transport.TTransport import TBufferedTransport
from thrift.protocol import TBinaryProtocol
from hbase import Hbase
from hbase import ttypes
from hbase.Hbase import Client, ColumnDescriptor, Mutation, BatchMutation
import time 
import datetime


transport = TBufferedTransport(TSocket("localhost", 9090))
transport.open()
protocol = TBinaryProtocol.TBinaryProtocol(transport)


client = Hbase.Client(protocol)
t = "sourcetable"
columns = []
col = ColumnDescriptor()
col.name = 'core:'
col.maxVersions = 10
columns.append(col)


columns.append(col)


try:
  print "creating table: %s" %(t)
  client.createTable(t, columns)
except ttypes.AlreadyExists, ae:
  print "WARN: " + ae.message


f = open("/home/amit/Downloads/matching_4.txt")
headers = f.readline()
headers = headers.split("\t")
headers = map(lambda x: x.strip(), headers)
bom = "\xef\xbb\xbf"
headers = map(lambda x: x.replace(bom, ''), headers)


batchMutations = []


for i, line in enumerate(f):
  lines = line.split("\t")
  lines = map(lambda x: x.strip(), lines)
  if len(lines) != len(headers):
    print lines
    break
  mapped = dict(map(None, headers, lines))
  
  timestamp = mapped['timestamp_update']
  if (timestamp.strip()):
    timestamp = timestamp.split("-")


    timestamp = (map(int,timestamp))
    tt = datetime.date(timestamp[0], timestamp[1], timestamp[2])
    tfromepoch = time.mktime(tt.timetuple())
    print(tfromepoch)
  mutations = [Mutation(column="core:%s" % k, value=v) for k,v in mapped.items()]
  if tfromepoch:
#    client.mutateRowTs(t, mapped["entity_id"], mutations, long(tfromepoch))
    mutations.append(Mutation(column="core:t", value=str(long(tfromepoch))))
#  else:
#    client.mutateRow(t, mapped["entity_id"], mutations)
  batchMutations.append(BatchMutation(mapped["entity_id"], mutations))
  if (i % 100 == 0):
    client.mutateRows(t, batchMutations)
    batchMutations = []


#flush:
if batchMutations:
  client.mutateRows(t, batchMutations)


transport.close()
