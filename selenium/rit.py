from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import Select
from selenium.common.exceptions import NoSuchElementException
import unittest, time, re

class RITReport(unittest.TestCase):
    def setUp(self):
        self.driver = webdriver.Firefox()
        self.driver.implicitly_wait(30)
        self.base_url = "https://requestit.lmig.com/LMAuth/requestit_login.fcc?TYPE=33554433&REALMOID=06-c04be491-ad51-1073-8933-83eaf6d90cb3&GUID=&SMAUTHREASON=0&METHOD=GET&SMAGENTNAME=-SM-K0D1Tpy2GzB%2fBKiy5lXFd5JNU9a%2bQE8p4dp5Ih16nDHlNQGv%2bxuP5FrKRmf6saem&TARGET=-SM-%2fweb%2fmyrequestit%2f"
        self.verificationErrors = []
        self.accept_next_alert = True
    
    def test_r_i_t_report(self):
        driver = self.driver
        driver.get(self.base_url + "/LMAuth/requestit_login.fcc?TYPE=33554433&REALMOID=06-c04be491-ad51-1073-8933-83eaf6d90cb3&GUID=&SMAUTHREASON=0&METHOD=GET&SMAGENTNAME=-SM-K0D1Tpy2GzB%2fBKiy5lXFd5JNU9a%2bQE8p4dp5Ih16nDHlNQGv%2bxuP5FrKRmf6saem&TARGET=-SM-%2fweb%2fmyrequestit%2f")
        driver.find_element_by_name("USER").clear()
        driver.find_element_by_name("USER").send_keys("n0083510")
        driver.find_element_by_name("PASSWORD").clear()
        driver.find_element_by_name("PASSWORD").send_keys("8077JWss")
        driver.find_element_by_name("submitButton").click()
        driver.find_element_by_link_text("Report Center").click()
        # ERROR: Caught exception [ERROR: Unsupported command [addSelection | name=ssg | label=Personal Insurance]]
        driver.find_element_by_name("node").clear()
        driver.find_element_by_name("node").send_keys("pn89kdc")
        driver.find_element_by_name("submitAction").click()
        driver.find_element_by_link_text("Export results to Excel-compatible file").click()
        # ERROR: Caught exception [ERROR: Unsupported command [selectWindow | name=MainWindow | ]]
        # ERROR: Caught exception [ERROR: Unsupported command [waitForPopUp | InfoWindow | 30000]]
        # ERROR: Caught exception [ERROR: Unsupported command [selectWindow | name=InfoWindow | ]]
        driver.find_element_by_link_text("Standard").click()
        # ERROR: Caught exception [ERROR: Unsupported command [selectWindow | name=MainWindow | ]]
        driver.find_element_by_link_text("Display detailed search results").click()
    
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

