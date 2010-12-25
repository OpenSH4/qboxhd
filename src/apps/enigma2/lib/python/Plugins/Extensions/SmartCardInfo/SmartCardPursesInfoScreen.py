from qboxhd import QBOXHD
from enigma import eTimer

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox
from Screens.ChoiceBox import ChoiceBox

from Components.ActionMap import ActionMap, NumberActionMap
from Components.MenuList import MenuList
from Components.Input import Input
from Components.Label import Label

from SmartCard import *


class SmartCardPursesInfoScreen(Screen):

	skin = """
		<screen position="center,center" size="450,300" title="SmartCard Purses Info" >
			<widget name="PurseList" position="0,0" size="440,290" scrollbarMode="showOnDemand" />
		</screen>"""

	def __init__(self, session, smartcard):
		Screen.__init__(self, session)
		self.session = session
		self.skin = SmartCardPursesInfoScreen.skin
		self.smartcard = smartcard

		self.list = []

		self["PurseList"] = MenuList( self.list )

		self.showSmartCardPursesInfo()

		self.timer = eTimer()
		self.timer.timeout.get().append(self.showSmartCardPursesInfo) 
		self.onShown.append(lambda: self.timer.start(500))

		self["actions"] = NumberActionMap(["WizardActions", "InputActions", "EPGSelectActions"],
		{
			"ok": self.exit,
			"back": self.exit,
		}, -1)	



	def showSmartCardPursesInfo(self):
		self.list = []
		if ( (self.smartcard.state != CARD_INITIALIZED) or (self.smartcard.codingsystem != SmartCardConax.CODINGSYSTEM_CONAX_IDENTIFIER) ):
			self.list.append(_("SmartCard not supported."))
		else:
			for purse in self.smartcard.purses:
				self.list.append( ("%s tokens") % (str(purse.label)) )
				self.list.append( ("Purse Balance: %s tokens") % (str(purse.balance) ) )
				self.list.append( _(" ") )

			if ( len(self.list) == 0 ):
				self.list.append( _("No purses defined.") )

		self["PurseList"].setList( self.list )

	def exit(self):
		self.timer.stop()
		self.close()	
