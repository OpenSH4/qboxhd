from qboxhd import QBOXHD
from __init__ import _
from Plugins.Plugin import PluginDescriptor
from MyTubeService import GoogleSuggestions
from MyTubeSearch import ConfigTextWithGoogleSuggestions
from Tools.BoundFunction import boundFunction
from Screens.MessageBox import MessageBox
from Screens.Screen import Screen
from Screens.ChoiceBox import ChoiceBox
from Screens.InfoBar import MoviePlayer
from Screens.VirtualKeyBoard import VirtualKeyBoard
from Components.ActionMap import ActionMap, NumberActionMap
from Components.Label import Label
from Components.ScrollLabel import ScrollLabel
from Components.ProgressBar import ProgressBar
from Components.Pixmap import Pixmap
from Components.Button import Button
from Components.Sources.List import List
from Components.MultiContent import MultiContentEntryText, MultiContentEntryPixmapAlphaTest
from Components.AVSwitch import AVSwitch
from Components.ActionMap import HelpableActionMap
from Components.config import config, Config, ConfigSelection, ConfigSubsection, ConfigText, getConfigListEntry, ConfigYesNo, ConfigIP, ConfigNumber,ConfigLocations
from Components.config import KEY_DELETE, KEY_BACKSPACE, KEY_LEFT, KEY_RIGHT, KEY_HOME, KEY_END, KEY_TOGGLEOW, KEY_ASCII, KEY_TIMEOUT
from Components.ConfigList import ConfigListScreen
from Components.ServiceEventTracker import ServiceEventTracker, InfoBarBase
from Components.Console import Console
from Components.Sources.Source import Source
from Components.Task import Task, Job, job_manager

from threading import Thread
from threading import Condition

from Tools.Directories import pathExists, fileExists, resolveFilename, SCOPE_PLUGINS, SCOPE_SKIN_IMAGE, SCOPE_HDD
from Tools.LoadPixmap import LoadPixmap
from Tools.Downloader import HTTPProgressDownloader, downloadWithProgress
from enigma import eTimer, quitMainloop,eListbox,ePoint, RT_HALIGN_LEFT, RT_HALIGN_RIGHT, RT_VALIGN_CENTER, eListboxPythonMultiContent, eListbox, gFont, getDesktop, ePicLoad, eServiceCenter, iServiceInformation, eServiceReference,iSeekableService,iServiceInformation, iPlayableService, iPlayableServicePtr
from os import path as os_path, system as os_system, unlink, stat, mkdir, popen, makedirs, listdir, access, rename, remove, W_OK, R_OK, F_OK
from twisted.web import client
from twisted.internet import reactor
from time import time

from Screens.InfoBarGenerics import InfoBarShowHide, InfoBarSeek, InfoBarNotifications, InfoBarServiceNotifications

config.plugins.mytube = ConfigSubsection()
config.plugins.mytube.search = ConfigSubsection()

config.plugins.mytube.search.searchTerm = ConfigTextWithGoogleSuggestions("", False, threaded = True)
config.plugins.mytube.search.orderBy = ConfigSelection(
				[
				 ("relevance", _("Relevance")),
				 ("viewCount", _("View Count")),
				 ("published", _("Published")),
				 ("rating", _("Rating"))
				], "relevance")
config.plugins.mytube.search.time = ConfigSelection(
				[
				 ("all_time", _("All Time")),
				 ("this_month", _("This Month")),
				 ("this_week", _("This Week")),
				 ("today", _("Today"))
				], "all_time")
config.plugins.mytube.search.racy = ConfigSelection(
				[
				 ("include", _("Yes")),
				 ("exclude", _("No"))
				], "include")
config.plugins.mytube.search.categories = ConfigSelection(
				[
				 (None, _("All")),
				 ("Film", _("Film & Animation")),
				 ("Autos", _("Autos & Vehicles")),
				 ("Music", _("Music")),
				 ("Animals", _("Pets & Animals")),
				 ("Sports", _("Sports")),
				 ("Travel", _("Travel & Events")),
				 ("Shortmov", _("Short Movies")),
				 ("Games", _("Gaming")),
				 ("Comedy", _("Comedy")),
				 ("People", _("People & Blogs")),
				 ("News", _("News & Politics")),
				 ("Entertainment", _("Entertainment")),
				 ("Education", _("Education")),
				 ("Howto", _("Howto & Style")),
				 ("Nonprofit", _("Nonprofits & Activism")),
				 ("Tech", _("Science & Technology"))
				], None)
config.plugins.mytube.search.lr = ConfigSelection(
				[
				 (None, _("All")),
				 ("au", _("Australia")),
				 ("br", _("Brazil")),
				 ("ca", _("Canada")),
				 ("cz", _("Czech Republic")),
				 ("fr", _("France")),
				 ("de", _("Germany")),
				 ("gb", _("Great Britain")),
				 ("au", _("Australia")),
				 ("nl", _("Holland")),
				 ("hk", _("Hong Kong")),
				 ("in", _("India")),
				 ("ie", _("Ireland")),
				 ("il", _("Israel")),
				 ("it", _("Italy")),
				 ("jp", _("Japan")),
				 ("mx", _("Mexico")),
				 ("nz", _("New Zealand")),
				 ("pl", _("Poland")),
				 ("ru", _("Russia")),
				 ("kr", _("South Korea")),
				 ("es", _("Spain")),
				 ("se", _("Sweden")),
				 ("tw", _("Taiwan")),
				 ("us", _("United States")) 
				], None)
config.plugins.mytube.search.sortOrder = ConfigSelection(
				[
				 ("ascending", _("Ascending")),
				 ("descending", _("Descending"))
				], "ascending")

config.plugins.mytube.general = ConfigSubsection()
config.plugins.mytube.general.showHelpOnOpen = ConfigYesNo(default = True)
config.plugins.mytube.general.loadFeedOnOpen = ConfigYesNo(default = True)
if QBOXHD:
	config.plugins.mytube.general.startFeed = ConfigSelection(
					[
					("hd", _("HD videos")),
					("most_viewed", _("Most viewed")),
					("top_rated", _("Top rated")),
					("recently_featured", _("Recently featured")),
					("most_discussed", _("Most discussed")),
					("top_favorites", _("Top favorites")),
					("most_linked", _("Most linked")),
					("most_responded", _("Most responded")),
					("most_recent", _("Most recent")),
					("stdfeed", _("Standard Feed")),
					], "stdfeed")
else:
	config.plugins.mytube.general.startFeed = ConfigSelection(
					[
					("hd", _("HD videos")),
					("most_viewed", _("Most viewed")),
					("top_rated", _("Top rated")),
					("recently_featured", _("Recently featured")),
					("most_discussed", _("Most discussed")),
					("top_favorites", _("Top favorites")),
					("most_linked", _("Most linked")),
					("most_responded", _("Most responded")),
					("most_recent", _("Most recent"))
					], "most_viewed")
						
config.plugins.mytube.general.on_movie_stop = ConfigSelection(default = "ask", choices = [
	("ask", _("Ask user")), ("quit", _("Return to movie list")), ("playnext", _("Play next video")), ("playagain", _("Play video again")) ])

config.plugins.mytube.general.on_exit = ConfigSelection(default = "ask", choices = [
	("ask", _("Ask user")), ("quit", _("Return to movie list"))])

default = resolveFilename(SCOPE_HDD)
tmp = config.movielist.videodirs.value
if default not in tmp:
	tmp.append(default)
config.plugins.mytube.general.videodir = ConfigSelection(default = default, choices = tmp)
config.plugins.mytube.general.history = ConfigText(default="")
config.plugins.mytube.general.clearHistoryOnClose = ConfigYesNo(default = False)
#config.plugins.mytube.general.useHTTPProxy = ConfigYesNo(default = False)
#config.plugins.mytube.general.ProxyIP = ConfigIP(default=[0,0,0,0])
#config.plugins.mytube.general.ProxyPort = ConfigNumber(default=8080)

class downloadJob(Job):
	def __init__(self, url, file, title):
		Job.__init__(self, title)
		downloadTask(self, url, file)

class downloadTask(Task):
	def __init__(self, job, url, file):
		Task.__init__(self, job, ("download task"))
		self.end = 100
		self.url = url
		self.local = file

	def prepare(self):
		self.error = None

	def run(self, callback):
		self.callback = callback
		self.download = downloadWithProgress(self.url,self.local)
		self.download.addProgress(self.http_progress)
		self.download.start().addCallback(self.http_finished).addErrback(self.http_failed)
	
	def http_progress(self, recvbytes, totalbytes):
		#print "[http_progress] recvbytes=%d, totalbytes=%d" % (recvbytes, totalbytes)
		self.progress = int(self.end*recvbytes/float(totalbytes))
	
	def http_finished(self, string=""):
		print "[http_finished]" + str(string)
		Task.processFinished(self, 0)
	
	def http_failed(self, failure_instance=None, error_message=""):
		if error_message == "" and failure_instance is not None:
			error_message = failure_instance.getErrorMessage()
			print "[http_failed] " + error_message
			Task.processFinished(self, 1)



from MyTubeService import myTubeService
from MyTubeSearch import MyTubeSettingsScreen,MyTubeTasksScreen,MyTubeHistoryScreen


