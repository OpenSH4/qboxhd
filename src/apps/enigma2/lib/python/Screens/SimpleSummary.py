from qboxhd import QBOXHD
from Screens.Screen import Screen
#from Components.Sources.Source import ObsoleteSource

class SimpleSummary(Screen):
	if QBOXHD:
		skin = """
		<screen name="SimpleSummary" position="0,0" size="320,240">
			<widget name="Title" position="6,0" size="300,240" font="Regular;38" halign="center" valign="center" />
		</screen>"""
	else:
		skin = """
		<screen position="0,0" size="132,64">
			<widget source="global.CurrentTime" render="Label" position="56,46" size="82,18" font="Regular;16">
				<convert type="ClockToText">WithSeconds</convert>
			</widget>
			<widget name="Title" position="6,4" size="120,42" font="Regular;18" />
		</screen>"""
	def __init__(self, session, parent):
		from Components.Label import Label
		Screen.__init__(self, session, parent = parent)

		names = parent.skinName
		if not isinstance(names, list):
		  names = [names]

		self.skinName = [ x + "_summary" for x in names ]
		self.skinName.append("SimpleSummary")

		# if parent has a "skin_summary" defined, use that as default
		self.skin = parent.__dict__.get("skin_summary", self.skin)

		self["Title"] = Label(parent.title or "")

	def setTitle(self, title):
		self["Title"].setText(title)
