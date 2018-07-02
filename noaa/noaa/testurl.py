#!/usr/bin/python


import urllib2, re, time, pprint, csv

#############proxy setup
proxy = urllib2.ProxyHandler({'ftp':'http://n0083510:1973jwpB@www-proxy:80'})
opener = urllib2.build_opener(proxy,urllib2.FTPHandler)
urllib2.install_opener(opener)

#############get raw lookup data


baseurl = 'ftp://WG\TelematicsPOC:Libertym+@207.115.82.98/RawDataFiles/Teletrac/'
#baseurl='ftp://ftp.ncdc.noaa.gov/pub/data/noaa/2012/'
response = urllib2.urlopen(baseurl).read()
print response




