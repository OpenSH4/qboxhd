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


class SmartCardSubscriptionsInfoScreen(Screen):

	skin = """
		<screen position="center,center" size="450,300" title="SmartCard Subscriptions Info" >
			<widget name="SubScriptionList" position="0,0" size="440,290" scrollbarMode="showOnDemand" />
		</screen>"""

	def __init__(self, session, smartcard):
		Screen.__init__(self, session)
		self.session = session
		self.skin = SmartCardSubscriptionsInfoScreen.skin
		self.smartcard = smartcard

		self.list = []

		self["SubScriptionList"] = MenuList( self.list )

		self.showSmartCardSubscriptionsInfo()

		self.timer = eTimer()
		self.timer.timeout.get().append(self.showSmartCardSubscriptionsInfo) 
		self.onShown.append(lambda: self.timer.start(500))

		self["actions"] = NumberActionMap(["WizardActions", "InputActions", "EPGSelectActions"],
		{
			"ok": self.exit,
			"back": self.exit,
		}, -1)	



	def showSmartCardSubscriptionsInfo(self):
		self.list = []
		if ( (self.smartcard.state != CARD_INITIALIZED) or (self.smartcard.codingsystem != SmartCardConax.CODINGSYSTEM_CONAX_IDENTIFIER) ):
			self.list.append(_("SmartCard not supported."))
		else:
			for subscription in self.smartcard.subscriptions:
				self.list.append( str(subscription.label) );
				self.list.append( ("%s - %s %s") % (str(subscription.start_date1), str(subscription.end_date1), str(subscription.entitlement1) ) )
				self.list.append( ("%s - %s %s") % (str(subscription.start_date2), str(subscription.end_date2), str(subscription.entitlement2) ) )
				self.list.append( _(" ") )

			if ( len(self.list) == 0 ):
				self.list.append( _("No subscription defined.") )

		self["SubScriptionList"].setList( self.list )



	def exit(self):
		self.timer.stop()
		self.close()	
