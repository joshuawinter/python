import saliencefive as SE5

# Define the path to Lexalytics root installation
LXA_HOME = '/opt/lexalytics/salience-5.0.6132'

def populateList():
    commentlist = []
    commentlist.append("The cruise was excellent and service from Princess was first class. Cruise lines should let people know that the on again off again tourist buses in the major cities are a very good tour and inexpensive. Our booking by our local Expedia agent Stuart Bustard was the best experience we have ever had. It was so well planned we had no worries at any time.",)
    commentlist.append("I found the temperature in the dining rooms very cold . Many passengers that we talked to seemed to agree. Otherwise the ship was very clean, comfortable and well maintained. The service and the food was excellent The outdoor movie theatre was a highlight for us.",)
    return commentlist

def demo():
	print 'Initializing'
   	print SE5.getVersion()

	session = SE5.openSession(LXA_HOME+'/license.v5',LXA_HOME+'/data')
	
	#Prepare the text.  This is done once and then you can carry
	#out multiple operations on the same document
	print 'Preparing the Collection...'
        lstContent = populateList()
        ret = SE5.prepareCollectionFromList(session, "demo_collection", lstContent)
	#ret = SE5.prepareCollectionFromFile(session,"c:/test/collection.txt")
	if ret == 0:
		print '\nNamed Entities:' 
		# Get the entities
		entitylist = SE5.getCollectionEntities(session)
		print 'There are %i entities' % len(entitylist)
		for entity in entitylist:
			print '%s' %entity["normalized"],
			print ': %s' %entity["type"]
			print 'Sentiment:\t POS=%s' %entity["positive_hits"],
			print '\t NEG=%s' %entity["negative_hits"],
			print '\t NEU=%s' %entity["neutral_hits"]

		print '\nFacets:' 
		# Get the facets
		facetlist = SE5.getCollectionFacets(session)
		print 'There are %i facets' % len(facetlist)
		for facet in facetlist:
			print '%s' %facet["facet"],
			print ': %s mentions' %len(facet["mentions"])
			print 'Sentiment:\t POS=%s' %facet["positive_hits"],
			print '\t NEG=%s' %facet["negative_hits"],
			print '\t NEU=%s' %facet["neutral_hits"]
			
		print '\nCollection Themes:'
		# Get the themes
		# These are returned as a list of dictionaries
		themelist = SE5.getCollectionThemes(session)
		for theme in themelist:
			print '%s' %theme["theme"],
			print ' (%f)' %theme["score"]
			print ' \tSentiment: (%f)' %theme["sentiment"]
        
		print '\nQuery-defined Topics (based on sample file in data directory):'
		#Get Query-defined topics
		ret = SE5.setOption_QueryTopicList(session,LXA_HOME+'/data/salience/tags/sampletags.dat')
		if ret==0:
                    topiclist = SE5.getCollectionQueryDefinedTopics(session)
                    for topic in topiclist:
			print '%s' %topic["topic"],
			print ': %i hits,' %topic["hits"],
			print ' Sentiment score: %f' %topic["score"]

		print '\nConcept-defined Topics (based on sample file in data directory):'
		#Get Concept-defined topics
		ret = SE5.setOption_ConceptTopicList(session,LXA_HOME+'/data/salience/concepts/example.dat')
		if ret==0:
                    conceptlist = SE5.getCollectionConceptDefinedTopics(session)
                    for concept in conceptlist:
			print '%s' %concept["topic"],
			print ': %i hits,' %concept["hits"],
			print ' Match score: %f' %concept["score"]
			
	#Close the session down.  You shouldn't use the session object after making this call
	#Its important to note that if you let the sesssion go out of scope this it will automatically
	#call closeSession
	print '\nClosing session'
	ret = SE5.closeSession(session)

	print 'Finished'

if __name__ == "__main__":
	demo()
