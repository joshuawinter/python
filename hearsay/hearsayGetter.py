#!/usr/bin/env	python

#Sandbox API Token 6ea1bc818f2f365fe3b784cf6231e904eaa5a280e06d8cfb
#https://api.hearsaysocial.com/v1/org/606/
#http://docs.hearsaysocial.com/index.html
#
import requests
import simplejson as json
#############proxy setup
proxies = {"https":"https://n0083510:BigBen24@www-proxy:80"}
token = '6ea1bc818f2f365fe3b784cf6231e904eaa5a280e06d8cfb'
rootep = 'https://api.hearsaysocial.com/v1/org/606/'
#rootep = 'https://api.hearsaysocial.com/v1/org/606/districts/'
headers = {'X-Auth-Token': token,'Content-type': 'application/json','Accept': 'text/plain'}


'''
response = requests.get(rootep, proxies=proxies, headers=headers, verify=False)
jbag = response.json()
print json.dumps(jbag, sort_keys=True, indent=4 * ' ')
'''

def getDistricts(rootep,headers):
    districtep = rootep+'districts/'
    data = []
    links = []
    response = requests.get(districtep,proxies=proxies,headers=headers,verify=False)
    jbag = response.json()
    next_link = jbag['meta']['next_link']
    links.append(next_link)
    data.append(jbag['data'])
    #print data
    for ln in links:
        response = requests.get(next_link,proxies=proxies,headers=headers,verify=False)
        jbag = response.json()
        if jbag['meta']:
            next_link = jbag['meta']['next_link']
            links.append(next_link)
        data.append(jbag['data'])
    return data


results = {}
results = json.dumps(getDistricts(rootep,headers),sort_keys=True, indent=4 * ' ')
print results












