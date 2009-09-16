
import HLExample

class MyPyClass(HLExample.pythExample):
	def __init__(self):
		super(MyPyClass, self).__init__()

		self.string = "My Python Class!"
		
	def setMsg(self, inString):
		self.string = inString
			
	def getMsg(self):
		return self.string
		
	def sayHallo(self):
		print "Hallo!\n"
		
			
def NewPythonClass():
	return MyPyClass()
			


