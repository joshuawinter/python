#!/usr/bin/python

import happybase

connection = happybase.Connection('vxkit-phdp0003')
connection.open()
print connection.tables()