class MyTubePlayerMainScreen(Screen, ConfigListScreen):
	BASE_STD_FEEDURL = "http://gdata.youtube.com/feeds/api/standardfeeds/"
	#(entry, Title, Description, TubeID, thumbnail, PublishedDate,Views,duration,ratings )	
	skin = """
		<screen name="MyTubePlayerMainScreen" flags="wfNoBorder" position="0,0" size="720,576" title="MyTubePlayerMainScreen..." >
			<ePixmap position="0,0" zPosition="-1" size="720,576" pixmap="~/mytubemain_bg.png" alphatest="on" transparent="1" backgroundColor="transparent"/>
			<widget name="config" zPosition="2" position="60,60" size="600,50" scrollbarMode="showNever" transparent="1" />
			<widget source="feedlist" render="Listbox" position="49,110" size="628,385" zPosition="1" scrollbarMode="showOnDemand" transparent="1" backgroundPixmap="~/list_bg.png" selectionPixmap="~/list_sel.png" >
				<convert type="TemplatedMultiContent">
				{"templates":
					{"default": (77,[
							MultiContentEntryPixmapAlphaTest(pos = (0, 0), size = (100, 75), png = 4), # index 4 is the thumbnail
							MultiContentEntryText(pos = (100, 1), size = (500, 22), font=0, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = 1), # index 1 is the Title
							MultiContentEntryText(pos = (100, 24), size = (300, 18), font=1, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = 5), # index 5 is the Published Date
							MultiContentEntryText(pos = (100, 43), size = (300, 18), font=1, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = 6), # index 6 is the Views Count
							MultiContentEntryText(pos = (400, 24), size = (200, 18), font=1, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = 7), # index 7 is the duration
							MultiContentEntryText(pos = (400, 43), size = (200, 18), font=1, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = 8), # index 8 is the ratingcount
						]),
					"state": (77,[
							MultiContentEntryText(pos = (10, 1), size = (560, 28), font=2, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = 0), # index 0 is the name
							MultiContentEntryText(pos = (10, 22), size = (560, 46), font=3, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = 1), # index 2 is the description
						])
					},
					"fonts": [gFont("Regular", 22),gFont("Regular", 18),gFont("Regular", 26),gFont("Regular", 20)],
					"itemHeight": 77
				}
				</convert>
			</widget>

			<ePixmap pixmap="skin_default/buttons/key_info.png" position="50,500" zPosition="4" size="35,25" alphatest="on" transparent="1" />
			<ePixmap pixmap="skin_default/buttons/key_menu.png" position="50,520" zPosition="4" size="35,25" alphatest="on" transparent="1" />
			<ePixmap position="90,500" size="100,40" zPosition="4" pixmap="~/plugin.png" alphatest="on" transparent="1" />
			<ePixmap position="190,500" zPosition="4" size="140,40" pixmap="skin_default/buttons/red.png" transparent="1" alphatest="on" />
			<ePixmap position="330,500" zPosition="4" size="140,40" pixmap="skin_default/buttons/green.png" transparent="1" alphatest="on" />
			<ePixmap position="470,500" zPosition="4" size="140,40" pixmap="skin_default/buttons/yellow.png" transparent="1" alphatest="on" />
			<widget name="key_red" position="190,500" zPosition="5" size="140,40" valign="center" halign="center" font="Regular;21" transparent="1" foregroundColor="white" shadowColor="black" shadowOffset="-1,-1" />
			<widget name="key_green" position="330,500" zPosition="5" size="140,40" valign="center" halign="center" font="Regular;21" transparent="1" foregroundColor="white" shadowColor="black" shadowOffset="-1,-1" />
			<widget name="key_yellow" position="470,500" zPosition="5" size="140,40" valign="center" halign="center" font="Regular;21" transparent="1" foregroundColor="white" shadowColor="black" shadowOffset="-1,-1" />
			<widget name="ButtonBlue" pixmap="skin_default/buttons/button_blue.png" position="610,510" zPosition="10" size="15,16" transparent="1" alphatest="on" />
			<widget name="VKeyIcon" pixmap="skin_default/vkey_icon.png" position="620,495" zPosition="10" size="60,48" transparent="1" alphatest="on" />
			<widget name="thumbnail" position="0,0" size="100,75" alphatest="on"/> # fake entry for dynamic thumbnail resizing, currently there is no other way doing this.
			<widget name="HelpWindow" position="160,255" zPosition="1" size="1,1" transparent="1" alphatest="on" />
		</screen>"""
		
	def __init__(self, session):
		Screen.__init__(self, session)
		self.session = session
		self.skin_path = plugin_path
		self.FeedURL = None
		self.ytfeed = None
		self.currentFeedName = None
		self.videolist = []
		self.thumbnails = []
		self.video_playlist = []
		self.statuslist = []
		self.mytubeentries = None
		self.index = 0
		self.maxentries = 0
		self.picloads = {}
		self.oldfeedentrycount = 0
		self.appendEntries = False
		self.lastservice = session.nav.getCurrentlyPlayingServiceReference()
		self.propagateUpDownNormally = True
		self.FirstRun = True
		self.HistoryWindow = None
		self.History = None
		self.searchtext = _("Welcome to the MyTube Youtube Player.\n\nWhile entering your search term(s) you will get suggestions displayed matching your search term.\n\nTo select a suggestion press DOWN on your remote, select the desired result and press OK on your remote to start the search.\n\nPress exit to get back to the input field.")
		self.feedtext = _("Welcome to the MyTube Youtube Player.\n\nUse the Bouquet+ button to navigate to the search field and the Bouquet- to navigate to the video entries.\n\nTo play a movie just press OK on your remote control.\n\nPress info to see the movie description.\n\nPress the Menu button for additional options.\n\nThe Help button shows this help again.")
		self.currList = "configlist"
		self.oldlist = None
		self.CleanupConsole = None
		self["feedlist"] = List(self.videolist)
		self["thumbnail"] = Pixmap()
		self["thumbnail"].hide()
		self["HelpWindow"] = Pixmap()
		self["HelpWindow"].hide()
		
		if QBOXHD:
			self["key_red"] = Button(_("Search"))
			self.feedName = ""
			self.TmrgetFeed = eTimer()
			self.TmrgetFeed.callback.append(self.TimergetFeed)
			
			self.TmrSearchFeed = eTimer()
			self.TmrSearchFeed.callback.append(self.TimerSearchFeed)
		else:
			self["key_red"] = Button(_("Close"))
			
		self["key_green"] = Button(_("Std. Feeds"))
		self["key_yellow"] = Button(_("History"))
		self["ButtonBlue"] = Pixmap()
		self["VKeyIcon"] = Pixmap()
		self["ButtonBlue"].hide()
		self["VKeyIcon"].hide()		

		if QBOXHD:
			self["searchactions"] = ActionMap(["ShortcutActions", "WizardActions", "HelpActions", "MediaPlayerActions"],
			{
				"ok": self.keyOK,
				"back": self.leavePlayer,
				"red": self.switchToConfigList,
				"green": self.keyStdFeed,
				"blue": self.openKeyboard,
				"yellow": self.handleHistory,
				"up": self.keyUp,
				"down": self.handleSuggestions,
				"left": self.keyLeft,
				"right": self.keyRight,
				"displayHelp": self.handleHelpWindow,
				"menu" : self.handleMenu,
			}, -2)
			
			self["suggestionactions"] = ActionMap(["ShortcutActions", "WizardActions", "MediaPlayerActions", "HelpActions"],
			{
				"ok": self.keyOK,
				"back": self.switchToConfigList,
				"red": self.switchToConfigList,
				"green": self.switchToFeedList,
				"up": self.keyUp,
				"down": self.keyDown,
				"left": self.keyLeft,
				"right": self.keyRight,
			}, -2)
	
			self["videoactions"] = ActionMap(["ShortcutActions", "WizardActions", "MediaPlayerActions", "MovieSelectionActions", "HelpActions"],
			{
				"ok": self.keyOK,
				"back": self.leavePlayer,
				"red": self.switchToConfigList,
				"green": self.keyStdFeed,
				"yellow": self.handleHistory,
				"up": self.keyUp,
				"down": self.keyDown,	
				"showEventInfo": self.showVideoInfo,
				"displayHelp": self.handleHelpWindow,
				"menu" : self.handleMenu,
			}, -2)			
			
			self["statusactions"] = ActionMap(["ShortcutActions", "WizardActions", "HelpActions", "MediaPlayerActions"],
			{
				"back": self.leavePlayer,
				"red": self.switchToConfigList,
				"green": self.keyStdFeed,
				"yellow": self.handleHistory,
			}, -2)
	
			self["historyactions"] = ActionMap(["ShortcutActions", "WizardActions", "MediaPlayerActions", "MovieSelectionActions", "HelpActions"],
			{
				"ok": self.keyOK,
				"back": self.closeHistory,
				"red": self.closeHistory,
				"green": self.keyStdFeed,
				"yellow": self.handleHistory,
				"up": self.keyUp,
				"down": self.keyDown,	
				"left": self.keyLeft,
				"right": self.keyRight,
			}, -2)
			
		else:
			self["searchactions"] = ActionMap(["ShortcutActions", "WizardActions", "HelpActions", "MediaPlayerActions"],
			{
				"ok": self.keyOK,
				"back": self.leavePlayer,
				"red": self.leavePlayer,
				"blue": self.openKeyboard,
				"yellow": self.handleHistory,
				"up": self.keyUp,
				"down": self.handleSuggestions,
				"left": self.keyLeft,
				"right": self.keyRight,
				"prevBouquet": self.switchToFeedList,
				"nextBouquet": self.switchToConfigList,
				"displayHelp": self.handleHelpWindow,
				"menu" : self.handleMenu,
			}, -2)

			self["suggestionactions"] = ActionMap(["ShortcutActions", "WizardActions", "MediaPlayerActions", "HelpActions"],
			{
				"ok": self.keyOK,
				"back": self.switchToConfigList,
				"red": self.switchToConfigList,
				"nextBouquet": self.switchToConfigList,
				"prevBouquet": self.switchToFeedList,
				"up": self.keyUp,
				"down": self.keyDown,
				"left": self.keyLeft,
				"right": self.keyRight,
			}, -2)
	
			self["videoactions"] = ActionMap(["ShortcutActions", "WizardActions", "MediaPlayerActions", "MovieSelectionActions", "HelpActions"],
			{
				"ok": self.keyOK,
				"back": self.leavePlayer,
				"red": self.leavePlayer,
				"yellow": self.handleHistory,
				"up": self.keyUp,
				"down": self.keyDown,	
				"nextBouquet": self.switchToConfigList,
				"green": self.keyStdFeed,
				"showEventInfo": self.showVideoInfo,
				"displayHelp": self.handleHelpWindow,
				"menu" : self.handleMenu,
			}, -2)			
			
			self["statusactions"] = ActionMap(["ShortcutActions", "WizardActions", "HelpActions", "MediaPlayerActions"],
			{
				"back": self.leavePlayer,
				"red": self.leavePlayer,
				"nextBouquet": self.switchToConfigList,
				"green": self.keyStdFeed,
				"yellow": self.handleHistory,
			}, -2)
	
			self["historyactions"] = ActionMap(["ShortcutActions", "WizardActions", "MediaPlayerActions", "MovieSelectionActions", "HelpActions"],
			{
				"ok": self.keyOK,
				"back": self.closeHistory,
				"red": self.closeHistory,
				"yellow": self.handleHistory,
				"up": self.keyUp,
				"down": self.keyDown,	
				"left": self.keyLeft,
				"right": self.keyRight,
			}, -2)

		self["videoactions"].setEnabled(False)
		self["statusactions"].setEnabled(False)
		self["historyactions"].setEnabled(False)
		self.timer = eTimer()
		self.timer.callback.append(self.picloadTimeout)
		self.SearchConfigEntry = None
		self.searchContextEntries = []
		config.plugins.mytube.search.searchTerm.value = ""
		ConfigListScreen.__init__(self, self.searchContextEntries, session)
		self.createSetup()
		self.onLayoutFinish.append(self.layoutFinished)
		self.onShown.append(self.setWindowTitle)
		self.onClose.append(self.__onClose)
		self.Timer = eTimer()
		self.Timer.callback.append(self.TimerFire)
		
	def __onClose(self):
		del self.Timer
		del self.timer
		self.session.nav.playService(self.lastservice)
		
	def layoutFinished(self):
		self.currList = "status"
		#self["key_green"].hide()
		current = self["config"].getCurrent()
		if current[1].help_window.instance is not None:
			current[1].help_window.instance.hide()
		if QBOXHD:
			if self.FirstRun == True:
				if config.plugins.mytube.general.loadFeedOnOpen.value:
					if config.plugins.mytube.general.startFeed.value == 'stdfeed':
						self.statuslist.append(( _("No videos to display"), _("Please select a standard feed or try searching for videos." ) ))
			else:
				self.statuslist.append(( _("Fetching feed entries"), _("Trying to download the Youtube feed entries. Please wait..." ) ))
		else:
			self.statuslist.append(( _("Fetching feed entries"), _("Trying to download the Youtube feed entries. Please wait..." ) ))
			
		self["feedlist"].style = "state"
		self['feedlist'].setList(self.statuslist)
		self.Timer.start(200)
		
	def TimerFire(self):
		self.Timer.stop()
		if config.plugins.mytube.general.loadFeedOnOpen.value:
			if QBOXHD:
				if config.plugins.mytube.general.startFeed.value == 'stdfeed':
					self.setState('noVideo')
				else:
					self.setState('getFeed')
			else:
				self.setState('getFeed')
		else:
			self.setState('byPass')
		
	def setWindowTitle(self):
		self.setTitle(_("MyTubePlayer"))

	def createSetup(self):
		self.searchContextEntries = []
		self.SearchConfigEntry = getConfigListEntry(_("Search Term(s)"), config.plugins.mytube.search.searchTerm)
		self.searchContextEntries.append(self.SearchConfigEntry)
		self["config"].list = self.searchContextEntries
		self["config"].l.setList(self.searchContextEntries)
		
	
	def TimergetFeed(self):
		self.TmrgetFeed.stop()
		self.getFeed(self.FeedURL, self.FeedName)
	

	def setState(self,status = None):
		if status:
			if self.FirstRun == True:
				self.appendEntries = False
				myTubeService.startService()
			self.currList = "status"
			self.statuslist = []
			self["videoactions"].setEnabled(False)
			self["searchactions"].setEnabled(False)
			#self["key_green"].hide()
			self["config_actions"].setEnabled(False)
			self["historyactions"].setEnabled(False)
			self["statusactions"].setEnabled(True)
			self["ButtonBlue"].hide()
			self["VKeyIcon"].hide()	
			if self.HistoryWindow is not None:
				self.HistoryWindow.deactivate()
				self.HistoryWindow.instance.hide()
			if status == 'getFeed':
				self.hideSuggestions()
				self.statuslist.append(( _("Fetching feed entries"), _("Trying to download the Youtube feed entries. Please wait..." ) ))
				self["feedlist"].style = "state"
				self['feedlist'].setList(self.statuslist)
			elif status == 'getSearchFeed':
				self.hideSuggestions()
				self.statuslist.append(( _("Fetching search entries"), _("Trying to download the Youtube search results. Please wait..." ) ))
				self["feedlist"].style = "state"
				self['feedlist'].setList(self.statuslist)
			elif status == 'Error':
				self.hideSuggestions()
				self.statuslist.append(( _("An error occured."), _("There was an error getting the feed entries. Please try again." ) ))
				self["feedlist"].style = "state"
				self['feedlist'].setList(self.statuslist)
			elif status == 'noVideos':
				self["key_green"].show()
				self.hideSuggestions()
				self.statuslist.append(( _("No videos to display"), _("Please select a standard feed or try searching for videos." ) ))
				self["feedlist"].style = "state"
				self['feedlist'].setList(self.statuslist)
			elif status == 'byPass':
				self.statuslist.append(( _("Not fetching feed entries"), _("Please enter your search term." ) ))
				self["feedlist"].style = "state"
				self['feedlist'].setList(self.statuslist)
				self.switchToConfigList()
			
			if self.FirstRun == True:
				if QBOXHD:
					if config.plugins.mytube.general.loadFeedOnOpen.value:
						if QBOXHD:
							if config.plugins.mytube.general.startFeed.value == 'hd':
								self.FeedURL = "http://gdata.youtube.com/feeds/api/videos/-/HD"
								self.FeedName = str(config.plugins.mytube.general.startFeed.value)
								self.TmrgetFeed.start(200, True)
							elif config.plugins.mytube.general.startFeed.value == 'stdfeed':
								self.keyStdFeed()
							else:
								self.FeedURL = self.BASE_STD_FEEDURL + str(config.plugins.mytube.general.startFeed.value)
								self.FeedName = str(config.plugins.mytube.general.startFeed.value)
								self.TmrgetFeed.start(200, True)
						else:
							if config.plugins.mytube.general.startFeed.value == 'hd':
								self.FeedURL = "http://gdata.youtube.com/feeds/api/videos/-/HD"
							else:
								self.FeedURL = self.BASE_STD_FEEDURL + str(config.plugins.mytube.general.startFeed.value)
								
							self.FeedName = str(config.plugins.mytube.general.startFeed.value)
							self.TmrgetFeed.start(200, True)
					else:
						self.keyStdFeed()
				else:
					if config.plugins.mytube.general.loadFeedOnOpen.value:
						if config.plugins.mytube.general.startFeed.value == 'hd':
							self.FeedURL = "http://gdata.youtube.com/feeds/api/videos/-/HD"
						else:
							self.FeedURL = self.BASE_STD_FEEDURL + str(config.plugins.mytube.general.startFeed.value)
						self.getFeed(self.FeedURL, str(config.plugins.mytube.general.startFeed.value))
				


	def handleHelpWindow(self):
		print "[handleHelpWindow]"
		if self.currList == "configlist":
			self.hideSuggestions()
			self.session.openWithCallback(self.ScreenClosed, MyTubeVideoHelpScreen, self.skin_path, wantedinfo = self.searchtext, wantedtitle = _("MyTubePlayer Help") )
		elif self.currList == "feedlist":
			self.session.openWithCallback(self.ScreenClosed, MyTubeVideoHelpScreen, self.skin_path, wantedinfo = self.feedtext, wantedtitle = _("MyTubePlayer Help") )
			
	def handleFirstHelpWindow(self):
		print "[handleFirstHelpWindow]"
		if config.plugins.mytube.general.showHelpOnOpen.value is True:
			if self.currList == "configlist":
				self.hideSuggestions()
				self.session.openWithCallback(self.firstRunHelpClosed, MyTubeVideoHelpScreen, self.skin_path,wantedinfo = self.feedtext, wantedtitle = _("MyTubePlayer Help") )
		else:
			self.FirstRun = False
			
	def firstRunHelpClosed(self):
		if self.FirstRun == True:	
			self.FirstRun = False
			self.switchToConfigList()
			

	def handleMenu(self):
		print "currlist im HandleMenu:",self.currList
		if self.currList == "configlist":
			menulist = (
					(_("MyTube Settings"), "settings"),
				)
			self.hideSuggestions()
			self.session.openWithCallback(self.openMenu, ChoiceBox, title=_("Select your choice."), list = menulist)

		elif self.currList == "feedlist":
			menulist = [(_("MyTube Settings"), "settings")]
			menulist.extend((
					(_("View related videos"), "related"),
					(_("View response videos"), "response")
				))
			if config.usage.setup_level.index >= 2: # expert+
				menulist.extend((
					(_("Download Video"), "download"),
					(_("View active downloads"), "downview")
				))
						
			self.hideSuggestions()
			self.session.openWithCallback(self.openMenu, ChoiceBox, title=_("Select your choice."), list = menulist)

	def openMenu(self, answer):
		answer = answer and answer[1]
		print "openMenu - ANSWER",answer
		if answer == "settings":
			print "settings selected"
			self.session.openWithCallback(self.ScreenClosed,MyTubeSettingsScreen, self.skin_path )
		elif answer == "related":
			current = self["feedlist"].getCurrent()[0]
			self.setState('getFeed')
			self.getRelatedVideos(current)
		elif answer == "response":
			current = self["feedlist"].getCurrent()[0]
			self.setState('getFeed')
			self.getResponseVideos(current)
		elif answer == "download":
			if self.currList == "feedlist":
				current = self[self.currList].getCurrent()
				if current:
					myentry = current[0]
					if myentry:
						myurl = myentry.getVideoUrl()
						filename = str(config.plugins.mytube.general.videodir.value)+ str(myentry.getTitle()) + '.mp4'
						if QBOXHD:
							if pathExists(str(config.plugins.mytube.general.videodir.value)):
								job_manager.AddJob(downloadJob(myurl,filename, str(myentry.getTitle())[:30]))
								self.session.open(MessageBox, _("Downloading %s started.") % str(myentry.getTitle()), type = MessageBox.TYPE_INFO)
							else:
								self.session.open(MessageBox, _("%s not exists.") % str(config.plugins.mytube.general.videodir.value), type = MessageBox.TYPE_INFO)
						else:
							job_manager.AddJob(downloadJob(myurl,filename, str(myentry.getTitle())[:30]))
	
		elif answer == "downview":
			self.tasklist = []
			for job in job_manager.getPendingJobs():
				self.tasklist.append((job,job.name,job.getStatustext(),int(100*job.progress/float(job.end)) ,str(100*job.progress/float(job.end)) + "%" ))
			self.session.open(MyTubeTasksScreen, self.skin_path , self.tasklist)
		elif answer == None:
			print "No menuentry selected, we should just switch back to old state."
			self.ScreenClosed()
	
	def openKeyboard(self):
		self.hideSuggestions()
		self.session.openWithCallback(self.SearchEntryCallback, VirtualKeyBoard, title = (_("Enter your search term(s)")), text = config.plugins.mytube.search.searchTerm.value)

	def ScreenClosed(self):
		print "ScreenCLosed, restoring old window state"
		if self.currList == "historylist":
			if self.HistoryWindow.status() is False:
				print "status is FALSE"
				self.HistoryWindow.activate()
				self.HistoryWindow.instance.show()
		elif self.currList == "configlist":
			self.switchToConfigList()
			if QBOXHD:
				config.save()
			else:
				ConfigListScreen.keyOK(self)
		elif self.currList == "feedlist":
			self.switchToFeedList()

	def SearchEntryCallback(self, callback = None):
		if callback is not None and len(callback):
			config.plugins.mytube.search.searchTerm.value = callback
			if QBOXHD:
				config.save()
			else:
				ConfigListScreen.keyOK(self)
			self["config"].getCurrent()[1].getSuggestions()
		current = self["config"].getCurrent()
		if current[1].help_window.instance is not None:
			current[1].help_window.instance.show()	
		if current[1].suggestionsWindow.instance is not None:
			current[1].suggestionsWindow.instance.show()
		self.propagateUpDownNormally = True

	def openStandardFeedClosed(self, answer):
		answer = answer and answer[1]
		print "openStandardFeedClosed - ANSWER",answer
		if answer is not None:
			if QBOXHD:
				self.setTitle(_("Standard Feeds"))
				self.switchToFeedList()
			if answer == 'hd':
				self.FeedURL = "http://gdata.youtube.com/feeds/api/videos/-/HD"
			else:
				self.FeedURL = self.BASE_STD_FEEDURL + str(answer)
			self.setState('getFeed')
			self.appendEntries = False
			if QBOXHD:
				self.FeedName = str(answer)
				self.TmrgetFeed.start(200, True)
			else:
				self.getFeed(self.FeedURL, str(answer))

	def handleLeave(self, how):
		self.is_closing = True
		if how == "ask":
			if self.currList == "configlist":
				list = (
					(_("Yes"), "quit"),
					(_("No"), "continue"),
					(_("No, but switch to video entries."), "switch2feed")
				)
			else:
				list = (
					(_("Yes"), "quit"),
					(_("No"), "continue"),
					(_("No, but switch to video search."), "switch2search")
				)					
			self.session.openWithCallback(self.leavePlayerConfirmed, ChoiceBox, title=_("Really quit MyTube Player?"), list = list)
		else:
			self.leavePlayerConfirmed([True, how])

	def leavePlayer(self):
		print "self.currList im leavePlayer",self.currList
		if self.HistoryWindow is not None:
			self.HistoryWindow.deactivate()
			self.HistoryWindow.instance.hide()
		if self.currList == "configlist":
			current = self["config"].getCurrent()
			if current[1].suggestionsWindow.activeState is True:
				self.propagateUpDownNormally = True
				current[1].deactivateSuggestionList()
				self["config"].invalidateCurrent()
			else:
				self.hideSuggestions()
				self.handleLeave(config.plugins.mytube.general.on_exit.value)
		else:
			self.hideSuggestions()
			self.handleLeave(config.plugins.mytube.general.on_exit.value)

	def leavePlayerConfirmed(self, answer):
		answer = answer and answer[1]
		print "ANSWER",answer
		if answer == "quit":
			cmd = "rm -rf /tmp/*.jpg"
			self.CleanupConsole = Console()
			self.CleanupConsole.ePopen(cmd, self.doQuit)
		elif answer == "continue":
			if self.currList == "historylist":
				if self.HistoryWindow.status() is False:
					print "status is FALSE"
					self.HistoryWindow.activate()
					self.HistoryWindow.instance.show()
			elif self.currList == "configlist":
				self.switchToConfigList()
			elif self.currList == "feedlist":
				self.switchToFeedList()				
		elif answer == "switch2feed":
			self.switchToFeedList()
		elif answer == "switch2search":
			self.switchToConfigList()
		elif answer == None:
			print "No menuentry selected, we should just switch back to old state."
			if self.currList == "historylist":
				if self.HistoryWindow.status() is False:
					print "status is FALSE"
					self.HistoryWindow.activate()
					self.HistoryWindow.instance.show()
			elif self.currList == "configlist":
				self.switchToConfigList()
			elif self.currList == "feedlist":
				self.switchToFeedList()

	def doQuit(self, result, retval,extra_args):
		if self["config"].getCurrent()[1].suggestionsWindow is not None:
			self.session.deleteDialog(self["config"].getCurrent()[1].suggestionsWindow)
		if self.HistoryWindow is not None:
			self.session.deleteDialog(self.HistoryWindow)
		if config.plugins.mytube.general.showHelpOnOpen.value is True:
			config.plugins.mytube.general.showHelpOnOpen.value = False
			config.plugins.mytube.general.showHelpOnOpen.save()
		print "self.History im doQuit:",self.History
		if not config.plugins.mytube.general.clearHistoryOnClose.value:
			if self.History and len(self.History):
				config.plugins.mytube.general.history.value = ",".join(self.History)
		else:
			config.plugins.mytube.general.history.value = ""
		config.plugins.mytube.general.history.save()
		config.plugins.mytube.general.save()
		config.plugins.mytube.save()
		if self.CleanupConsole is not None:
			if len(self.CleanupConsole.appContainers):
				for name in self.CleanupConsole.appContainers.keys():
					self.CleanupConsole.kill(name)
		self.close()
			
	def keyOK(self):
		print "self.currList----->",self.currList
		if self.currList == "configlist" or self.currList == "suggestionslist":
			self["config"].invalidateCurrent()
			if config.plugins.mytube.search.searchTerm.value != "":
				self.add2History()
				searchContext = config.plugins.mytube.search.searchTerm.value
				print "Search searchcontext",searchContext
				if isinstance(self["config"].getCurrent()[1], ConfigTextWithGoogleSuggestions) and not self.propagateUpDownNormally:
					self.propagateUpDownNormally = True
					self["config"].getCurrent()[1].deactivateSuggestionList()
				self.setState('getSearchFeed')
				self.runSearch(searchContext)
		elif self.currList == "feedlist":
			current = self[self.currList].getCurrent()
			if current:
				print current
				myentry = current[0]
				if myentry is not None:
					myurl = myentry.getVideoUrl()
					print "Playing URL",myurl
					if myurl is not None:
						myreference = eServiceReference(4097,0,myurl)
						myreference.setName(myentry.getTitle())
						self.session.open(MyTubePlayer, myreference, self.lastservice, infoCallback = self.showVideoInfo, nextCallback = self.getNextEntry, prevCallback = self.getPrevEntry )
					else:
						self.session.open(MessageBox, _("Sorry, video is not available!"), MessageBox.TYPE_INFO)
		elif self.currList == "historylist":
			if self.HistoryWindow is not None:
				if QBOXHD:
					if self.HistoryWindow.getlistlenght() > 0:
						config.plugins.mytube.search.searchTerm.value = self.HistoryWindow.getSelection()
					else:
						config.plugins.mytube.search.searchTerm.value = ""
				else:
					config.plugins.mytube.search.searchTerm.value = self.HistoryWindow.getSelection()
			self["config"].invalidateCurrent()
			if config.plugins.mytube.search.searchTerm.value != "":
				searchContext = config.plugins.mytube.search.searchTerm.value
				print "Search searchcontext",searchContext
				self.setState('getSearchFeed')
				self.runSearch(searchContext)

	def keyUp(self):
		print "self.currList im KeyUp",self.currList
		if self.currList == "suggestionslist":
			if config.plugins.mytube.search.searchTerm.value != "":
				if not self.propagateUpDownNormally:
					self["config"].getCurrent()[1].suggestionListUp()
					self["config"].invalidateCurrent()
		elif self.currList == "feedlist":
			self[self.currList].selectPrevious()
		elif self.currList == "historylist":
			if self.HistoryWindow is not None and self.HistoryWindow.shown:
				self.HistoryWindow.up()

	def keyDown(self):
		print "self.currList im KeyDown",self.currList
		if self.currList == "suggestionslist":
			if config.plugins.mytube.search.searchTerm.value != "":
				if not self.propagateUpDownNormally:
					self["config"].getCurrent()[1].suggestionListDown()
					self["config"].invalidateCurrent()
		elif self.currList == "feedlist":
			print self[self.currList].count()
			print self[self.currList].index
			if self[self.currList].index == self[self.currList].count()-1 and myTubeService.getNextFeedEntriesURL() is not None:
				self.session.openWithCallback(self.getNextEntries, MessageBox, _("Do you want to see more entries?"))
			else:
				self[self.currList].selectNext()
		elif self.currList == "historylist":
			if self.HistoryWindow is not None and self.HistoryWindow.shown:
				self.HistoryWindow.down()
				
	def keyRight(self):
		print "self.currList im KeyRight",self.currList
		if self.propagateUpDownNormally:
			ConfigListScreen.keyRight(self)
		else:
			if self.currList == "suggestionslist":
				if config.plugins.mytube.search.searchTerm.value != "":
					self["config"].getCurrent()[1].suggestionListPageDown()
					self["config"].invalidateCurrent()
			elif self.currList == "historylist":
				if self.HistoryWindow is not None and self.HistoryWindow.shown:
					self.HistoryWindow.pageDown()

	def keyLeft(self):
		print "self.currList im kEyLeft",self.currList
		if self.propagateUpDownNormally:
			ConfigListScreen.keyLeft(self)
		else:
			if self.currList == "suggestionslist":
				if config.plugins.mytube.search.searchTerm.value != "":
					self["config"].getCurrent()[1].suggestionListPageUp()
					self["config"].invalidateCurrent()
			elif self.currList == "historylist":
				if self.HistoryWindow is not None and self.HistoryWindow.shown:
					self.HistoryWindow.pageDown()
	def keyStdFeed(self):
		if QBOXHD:
			if self.FirstRun == True:	
				self.FirstRun = False

		self.hideSuggestions()
		menulist = [(_("HD videos"), "hd")]
		menulist.extend((
			(_("Top rated"), "top_rated"),
			(_("Top favorites"), "top_favorites"),
			(_("Most viewed"), "most_viewed"),
			(_("Most popular"), "most_popular"),
			(_("Most recent"), "most_recent"),
			(_("Most discussed"), "most_discussed"),
			(_("Most linked"), "most_linked"),
			(_("Recently featured"), "recently_featured"),
			(_("Most responded"), "most_responded")
		))
		self.session.openWithCallback(self.openStandardFeedClosed, ChoiceBox, title=_("Select new feed to view."), list = menulist)

	def handleSuggestions(self):
		print "handleSuggestions"
		print "self.currList",self.currList
		if self.currList == "configlist":
			if QBOXHD:
				current = self["config"].getCurrent()
				if current[1].help_window.instance is not None:
					current[1].help_window.instance.hide()
					
				if current[1].suggestionsWindow.instance is not None:
					print "current[1].suggestionsWindow.getlistlenght(): %d" % current[1].suggestionsWindow.getlistlenght()
					if current[1].suggestionsWindow.getlistlenght() > 0:
						self.switchToSuggestionsList()
					else:
						self.switchToFeedList()
			else:
				self.switchToSuggestionsList()
				
		elif self.currList == "historylist":
			if self.HistoryWindow is not None and self.HistoryWindow.shown:
				self.HistoryWindow.down()

	def switchToSuggestionsList(self):
		print "switchToSuggestionsList"
		self.currList = "suggestionslist"
		self["ButtonBlue"].hide()
		self["VKeyIcon"].hide()	
		self["statusactions"].setEnabled(False)
		self["config_actions"].setEnabled(False)	
		self["videoactions"].setEnabled(False)
		self["searchactions"].setEnabled(False)
		self["suggestionactions"].setEnabled(True)
		self["historyactions"].setEnabled(False)
		
		if not QBOXHD:
			self["key_green"].hide()
			
		self.propagateUpDownNormally = False
		self["config"].invalidateCurrent()
		if self.HistoryWindow is not None and self.HistoryWindow.shown:
			self.HistoryWindow.deactivate()
			self.HistoryWindow.instance.hide()
	
	def switchToConfigList(self):
		print "switchToConfigList"
		self.setTitle(_("Search"))
		self.currList = "configlist"
		self["config_actions"].setEnabled(True)	
		self["historyactions"].setEnabled(False)
		self["statusactions"].setEnabled(False)
		self["videoactions"].setEnabled(False)
		self["suggestionactions"].setEnabled(False)
		self["searchactions"].setEnabled(True)
		
		if not QBOXHD:
			self["key_green"].hide()
			
		self["ButtonBlue"].show()
		self["VKeyIcon"].show()
		self["config"].invalidateCurrent()
		helpwindowpos = self["HelpWindow"].getPosition()
		current = self["config"].getCurrent()
		
		if current[1].help_window.instance is not None:
			current[1].help_window.instance.move(ePoint(helpwindowpos[0],helpwindowpos[1]))
			current[1].help_window.instance.show()
			
		if current[1].suggestionsWindow.instance is not None:
			current[1].suggestionsWindow.instance.show()
			self["config"].getCurrent()[1].getSuggestions()
			
		self.propagateUpDownNormally = True
		if self.HistoryWindow is not None and self.HistoryWindow.shown:
			self.HistoryWindow.deactivate()
			self.HistoryWindow.instance.hide()
			
		if self.FirstRun == True:
			self.handleFirstHelpWindow()


	def switchToFeedList(self, append = False):
		print "switchToFeedList"
		print "switching to feedlist from:",self.currList
		print "len(self.videolist):",len(self.videolist)
		if self.HistoryWindow is not None and self.HistoryWindow.shown:
			self.HistoryWindow.deactivate()
			self.HistoryWindow.instance.hide()
		self.hideSuggestions()
		if len(self.videolist):
			self.currList = "feedlist"
			self["ButtonBlue"].hide()
			self["VKeyIcon"].hide()	
			self["videoactions"].setEnabled(True)
			self["suggestionactions"].setEnabled(False)
			self["searchactions"].setEnabled(False)
			self["statusactions"].setEnabled(False)
			self["historyactions"].setEnabled(False)
			
			if not QBOXHD:
				self["key_green"].show()
				
			self["config_actions"].setEnabled(False)
			if not append:
				self[self.currList].setIndex(0)
		else:
			self.setState('noVideos')


	def switchToHistory(self):
		print "switchToHistory"
		self.oldlist = self.currList
		self.currList = "historylist"
		print "switchToHistory currentlist",self.currList
		print "switchToHistory oldlist",self.oldlist
		self.hideSuggestions()
		self["ButtonBlue"].hide()
		self["VKeyIcon"].hide()	
		
		if not QBOXHD:
			self["key_green"].hide()
			
		self["videoactions"].setEnabled(False)
		self["suggestionactions"].setEnabled(False)
		self["searchactions"].setEnabled(False)
		self["statusactions"].setEnabled(False)
		self["config_actions"].setEnabled(False)
		self["historyactions"].setEnabled(True)
		self.HistoryWindow.activate()
		self.HistoryWindow.instance.show()	

	def handleHistory(self):
		if self.HistoryWindow is None:
			self.HistoryWindow = self.session.instantiateDialog(MyTubeHistoryScreen)
		if self.currList in ("configlist","feedlist"):
			if self.HistoryWindow.status() is False:
				print "status is FALSE,switchToHistory"
				self.switchToHistory()
		elif self.currList == "historylist":
			self.closeHistory()

	def closeHistory(self):
		print "closeHistory currentlist",self.currList
		print "closeHistory oldlist",self.oldlist
		if self.currList == "historylist":
			if self.HistoryWindow.status() is True:
				print "status is TRUE, closing historyscreen"
				self.HistoryWindow.deactivate()
				self.HistoryWindow.instance.hide()
				if self.oldlist == "configlist":
					self.switchToConfigList()
				elif self.oldlist == "feedlist":
					self.switchToFeedList()

	def add2History(self):
		if self.History is None:
			self.History = config.plugins.mytube.general.history.value.split(',')
		if self.History[0] == '':
			del self.History[0]
		print "self.History im add",self.History
		if config.plugins.mytube.search.searchTerm.value in self.History:
			self.History.remove((config.plugins.mytube.search.searchTerm.value))
		self.History.insert(0,(config.plugins.mytube.search.searchTerm.value))
		if len(self.History) == 30:
			self.History.pop()
		config.plugins.mytube.general.history.value = ",".join(self.History)
		config.plugins.mytube.general.history.save()
		print "configvalue",config.plugins.mytube.general.history.value

	def hideSuggestions(self):
		current = self["config"].getCurrent()
		if current[1].help_window.instance is not None:
			current[1].help_window.instance.hide()	
		if current[1].suggestionsWindow.instance is not None:
			current[1].suggestionsWindow.instance.hide()
		self.propagateUpDownNormally = True

	def getFeed(self, feedUrl, feedName):
		try:
			feed = myTubeService.getFeed(feedUrl)
		except Exception, e:
			feed = None
			print "Error querying feed :",feedName
			print "E-->",e
			self.setState('Error')
		if feed is not None:
			self.ytfeed = feed
		self.loadPreviewpics()

	def getNextEntries(self, result):
		if not result:
			return
		nextUrl = myTubeService.getNextFeedEntriesURL()
		if nextUrl is not None:
			self.appendEntries = True
			if QBOXHD:
				self.FeedURL = str(nextUrl)
				self.FeedName = _("More video entries.")
				self.TmrgetFeed.start(200, True)
				self.msgWaitNextFeeds = self.session.openWithCallback(None, MessageBox, _("Loading videos, be patient ..."), type = MessageBox.TYPE_INFO, enable_input = False)
			else:
				self.getFeed(nextUrl, _("More video entries."))

	def getRelatedVideos(self, myentry):
		if myentry:
			myurl =  myentry.getRelatedVideos()
			print "RELATEDURL--->",myurl
			if myurl is not None:
				if QBOXHD:
					self.FeedURL = str(myurl)
					self.FeedName = _("Related video entries.")
					self.TmrgetFeed.start(200, True)
				else:
					self.getFeed(myurl, _("Related video entries."))

	def getResponseVideos(self, myentry):
		if myentry:
			myurl =  myentry.getResponseVideos()
			print "RESPONSEURL--->",myurl
			if myurl is not None:
				if QBOXHD:
					self.FeedURL = str(myurl)
					self.FeedName = _("Response video entries.")
					self.TmrgetFeed.start(200, True)
				else:				
					self.getFeed(myurl, _("Response video entries."))
				
				
	def TimerSearchFeed(self):
		self.TmrSearchFeed.stop()
		self.searchFeed(config.plugins.mytube.search.searchTerm.value)

	def runSearch(self, searchContext = None):
		print "[MyTubePlayer] runSearch"
		if searchContext is not None:
			print "[MyTubePlayer] searchDialogClosed: ", searchContext
			if QBOXHD:
				self.TmrSearchFeed.start(200, True)
			else:
				self.searchFeed(searchContext)

	def searchFeed(self, searchContext):
		print "[MyTubePlayer] searchFeed"
		self.appendEntries = False
		try:
			feed = myTubeService.search(searchContext, 
					orderby = config.plugins.mytube.search.orderBy.value,
					racy = config.plugins.mytube.search.racy.value,
					lr = config.plugins.mytube.search.lr.value,
					categories = [ config.plugins.mytube.search.categories.value ],
					sortOrder = config.plugins.mytube.search.sortOrder.value)
		except Exception, e:
			feed = None
			print "Error querying search for :",config.plugins.mytube.search.searchTerm.value
			print "E-->",e
			self.setState('Error')
		if feed is not None:
			self.ytfeed = feed
		if self.FirstRun == True:	
			self.FirstRun = False
		self.loadPreviewpics()

	def loadPreviewpics(self):
		self.thumbnails = []
		self.mytubeentries = None
		self.index = 0
		self.maxentries = 0
		self.picloads = {}
		self.mytubeentries = myTubeService.getEntries()
		self.maxentries = len(self.mytubeentries)-1
		if self.mytubeentries and len(self.mytubeentries):
			currindex = 0
			for entry in self.mytubeentries:
				TubeID = entry.getTubeId()
				thumbnailFile = "/tmp/" + str(TubeID) + ".jpg"
				if QBOXHD:
					currPic = [currindex,TubeID,thumbnailFile,None,False]
				else:
					currPic = [currindex,TubeID,thumbnailFile,None]
				self.thumbnails.append(currPic)
				thumbnailUrl = None
				thumbnailUrl = entry.getThumbnailUrl(0)
				if thumbnailUrl is not None:
					client.downloadPage(thumbnailUrl,thumbnailFile).addCallback(self.fetchFinished,currindex,str(TubeID)).addErrback(self.fetchFailed,currindex,str(TubeID))
				currindex +=1
		else:
			pass

	def fetchFailed(self, string, index, id):
		print "[fetchFailed] for index:" + str(index) + "for YoutubeID:" + id + string.getErrorMessage()
		if QBOXHD: #Mark a flag of finish
			self.thumbnails[index][4] = True
			for entry in self.thumbnails:
				if entry[4] == False:
					return
				
			#All thumbnails are finished so i close wait msg
			if QBOXHD:
				if self.msgWaitNextFeeds is not None:
					self.msgWaitNextFeeds.close(True)
					self.msgWaitNextFeeds = None


	def fetchFinished(self, string, index, id):
		print "[fetchFinished] for index:" + str(index) + " for YoutubeID:" + id
		if QBOXHD: #Mark a flag of finish
			self.thumbnails[index][4] = True
		self.decodePic(index)

	def decodePic(self, index):
		sc = AVSwitch().getFramebufferScale()
		self.picloads[index] = ePicLoad()
		self.picloads[index].PictureData.get().append(boundFunction(self.finish_decode, index))
		for entry in self.thumbnails:
			if entry[0] == index:
				self.index = index
				thumbnailFile = entry[2]
				if (os_path.exists(thumbnailFile) == True):
					#print "[decodePic] DECODING THUMBNAIL for INDEX:"+  str(self.index) + "and file: " + thumbnailFile
					self.picloads[index].setPara((self["thumbnail"].instance.size().width(), self["thumbnail"].instance.size().height(), sc[0], sc[1], False, 1, "#00000000"))
					self.picloads[index].startDecode(thumbnailFile)
				else:
					print "[decodePic] Thumbnail file NOT FOUND !!!-->:",thumbnailFile

	def finish_decode(self, picindex = None, picInfo=None):
		#print "finish_decode - of INDEX", picindex
		ptr = self.picloads[picindex].getData()
		if QBOXHD:
			if ptr != None:
				print ptr
				self.thumbnails[picindex][3] = ptr
				if (os_path.exists(self.thumbnails[picindex][2]) == True):
					#print "removing", self.thumbnails[picindex][2]
					remove(self.thumbnails[picindex][2])
					
				del self.picloads[picindex]
				if len(self.picloads) == 0:
					self.timer.startLongTimer(3)
			else:
				if (os_path.exists(self.thumbnails[picindex][2]) == True):
					#print "removing", self.thumbnails[picindex][2]
					remove(self.thumbnails[picindex][2])
					
				del self.picloads[picindex]
				if len(self.picloads) == 0:
					self.timer.startLongTimer(3)
		else:
			if ptr != None:
				print ptr
				self.thumbnails[picindex][3] = ptr
				if (os_path.exists(self.thumbnails[picindex][2]) == True):
					#print "removing", self.thumbnails[picindex][2]
					remove(self.thumbnails[picindex][2])
					del self.picloads[picindex]
					if len(self.picloads) == 0:
						self.timer.startLongTimer(3)

	def picloadTimeout(self):
		self.timer.stop()
		if len(self.picloads) == 0:
			print "all decodes should be now really finished"
			self.buildEntryList()
		else:
			self.timer.startLongTimer(2)

	def buildEntryList(self):
		myindex = 0
		if self.appendEntries == False:
			self.videolist = []
			for entry in self.mytubeentries:
				self.videolist.append(self.buildEntryComponent(entry, myindex))
				myindex +=1
			if len(self.videolist):
				self["feedlist"].style = "default"
				self["feedlist"].disable_callbacks = True
				self["feedlist"].list = self.videolist
				self["feedlist"].disable_callbacks = False
				self["feedlist"].setIndex(0)
				self["feedlist"].setList(self.videolist)
				self["feedlist"].updateList(self.videolist)
				if self.FirstRun == True:	
					self.switchToConfigList()
				else:
					self.switchToFeedList()
		else:		
			if QBOXHD:
				if self.msgWaitNextFeeds is not None:
					self.msgWaitNextFeeds.close(True)
					self.msgWaitNextFeeds = None
			
			self.oldfeedentrycount = self["feedlist"].count()
			for entry in self.mytubeentries:
				self.videolist.append(self.buildEntryComponent(entry, myindex))
				myindex +=1
			if len(self.videolist):
				self["feedlist"].style = "default"
				old_index = self["feedlist"].index
				self["feedlist"].disable_callbacks = True
				self["feedlist"].list = self.videolist
				self["feedlist"].disable_callbacks = False
				self["feedlist"].setList(self.videolist)
				self["feedlist"].setIndex(old_index)
				self["feedlist"].updateList(self.videolist)
				self["feedlist"].selectNext()
				self.switchToFeedList(True)

	
	def buildEntryComponent(self, entry, index):
		Title = entry.getTitle()
		print "Titel-->",Title
		Description = entry.getDescription()
		TubeID = entry.getTubeId()
		PublishedDate = entry.getPublishedDate()
		if PublishedDate is not "unknown":
			published = PublishedDate.split("T")[0]
		else:
			published = "unknown"
		Views = entry.getViews()
		if Views is not "not available":
			views = Views
		else:
			views = "not available"
		Duration = entry.getDuration()
		if Duration is not 0:
			durationInSecs = int(Duration)
			mins = int(durationInSecs / 60)
			secs = durationInSecs - mins * 60
			duration = "%d:%02d" % (mins, secs)
		else:
			duration = "not available"
		Ratings = entry.getNumRaters()
		if Ratings is not "":
			ratings = Ratings
		else:
			ratings = ""
		thumbnail = self.thumbnails[index][3]
		return((entry, Title, Description, TubeID, thumbnail, _("Added: ") + str(published), _("Views: ") + str(views), _("Duration: ") + str(duration), _("Ratings: ") + str(ratings) ))	

	def getNextEntry(self):
		i = self["feedlist"].getIndex() + 1
		if i < len(self.videolist):
			self["feedlist"].selectNext()
			current = self["feedlist"].getCurrent()
			if current:
				myentry = current[0]
				if myentry:
					myurl = myentry.getVideoUrl()
					if myurl is not None:
						print "Got a URL to stream"
						myreference = eServiceReference(4097,0,myurl)
						myreference.setName(myentry.getTitle())
						return myreference,False
					else:
						print "NoURL im getNextEntry"
						return None,True
						
		print "no more entries to play"
		return None,False

	def getPrevEntry(self):
		i = self["feedlist"].getIndex() - 1
		if i >= 0:
			self["feedlist"].selectPrevious()
			current = self["feedlist"].getCurrent()
			if current:
				myentry = current[0]
				if myentry:
					myurl = myentry.getVideoUrl()
					if myurl is not None:
						print "Got a URL to stream"
						myreference = eServiceReference(4097,0,myurl)
						myreference.setName(myentry.getTitle())
						return myreference,False
					else:
						return None,True
		return None,False

	def showVideoInfo(self):
		if self.currList == "feedlist":
			cmd = "rm -rf /tmp/*.jpg"
			if self.CleanupConsole is None:
				self.CleanupConsole = Console()
			self.CleanupConsole.ePopen(cmd, self.openInfoScreen)
	
	def openInfoScreen(self, result, retval,extra_args):
		if self.currList == "feedlist":
			current = self[self.currList].getCurrent()
			if current:
				myentry = current[0]
				if myentry:
					print "Title im showVideoInfo",myentry.getTitle()
					videoinfos = myentry.PrintEntryDetails()
					self.session.open(MyTubeVideoInfoScreen, self.skin_path, videoinfo = videoinfos )
		

