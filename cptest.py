#!/usr/bin/python
import cherrypy



import cherrypy

class HelloWorld:
   def index(self):
      return "Fuck You!"
   index.exposed = True

cherrypy.quickstart(HelloWorld())

