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


class SmartCardATRInfoScreen(Screen):

	skin = """
		<screen position="center,center" size="450,150" title="SmartCard ATR Info" >
			<widget name="CodingSystemLabel" position="0,20" size="140,25" font="Regular;20" />
			<widget name="CodingSystemdataLabel" position="145,20" size="300,25" font="Regular;20" />
			<widget name="ATRdataLabel" position="0,50" size="400,25" font="Regular;20" />
		</screen>"""

	def __init__(self, session, smartcard):
		Screen.__init__(self, session)
		self.session = session
		self.skin = SmartCardATRInfoScreen.skin
		self.smartcard = smartcard

		self["CodingSystemLabel"] = Label(_("Coding System: "))
		self["CodingSystemdataLabel"] = Label()
		self["ATRdataLabel"] = Label()

		self.showSmartCardATRInfo()

		self.timer = eTimer()
		self.timer.timeout.get().append(self.showSmartCardATRInfo) 
		self.onShown.append(lambda: self.timer.start(500))

		self["actions"] = NumberActionMap(["WizardActions", "InputActions", "EPGSelectActions"],
		{
			"ok": self.exit,
			"back": self.exit,
		}, -1)	



	def showSmartCardATRInfo(self):
		if ( self.smartcard.state == CARD_INITIALIZED ):
			self["CodingSystemdataLabel"].setText( str(self.smartcard.codingsystem) )		
			self["ATRdataLabel"].setText( str(self.smartcard.atr) )
		else:
			self["CodingSystemdataLabel"].setText( _(" ") )
			self["ATRdataLabel"].setText( _("...") )


	def exit(self):
		self.timer.stop()
		self.close()	