class MyTubeVideoInfoScreen(Screen):
	skin = """
		<screen name="MyTubeVideoInfoScreen" flags="wfNoBorder" position="0,0" size="720,576" title="MyTubePlayerMainScreen..." >
			<ePixmap position="0,0" zPosition="-1" size="720,576" pixmap="~/mytubemain_bg.png" alphatest="on" transparent="1" backgroundColor="transparent"/>
			<widget name="title" position="60,50" size="600,50" zPosition="5" valign="center" halign="left" font="Regular;21" transparent="1" foregroundColor="white" shadowColor="black" shadowOffset="-1,-1" />
			<widget name="starsbg" pixmap="~/starsbar_empty.png" position="560,220" zPosition="5" size="100,20" transparent="1" alphatest="on" />
			<widget name="stars" pixmap="~/starsbar_filled.png" position="560,220" zPosition="6" size="100,20"  transparent="1" />
			<widget source="infolist" render="Listbox" position="50,110" size="620,110" zPosition="6" scrollbarMode="showNever" selectionDisabled="1" transparent="1">
				<convert type="TemplatedMultiContent">
				{"templates":
					{"default": (110,[
							MultiContentEntryPixmapAlphaTest(pos = (0, 4), size = (130, 98), png = 0), # index 0 is the thumbnail
							MultiContentEntryPixmapAlphaTest(pos = (130, 4), size = (130, 98), png = 1), # index 0 is the thumbnail
							MultiContentEntryPixmapAlphaTest(pos = (260, 4), size = (130, 98), png = 2), # index 0 is the thumbnail
							MultiContentEntryPixmapAlphaTest(pos = (390, 4), size = (130, 98), png = 3), # index 0 is the thumbnail
						]),
					"state": (110,[
							MultiContentEntryText(pos = (10, 40), size = (550, 38), font=2, flags = RT_HALIGN_LEFT | RT_VALIGN_TOP| RT_WRAP, text = 0), # index 0 is the name
						])
					},
					"fonts": [gFont("Regular", 20),gFont("Regular", 14),gFont("Regular", 28)],
					"itemHeight": 110
				}
				</convert>
			</widget>
			<widget name="author" position="60,220" size="300,20" zPosition="10" font="Regular;21" transparent="1" halign="left" valign="top" />
			<widget name="duration" position="370,220" size="200,20" zPosition="10" font="Regular;21" transparent="1" halign="left" valign="top" />
			<widget name="published" position="60,245" size="300,20" zPosition="10" font="Regular;21" transparent="1" halign="left" valign="top" />
			<widget name="views" position="370,245" size="200,20" zPosition="10" font="Regular;21" transparent="1" halign="left" valign="top" />
			<widget name="tags" position="60,270" size="600,20" zPosition="10" font="Regular;21" transparent="1" halign="left" valign="top" />
			<widget name="detailtext" position="60,300" size="610,200" zPosition="10" font="Regular;21" transparent="1" halign="left" valign="top"/>
			<ePixmap position="100,500" size="100,40" zPosition="0" pixmap="~/plugin.png" alphatest="on" transparent="1" />
			<ePixmap position="220,500" zPosition="4" size="140,40" pixmap="skin_default/buttons/red.png" transparent="1" alphatest="on" />
			<widget name="key_red" position="220,500" zPosition="5" size="140,40" valign="center" halign="center" font="Regular;21" transparent="1" foregroundColor="white" shadowColor="black" shadowOffset="-1,-1" />
			<widget name="thumbnail" position="0,0" size="130,98" alphatest="on"/> # fake entry for dynamic thumbnail resizing, currently there is no other way doing this.
		</screen>"""
		
	def __init__(self, session, plugin_path, videoinfo = None):
		Screen.__init__(self, session)
		self.session = session
		self.skin_path = plugin_path
		self.videoinfo = videoinfo
		self.infolist = []
		self.thumbnails = []
		self.picloads = {}
		self["title"] = Label()
		self["key_red"] = Button(_("Close"))
		self["thumbnail"] = Pixmap()
		self["thumbnail"].hide()
		self["detailtext"] = ScrollLabel()
		self["starsbg"] = Pixmap()
		self["stars"] = ProgressBar()
		self["duration"] = Label()
		self["author"] = Label()
		self["published"] = Label()
		self["views"] = Label()
		self["tags"] = Label()
		self["shortcuts"] = ActionMap(["ShortcutActions", "WizardActions", "MovieSelectionActions"],
		{
			"back": self.close,
			"red": self.close,
			"up": self.pageUp,
			"down":	self.pageDown,
			"left":	self.pageUp,
			"right": self.pageDown,
		}, -2)
		
		self["infolist"] = List(self.infolist)
		self.timer = eTimer()
		self.timer.callback.append(self.picloadTimeout)
		self.onLayoutFinish.append(self.layoutFinished)
		self.onShown.append(self.setWindowTitle)

	def layoutFinished(self):
		self.statuslist = []
		self.statuslist.append(( _("Downloading screenshots. Please wait..." ),_("Downloading screenshots. Please wait..." ) ))
		self["infolist"].style = "state"
		self['infolist'].setList(self.statuslist)
		self.loadPreviewpics()		
		if self.videoinfo["Title"] is not None:
			self["title"].setText(self.videoinfo["Title"])
		Description = None
		if self.videoinfo["Description"] is not None:
			Description = self.videoinfo["Description"]
		else:
			Description = None
		if Description is not None:
			self["detailtext"].setText(Description.strip())

		if self.videoinfo["RatingAverage"] is not 0:
			ratingStars = int(round(20 * float(self.videoinfo["RatingAverage"]), 0))
			self["stars"].setValue(ratingStars)
		else:
			self["stars"].hide()
			self["starsbg"].hide()
		
		if self.videoinfo["Duration"] is not 0:
			durationInSecs = int(self.videoinfo["Duration"])
			mins = int(durationInSecs / 60)
			secs = durationInSecs - mins * 60
			duration = "%d:%02d" % (mins, secs)
			self["duration"].setText(_("Duration: ") + str(duration))
		
		if self.videoinfo["Author"] is not None or '':
			self["author"].setText(_("Author: ") + self.videoinfo["Author"])

		if self.videoinfo["Published"] is not "unknown":
			self["published"].setText(_("Added: ") + self.videoinfo["Published"].split("T")[0])
			
		if self.videoinfo["Views"] is not "not available":
			self["views"].setText(_("Views: ") + str(self.videoinfo["Views"]))

		if self.videoinfo["Tags"] is not "not available":
			self["tags"].setText(_("Tags: ") + str(self.videoinfo["Tags"]))

	def setWindowTitle(self):
		self.setTitle(_("MyTubeVideoInfoScreen"))

	def pageUp(self):
		self["detailtext"].pageUp()

	def pageDown(self):
		self["detailtext"].pageDown()

	def loadPreviewpics(self):
		self.thumbnails = []
		self.mythumbubeentries = None
		self.index = 0
		self.maxentries = 0
		self.picloads = {}
		self.mythumbubeentries = self.videoinfo["Thumbnails"]
		self.maxentries = len(self.mythumbubeentries)-1
		if self.mythumbubeentries and len(self.mythumbubeentries):
			currindex = 0
			for entry in self.mythumbubeentries:
				TubeID = self.videoinfo["TubeID"]
				ThumbID = TubeID + str(currindex)
				thumbnailFile = "/tmp/" + ThumbID + ".jpg"
				currPic = [currindex,ThumbID,thumbnailFile,None]
				self.thumbnails.append(currPic)
				thumbnailUrl = None
				thumbnailUrl = entry
				if thumbnailUrl is not None:
					client.downloadPage(thumbnailUrl,thumbnailFile).addCallback(self.fetchFinished,currindex,ThumbID).addErrback(self.fetchFailed,currindex,ThumbID)
				currindex +=1
		else:
			pass

	def fetchFailed(self, string, index, id):
		print "[fetchFailed] for index:" + str(index) + "for ThumbID:" + id + string.getErrorMessage()

	def fetchFinished(self, string, index, id):
		print "[fetchFinished] for index:" + str(index) + " for ThumbID:" + id
		self.decodePic(index)

	def decodePic(self, index):
		sc = AVSwitch().getFramebufferScale()
		self.picloads[index] = ePicLoad()
		self.picloads[index].PictureData.get().append(boundFunction(self.finish_decode, index))
		for entry in self.thumbnails:
			if entry[0] == index:
				self.index = index
				thumbnailFile = entry[2]
				if (os_path.exists(thumbnailFile) == True):
					print "[decodePic] DECODING THUMBNAIL for INDEX:"+  str(self.index) + "and file: " + thumbnailFile
					self.picloads[index].setPara((self["thumbnail"].instance.size().width(), self["thumbnail"].instance.size().height(), sc[0], sc[1], False, 1, "#00000000"))
					self.picloads[index].startDecode(thumbnailFile)
				else:
					print "[decodePic] Thumbnail file NOT FOUND !!!-->:",thumbnailFile

	def finish_decode(self, picindex = None, picInfo=None):
		print "finish_decode - of INDEX", picindex
		ptr = self.picloads[picindex].getData()
		if ptr != None:
			print ptr
			self.thumbnails[picindex][3] = ptr
			if (os_path.exists(self.thumbnails[picindex][2]) == True):
				print "removing", self.thumbnails[picindex][2]
				remove(self.thumbnails[picindex][2])
				del self.picloads[picindex]
				if len(self.picloads) == 0:
					self.timer.startLongTimer(3)

	def picloadTimeout(self):
		self.timer.stop()
		if len(self.picloads) == 0:
				print "all decodes should be now really finished"
				self.buildInfoList()
		else:
			self.timer.startLongTimer(2)

	def buildInfoList(self):
		print "blasel"
		self.infolist = []
		Thumbail0 = None
		Thumbail1 = None
		Thumbail2 = None
		Thumbail3 = None
		if self.thumbnails[0][3] is not None:
			Thumbail0 = self.thumbnails[0][3]
		if self.thumbnails[1][3] is not None:
			Thumbail1 = self.thumbnails[1][3]
		if self.thumbnails[2][3] is not None:
			Thumbail2 = self.thumbnails[2][3]
		if self.thumbnails[3][3] is not None:
			Thumbail3 = self.thumbnails[3][3]
		self.infolist.append(( Thumbail0, Thumbail1, Thumbail2, Thumbail3))
		if len(self.infolist):
			self["infolist"].style = "default"
			self["infolist"].disable_callbacks = True
			self["infolist"].list = self.infolist
			self["infolist"].disable_callbacks = False
			self["infolist"].setIndex(0)
			self["infolist"].setList(self.infolist)
			self["infolist"].updateList(self.infolist)


