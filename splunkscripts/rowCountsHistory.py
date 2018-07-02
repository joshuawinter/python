#!/usr/bin/env	python
# author:  josh parsons, architect PI
# date: 2014-01-20
# EDW table sizes for splunk input....HISTORY....one time load.
# Make sure the last date in the eventid.txt is day-1.  



import pyodbc
import os, sys, time, csv
from datetime import date

timestr = time.strftime("%Y-%m-%d %H:%M:%S")
filestr = timestr

# File setup
ftables = csv.reader(open('/home/n0083510/workspace/python/splunkscripts/histtables.txt','r'),delimiter='\t')
fcntsout = open('/home/n0083510/workspace/python/splunkscripts/'+filestr+'.txt','w')

# TD Setup
dconn = 'DRIVER={Teradata};DBCNAME=TPPMPBI;UID=PM_EDW_SPLUNK_READ;PWD=vBpMa1Di;QUIETMODE=YES;'
pyodbc.pooling = False


stmt1 = """locking row for access select TSH.LogDate AS LOGDATE, TSH.Tablename AS TABLENAME,TSH.CURRENTPERM AS TABSIZE FROM PDCRINFO.TableSpace_Hst TSH \
				WHERE TSH.DatabaseName = 'PM_EDW_MSTR_D'\
				AND TSH.LogDate = """+"""'"""+str(last_eventid)+"""';"""
				# and TSH.Tablename like 'B_%';"""
stmt2 = """locking row for access select cast(count(*)as BigInt) as rowcnts from PM_EDW_MSTR_D."""
#stmt3 = """SELECT CAST(DM_ROW_PRCS_DTM AS DATE) as prcsdate, COUNT(*) FROM PM_EDW_MSTR_VIEW_OS_D.d_POLICY GROUP BY 1"""



# Run the statements and generate output to fhistout
try:
	cnx = pyodbc.connect(dconn, autocommit = True, ansi = True)
	cursor = cnx.cursor()
	cursor.execute(stmt1)
	rows1 = cursor.fetchall()
	for row1 in rows1:
		cursor.execute(stmt2+row1[1]+""";""")
		rows2 = cursor.fetchall()
   		for row2 in rows2:
			fhistout.writelines(str(row1[0])+' 00:00:00'+' loaddate='+str(row1[0])+' name='+row1[1]+' cnt='+str(row2[0])+' size='+str(row1[2])+' database='+targetdb+'\n')
	fhistout.close()
	this_last_eventid = str(date.today())

# Catch the exception. 
except pyodbc.Error,e: 
	sys.stderr.write('Database Connection Error!\n')
	sys.exit(2)
	
# Close the cursor and connection 
finally:
	cursor.close()
	cnx.close()
	
# Append last_eventid in last_eventid_fpath
if last_eventid < this_last_eventid:
	try:
		last_eventid_file = open(last_eventid_fpath,'a')
		last_eventid_file.writelines(this_last_eventid+'\n')
		last_eventid_file.close()   
	except IOError:
		sys.stderr.write('Error writing last_eventid to file: ' + last_eventid_filepath + '\n')
		sys.exit(2)
     
'''
