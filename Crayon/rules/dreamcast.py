from SCons.Script import *	# Needed so we can use scons stuff like builders

def build_id():
   """Return a build ID (stub version)"""
   return "100"
def MakeWorkDir(workdir):
   """Create the specified dir immediately"""
   Execute(Mkdir(workdir))