class MyTubeVideoHelpScreen(Screen):
	skin = """
		<screen name="MyTubeVideoHelpScreen" flags="wfNoBorder" position="0,0" size="720,576" title="MyTubePlayerMainScreen..." >
			<ePixmap position="0,0" zPosition="-1" size="720,576" pixmap="~/mytubemain_bg.png" alphatest="on" transparent="1" backgroundColor="transparent"/>
			<widget name="title" position="60,50" size="600,50" zPosition="5" valign="center" halign="left" font="Regular;21" transparent="1" foregroundColor="white" shadowColor="black" shadowOffset="-1,-1" />
			<widget name="detailtext" position="60,120" size="610,370" zPosition="10" font="Regular;21" transparent="1" halign="left" valign="top"/>
			<ePixmap position="100,500" size="100,40" zPosition="0" pixmap="~/plugin.png" alphatest="on" transparent="1" />
			<ePixmap position="220,500" zPosition="4" size="140,40" pixmap="skin_default/buttons/red.png" transparent="1" alphatest="on" />
			<widget name="key_red" position="220,500" zPosition="5" size="140,40" valign="center" halign="center" font="Regular;21" transparent="1" foregroundColor="white" shadowColor="black" shadowOffset="-1,-1" />
		</screen>"""
		
	def __init__(self, session, plugin_path, wantedinfo = None, wantedtitle = None):
		Screen.__init__(self, session)
		self.session = session
		self.skin_path = plugin_path
		self.wantedinfo = wantedinfo
		self.wantedtitle = wantedtitle
		self["title"] = Label()
		self["key_red"] = Button(_("Close"))
		self["detailtext"] = ScrollLabel()
		
		self["shortcuts"] = ActionMap(["ShortcutActions", "WizardActions"],
		{
			"back": self.close,
			"red": self.close,
			"up": self.pageUp,
			"down":	self.pageDown,
			"left":	self.pageUp,
			"right": self.pageDown,
		}, -2)
		
		self.onLayoutFinish.append(self.layoutFinished)
		self.onShown.append(self.setWindowTitle)

	def layoutFinished(self):
		if self.wantedtitle is None:
			self["title"].setText(_("Help"))
		else:
			self["title"].setText(self.wantedtitle)
		if self.wantedinfo is None:
			self["detailtext"].setText(_("This is the help screen. Feed me with something to display."))
		else:
			self["detailtext"].setText(self.wantedinfo)
	
	def setWindowTitle(self):
		self.setTitle(_("MyTubeVideohelpScreen"))

	def pageUp(self):
		self["detailtext"].pageUp()

	def pageDown(self):
		self["detailtext"].pageDown()


