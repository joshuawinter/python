#!/usr/bin/python
##team of rivals, doris goodwin
##author:  joshua0055.parsons@libertymutual.com
##

import csv, socket, datetime, smtplib, sys, string


cfgfile = csv.reader(open('envs.cfg', 'rU'), delimiter=',')
msgfile = open('message.out','w')
smtphost = 'SMTPRELAY.lmig.com'
sender = 'DOWN@libertymutual.com'
receivers = ['PMOASIS_ADMIN@libertymutual.com','shannon.may@libertymutual.com','joshua0055.parsons@libertymutual.com']
#tier2 = 'somebody@somebody.com'
subject = 'ISERVER DOWN EVENT'

####function to check if you can connect to target host and port.
def isOpen(host,port):    
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)    
	try:     
		s.connect((host, int(port)))     
		s.shutdown(2)
		return 'UP'
	except:    
		return 'DOWN'
###load cfg file and check to see if target socket is open and build message file.
for lines in cfgfile:
	try:
		status = isOpen(lines[1],lines[3])
		if status == 'DOWN':
			#print lines[0] + ' iserver: ' + lines[1] + ' status: ' + status + ' @ ' + str(datetime.datetime.now())+ '\n'
			msgfile.writelines(lines[0] + ' iserver: ' + lines[1] + ' status: ' + status + ' @ ' + str(datetime.datetime.now())+ '\n')
			s.close()
	except:
		print '\n'
		#print "Unexpected error:", sys.exc_info()[0]
msgfile.close()
###read message file output from  and load into message
buffer = ""
message = open('message.out','rU').readlines()
for m in message:
	buffer += m + '\n'
message = buffer
msgfile.close()

#send messages
msg1 = 'From: %s\r\nTo: %s\r\nSubject: %s\r\nX-Mailer: My-Mail\r\n\r\n' % (sender, receivers, subject)
msg2 = '\n'+'\n'+'PLEASE ACKNOWLEDGE'

if len(message) > 1: 
	try:
		server = smtplib.SMTP(smtphost,25)
		server.sendmail(sender, receivers, msg1+message+msg2)  
	except:
		print '\n'
		#print "Unexpected error:", sys.exc_info()[0]
	server.close()




