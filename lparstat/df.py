#!/usr/bin/python



import os, subprocess, csv, sys

lparfile = csv.reader(open('lpars.sys', 'rU'), delimiter='\t')
cfgfile = open('df_nonprd.cfg','w')
errfile = open('dferr.log', 'w')
cnt = 1

for lines in lparfile:
	try:
		env = lines[1]
		lpar = lines[0]
		if env != None:
		#if env == 'PERF':
			#print env + ' ' + lpar
			cmd = "nexec " + lpar + " df -m"
			#print cmd
			p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			cmdout, err = p.communicate()
			cfgfile.writelines('SystemNo' + '\t' + str(cnt) + '\n')
			cfgfile.writelines('Environment' + '\t' + env + '\n')
			cfgfile.writelines('LPAR' + '\t' + lpar + '\n')
			cfgfile.writelines(cmdout.replace(': ','\t') + '\n')
			errfile.writelines(err + '\n')
	except:
		print '\n'
		#print "Unexpected error:", sys.exc_info()[0]
	cnt +=1
cfgfile.close()




