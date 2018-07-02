#!/usr/bin/env	python
#Sandbox API Token 6ea1bc818f2f365fe3b784cf6231e904eaa5a280e06d8cfb
#https://api.hearsaysocial.com/v1/org/606/
#http://docs.hearsaysocial.com/index.html
#
#


import urllib, urllib2
import simplejson as json

#############proxy setup
proxy = urllib2.ProxyHandler({'https':'https://n0083510:8077JWss@www-proxy:80'})
opener = urllib2.build_opener(proxy,urllib2.HTTPHandler)
urllib2.install_opener(opener)

token = '6ea1bc818f2f365fe3b784cf6231e904eaa5a280e06d8cfb'
url = 'https://api.hearsaysocial.com/v1/org/606/districts/104054'
jvalues = {"name":"LSO Portsmouth","reference_id":"03801"}
data = json.dumps(jvalues)


request = urllib2.Request(url)
request.add_header('X-Auth-Token', token)
request.add_header('Content-type', 'application/json')
request.add_header('Accept', 'text/plain')
response = urllib2.urlopen(request).read()
json_input = response

try:
	decoded = json.loads(json_input)
	print json.dumps(decoded, sort_keys=True, indent=4)
	
except (ValueError, KeyError, TypeError):    
	print "JSON format error"
