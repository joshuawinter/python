#!/usr/bin/env python
###author: josh parsons, architect LMIG.
###date: 8/1/2013


import urllib, urllib2, re, time, pprint, csv, codecs
from bs4 import BeautifulSoup



#############proxy setup
 
proxy = urllib2.ProxyHandler({'http':'http://n0083510:1973JWpb@www-proxy:80'})
opener = urllib2.build_opener(proxy,urllib2.HTTPHandler)
opener.addheaders = [('User-Agent','Mozilla/5.0')]
urllib2.install_opener(opener)



	
#############get raw lookup data and write it out to file.
baseurl = 'http://www.vegasinsider.com/nfl/odds/las-vegas/money/'
response = urllib2.urlopen(baseurl).read()
soup = BeautifulSoup(response,from_encoding="windows-1252")
rows = soup.findAll("tr")
print str(rows)





	
	
	
	
	
	
	
	
'''
urllist = open('/home/n0083510/workspace/python/splunk-sdk-python-1.1/mother/moneylines.csv','wb')
i=1
for l in bag:
	liner = str(i)+'\t'+l+'\t'+baseurl+urllib.quote_plus(l)+'\t'+'-NONE-\n'
	urllist.write(liner)
	i += 1
urllist.close()
	
	

###execute url update status and save file
datdir = '/data/boilerroom/ncdc/'
with open('/home/n0083510/workspace/python/noaa/ncdc_urls.csv','r+') as hitlist:
	fReader = csv.reader(hitlist,delimiter='\t')
	for row in fReader:
		response = urllib2.urlopen(row[2])
		the_data = response.read()
		the_file = row[1]
		f = open(datdir+the_file,'wb')
		f.writelines(the_data)
		f.close()
		row[3].replace('-NONE-','-DOWNLOADED-')

fReader.close()
		

	
'''










