#!/usr/bin/env	python
#Sandbox API Token 6ea1bc818f2f365fe3b784cf6231e904eaa5a280e06d8cfb
#https://api.hearsaysocial.com/v1/org/606/
#http://docs.hearsaysocial.com/index.html
#
#


import requests
import simplejson as json

#############proxy setup
proxies = {"https":"https://n0083510:BigBen73@www-proxy:80"}


token = '6ea1bc818f2f365fe3b784cf6231e904eaa5a280e06d8cfb'
url = 'https://api.hearsaysocial.com/v1/org/606/districts/?next_id=114859'
payload = {"name":"LSO Portsmouth","reference_id":"03801"}
data = json.dumps(payload)
headers = {'X-Auth-Token': token,'Content-type': 'application/json','Accept': 'text/plain'}
r = requests.get(url, proxies=proxies, headers=headers, verify=False)
decoded = r.json()




try:
	#decoded = json.loads(json_input)
	print json.dumps(decoded, sort_keys=True, indent=4 * ' ')

except (ValueError, KeyError, TypeError):    
	print "JSON format error"
