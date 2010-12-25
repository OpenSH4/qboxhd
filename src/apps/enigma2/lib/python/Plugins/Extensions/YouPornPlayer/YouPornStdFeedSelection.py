from qboxhd import QBOXHD
from Screens.Screen import Screen
from Components.Sources.List import List
from Components.ActionMap import ActionMap
from Components.Sources.StaticText import StaticText

from . import _

class YouPornStdFeedSelectionScreen(Screen):
	
	def __init__(self, session):
		Screen.__init__(self, session)

		self["actions"] = ActionMap(["OkCancelActions"],
			{
				"ok"		:	self.ok,
				"cancel"	:	self.close
			})

		menu = [(_("Most Viewed"), "most_viewed")]
		menu.append((_("Top Rated"), "top_rated"))
		menu.append((_("Newest Video"), "browse"))

		self["menu"] = List(menu)


	def ok(self):
		Screen.close(self, self["menu"].getCurrent()[1])


	def close(self):
		Screen.close(self, None)
