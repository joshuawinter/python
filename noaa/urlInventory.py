#!/usr/bin/python
"""
    Downloads all links from a specified location and saves to machine.
    Downloaded links will only be of a lower level then links specified.
    To use: python downloader.py link
"""
import sys,re,os,urllib2,urllib,urlparse



tocrawl = set([sys.argv[1]])

#tocrawl = set('ftp://ftp.ncdc.noaa.gov/pub/data/noaa/2012/')
# linkregex = re.compile('<a\s*href=[\'|"](.*?)[\'"].*?')

linkregex = re.compile('href=[\'|"](.*?)[\'"].*?')
linksrc = re.compile('src=[\'|"](.*?)[\'"].*?')

def main():
    proxy = urllib2.ProxyHandler({'ftp':'http://n0083510:1973jwpB@www-proxy:80'})
    opener = urllib2.build_opener(proxy,urllib2.FTPHandler)
    urllib2.install_opener(opener)
    
    link_list = []##create a list of all found links so there are no duplicates
    restrict = sys.argv[1]##used to restrict found links to only have lower level
    link_list.append(restrict)
    parent_folder = restrict.rfind('/', 0, len(restrict)-1)
    ##a.com/b/c/d/ make /d/ as parent folder
    while 1:
        try:
            crawling = tocrawl.pop()
            #print crawling
        except KeyError:
            break
        url = urlparse.urlparse(crawling)##splits url into sections
        try:   
            response = urllib2.urlopen(crawling)##try to open the url
        except:
            continue
        msg = response.read()##save source of url
        links = linkregex.findall(msg)##search for all href in source
        links = links + linksrc.findall(msg)##search for all src in source
        for link in (links.pop(0) for _ in xrange(len(links))):
            if link.startswith('/'):
                ##if /xxx a.com/b/c/ -> a.com/b/c/xxx
                link = 'http://' + url[1] + link
            elif ~link.find('#'):
                continue
            elif link.startswith('../'):
                if link.find('../../'):##only use links that are max 1 level above reference
                    ##if ../xxx.html a.com/b/c/d.html -> a.com/b/xxx.html
                    parent_pos = url[2].rfind('/')
                    parent_pos = url[2].rfind('/', 0, parent_pos-2) + 1
                    parent_url = url[2][:parent_pos]
                    new_link = link.find('/')+1
                    link = link[new_link:]
                    link = 'http://' + url[1] + parent_url + link
                else:
                    continue
            elif not link.startswith('http'):
                if url[2].find('.html'):
                    ##if xxx.html a.com/b/c/d.html -> a.com/b/c/xxx.html
                    a = url[2].rfind('/')+1
                    parent = url[2][:a]
                    link = 'http://' + url[1] + parent + link
                else:
                    ##if xxx.html a.com/b/c/ -> a.com/b/c/xxx.html
                    link = 'http://' + url[1] + url[2] + link
            if link not in link_list:
                link_list.append(link)##add link to list of already found links
                if (~link.find(restrict)):
                ##only grab links which are below input site
                    print link ##print downloaded link
                    tocrawl.add(link)##add link to pending view links
                    file_name = link[parent_folder+1:]##folder structure for files to be saved
                    filename = file_name.rfind('/')
                    folder = file_name[:filename]##creates folder names
                    folder = os.path.abspath(folder)##creates folder path
                    if not os.path.exists(folder):
                        os.makedirs(folder)##make folder if it does not exist
                    try:
                        urllib.urlretrieve(link, file_name)##download the link
                    except:
                        print "could not download %s"%link
                else:
                    continue
if __name__ == "__main__":
    main() 