class MyTubePlayer(Screen, InfoBarNotifications):
	STATE_IDLE = 0
	STATE_PLAYING = 1
	STATE_PAUSED = 2
	ENABLE_RESUME_SUPPORT = True
	ALLOW_SUSPEND = True

	skin = """<screen name="MyTubePlayer" flags="wfNoBorder" position="0,380" size="720,160" title="InfoBar" backgroundColor="transparent">
		<ePixmap position="0,0" pixmap="skin_default/info-bg_mp.png" zPosition="-1" size="720,160" />
		<ePixmap position="29,40" pixmap="skin_default/screws_mp.png" size="665,104" alphatest="on" />
		<ePixmap position="48,70" pixmap="skin_default/icons/mp_buttons.png" size="108,13" alphatest="on" />
		<ePixmap pixmap="skin_default/icons/icon_event.png" position="207,78" size="15,10" alphatest="on" />
		<widget source="session.CurrentService" render="Label" position="230,73" size="360,40" font="Regular;20" backgroundColor="#263c59" shadowColor="#1d354c" shadowOffset="-1,-1" transparent="1">
			<convert type="ServiceName">Name</convert>
		</widget>
		<widget source="session.CurrentService" render="Label" position="580,73" size="90,24" font="Regular;20" halign="right" backgroundColor="#4e5a74" transparent="1">
			<convert type="ServicePosition">Length</convert>
		</widget>
		<widget source="session.CurrentService" render="Label" position="205,129" size="100,20" font="Regular;18" halign="center" valign="center" backgroundColor="#06224f" shadowColor="#1d354c" shadowOffset="-1,-1" transparent="1">
			<convert type="ServicePosition">Position</convert>
		</widget>
		<widget source="session.CurrentService" render="PositionGauge" position="300,133" size="270,10" zPosition="2" pointer="skin_default/position_pointer.png:540,0" transparent="1" foregroundColor="#20224f">
			<convert type="ServicePosition">Gauge</convert>
		</widget>
		<widget source="session.CurrentService" render="Label" position="576,129" size="100,20" font="Regular;18" halign="center" valign="center" backgroundColor="#06224f" shadowColor="#1d354c" shadowOffset="-1,-1" transparent="1">
			<convert type="ServicePosition">Remaining</convert>
		</widget>
		</screen>"""

	def __init__(self, session, service, lastservice, infoCallback = None, nextCallback = None, prevCallback = None):
		Screen.__init__(self, session)
		InfoBarNotifications.__init__(self)
		self.session = session
		self.service = service
		self.infoCallback = infoCallback
		self.nextCallback = nextCallback
		self.prevCallback = prevCallback
		self.screen_timeout = 5000
		self.nextservice = None

		print "evEOF=%d" % iPlayableService.evEOF
		self.__event_tracker = ServiceEventTracker(screen = self, eventmap =
			{
				iPlayableService.evSeekableStatusChanged: self.__seekableStatusChanged,
				iPlayableService.evStart: self.__serviceStarted,
				iPlayableService.evEOF: self.__evEOF,
			})
		
		self["actions"] = ActionMap(["OkCancelActions", "InfobarSeekActions", "MediaPlayerActions", "MovieSelectionActions"],
		{
				"ok": self.ok,
				"cancel": self.leavePlayer,
				"stop": self.leavePlayer,
				"playpauseService": self.playpauseService,
				"seekFwd": self.playNextFile,
				"seekBack": self.playPrevFile,
				"showEventInfo": self.showVideoInfo,
			}, -2)


		self.lastservice = lastservice

		self.hidetimer = eTimer()
		self.hidetimer.timeout.get().append(self.ok)
		self.returning = False

		self.state = self.STATE_PLAYING
		self.lastseekstate = self.STATE_PLAYING

		self.onPlayStateChanged = [ ]
		self.__seekableStatusChanged()
	
		self.play()
		self.onClose.append(self.__onClose)
		
	def __onClose(self):
		self.session.nav.stopService()

	def __evEOF(self):
		print "evEOF=%d" % iPlayableService.evEOF
		print "Event EOF"
		if QBOXHD:
			self.close()
		else:
			self.handleLeave(config.plugins.mytube.general.on_movie_stop.value)

	def __setHideTimer(self):
		self.hidetimer.start(self.screen_timeout)

	def showInfobar(self):
		self.show()
		if self.state == self.STATE_PLAYING:
			self.__setHideTimer()
		else:
			pass

	def hideInfobar(self):
		self.hide()
		self.hidetimer.stop()

	def ok(self):
		if self.shown:
			self.hideInfobar()
		else:
			self.showInfobar()

	def showVideoInfo(self):
		if self.shown:
			self.hideInfobar()
		if self.infoCallback is not None:	
			self.infoCallback()


	def playNextFile(self):
		print "playNextFile"
		nextservice,error = self.nextCallback()
		print "nextservice--->",nextservice
		if nextservice is None:
			self.handleLeave(config.plugins.mytube.general.on_movie_stop.value, error)
		else:
			self.playService(nextservice)
			self.showInfobar()

	def playPrevFile(self):
		print "playPrevFile"
		prevservice,error = self.prevCallback()
		if prevservice is None:
			self.handleLeave(config.plugins.mytube.general.on_movie_stop.value, error)
		else:
			self.playService(prevservice)
			self.showInfobar()

	def playagain(self):
		print "playagain"
		if self.state != self.STATE_IDLE:
			self.stopCurrent()
		self.play()
	
	def playService(self, newservice):
		if self.state != self.STATE_IDLE:
			self.stopCurrent()
		self.service = newservice
		self.play()

	def play(self):
		if self.state == self.STATE_PAUSED:
			if self.shown:
				self.__setHideTimer()	
		self.state = self.STATE_PLAYING
		print "self.state--->",self.state
		self.session.nav.playService(self.service)
		if self.shown:
			self.__setHideTimer()

	def stopCurrent(self):
		print "stopCurrent"
		self.session.nav.stopService()
		#if self.state == self.STATE_IDLE:
		#	return
		self.state = self.STATE_IDLE
		print "self.state--->",self.state
		

	def playpauseService(self):
		print "playpauseService"
		#print "self.state--->",self.state
		if self.state == self.STATE_PLAYING:
			self.pauseService()
		elif self.state == self.STATE_PAUSED:
			self.unPauseService()

	def pauseService(self):
		print "pauseService"
		if self.state == self.STATE_PLAYING:
			#print "self.state--->",self.state
			print "calling setseekstate pause"
			self.setSeekState(self.STATE_PAUSED)
		
	def unPauseService(self):
		print "unPauseService"
		if self.state == self.STATE_PAUSED:
			#print "self.state--->",self.state
			print "calling setseekstate playing"
			self.setSeekState(self.STATE_PLAYING)


	def getSeek(self):
		service = self.session.nav.getCurrentService()
		if service is None:
			return None

		seek = service.seek()

		if seek is None or not seek.isCurrentlySeekable():
			return None

		return seek

	def isSeekable(self):
		if self.getSeek() is None:
			return False
		return True

	def __seekableStatusChanged(self):
		print "seekable status changed!"
		if not self.isSeekable():
			#self["SeekActions"].setEnabled(False)
			print "not seekable, return to play"
			self.setSeekState(self.STATE_PLAYING)
		else:
