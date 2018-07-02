#!/usr/bin/env python
 
import json
import sys
import urllib2
import os
import lxml
from bs4 import BeautifulSoup

 
proxy = urllib2.ProxyHandler({'http':'http://n0083510:1973JWpb@www-proxy:80'})
opener = urllib2.build_opener(proxy,urllib2.HTTPHandler)
urllib2.install_opener(opener)
 
# The screen name in Twitter, is the screen name of the user for whom to return results for. 
 
# Set the screen name to the second argument
search = "Liberty+Mutual"
 
# Open the twitter search URL the result will be shown in json format
response = urllib2.urlopen("http://search.twitter.com/search.json?q="+search).read()
soup = BeautifulSoup(response)
print(soup.prettify(formatter="html"))

'''
#convert the data and load it into json
data = json.load(url)
 
#to print out how many tweets there are
print len(data), "tweets"
 
# Start parse the tweets from the result
 
# Get only text
for tweet in data["results"]:
    print tweet["text"]
 
# Get the status and print out the contents
for status in data['results']:
    print "(%s) %s" % (status["created_at"], status["text"])
    
'''
