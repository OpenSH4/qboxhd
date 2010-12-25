from qboxhd import QBOXHD

from Components.MenuList import MenuList
from Screens.Screen import Screen
from Components.ActionMap import ActionMap


class YouPornEntryContextMenuList(MenuList):
	def __init__(self):
		self.menuList = []
		MenuList.__init__(self, self.menuList)

		
	def appendEntry(self, entry):
		self.menuList.append(entry)


class YouPornEntryContextMenu(Screen):
	def __init__(self, session, menuList, title):
		Screen.__init__(self, session)
		self.tmpTitle = title

		self["actions"] = ActionMap(["OkCancelActions"],
		{
			"ok": self.okbuttonClick,
			"cancel": self.cancelClick
		})
		self["menu"] = menuList
		
		self.onFirstExecBegin.append(self.setTitleDelaied)


	def okbuttonClick(self):
		self.close(self["menu"].getCurrent()[1])
		

	def cancelClick(self):
		self.close(None)


	def setTitleDelaied(self):
		self.setTitle(self.tmpTitle)
