
# This application expects the user name, password, and
# developer token to be provided in the source below.
# Command line usage: python GetHeaders.py
# 

import elementtree.ElementTree as ET
import requests	

# This example host is for the sandbox environment.
# Update the URL as needed when you use the production environment.
host = "sharedservices.api.sandbox.bingads.microsoft.com"

# The following is the production host.
# host = "sharedservices.adcenterapi.microsoft.com"

# The Web service URI, proxy, and service operation definitions.
proxies = {"https":"https://n0083510:BigBen24@www-proxy:80"}
URI = "https://" + host + "/Api/CustomerManagement/V8/"
customerProxy = URI + "CustomerManagementService.svc?wsdl"

# The namespace definitions.
ns_cust = "https://adcenter.microsoft.com/api/customermanagement"
ns_ent = "https://adcenter.microsoft.com/api/customermanagement/Entities"

username = "test_liberty1"
password = "Liberty123"
developertoken = "BBD37VB98"


# Create a Web service client, and then execute the
# getCustomersInfo method.
def getCustomersInfo(): 

    soapStr = """<?xml version="1.0" encoding="utf-8"?>
    <soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:cus="https://adcenter.microsoft.com/api/customermanagement">
       <soapenv:Header>
          <cus:UserName>%s</cus:UserName>
          <cus:Password>%s</cus:Password>
          <cus:DeveloperToken>%s</cus:DeveloperToken>
       </soapenv:Header>
       <soapenv:Body>
          <cus:GetCustomersInfoRequest>

          </cus:GetCustomersInfoRequest>
       </soapenv:Body>
    </soapenv:Envelope>""" % (username, password, developertoken)

    # Create the Web service client, and then add the required headers.
    headers = {"Accept":"text/xml","Accept":"multipart/*","Content-type":"text/xml; charset=\"UTF-8\"","SOAPAction":"SubmitGenerateReport"}
    r = requests.post(customerProxy, data=soapStr, headers=headers, proxies=proxies, verify=False)
    #print r.text
    
    # Get the response message and results.

    
    if r.status_code == requests.codes.ok:
    
        response = ET.fromstring(r)
        
    else:

        # The method call failed.
        print r.text
        
        faultTree = ET.fromstring(r)
        print faultTree.findtext(".//faultcode"), " ", \
            faultTree.findtext(".//faultstring")
        
    return r if r else None

getCustomersInfo()   

'''
# Create a Web service client, and then execute the
# getAccountsInfo method.
def getAccountsInfo(customerId, onlyParentAccounts): 

    soapStr = """<?xml version="1.0" encoding="utf-8"?>
    <soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:cus="https://adcenter.microsoft.com/api/customermanagement">
       <soapenv:Header>
          <cus:UserName>%s</cus:UserName>
          <cus:Password>%s</cus:Password>
          <cus:DeveloperToken>%s</cus:DeveloperToken>
       </soapenv:Header>
       <soapenv:Body>
          <cus:GetAccountsInfoRequest>
             <cus:CustomerId>%s</cus:CustomerId>
             <cus:OnlyParentAccounts>%s</cus:OnlyParentAccounts>
          </cus:GetAccountsInfoRequest>
       </soapenv:Body>
    </soapenv:Envelope>""" % (username, password, developertoken, customerId, onlyParentAccounts)
    
    
    headers = {"Accept":"text/xml","Accept":"multipart/*","Content-type":"text/xml; charset=\"UTF-8\"","SOAPAction":"GetAccountsInfo"}
    r = requests.post(customerProxy, data=soapStr, headers=headers, proxies=proxies, verify=False)
    print r.text
customerId=21026476
onlyParentAccounts='true'
getAccountsInfo(customerId,onlyParentAccounts)
    
    

    # Create the Web service client, and then add the required headers.
    _service = httplib.HTTPS(host)
    _service.putrequest("POST", customerProxy)
    _service.putheader("Accept","text/xml")
    _service.putheader("Accept","multipart/*");
    _service.putheader("Content-type", "text/xml; charset=\"UTF-8\"")
    _service.putheader("Content-length", "%d" % len(soapStr))
    _service.putheader("SOAPAction", "GetAccountsInfo")
    _service.putheader("HOST", str(host))
    _service.endheaders()
    
    # Execute the Web service request.
    _service.send(soapStr)
    
    # Get the response message and results.
    statuscode, statusmessage, header = _service.getreply()
    res = _service.getfile().read()
    
    response = None
    
    if statusmessage == "OK":
    
        response = ET.fromstring(res)
        
    else:

        # The method call failed.
        print soapStr
        print "GetAccountsInfo failed.\n"
        print "Status Code: ", statuscode, statusmessage, "\n"
        print "Header: ", header, "\n"
        print res
        
        faultTree = ET.fromstring(res)
        print faultTree.findtext(".//faultcode"), " ", \
            faultTree.findtext(".//faultstring")
        
    return response if response else None
   

if __name__ == "__main__":
    
    print "Bing Ads API Header Elements for Current User\n\n"
    
    print "--------------------"
    print "Required Elements"
    print "--------------------"
    print "UserName: %s" % (username)
    print "Password: %s" % (password)
    print "DeveloperToken: %s\n" % (developertoken)
        
    print "--------------------"
    print "Elements Required for Some Service Operations"
    print "--------------------"
    
    customersinforesponse = getCustomersInfo (str(""), str(100), "Advertiser")
    
    if customersinforesponse:
        cInfos = customersinforesponse.findall(".//{" + ns_ent + "}CustomerInfo")
        for cInfo in cInfos:
            cId = (cInfo.find("{" + ns_ent + "}Id"))
            print "CustomerId: ", cId.text
            accountsinforesponse = getAccountsInfo (cId.text, "true")
            
            if accountsinforesponse:
                aInfos = accountsinforesponse.findall(".//{" + ns_ent + "}AccountInfo")
                for aInfo in aInfos:
                    aId = (aInfo.find("{" + ns_ent + "}Id"))
                    print "CustomerAccountId: ", aId.text
                print # blank line between customers
        
    print "--------------------"
    print "Elements Not Required and Reserved for Future Use"
    print "--------------------"
    print "ApplicationToken\n"
    
'''
