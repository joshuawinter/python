#!/usr/bin/env	python


from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import Select
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.common.proxy import *
import unittest, time, re, os


ffbin = webdriver.firefox.firefox_binary.FirefoxBinary("/data/workspace/firefox/firefox")


profile = webdriver.FirefoxProfile()#"/home/n0083510/.mozilla/firefox/mxf5h0du.Selenium/") 

profile.set_preference("network.proxy.type", 1)
profile.set_preference("network.proxy.http", "www-proxy")
profile.set_preference("network.proxy.http_port", 80)
profile.set_preference("network.proxy.ssl", "www-proxy")
profile.set_preference("network.proxy.ssl_port", 80)
profile.set_preference("network.proxy.ftp", "www-proxy")
profile.set_preference("network.proxy.ftp_port", 80)
profile.set_preference("network.proxy.socks", "www-proxy")
profile.set_preference("network.proxy.socks_port", 80)
profile.set_preference("network.proxy.user", "LM\\N0083510")
profile.set_preference("network.proxy.password", "1973JWPb")
profile.set_preference("network.proxy.no_proxies_on","localhost,127.0.0.1")
profile.update_preferences() 


driver = webdriver.Firefox(firefox_profile=profile,firefox_binary=ffbin)
driver.get("http://www.python.org")
assert "Python" in driver.title
elem = driver.find_element_by_name("q")
elem.send_keys("selenium")
elem.send_keys(Keys.RETURN)
assert "Google" in driver.title
driver.close()


'''
class HearsayTest(unittest.TestCase):
    def setUp(self):
        self.driver = webdriver.Firefox(firefox_profile=profile,firefox_binary=ffbin)
        self.driver.implicitly_wait(30)
        self.base_url = "https://tst-hearsay.libertymutual.com/hearsay/"
        self.verificationErrors = []
        self.accept_next_alert = True
    
    def test_hearsay(self):
        driver = self.driver
        driver.get(self.base_url + "/LMAuth/login.fcc?TYPE=33554433&REALMOID=06-000b2d15-6429-1265-aeae-17260afdf095&GUID=0&SMAUTHREASON=0&METHOD=GET&SMAGENTNAME=-SM-nwj%2frRdgAus%2fEY2YhGOfaYstnvVmFpseAykQiTduS1DEMVKer0VheIOtue9jlbjH2zJH9JueK4KgA6SY4mZ78lgkdtY0dSRe&TARGET=-SM-http%3a%2f%2ftst--hearsay%2elibertymutual%2ecom%2fhearsay%2f")
        driver.find_element_by_name("USER").clear()
        driver.find_element_by_name("USER").send_keys("n0083510")
        driver.find_element_by_name("PASSWORD").clear()
        driver.find_element_by_name("PASSWORD").send_keys("BigBen8077")
        driver.find_element_by_css_selector("input[type=\"button\"]").click()
        driver.close()
    
    def is_element_present(self, how, what):
        try: self.driver.find_element(by=how, value=what)
        except NoSuchElementException, e: return False
        return True
    
    def is_alert_present(self):
        try: self.driver.switch_to_alert()
        except NoAlertPresentException, e: return False
        return True
    
    def close_alert_and_get_its_text(self):
        try:
            alert = self.driver.switch_to_alert()
            alert_text = alert.text
            if self.accept_next_alert:
                alert.accept()
            else:
                alert.dismiss()
            return alert_text
        finally: self.accept_next_alert = True
    
    def tearDown(self):
        self.driver.quit()
        self.assertEqual([], self.verificationErrors)

if __name__ == "__main__":
    unittest.main()
 '''  
