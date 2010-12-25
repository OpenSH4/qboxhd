from qboxhd import QBOXHD
from YouPornInterface import porninterface

from Components.ActionMap import ActionMap
from Components.MenuList import MenuList
from Components.Label import Label
from Components.ScrollLabel import ScrollLabel
from Components.Pixmap import Pixmap
from Components.ProgressBar import ProgressBar

from Components.MultiContent import MultiContentEntryText, MultiContentEntryPixmapAlphaTest
from Tools.Directories import resolveFilename, SCOPE_PLUGINS, SCOPE_SKIN_IMAGE
from enigma import eListboxPythonMultiContent, gFont, RT_HALIGN_LEFT, RT_VALIGN_TOP, RT_WRAP
from enigma import eTimer

from Tools.NumericalTextInput import NumericalTextInput

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox

from Components.config import config

from Plugins.Extensions.VlcPlayer.VlcServerConfig import vlcServerConfig
from Plugins.Extensions.VlcPlayer.VlcServer import VlcServer
from Plugins.Extensions.VlcPlayer.VlcServerList import VlcServerListScreen

from YouPornContextMenu import YouPornEntryContextMenu, YouPornEntryContextMenuList

from Tools.BoundFunction import boundFunction

from YouPornPlayer import YouPornPlayer

from Tools.LoadPixmap import LoadPixmap

from . import _

def YouPornEntryComponent(entry):
	res = [ entry ]
# 385
	res.append(MultiContentEntryText(pos = (150, 5), size = (370, 42), font = 0, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = entry.getTitle()))
	res.append(MultiContentEntryText(pos = (150, 26), size = (370, 56), font = 1, color = 0xFFA323, color_sel = 0xFFA323, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = entry.getVideoUrl()))
	res.append(MultiContentEntryText(pos = (150, 55), size = (100, 56), font = 1, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = _("Duration: ")+ str(entry.getDuration())))
	res.append(MultiContentEntryText(pos = (300, 55), size = (100, 56), font = 1, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = _("Views: ")+ str(entry.getViewCount())))
	
	res.append(MultiContentEntryText(pos = (150, 80), size = (300, 56), font = 1, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = _("Rating: ")+ str(entry.getNumRaters()) + "/" + str(entry.getTopRaters())))
	
	pngstar = LoadPixmap("/usr/lib/enigma2/python/Plugins/Extensions/YouPornPlayer/starsbar_empty.png")
	res.append(MultiContentEntryPixmapAlphaTest(pos = (300, 75), size = (130, 97), png = pngstar))
	
	RatingScheme = int(round(20 * float(entry.getNumRaters())))
	
	pngstarbar = LoadPixmap("/usr/lib/enigma2/python/Plugins/Extensions/YouPornPlayer/starsbar_filled.png")
	res.append(MultiContentEntryPixmapAlphaTest(pos = (300, 75), size = (RatingScheme, 97), png = pngstarbar))

	if entry.thumbnail["0"] is None:
		png = LoadPixmap("/usr/lib/enigma2/python/Plugins/Extensions/YouPornPlayer/nothumb-big.png")
	else:
		png = entry.thumbnail["0"]
	res.append(MultiContentEntryPixmapAlphaTest(pos = (10, 5), size = (130, 97), png = png))

	return res


class YouPornVideoDetailsScreen(Screen):
	def __init__(self, session, entry):
		Screen.__init__(self, session)
		self.entry = entry
		self["video_description"] = ScrollLabel(entry.getDescription())
		duration = entry.getDuration()
		
		self["label_video_duration"] = Label(_("Duration") + ":")
		self["video_duration"] = Label(duration)
		
		self["label_video_rating_average"] = Label(_("Rate") + ":")
		self["starsbg"] = Pixmap()
		self["stars"] = ProgressBar()

		self["label_video_numraters"] = Label(_("Ratings") + ":")
		self["video_numraters"] = Label(str(entry.getNumRaters()) + "/" + str(entry.getTopRaters()))
		
		self["label_video_statistics_view_count"] = Label(_("Views") + ":")
		self["video_statistics_view_count"] = Label(str(entry.getViewCount()))

		self["video_thumbnail_1"] = Pixmap()

		self["actions"] = ActionMap(["YouPornVideoDetailsScreenActions"],
		{
			"ok"		:	self.close,
			"cancel"	:	self.close,
			"up"		:	self.pageUp,
			"down"		:	self.pageDown,
			"left"		:	self.pageUp,
			"right"		:	self.pageDown
		})

		self.onFirstExecBegin.append(self.setPixmap)
		self.onFirstExecBegin.append(self.setInitialize)


	def pageUp(self):
		self["video_description"].pageUp()


	def pageDown(self):
		self["video_description"].pageDown()


	def setInitialize(self):
		Screen.setTitle(self, self.entry.getTitle())
		ratingStars = int(round(20 * float(self.entry.getNumRaters()), 0))
		self["stars"].setValue(ratingStars)


	def setPixmap(self):
		self["video_thumbnail_1"].instance.setPixmap(self.entry.thumbnail["0"].__deref__())



