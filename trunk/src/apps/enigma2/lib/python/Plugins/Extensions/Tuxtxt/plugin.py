from qboxhd import QBOXHD
from enigma import *
from Screens.Screen import Screen
from Plugins.Plugin import PluginDescriptor
from enigma import iServiceInformation

class ShellStarter(Screen):
	skin = """
		<screen position="1,1" size="1,1" title="TuxTXT" >
                </screen>"""

        def __init__(self, session, args = None):
        	self.skin = ShellStarter.skin
		Screen.__init__(self, session)
		self.container=eConsoleAppContainer()
		if QBOXHD:
			self.container.appClosed.append(self.finished)
		else:
			self.container.appClosed.get().append(self.finished)
		self.runapp()
		
	def runapp(self):

		pids = self.session.nav.getCurrentService()
		if pids is not None:
		 	self.info = pids.info()
		 	txtPID = self.info.getInfo(iServiceInformation.sTXTPID)
		else:
			self.info = None

		eDBoxLCD.getInstance().lock()
		eRCInput.getInstance().lock()
		fbClass.getInstance().lock()
		
		if self.container.execute("/usr/local/bin/tuxtxt "+self.getValue(iServiceInformation.sTXTPID)):
			self.finished(-1)

	def finished(self,retval):
		if QBOXHD:
			fbClass.getInstance().unlock_parm(0)
		else:
			fbClass.getInstance().unlock()
		
		eRCInput.getInstance().unlock()
		eDBoxLCD.getInstance().unlock()
		
		if QBOXHD:
			eDBoxLCD.getInstance().enable_update()
#		if self.oldService is not None:
#			self.session.nav.playService(self.oldService)
		
		self.close()

	def getValue(self, what):
		if self.info is None:
			return ""
		
		v = "%d" % (self.info.getInfo(what))

		return v


def main(session, **kwargs):
	session.open(ShellStarter)

def Plugins(**kwargs):
	return PluginDescriptor(name="TuxTXT", description="Videotext", where = PluginDescriptor.WHERE_TELETEXT, fnc=main)
