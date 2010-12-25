from qboxhd import QBOXHD, QBOXHD_MINI

import os
from enigma import eTimer
from Components.ActionMap import ActionMap
from Components.Sources.List import List
from Components.Sources.StaticText import StaticText
from Components.ConfigList import ConfigList
from Components.config import *
from Screens.Screen import Screen
from Screens.MessageBox import MessageBox
from Plugins.Plugin import PluginDescriptor

try:
	from twisted.web.client import getPage
except Exception, e:
	print "Media Center: Import twisted.web.client failed"

# MC Plugins
from MC_AudioPlayer import MC_AudioPlayer
from MC_VideoPlayer import MC_VideoPlayer
from MC_RadioPlayer import MC_RadioPlayer
from MC_VLCPlayer import MC_VLCServerlist
from MC_PictureViewer import MC_PictureViewer
from MC_WeatherInfo import MC_WeatherInfo
from MC_RSSReader import MC_RSSReader
from MC_Settings import MC_Settings, MCS_Update
if QBOXHD:
	from MC_iDevsPlayer import MC_iDevsPlayer

#------------------------------------------------------------------------------------------
	
class DMC_MainMenu(Screen):
	def __init__(self, session):
		Screen.__init__(self, session)

		self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
		self.session.nav.stopService()

		# Disable OSD Transparency
		try:
			self.can_osd_alpha = open("/proc/stb/video/alpha", "r") and True or False
		except:
			self.can_osd_alpha = False

		if self.can_osd_alpha:
			open("/proc/stb/video/alpha", "w").write(str("255"))

		# Show Background MVI
		if not QBOXHD:
			os.system("/usr/bin/showiframe /usr/local/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/background.mvi &")
		
		list = []
		#list.append(("Titel", "nothing", "entryID", "weight"))
		list.append(("My Music", "MC_AudioPlayer", "menu_music", "50"))
		if QBOXHD:
			list.append(("My iDevs", "MC_iDevsPlayer", "menu_myidevs", "50"))
		list.append(("My Videos", "MC_VideoPlayer", "menu_video", "50"))
		list.append(("My Pictures", "MC_PictureViewer", "menu_pictures", "50"))
		if QBOXHD:
			list.append(("My Tube", "MC_MyTube", "menu_mytube", "50"))
			list.append(("Internet", "MC_WebBrowser", "menu_webbrowser", "50"))
		if not QBOXHD:
			list.append(("DVD Player", "MC_DVDPlayer", "menu_dvd", "50"))
			list.append(("VLC Player", "MC_VLCPlayer", "menu_vlc", "50"))
		if not QBOXHD:
			list.append(("Radio Player", "MC_RadioPlayer", "menu_radio", "50"))
			list.append(("RSS-Reader", "MC_RSSReader", "menu_rss", "50"))
		list.append(("Weather Info", "MC_WeatherInfo", "menu_weather", "50"))
		list.append(("Settings", "MC_Settings", "menu_settings", "50"))
		#list.append(("Exit", "Exit", "menu_exit", "50"))
		self["menu"] = List(list)
		self["title"] = StaticText("")
		self["welcomemessage"] = StaticText("")
		
		self["actions"] = ActionMap(["OkCancelActions"],
		{
			"cancel": self.Exit,
			"ok": self.okbuttonClick
		}, -1)

		if not QBOXHD:
			if config.plugins.mc_globalsettings.checkforupdate.value == True:
				self.onFirstExecBegin.append(self.CheckForMCUpdate)
				
		if QBOXHD:
			self.onClose.append(self.__close)
		
	def __close(self):
		os.system("killall -9 showiframe")

	def CheckForMCUpdate(self):
		#Get Weather Info from my webserver
		self.url = "http://www.homeys-bunker.de/dm800/projects/MediaCenter/currentversion.txt"
		try:
			getPage(self.url).addCallback(self.GotMCUpdateInfo).addErrback(self.error)
			self["welcomemessage"].setText(_("Checking for updates ..."))
		except Exception, e:
			print "MediaCenter: Could not download HTTP Page"

	def GotMCUpdateInfo(self, html):
		tmp_infolines = html.splitlines()
		
		remoteversion = tmp_infolines[0]

		if config.plugins.mc_globalsettings.currentversion.value < remoteversion:
			self["welcomemessage"].setText("A new version of MediaCenter is available :-)")
			self.session.openWithCallback(self.startmcupdate,MessageBox,_("A new version of MediaCenter is available for download!\nDo you want to download and install now?"), MessageBox.TYPE_YESNO)
		else:
			self["welcomemessage"].setText("")

	def startmcupdate(self, answer):
		if answer is True:
			self.session.open(MCS_Update)
			
	def okbuttonClick(self):
		print "okbuttonClick"
		selection = self["menu"].getCurrent()
		if selection is not None:
			if selection[1] == "MC_VideoPlayer":
				self.session.open(MC_VideoPlayer)
			elif selection[1] == "MC_PictureViewer":
				self.session.open(MC_PictureViewer)
			elif selection[1] == "MC_AudioPlayer":
				self.session.open(MC_AudioPlayer)
			elif QBOXHD and selection[1] == "MC_iDevsPlayer":
				self.session.open(MC_iDevsPlayer)
			elif selection[1] == "MC_RadioPlayer":
				self.session.open(MC_RadioPlayer)
			elif QBOXHD and selection[1] == "MC_MyTube":
				if os.path.exists("/usr/lib/enigma2/python/Plugins/Extensions/MyTube/") == True:
					from Plugins.Extensions.MyTube.plugin import MyTubePlayerMainScreen
					self.session.open(MyTubePlayerMainScreen)
				else:
					self.session.open(MessageBox,"Error: MyTube Plugin not installed ...",  MessageBox.TYPE_INFO)
			elif QBOXHD and selection[1] == "MC_WebBrowser":
				if os.path.exists("/usr/lib/enigma2/python/Plugins/Extensions/WebBrowser/") == True:
					from Plugins.Extensions.WebBrowser.plugin import WebBrowserShell
					self.session.open(WebBrowserShell)
				else:
					self.session.open(MessageBox,"Error: WebBrowser Plugin not installed ...",  MessageBox.TYPE_INFO)
					
			elif selection[1] == "MC_DVDPlayer" and not QBOXHD:
				if os.path.exists("/usr/lib/enigma2/python/Plugins/Extensions/DVDPlayer/") == True:
					from Plugins.Extensions.DVDPlayer.plugin import DVDPlayer
					self.session.open(DVDPlayer)
				else:
					self.session.open(MessageBox,"Error: DVD-Player Plugin not installed ...",  MessageBox.TYPE_INFO)
			elif selection[1] == "MC_VLCPlayer" and not QBOXHD:
				if os.path.exists("/usr/lib/enigma2/python/Plugins/Extensions/VlcPlayer/") == True:
					self.session.open(MC_VLCServerlist)
				else:
					self.session.open(MessageBox,"Error: VLC-Player Plugin not installed ...",  MessageBox.TYPE_INFO)
			elif selection[1] == "MC_WeatherInfo":
				self.session.open(MC_WeatherInfo)
			elif selection[1] == "MC_RSSReader" and not QBOXHD:
				self.session.open(MC_RSSReader)
			elif selection[1] == "MC_Settings":
				self.session.open(MC_Settings)
			elif selection[1] == "Exit":
				self.Exit()
			else:
				self.session.open(MessageBox,("Error: Could not find plugin %s\ncoming soon ... :)") % (selection[1]),  MessageBox.TYPE_INFO)

	def error(self, error):
		self.session.open(MessageBox,("UNEXPECTED ERROR:\n%s") % (error),  MessageBox.TYPE_INFO)

	def Ok(self):
		self.session.open(MPD_PictureViewer)
		
	def Exit(self):
		# Restart old service
		self.session.nav.stopService()
		self.session.nav.playService(self.oldService)
		
		## Restore OSD Transparency Settings
		os.system("echo " + hex(0)[2:] + " > /proc/stb/vmpeg/0/dst_top")
		os.system("echo " + hex(0)[2:] + " > /proc/stb/vmpeg/0/dst_left")
		os.system("echo " + hex(720)[2:] + " > /proc/stb/vmpeg/0/dst_width")
		os.system("echo " + hex(576)[2:] + " > /proc/stb/vmpeg/0/dst_height")

		if self.can_osd_alpha:
			try:
				open("/proc/stb/video/alpha", "w").write(str(config.av.osd_alpha.value))
			except:
				print "Set OSD Transparacy failed"
		
		#configfile.save()
		self.close()
		
