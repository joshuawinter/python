
#!/usr/bin/env	python

import pyodbc
import csv
from pprint import pprint


pyodbc.pooling = False
cnx = pyodbc.connect('DRIVER={Teradata};DBCNAME=tppmpbicop1;UID=n0083510;PWD=8077JWss;QUIETMODE=YES;', autocommit = True, ansi = True)
cursor = cnx.cursor()
cursor.execute("""select TSH.LogDate AS LOGDATE, TSH.Tablename AS TABLENAME,TSH.CURRENTPERM AS TABSIZE FROM PDCRINFO.TableSpace_Hst TSH 
				WHERE TSH.DatabaseName = 'PM_EDW_MSTR_D'\
				AND TSH.LogDate = (CURRENT_DATE -1) \
				AND TSH.Tablename like 'B_%'""")



rows1 = cursor.fetchall()
for row1 in rows1:
	cursor.execute("""select cast(count(*)as BigInt) as rowcnts from PM_EDW_MSTR_VIEW_OS_D."""+row1[1])
	rows2 = cursor.fetchall()
   	for row2 in rows2:
   			print str(row1[0])+','+row1[1]+','+str(row2[0])+','+str(row1[2])
cursor.close()

'''
with open('/home/n0083510/workspace/python/NOTES.data', 'w') as source_file:
    csv.writer(source_file, quoting=csv.QUOTE_NONNUMERIC).writerows(cursor)
source_file.close()
'''



