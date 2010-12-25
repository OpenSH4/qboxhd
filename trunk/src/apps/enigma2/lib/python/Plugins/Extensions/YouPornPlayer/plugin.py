from qboxhd import QBOXHD
from Plugins.Plugin import PluginDescriptor
from Tools.BoundFunction import boundFunction

from YouPornList import YouPornListScreen
from YouPornSearchDialog import YouPornSearchDialog, SEARCH, STDFEEDS, CANCEL
from YouPornStdFeedSelection import YouPornStdFeedSelectionScreen
from YouPornInterface import porninterface, YouPornInterface
from SkinLoader import loadPluginSkin
from Screens.MessageBox import MessageBox

import gettext


def _(txt):
	t = gettext.dgettext("YouPorn", txt)
	if t == txt:
		print "[YOUPORN] fallback to default translation for", txt
		t = gettext.gettext(txt)
	return t


class YouPornManager():
	def __init__(self, session):
		self.session = session
		porninterface.open()


	def openSearchDialog(self):
		self.session.openWithCallback(self.searchDialogClosed, YouPornSearchDialog)


	def searchDialogClosed(self, what, searchContext = None):
		print "[YOUPORN] searchDialogClosed: ", what
		if what == SEARCH:
			dlg = self.session.openWithCallback(self.YouPornListScreenClosed, YouPornListScreen)
			dlg.searchPage(searchContext)
		elif what == CANCEL:
			porninterface.close()
		elif what == STDFEEDS:
			self.openStandardPages()


	def openStandardPages(self):
		self.session.openWithCallback(self.standardPageSelected, YouPornStdFeedSelectionScreen)


	def standardPageSelected(self, stdPageUrl):
		if stdPageUrl is not None:
			dlg = self.session.openWithCallback(self.YouPornListScreenClosed, YouPornListScreen)
			dlg.loadStandardPage(stdPageUrl)
		else:
			self.openSearchDialog()


	def backToSearchDialog(self, dummy = True):
		self.openSearchDialog()


	def YouPornListScreenClosed(self, proceed):
		if proceed:
			self.openSearchDialog()
		else:
			porninterface.close()


def main(session, **kwargs):
	try:
		youPornManager = YouPornManager(session)
	except Exception, e:
		session.open(MessageBox, _("Error contacting YouPorn:\n%s" % e), MessageBox.TYPE_ERROR)
	else:
		youPornManager.openSearchDialog()


def Plugins(**kwargs):
	loadPluginSkin(kwargs["path"])
	return PluginDescriptor(
		name="YouPorn Player",
		description=_("Search and play YouPorn movies"),
		where = [ PluginDescriptor.WHERE_EXTENSIONSMENU, PluginDescriptor.WHERE_PLUGINMENU ],
		icon = "plugin.png", fnc = boundFunction(main))
