#!/usr/bin/python

import cherrypy
from mako.template import Template
from mako.lookup import TemplateLookup



mylookup = TemplateLookup(directories=['html_stubs'])

class arjunaSampleApplication:
        @cherrypy.expose
        def index(self):
            mytemplate=mylookup.get_template("arjunaTest.html")
            return mytemplate.render()

        @cherrypy.expose
        def arjunaOutputPage(self):
            mytemplate=mylookup.get_template("arjunaTestOutput.html")
            return mytemplate.render(arjunaName="arjuna", arjunaWorld="planet of cherries")



#The Application root
arjunaroot=arjunaSampleApplication()

# Set up global configuration
cherrypy.config.update("arjunaGlobal.conf")

# Set up application level configuration
cherrypy.tree.mount(arjunaroot,'/',"arjunaApplication.conf")

#start the server
cherrypy.quickstart(arjunaSampleApplication())
#cherrypy.engine.start() 
