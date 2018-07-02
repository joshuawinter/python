from distutils.core import setup, Extension
import os, sys
import platform

# Should we build in 32bit compatibility mode? (implies -m32)
build_32bit_compat = False

py_bits = platform.architecture()[0]  # e.g. 32bit 
machine = platform.machine()          # e.g. x86_64

if py_bits == '32bit' and machine == 'x86_64':
    # we're using a 32bit python on a 64bit machine
    build_32bit_compat = True

extra_compile_args = []
extra_link_args = ['-pthread', '-shared']

if build_32bit_compat:
    extra_compile_args.append('-m32')
    extra_link_args.append('-m32')
    
pysaliencefive = Extension('saliencefive',
                  extra_compile_args = extra_compile_args,
                  extra_link_args = extra_link_args,
                  include_dirs = ['../../include'],
                  library_dirs = ['../../../lib'],
                  libraries = ['SalienceFive'],
                  sources = ['py_saliencefive.cpp'])

r = setup(name = 'SalienceEngine',
      version = '5.0',
      description = 'API Wrapper for Lexalytics Salience Engine',
      url = 'http://dev.lexalytics.com/wiki/',
      author = 'Lexalytics Inc',
      author_email = 'info@lexalytics.com',
      ext_modules = [pysaliencefive])

if os.path.isfile('/usr/bin/chcon') and 'install' in r.commands:
    print
    print "NOTE: You may need to run the following before you can use the python extension under SELinux:"
    print "   chcon -t texrel_shlib_t %s/lib/python2.x/site-packages/saliencefive.so" % sys.prefix