class PatientMessageBox(MessageBox):
	def __init__(self, session, text, type = 1, timeout = -1, close_on_any_key = False, default = True):
		MessageBox.__init__(self, session, text, type, timeout, close_on_any_key, default)
		self.skinName = "MessageBox"


	def processDelayed(self, function):
		self.delay_timer = eTimer()
		self.delay_timer.callback.append(self.processDelay)
		self.delay_timer.start(0, 1)
		self.function = function


	def processDelay(self):
		self.function()


	def cancel(self):
		pass


	def ok(self):
		pass


	def alwaysOK(self):
		pass


class YouPornList(MenuList):
	def __init__(self, list, enableWrapAround = False):
		MenuList.__init__(self, list, enableWrapAround, eListboxPythonMultiContent)
		self.l.setFont(0, gFont("Regular", 18))
		self.l.setFont(1, gFont("Regular", 14))
		self.l.setItemHeight(105)


class YouPornListScreen(Screen, NumericalTextInput):
	def __init__(self, session):
		Screen.__init__(self, session)
		NumericalTextInput.__init__(self)

		self.session = session
		self.serverName = config.plugins.youpornplayer.serverprofile.value
		self.currentServer = vlcServerConfig.getServerByName(self.serverName)

		self["red"] = Label(_("Select a VLC-Server"))
		self["green"] = Label(_("New YouPorn search"))
		
		self.list = []
		self["list"] = YouPornList(self.list)

		self["label_total_results"] = Label(_("Total pages") + ":")
		self["total_results"] = Label("")

		self["label_currently_shown"] = Label(_("Shown page") + ":")
		self["currently_shown"] = Label("")

		self.patientDialog = None
		
		self["actions"] = ActionMap(["YouPornVideoListActions"],
		{
			"play"			:	self.tryToPlay,
			"select"		:	self.justSelectServer,
			"search"		:	self.searchAgain,
			"menu"			:	self.openContextMenu,
			"left"			:	self.keyLeft,
			"right"			:	self.keyRight,
			"up"			:	self.keyUp,
			"down"			:	self.keyDown,
			"info"			:	self.showVideoInfo,
			"cancel"		:	self.close
		}, -1)


	def keyLeft(self):
		self["list"].pageUp()


	def keyRight(self):
		if self["list"].getSelectionIndex() == len(self.list) - 1 and self.page.getNextPage() is not None:
			dlg = self.session.openWithCallback(self.loadNextPage, MessageBox, _("Load another Page of current category?"))
		else:
			self["list"].pageDown()


	def keyUp(self):
		self["list"].up()


	def keyDown(self):
		if self["list"].getSelectionIndex() == len(self.list) - 1 and self.page.getNextPage() is not None:
			dlg = self.session.openWithCallback(self.loadNextPage, MessageBox, _("Load another Page of current category?"))
		else:
			self["list"].down()


	def insertEntry(self, entry):
		print "[YOUPORN] YouPornTest::updateFinished()"
		self.list.append(YouPornEntryComponent(entry))
		self["list"].setList(self.list)


	def closePatientDialogDelayed(self):
		if self.patientDialog:
			self.patientDialog.close()
			self.patientDialog = None
		self["list"].setList(self.list)


	def showPage(self, page, append):
		if page is not None:
			self.page = page
			self.setTitle(page.getTitle())
			self["total_results"].setText( str(page.getTotalPages()) )
			self["currently_shown"].setText( str(page.getCurrentPage()) )
			if not append:
				self.list = []
				self["list"].setList(self.list)
			self.page.loadThumbnails(self.insertEntry)
		self.delay_timer = eTimer()
		self.delay_timer.callback.append(self.closePatientDialogDelayed)
		self.delay_timer.start(100, 1)


	def searchPageReal(self, searchContext, pageidx):
		print "[YOUPORN] searchPageReal"
		try:
			page = porninterface.search(pageidx = pageidx,
					query = str(searchContext.searchTerm.value),
					type = str(searchContext.type.value),
					sortby = str(searchContext.sortOrder.value))
		except Exception, e:
			page = None
			self.session.open(MessageBox, _("Error querying page for search term %s:\n%s" %
					(searchContext.searchTerm.value, e)), MessageBox.TYPE_ERROR)
					
		self.showPage(page, False)


	def searchPage(self, searchContext):
		self.page = None
		self.patientDialog = self.session.open(PatientMessageBox, _("Searching, be patient ..."))
		self.patientDialog.processDelayed(boundFunction(self.searchPageReal, searchContext = searchContext, pageidx=1))


	def loadStandardPage(self, url):
		self.page = None
		self.loadPage(_("Loading standard page, be patient ..."), url, 1)


	def loadPageReal(self, PageUrl, pageidx, append = False):
		try:
			page = porninterface.getPage(PageUrl, self.page, pageidx)
		except Exception, e:
			page = None
			self.session.open(MessageBox, _("Error querying page %s:\n%s" %
					(PageUrl, e)), MessageBox.TYPE_ERROR)
					
		self.showPage(page, append)


	def loadPage(self, text, PageUrl, pageidx, append = False):
		self.patientDialog = self.session.open(PatientMessageBox, text)
		self.patientDialog.processDelayed(boundFunction(self.loadPageReal, PageUrl, pageidx, append = append))
		

	def loadNextPage(self, result):
		if not result:
			return
		nextpage = self.page.getNextPage()
		if nextpage is not None:
			self.loadPage(_("Loading additional videos, be patient ..."), self.page.getPageUrl(), nextpage, True)

	
	def showVideoInfo(self):
		self.session.open(YouPornVideoDetailsScreen, self["list"].getCurrent()[0])


	def justSelectServer(self):
		defaultServer = vlcServerConfig.getServerByName(config.plugins.youpornplayer.serverprofile.value)
		self.selectServer(self.serverSelectedCB, defaultServer)


	def selectServer(self, callback, currentServer):
		self.session.openWithCallback(callback, VlcServerListScreen, currentServer)


	def serverSelectedCB(self, selectedServer, defaultServer):
		if selectedServer is not None:
			self.currentServer = selectedServer
		elif defaultServer is not None:
				self.currentServer = defaultServer
		if defaultServer is not None:
			config.plugins.youpornplayer.serverprofile.value = defaultServer.getName()
			config.plugins.youpornplayer.serverprofile.save()


	def selectAndPlayCB(self, selectedServer, defaultServer):
		self.serverSelectedCB(selectedServer, defaultServer)
		self.tryToPlay()


	def tryToPlay(self):
		if self.currentServer is not None:
			self.play()
		else:
			self.selectServer(self.selectAndPlayCB, None)


	def getVideoUrl(self, YouPornEntry):
		mrl = YouPornEntry.getVideoUrl()
		if mrl is None:
			self.session.open(MessageBox, _("Could not retrive video url."), MessageBox.TYPE_ERROR)
		return mrl


	def getRealVideoMrl(self, YouPornEntry):
		mrl = porninterface.getRealVideoMrl(YouPornEntry.getVideoUrl())
		
		print "[YOUPORN] MRL: ", str(mrl)
		if mrl is None:
			self.session.open(MessageBox, _("Could not retrive video url."), MessageBox.TYPE_ERROR)
		return mrl
		
		
		
	def playCallback(self):
		YouPornEntry = self["list"].getCurrent()[0]
		mrl = self.getRealVideoMrl(YouPornEntry)
		
		self.delay_timer = eTimer()
		self.delay_timer.callback.append(self.closePatientDialogDelayed)
		self.delay_timer.start(100, 1)
		
		#mrl = "http://download.youporn.com/download/338957/flv/336725_Kitten_and_Molly_Cavalli_getting_down.flv?download=1&ll=1&t=dd"
		if mrl is not None:
			entries = []
			entries.append((_("Show video detail info"), [self.showVideoInfo, False]))
			
			self.currentServer.play(self.session, mrl, YouPornEntry.getTitle(), self,
								player = boundFunction(YouPornPlayer, contextMenuEntries = entries, infoCallback = self.showVideoInfo, name = self["list"].getCurrent()[0].getTitle()))
		else:
			print "[YOUPORN] No valid flv-mrl found"
		
		

	def play(self):
		print "[YOUPORN] Play()"
		self.patientDialog = self.session.open(PatientMessageBox, _("Playing video, be patient ..."))
		self.patientDialog.processDelayed(boundFunction(self.playCallback))
		
		
		



	def getNextFile(self):
		i = self["list"].getSelectedIndex() + 1
		if i < len(self.list):
			self["list"].moveToIndex(i)
			YouPornEntry = self["list"].getCurrent()[0]
			return self.getVideoUrl(YouPornEntry), YouPornEntry.getTitle()
		return None, None


	def getPrevFile(self):
		i = self["list"].getSelectedIndex() - 1
		if i >= 0:
			self["list"].moveToIndex(i)
			YouPornEntry = self["list"].getCurrent()[0]
			return self.getVideoUrl(YouPornEntry), YouPornEntry.getTitle()
		return None, None


	def openContextMenu(self):
		contextMenuList = YouPornEntryContextMenuList()
		contextMenuList.appendEntry((_("Show video detail info"), self.showVideoInfo))
		self.session.openWithCallback(self.menuActionCoosen, YouPornEntryContextMenu, contextMenuList, self["list"].getCurrent()[0].getTitle())


	def menuActionCoosen(self, function):
		if function is not None:
			function()


	def searchAgain(self):
		Screen.close(self, True)


	def close(self):
		Screen.close(self, False)
