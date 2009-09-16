import GEUtil
import sys

class OutputCatcher:
	def __init__(self):
		self.data=''
		
	def write(self, stuff):
		GEUtil.Msg(stuff)	

oc = OutputCatcher();		
sys.stdout = oc	
sys.__stdout__ = oc	


class OutputErrCatcher:
	def __init__(self):
		self.data=''
		
	def write(self, stuff):
		GEUtil.Warning(stuff)


oec = OutputErrCatcher();
sys.stderr = oec
sys.__stderr__ = oec
