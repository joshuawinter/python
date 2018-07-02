#!/usr/bin/env python



from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import Select
from selenium.common.exceptions import NoSuchElementException
import os, unittest, time, re, csv

i=0
ufile = csv.reader(open('C:\\testcases\\001_Hearsay\\testusers.txt','rU'),delimiter=',')
for u in ufile:
    user = u[0]
    pwd = u[1]
    profilepath = 'C:\\testcases\\001_Hearsay\\profiles\\' + str(i) + '_' + user
    if not os.path.exists(profilepath):
            os.makedirs(profilepath)
    #print profilepath

    profile = webdriver.FirefoxProfile(profile_directory=profilepath)
    profile.set_preference("network.proxy.type", 1)
    profile.set_preference("network.proxy.http", "www-proxy")
    profile.set_preference("network.proxy.http_port", 80)
    profile.set_preference("network.proxy.ssl", "www-proxy")
    profile.set_preference("network.proxy.ssl_port", 80)
    profile.set_preference("network.proxy.ftp", "www-proxy")
    profile.set_preference("network.proxy.ftp_port", 80)
    profile.set_preference("network.proxy.socks", "www-proxy")
    profile.set_preference("network.proxy.socks_port", 80)
    profile.set_preference("network.proxy.no_proxies_on","localhost,127.0.0.1")
    profile.update_preferences()



    driver = webdriver.Firefox(profile)
    driver.implicitly_wait(30)
    base_url = "https://hearsay.libertymutual.com/hearsay/"
    driver.get(base_url + "LMAuth/login.fcc?TYPE=33554433&REALMOID=06-000ba503-6f7e-128e-89bf-07230afdf045&GUID=0&SMAUTHREASON=0&METHOD=GET&SMAGENTNAME=-SM-jlsDJYBwyuVPGbHFYOZjclO%2brAvQrTZLHemjnAsLoN%2fWwGws0idcrmp%2fyHSWtOsP&TARGET=-SM-http%3a%2f%2fhearsay%2elibertymutual%2ecom%2fhearsay%2f")
    driver.find_element_by_name("USER").clear()
    driver.find_element_by_name("USER").send_keys(user)
    driver.find_element_by_name("PASSWORD").clear()
    driver.find_element_by_name("PASSWORD").send_keys(pwd)
    driver.find_element_by_css_selector("input[type=\"button\"]").click()
    i+=1
driver.quit()


    


