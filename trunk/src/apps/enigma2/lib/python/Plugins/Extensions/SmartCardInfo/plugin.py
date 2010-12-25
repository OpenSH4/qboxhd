from qboxhd import QBOXHD
from Screens.Screen import Screen
from Screens.MessageBox import MessageBox
from Screens.ChoiceBox import ChoiceBox
from Plugins.Plugin import PluginDescriptor

from Components.MenuList import MenuList
from Components.ActionMap import ActionMap, NumberActionMap
from Components.Input import Input
from Components.Label import Label

from SmartCard import SmartCard
from SmartCard import SmartCardConax
from SmartCard import SubscriptionStatus
from SmartCard import PurseStatus
from SmartCardSelectDetailsScreen import SmartCardSelectDetailsScreen



class SmartCardSelectScreen(Screen):

	skin = """
		<screen position="center,center" size="400,200" title="Select SmartCard Slot" >
			<widget name="menu" position="0,0" size="250,200" scrollbarMode="showOnDemand" />
		</screen>"""

	def __init__(self, session):
		Screen.__init__(self, session)
		self.session = session

		self.skin = SmartCardSelectScreen.skin

		list = []
		list.append(_("Slot 1"))
		list.append(_("Slot 2"))

		self["menu"] = MenuList(list)

		self["actions"] = NumberActionMap(["WizardActions", "InputActions", "EPGSelectActions"],
		{
			"ok": self.ok,
			"back": self.exit,
		}, -1)



	def ok(self):
		idx = self["menu"].getSelectedIndex()
		if idx is 0:
			self.requestInfoaboutSmartCard( idx )

		elif idx is 1:
			self.requestInfoaboutSmartCard( idx )

		else:
			print "[plugin.py:SmartCardInfoScreen] Unknown Menupoint"




	def requestInfoaboutSmartCard(self, idx):
		self.session.open( SmartCardSelectDetailsScreen, idx )
		print "[plugin.py:SmartCardSelectDetailsScreen] Starting SmartCardSelectDetailsScreen Slot "+str(idx)




	def exit(self):
		self.close()




def main(session, **kwargs):
	session.open(SmartCardSelectScreen)

def menu(menuid, **kwargs):
	if menuid == "information":
		return [(_("SmartCard Information"), main, "smartcardinfo", None)]
	return []

def Plugins(**kwargs):
	return PluginDescriptor(name="SmartCard Information", description="plugin to view smartcard informations", where = PluginDescriptor.WHERE_MENU, fnc=menu)
