from qboxhd import QBOXHD
from enigma import eTimer

from Screens.Screen import Screen

from Components.ActionMap import ActionMap, NumberActionMap
from Components.MenuList import MenuList

from SmartCard import *
from SmartCardATRInfoScreen import *
from SmartCardSubscriptionsInfoScreen import *
from SmartCardPursesInfoScreen import *



class SmartCardSelectDetailsScreen(Screen):
	""" 
	SmartCard view details"""

	skin = """
		<screen position="center,center" size="450,200" title="View SmartCard Details" >
			<widget name="menu" position="0,0" size="250,200" scrollbarMode="showOnDemand" />
		</screen>"""

	def __init__(self, session, idx):
		Screen.__init__(self, session)
		self.session = session
		self.skin = SmartCardSelectDetailsScreen.skin
		self.idx = idx
		self.list = []
		self.smartcard = SmartCardConax( idx )

		self["menu"] = MenuList(self.list)

		self.PollingSmartCardShowDetails()

		self.timer = eTimer()
		self.timer.timeout.get().append(self.PollingSmartCardShowDetails) 
		self.onShown.append(lambda: self.timer.start(1000))

		self["actions"] = NumberActionMap(["WizardActions", "InputActions", "EPGSelectActions"],
		{
			"ok": self.ok,
			"back": self.exit,
		}, -1)
		
	def PollingSmartCardShowDetails(self):

#		self.timer.stop()
		
		smartinfo = SmartCardConfig()
		self.list = []
		
		self.list.append(_("ATR"))
		
		#if smartinfo.checkSmartCardInserted( self.idx, self.smartcard ) == True:
		
		if ( smartinfo.loadSmartCardConfigfromSock( self.idx, self.smartcard ) ):
			if ( self.smartcard.codingsystem == SmartCardConax.CODINGSYSTEM_CONAX_IDENTIFIER ):
				self.list.append(_("Subscriptions"))
				self.list.append(_("Tokens"))

		self["menu"].setList( self.list )
		
#		self.timer.start(1000)



	def ok(self):
		idx = self["menu"].getSelectedIndex()
		if idx is 0:
			self.session.open( SmartCardATRInfoScreen, self.smartcard )
			print "[plugin.py:SmartCardATRInfoScreen] Select ATR"

		elif idx is 1:
			if ( self.smartcard.codingsystem == SmartCardConax.CODINGSYSTEM_CONAX_IDENTIFIER ):
				print "[plugin.py:SmartCardSubscriptionsInfoScreen] Select Subscriptions"
				self.session.open( SmartCardSubscriptionsInfoScreen, self.smartcard )
			else:
				print "[plugin.py:SmartCardSubscriptionsInfoScreen] Select Subscriptions - Not supported for this card"
		elif idx is 2:
			if ( self.smartcard.codingsystem == SmartCardConax.CODINGSYSTEM_CONAX_IDENTIFIER ):
				print "[plugin.py:SmartCardSelectDetailsScreen] Select Tokens"
				self.session.open( SmartCardPursesInfoScreen, self.smartcard )
			else:
				print "[plugin.py:SmartCardSelectDetailsScreen] Select Tokens - Not supported for this card"
		else:
			print "[plugin.py:SmartCardSelectDetailsScreen] Unknown Menupoint"

	
	def exit(self):
		self.timer.stop()
		self.close()
