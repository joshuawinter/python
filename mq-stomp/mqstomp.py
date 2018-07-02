#!/usr/bin/env python
'''
import time
import sys
import stomp

class MyListener(object):
    def on_error(self, headers, message):
        print 'received an error %s' % message

    def on_message(self, headers, message):
        print 'received a message %s' % message

conn = stomp.Connection()
conn.set_listener('', MyListener())
conn.start()
conn.connect()

conn.subscribe(destination='/queue/mq-0001', ack='auto')

conn.send(' '.join(sys.argv[1:]), destination='/queue/mq-0001')

time.sleep(2)
conn.disconnect()
'''

import stomp
import time
import logging
import sys
import random

logging.basicConfig()

class MyListener(stomp.ConnectionListener):
    def on_error(self, headers, message):
        print('received an error %s' % message)
    def on_message(self, headers, message):
        for k,v in headers.iteritems():
            print('header: key %s , value %s' %(k,v))
        print('received message\n %s'% message)


dest='/queue/mq-0001'
conn=stomp.Connection([('localhost',61612)])
print('set up Connection')
conn.set_listener('somename',MyListener())
print('Set up listener')

conn.start()
print('started connection')

conn.connect(wait=True)
print('connected')
conn.subscribe(destination=dest, ack='auto')
print('subscribed')

message='hello cruel world'
conn.send(message=message, destination=dest,headers={'seltype':'mandi-age-to-man','type':'textMessage','MessageNumber':random.randint(0,65535)},ack='auto')
print('sent message')
time.sleep(2)
print('slept')
conn.disconnect()
print('disconnected')



