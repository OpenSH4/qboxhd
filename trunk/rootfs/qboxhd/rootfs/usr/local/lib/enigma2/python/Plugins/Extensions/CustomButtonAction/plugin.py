from qboxhd import QBOXHD
from Screens.Screen import Screen
from Screens.MessageBox import MessageBox
from Screens.ChoiceBox import ChoiceBox
from Plugins.Plugin import PluginDescriptor
from Tools.Directories import resolveFilename, SCOPE_SKIN

import xml.dom.minidom
from xml.dom.minidom import Node

from os import system

#from Components.ConfigList import ConfigList
from Components.MenuList import MenuList
from Components.ActionMap import ActionMap, NumberActionMap
from Components.Input import Input
from Components.Label import Label
from Components.SystemInfo import SystemInfo


keycustomfile = file(resolveFilename(SCOPE_SKIN, 'keycustomactions.xml'), 'r')
keycustomxml = xml.dom.minidom.parseString(keycustomfile.read())
keycustomfile.close()

class CustomButtonActionMenu(Screen):
	
	skin = """
		<screen position="135,144" size="450,300" title="Define Custom Button action ..." >
			<widget name="actionlist" position="10,10" size="430,240" />
		</screen>"""

	def __init__(self, session):
		Screen.__init__(self, session)
		self.session = session
		
		self.skin = CustomButtonActionMenu.skin
		self.list = [ ]
		self["actionlist"] =  MenuList(self.list)
		
		activecustom = ""
		
		try:
			fp = file('/var/custombutton.dat', 'r')
			activecustom = fp.readline()
			print "ACTIVE CUSTOM:", str(activecustom)
			fp.close()
		except:
			pass
		
		xmldata = keycustomxml.childNodes[0]
		entries = xmldata.childNodes
		idx=-1
		self.idxactive=-1
		for x in entries:             #walk through the actual nodelist
			if (x.nodeType == Node.ELEMENT_NODE and x.tagName == 'item'):
				
				if ((len(str(x.getAttribute("requires")))>0) and (not SystemInfo.get(str(x.getAttribute("requires")), False))):
					pass
				else:
					idx = idx + 1
					print "FOUND NODE ELEMENT:", x.getAttribute("name")
					self.list.append( str(x.getAttribute("name")) )
					if len(activecustom) > 0:
						if activecustom == str(x.getAttribute("name")):
							self.idxactive=idx
						
		self["actionlist"].l.setList(self.list)
		
		self["actions"] = ActionMap(["DirectionActions", "OkCancelActions"],
		{
			"ok": self.ok,
			"cancel": self.exit,
		}, -1)

		self.onShown.append( self.selectActive )
		
	def selectActive(self):
		if (self.idxactive != -1):
			print "Move to %s\n" % str(self.idxactive)
			self["actionlist"].moveToIndex(self.idxactive)
			
	def okCB(self, result):
		self.close()

	def ok(self):
		if self.idxactive != self["actionlist"].getSelectedIndex():
			print "Selected : %s\n" % self["actionlist"].getCurrent()
			
			fp = file('/var/custombutton.dat', 'w')
			fp.write("%s" % self["actionlist"].getCurrent())
			fp.close()
			system("sync")
			
			self.session.openWithCallback(self.okCB, MessageBox, _("Your new Custom Buttom is %s.") % str(self["actionlist"].getCurrent()), type = MessageBox.TYPE_INFO)
		else:
			self.close()

	
	def exit(self):
		self.close()




def main(session, **kwargs):
	session.open(CustomButtonActionMenu)
	
def menu(menuid, **kwargs):
	if menuid == "miscellaneous":
		return [(_("Define Custom Button"), main, "custombuttondefine", None)]
	return []
	
def Plugins(**kwargs):
	return PluginDescriptor(name="Define Custom Button", description="define custom button", where = PluginDescriptor.WHERE_MENU, fnc=menu)
