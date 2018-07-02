#!/usr/bin/python

import _mssql
import os
import sys
from time import localtime,strftime
import time

sql_server = "SQLserver" #Address to database server
database = "hello_db_database"
sql_uname = "splunk_user"
sql_pw = "changeme"
columns = 'TOP 1000 eventID, transactionID, transactionStatus'
table = 'hello_table'

countkey = 'eventID'

last_eventid_filepath = "" # user supplies correct path

# Open file containing the last event ID and get the last record read
last_eventid = 0;
if os.path.isfile(last_eventid_filepath):
    try:
        last_eventid_file = open(last_eventid_filepath,'r')
        last_eventid = int(last_eventid_file.readline())
        last_eventid_file.close()

    # Catch the exception. Real exception handler would be more robust    
    except IOError:
        sys.stderr.write('Error: failed to read last_eventid file, ' + last_eventid_filepath + '\n')
        sys.exit(2)
else:
    sys.stderr.write('Error: ' + last_eventid_filepath + ' file not found! Starting from zero. \n')

# Fetch 1000 rows starting from the last event read
# SELECT TOP 1000 eventID, transactionID, transactionStatus FROM table WHERE eventID > lastEventID ORDER BY eventID
sql_query = 'SELECT ' + columns + ' FROM ' + table + ' WHERE ' + countkey + ' > ' + str(last_eventid) + ' ORDER BY ' + countkey

try:
    conn = _mssql.connect(sql_server, sql_uname, sql_pw, database)
    conn.execute_query(sql_query)
    # timestamp the returned data
    indexTime = "[" + strftime("%m/%d/%Y %H:%M:%S %p %Z",localtime()) + "]"
    for row in conn:
    	print "%s eventID=%s, transactionID=%s, transactionStatus=%s" % (indexTime, row['eventID'], row['transactionID'], row['transactionStatus'])

    this_last_eventid = row['eventID'] 

# Catch the exception. Real exception handler would be more robust    
except _mssql.MssqlDatabaseException,e: 
    sys.stderr.write('Database Connection Error!\n')
    sys.exit(2)
    
finally:
    conn.close()
    
if this_last_eventid > 0:
   try:
     last_eventid_file = open(last_eventid_filepath,'w')
     last_eventid_file.write(this_last_eventid)
     last_eventid_file.close()
   # Catch the exception. Real exception handler would be more robust    
   except IOError:
     sys.stderr.write('Error writing last_eventid to file: ' + last_eventid_filepath + '\n')
     sys.exit(2)
