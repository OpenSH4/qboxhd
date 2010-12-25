from qboxhd import QBOXHD
from Plugins.Extensions.VlcPlayer.VlcPlayer import VlcPlayer
from Components.ActionMap import ActionMap

from YouPornContextMenu import YouPornEntryContextMenu, YouPornEntryContextMenuList


class YouPornPlayer(VlcPlayer):
	def __init__(self, session, server, currentList, contextMenuEntries, infoCallback, name):
		VlcPlayer.__init__(self, session, server, currentList)
		self.contextMenuEntries = contextMenuEntries
		self.infoCallback = infoCallback
		self.name = name

		self["menuactions"] = ActionMap(["YouPornPlayerScreenActions"],
		{
			"menu"	:	self.openContextMenu,
			"info"	:	self.showVideoInfo,
		}, -1)


	def showVideoInfo(self):
		if self.shown:
			self.hideInfobar()
		self.infoCallback()


	def openContextMenu(self):
		if self.shown:
			self.hideInfobar()
		contextMenuList = YouPornEntryContextMenuList()
		for entry in self.contextMenuEntries:
			contextMenuList.appendEntry(entry)
		self.session.openWithCallback(self.menuActionCoosen, YouPornEntryContextMenu, contextMenuList, self.name)


	def menuActionCoosen(self, cookie):
		if cookie is not None:
			if cookie[1]:
				self.stop()
			cookie[0]()
