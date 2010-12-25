from qboxhd import QBOXHD
from Screens.Screen import Screen

from Components.config import config
from Components.config import Config
from Components.config import ConfigSelection
from Components.config import ConfigText
from Components.config import getConfigListEntry
from Components.ConfigList import ConfigListScreen
from Components.ActionMap import ActionMap
from Components.Button import Button
from Screens.ParentalControlSetup import ProtectedScreen
from Screens.MessageBox import MessageBox

from . import _

searchContext = Config()
searchContext.searchTerm = ConfigText("", False)
searchContext.sortOrder = ConfigSelection(
				[
				 ("relevance", _("Relevance")),
				 ("rating", _("Rating")),
				 ("views", _("Views")),
				 ("duration", _("Duration")),
				 ("time", _("Date")),
				], "relevance")
				
searchContext.type = ConfigSelection(
				[
				 ("straight", _("Straight")),
				 ("gay", _("Gay")),
				 ("cocks", _("Cocks")),
				], "straight")

SEARCH		= 1
STDFEEDS 	= 2
CANCEL		= 5


class YouPornSearchDialog(Screen, ProtectedScreen, ConfigListScreen):
	def __init__(self, session):
		Screen.__init__(self, session)
		ProtectedScreen.__init__(self)

		self.session = session

		self.propagateUpDownNormally = True
		
		self["actions"] = ActionMap(["YouPornSearchDialogActions"],
		{
			"standard"	:	self.keyStdFeeds,
			"search"	:	self.keySearch,

			"cancel"	:	self.keyCancel,
			"left"		:	self.keyLeft,
			"right"		:	self.keyRight,
			"up"		:	self.keyUp,
			"down"		:	self.keyDown,
		}, -2)

		self["key_red"] = Button(_("Std.Feeds"))
		self["key_green"] = Button(_("Search"))

		searchContextEntries = []
		searchContextEntries.append(getConfigListEntry(_("Search Term(s)"), searchContext.searchTerm))
		searchContextEntries.append(getConfigListEntry(_("Type"), searchContext.type))
		searchContextEntries.append(getConfigListEntry(_("Sort Order"), searchContext.sortOrder))

		ConfigListScreen.__init__(self, searchContextEntries, session)
		

	def isProtected(self):
		return config.ParentalControl.setuppinactive.value and config.ParentalControl.configured.value
	
	def pinEntered(self, result):
		if result is None:
			self.close(CANCEL)
		elif not result:
			self.session.openWithCallback(self.closeWrongPIN, MessageBox, _("The pin code you entered is wrong."), MessageBox.TYPE_ERROR)	


	def closeWrongPIN(self, *ret):
		self.close(CANCEL)

	def keyOK(self):
		ConfigListScreen.keyOK(self)


	def keyUp(self):
		if self.propagateUpDownNormally:
			self["config"].instance.moveSelection(self["config"].instance.moveUp)
		else:
			self["config"].getCurrent()[1].suggestionListUp()
			self["config"].invalidateCurrent()


	def keyDown(self):
		if self.propagateUpDownNormally:
			self["config"].instance.moveSelection(self["config"].instance.moveDown)
		else:
			self["config"].getCurrent()[1].suggestionListDown()
			self["config"].invalidateCurrent()


	def keyRight(self):
		if self.propagateUpDownNormally:
			ConfigListScreen.keyRight(self)
		else:
			self["config"].getCurrent()[1].suggestionListPageDown()
			self["config"].invalidateCurrent()


	def keyLeft(self):
		if self.propagateUpDownNormally:
			ConfigListScreen.keyLeft(self)
		else:
			self["config"].getCurrent()[1].suggestionListPageUp()
			self["config"].invalidateCurrent()


	def keyCancel(self):
		if self.propagateUpDownNormally:
			self.close(CANCEL)
		else:
			self.propagateUpDownNormally = True
			self["config"].getCurrent()[1].cancelSuggestionList()
			self["config"].invalidateCurrent()


	def keySearch(self):
		if searchContext.searchTerm.value != "":
			self.close(SEARCH, searchContext)


	def keyStdFeeds(self):
		self.close(STDFEEDS)
