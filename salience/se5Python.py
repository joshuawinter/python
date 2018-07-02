import saliencefive as SE5

# Define the path to Lexalytics root installation
LXA_HOME = '/opt/lexalytics/salience-5.0.6132'

def demo():
	print 'Initializing'
        print SE5.getVersion()

	session = SE5.openSession(LXA_HOME+'/license.v5',LXA_HOME+'/data')
	SE5.setOption_FailOnLongSentences(session,0)
	
	#Prepare the text.  This is done once and then you can carry
	#out multiple operations on the same document
	print 'Preparing Text...'
	'''
	ret = SE5.prepareText(session,"""Lexalytics Announces Salience 5.0, an Evolution in Semantic Understanding. Product Update Includes Concept Matrix, Collections, Facets, Additional Language Support and Emotional State Recognition.
	BOSTON, MA--(Marketwire - Apr 13, 2011) - Lexalytics, Inc. (www.lexalytics.com), a leading provider of text and sentiment analysis software, today announced a major upgrade to its Salience product at the company's annual Users Group event.

\"This release represents a quantum leap in the underlying technology for text analytics,\" said Jeff Catlin, CEO of Lexalytics, Inc.

Built around Lexalytics new patent pending technology \"Concept Matrices\", the Salience 5.0 release brings a whole new set of semantic features to the table. \"With this upgrade we're allowing our customers to understand the meanings hidden within their content,\" continued Catlin.

Salience 5.0 will be available in late summer 2011, and will enter a limited Beta in the early summer.

Lexalytics has created the \"Concept Matrix\" by digesting the entire human generated encyclopedia, Wikipedia (a registered trademark of the Wikimedia Foundation), which has enabled it to understand the complex relationships between words and meaning.

For example, the \"Concept Matrix\" allows Lexalytics to collapse phrases like: \"9-iron,\" \"golf club\" and \"Driver\" into the common concept of \"golf club.\" Furthermore, the concept of golf club is implicitly related to concepts like \"recreation\" and \"outdoors,\" allowing for comparisons at many different levels of meaning.

Building upon this new paradigm Lexalytics is introducing a number of powerful features, including:

    Facets: Extracts important subjects and their pertinent attributes.
    Collections: Will utilize the metadata across many documents to produce enhanced results.
    Conceptual Topics: The ability to define a classification subject with only a few simple concept keywords. Food for example could be defined as nothing more than (Food, Restaurant and Dining). This definition would sweep in content about gourmet pizza, or \"a cool new bar.\"
    Improved sentiment scoring via the understanding of subjective vs. objective phrasing.

About Lexalytics, Inc.
Lexalytics, Inc. is a software and services company specializing in text and sentiment analysis for social media monitoring, reputation management and entity-level text and sentiment analysis. By enabling organizations to make sense of the vast content repositories on sources like Twitter, blogs, forums, web sites and in-house documents, Lexalytics provides the context necessary for informed critical business decisions. Serving a range of Fortune 500 companies across a wide spectrum, Lexalytics partners with industry leaders such as Endeca, ThomsonReuters, Radian 6 and TripAdvisor to deliver the most effective sentiment and text analysis solutions in the industry. For more information, visit http://www.lexalytics.com. Lexalytics also provides expert commentary on text and sentiment analysis on its blog and Twitter via @Lexalytics.""")
'''
	ret = SE5.prepareTextFromFile(session,"/home/n0083510/workspace/python/.data")
	if ret == 0:
		print '\nDocument Statistics:'
		# Get the document details.
		details = SE5.getDocumentDetails(session)
		print 'Number of words: %s' % details['word_count']
		print 'Number of sentences: %s' % details['sentence_count']

		print '\nDocument Summary:'
		# Get the document summary. You can pass in the number of sentences you want in it.
		# This is returned as a string
		summary = SE5.getSummary(session,4)
		print '%s' % summary

		print '\nDocument Sentiment:'    
		# Get the sentiment of the document
		# This is returned as a float
		sentiment = SE5.getDocumentSentiment(session)
		print 'The sentiment is %f' % sentiment["score"]
    
		print 'There are %i sentiment phrases' % len(sentiment["phrases"])
		for phrase in sentiment["phrases"]:
			print '%s' %phrase["phrase"],
			print ' (%f)' %phrase["score"]
		#other dictionary strings are: sentence, word, length, byte_offset, byte_length, source
			#Added in SE4.4: is_negated, supporting_phrase
			if phrase["is_negated"] == 1:
				print ' Negated by: %s' %phrase["supporting_phrase"]["phrase"]

		print '\nDocument Themes:'
		# Get the themes
		# These are returned as a list of dictionaries
		themelist = SE5.getDocumentThemes(session)
		for theme in themelist:
			print '%s' %theme["theme"],
			print ' (%f)' %theme["score"]
		#other entries: stemmed_theme, theme_type, about. if theme_type=1, this is a metatheme. if about=1, theme appears in summary
        
		print '\nQuery-defined Document Topics (based on sample file in data directory):'
		#Get Query-defined topics
		ret = SE5.setOption_QueryTopicList(session,LXA_HOME+'/data/salience/tags/sampletags.dat')
		if ret==0:
        		topiclist = SE5.getQueryDefinedTopics(session)
                	for topic in topiclist:
                        	print '%s' %topic["topic"],
                                print ': %i hits,' %topic["hits"],
                                print ' Sentiment score: %f' %topic["score"]

		print '\nConcept-defined Document Topics (based on sample file in data directory):'
		#Get Concept-defined topics
		ret = SE5.setOption_ConceptTopicList(session,LXA_HOME+'/data/salience/concepts/example.dat')
		if ret==0:
                        conceptlist = SE5.getConceptDefinedTopics(session)
        		for concept in conceptlist:
                		print '%s' %concept["topic"],
                        	print ': %i hits,' %concept["hits"],
                                print ' Match score: %f' %concept["score"]
			
		print '\nNamed Entities:' 
		# Get the entities
		# These are returned as a list containing dictionary objects.
		entitylist = SE5.getNamedEntities(session)
		print 'There are %i entities' % len(entitylist)
		for entity in entitylist:
			print '%s' %entity["normalized_form"],
			print ': %s' %entity["type"]
		#other dictionary entries: label, evidence, summary, confident, about, themes(a list), mentions( a list)

		print '\nNamed Entity Relationships:'
		# Get the relationships
		relationlist = SE5.getNamedEntityRelationships(session)
		for relationship in relationlist:
			print '%s' %relationship["type"],
			print ' (%s)'		%relationship["extra"]
			for entity in relationship["entities"]:
				print '   %s' %entity["normalized_form"]
	
		print '\nNamed Entity Opinions:'
		#Get Opinions (Speaker,Topic,Sentiment) where topic is a theme or entity. Quotation is an additional field containing the opinion text.
		opinionlist = SE5.getNamedEntityOpinions(session);
		print 'There are %i opinions' % len(opinionlist)
		for opinion in opinionlist:
                        # Determine if it is an entity topic or theme topic
                        if "normalized_form" in opinion["topic"]:
        			print '%s' %opinion["speaker"]["normalized_form"],
                		print ' -> %s' %opinion["topic"]["normalized_form"],
                        	print ' (%f)' %opinion["sentiment"]
                        else:
        			print '%s' %opinion["speaker"]["normalized_form"],
                		print ' -> %s' %opinion["topic"]["theme"],
                        	print ' (%f)' %opinion["sentiment"]

		
		print '\nEntity-tagged Text:'
		#Get the entity tagged text
		print '%s' % SE5.getNamedEntityTaggedText(session)

		print '\nPOS-tagged Text:'
		#Get the POS tagged text
		print '%s' % SE5.getPOSTaggedText(session)

		print '\nSentiment-tagged Text:'
		#Get the sentiment tagged text
		print '%s' % SE5.getSentimentTaggedText(session)

	#Close the session down.  You shouldn't use the session object after making this call
	#Its important to note that if you let the sesssion go out of scope this it will automatically
	#call closeSession
	print '\nClosing session'
	ret = SE5.closeSession(session)

	print 'Finished'

if __name__ == "__main__":
	demo()
