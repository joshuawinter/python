#!/usr/bin/env python
#http://answers.oreilly.com/topic/1974-use-beautifulsoup-to-parse-data-gov/
#python pull_datadotgov.py < urls.txt > usage.txt



import urllib, urllib2
from bs4 import BeautifulSoup
import sys

def pull_downloads( url):
	ret_val = ""   
	txt = urllib.urlopen(url).read()  #Pull in the url   
	soup = BeautifulSoup(txt)   
	rows = soup.findAll("div", {"class" : "categories"})
	for row in rows:
		cat = row.findAll('h2');
		if len(cat) > 0:
			if str(cat[0]).find("Dataset Metrics") > 0:
				recs = row.findAll("td", { "class": "tablepad data"})            
				for rec in recs:               
					for c in rec.renderContents():                  
						if c in ["0","1","2","3,","4","5","6","7","8","9"]: ret_val += c   
return ret_val

urls = sys.stdin.readlines()
for url in urls:
	u = url.split("\n")[0]
	n = pull_downloads(u)
	print "%s\t%s" % (u, n)
