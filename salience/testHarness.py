#!/usr/bin/python



import saliencefive as SE5

# Define the path to Lexalytics root installation
LXA_HOME = '/opt/lexalytics/salience-5.0.6132'

print 'Initializing'
print SE5.getVersion()

session = SE5.openSession(LXA_HOME+'/license.v5',LXA_HOME+'/data')
SE5.setOption_FailOnLongSentences(session,0)
SE5.setOption_ConceptTopicList(session,LXA_HOME+'/data/salience/concepts/example.dat')
	
#Prepare the text.
print 'Preparing Text...'
ret = SE5.prepareTextFromFile(session,"/home/n0083510/workspace/python/clean.data")



if (ret == 0):
	
	print '\nDocument Statistics:'
	# Get the document details.
	details = SE5.getDocumentDetails(session)
	print 'Number of words: %s' % details['word_count']
	print 'Number of sentences: %s' % details['sentence_count']
	
	print '\nNamed Entities:' 
	# Get the entities
	# These are returned as a list containing dictionary objects.
	entitylist = SE5.getNamedEntities(session)
	print 'There are %i entities' % len(entitylist)
	for entity in entitylist:
		print '%s' %entity["normalized_form"],
		print ': %s' %entity["type"]
	#other dictionary entries: label, evidence, summary, confident, about, themes(a list), mentions( a list)



else:
	if (ret == 6):
		print SE5.getLastWarnings(session) 

SE5.closeSession(session)

