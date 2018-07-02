#!/usr/local/bin/python2.7


import xml.etree.ElementTree as ET


tree = ET.parse('/home/n0083510/workspace/hadoop/weblogs/mstrweb/MSTRLog12272010_1.log')
doc = tree.getroot()

lst = []

for record in doc.iterfind('record'):
	print record
	
	
	
	
	
	
	
	
	
	'''for elem in record.iter():
		print elem.tag
		'''


#python xml2csv.py --input "/home/n0083510/workspace/hadoop/weblogs/mstrweb/MSTRLog12272010_1.log" --output "/home/n0083510/workspace/hadoop/weblogs/mstrweb/MSTRLog12272010_1.csv" --tag "record"