#			self["SeekActions"].setEnabled(True)
			print "seekable"

	def __serviceStarted(self):
		self.state = self.STATE_PLAYING
		self.__seekableStatusChanged()
		
	def __evServerError(self):
		print "############################# Error from server"

	def setSeekState(self, wantstate):
		print "setSeekState"
		#print "current state--->",self.state
		#print " wanted state--->",wantstate
		if wantstate == self.STATE_PAUSED:
			print "trying to switch to Pause- state:",self.STATE_PAUSED
		elif wantstate == self.STATE_PLAYING:
			print "trying to switch to playing- state:",self.STATE_PLAYING
		service = self.session.nav.getCurrentService()
		if service is None:
			print "No Service found"
			return False
		pauseable = service.pause()
		if pauseable is None:
			print "not pauseable."
			self.state = self.STATE_PLAYING

		if pauseable is not None:
			print "service is pausable"
			if wantstate == self.STATE_PAUSED:
				print "WANT TO PAUSE"
				print "current state --->",self.state
				print "wanted state  --->",wantstate
				pauseable.pause()
				self.state = self.STATE_PAUSED
				if not self.shown:
					self.hidetimer.stop()
					self.show()
			elif wantstate == self.STATE_PLAYING:
				print "WANT TO PLAY"
				print "current state --->",self.state
				print "wanted state  --->",wantstate
				pauseable.unpause()
				self.state = self.STATE_PLAYING
				if self.shown:
					self.__setHideTimer()

		for c in self.onPlayStateChanged:
			c(self.state)
		
		return True

	def handleLeave(self, how, error = False):
		self.is_closing = True
		if how == "ask":
			list = (
				(_("Yes"), "quit"),
				(_("No, but play video again"), "playagain"),
				(_("Yes, but play next video"), "playnext"),
				(_("Yes, but play previous video"), "playprev"),
			)
			if error is False:
				self.session.openWithCallback(self.leavePlayerConfirmed, ChoiceBox, title=_("Stop playing this movie?"), list = list)
			else:
				self.session.openWithCallback(self.leavePlayerConfirmed, ChoiceBox, title=_("No playable video found! Stop playing this movie?"), list = list)
		else:
			self.leavePlayerConfirmed([True, how])

	def leavePlayer(self):
		self.handleLeave(config.plugins.mytube.general.on_movie_stop.value)

	def leavePlayerConfirmed(self, answer):
		answer = answer and answer[1]
		print "ANSWER im player leave",answer
		if answer == "quit":
			self.close()
		elif answer == "playnext":
			self.playNextFile()
		elif answer == "playprev":
			self.playPrevFile()
		elif answer == "playagain":
			self.playagain()
			
	def doEofInternal(self, playing):
		if not self.execing:
			return
		if not playing :
			return
		self.handleLeave(config.usage.on_movie_eof.value)


def MyTubeMain(session, **kwargs):
	Console().ePopen(("rm -rf /tmp/*.jpg"))
	session.open(MyTubePlayerMainScreen)


def Plugins(path, **kwargs):
	global plugin_path
	plugin_path = path
	return PluginDescriptor(
		name=_("My TubePlayer"),
		description=_("Play YouTube movies"),
		where = [ PluginDescriptor.WHERE_EXTENSIONSMENU, PluginDescriptor.WHERE_PLUGINMENU ],
		icon = "plugin.png", fnc = MyTubeMain)
