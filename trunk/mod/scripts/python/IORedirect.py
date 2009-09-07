import HLUtil
import sys

class OutputCatcher:
	def __init__(self):
		self.data=''
		
	def write(self, stuff):
		HLUtil.Msg(stuff)	

oc = OutputCatcher();		
sys.stdout = oc	


class OutputErrCatcher:
	def __init__(self):
		self.data=''
		
	def write(self, stuff):
		HLUtil.Warning(stuff)


oec = OutputErrCatcher();
sys.stderr = oec
