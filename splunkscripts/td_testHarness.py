#!/usr/bin/env	python

import pyodbc
import csv
from pprint import pprint
import os, sys, time
from time import localtime,strftime

#####dbms
dconn = 'DRIVER={Teradata};DBCNAME=tkpmqbi;UID=n0083510;PWD=8077JWss;QUIETMODE=YES;'
stmt1 = """select PERSN_SID from pm_edw_mstr_view_os_d.B_OCCURRENCE_WITNESS SAMPLE 10;"""
pyodbc.pooling = False

print stmt1


cnx = pyodbc.connect(dconn, autocommit = True, ansi = True)
cursor = cnx.cursor()
cursor.execute(stmt1)
rows1 = str(cursor.fetchall())
for row1 in rows1:
	print row1
		

