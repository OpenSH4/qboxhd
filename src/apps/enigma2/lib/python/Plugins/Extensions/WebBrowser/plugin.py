from qboxhd import QBOXHD
from enigma import *
from Screens.Screen import Screen
from Plugins.Plugin import PluginDescriptor
from enigma import iServiceInformation
from Components.About import about
import os
from Tools.Notifications import AddPopup
from Screens.MessageBox import MessageBox
from time import sleep

class WebBrowserShell(Screen):
	skin = """
		<screen position="1,1" size="1,1" title="WebBrowser" >
                </screen>"""

        def __init__(self, session, args = None):
        	self.skin = WebBrowserShell.skin
		Screen.__init__(self, session)
		self.container=eConsoleAppContainer()
		
		self.container.appClosed.append(self.finished)
		
		self.Firstrun = True
		self.onShown.append(self.layoutFinished)

		
		
	def layoutFinished(self):
		if self.Firstrun:
			self.Firstrun = False
		else:
			return
			
		self.runapp()
		
		
	def showMsgCB(self, result):
		self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
		self.session.nav.stopService()
		
		eDBoxLCD.getInstance().lock()
		eRCInput.getInstance().lock()
		fbClass.getInstance().lock()
		
		res = open("/proc/stb/video/videomode", "r").read()
		if res.find("480") !=-1 :
			pos_x=46
			pos_y=16
			s_x=630
			s_y=420
		elif  res.find("576") !=-1 :
			pos_x=46
			pos_y=16
			s_x=630
			s_y=545	
		elif  res.find("720") !=-1 :
			pos_x=40
			pos_y=20
			s_x=1200
			s_y=630	
		elif  res.find("1080") !=-1 :
			pos_x=60
			pos_y=40
			s_x=1795
			s_y=1000
		else:
			print "Unknown resolutio set 720x576"		
			pos_x=46
			pos_y=16
			s_x=630
			s_y=545	
		
		cmd="links -g www.google.it %s %s %s %s " % (str(pos_x),str(pos_y),str(s_x),str(s_y) )
		print "cmd: ",cmd 
		if self.container.execute(cmd):
			self.finished(-1)
			
	def exitCB(self, result):
		self.close()
		
	def runapp(self):

		if (about.getMouseInfo() == "None") or (about.getKeyBoardInfo() == "None"):
			print "Keyboard or Mouse doesn\'t connect"
			self.session.openWithCallback(self.exitCB, MessageBox, _("No Keyboard or Mouse detected!\nTo launch the Web Browser you need to have a Keyboard and Mouse connected on the USB Port.\nPlease connect any USB Keyboard and Mouse into any USB Port on the back side of your QBox receiver.\n\nTIP: If you have no USB Port free left, please use a USB Hub!"), type = MessageBox.TYPE_INFO)

			return

		self.session.openWithCallback(self.showMsgCB, MessageBox, _("Short Help:\n\nESC	display menu\nctrl+C or q	quit\ng	go to url\nctrl+R	reload\nup,down	select link\nright,enter	follow link\nleft,z	go back\n/	search\nn	find next\nN	find previous\nd	download\n|	HTTP header\n\nPress OK to continue"), type = MessageBox.TYPE_INFO)
		


	def finished(self,retval):
		
		print "Re-create the correct \'directfbrc\' link for e2"
		os.system("rm -fr /etc/directfbrc")
		os.system("ln -s /etc/directfbrc_e2 /etc/directfbrc")
		sleep(1)
		
		fbClass.getInstance().unlock()
		eRCInput.getInstance().unlock()
		eDBoxLCD.getInstance().unlock()
		eDBoxLCD.getInstance().enable_update()
		
		if self.oldService is not None:
			self.session.nav.playService(self.oldService)
		
		self.session.openWithCallback(self.exitCB, MessageBox, _("Exit from web browser successfully"), type = MessageBox.TYPE_INFO, timeout = 3)