#------------------------------------------------------------------------------------------

def main(session, **kwargs):
	session.open(DMC_MainMenu)

def menu(menuid, **kwargs):
	if menuid == "mainmenu":
		return [(_("Media Center"), main, "dmc_mainmenu", 44)]
	return []

def Plugins(**kwargs):
	
	if QBOXHD:
		return [
			PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", icon="plugin.png", where = PluginDescriptor.WHERE_PLUGINMENU, fnc = main),
			PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", where = PluginDescriptor.WHERE_MENU, fnc = menu)]
	else:
		if config.plugins.mc_globalsettings.showinmainmenu.value == True and config.plugins.mc_globalsettings.showinextmenu.value == True:
			return [
				PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", icon="plugin.png", where = PluginDescriptor.WHERE_PLUGINMENU, fnc = main),
				PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", where = PluginDescriptor.WHERE_MENU, fnc = menu),
				PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", icon="plugin.png", where = PluginDescriptor.WHERE_EXTENSIONSMENU, fnc=main)]	
		elif config.plugins.mc_globalsettings.showinmainmenu.value == True and config.plugins.mc_globalsettings.showinextmenu.value == False:
			return [
				PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", icon="plugin.png", where = PluginDescriptor.WHERE_PLUGINMENU, fnc = main),
				PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", where = PluginDescriptor.WHERE_MENU, fnc = menu)]
		elif config.plugins.mc_globalsettings.showinmainmenu.value == False and config.plugins.mc_globalsettings.showinextmenu.value == True:
			return [
				PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", icon="plugin.png", where = PluginDescriptor.WHERE_PLUGINMENU, fnc = main),
				PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", icon="plugin.png", where = PluginDescriptor.WHERE_EXTENSIONSMENU, fnc=main)]
		else:
			return [
				PluginDescriptor(name = "Media Center", description = "Media Center Plugin for your QBoxHD", icon="plugin.png", where = PluginDescriptor.WHERE_PLUGINMENU, fnc = main)]
