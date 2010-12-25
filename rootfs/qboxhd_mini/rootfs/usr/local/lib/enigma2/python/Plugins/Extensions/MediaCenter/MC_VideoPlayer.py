from qboxhd import QBOXHD, QBOXHD_MINI
from enigma import iPlayableService, eTimer, eWidget, eRect, eServiceReference, iServiceInformation
from Screens.Screen import Screen
from Screens.ServiceInfo import ServiceInfoList, ServiceInfoListEntry
from Screens.HelpMenu import HelpableScreen
from Screens.MessageBox import MessageBox
from Components.ActionMap import ActionMap, NumberActionMap, HelpableActionMap
from Components.Pixmap import Pixmap, MovingPixmap
from Components.Label import Label
from Components.Button import Button
from Components.ServiceEventTracker import ServiceEventTracker
from Components.ConfigList import ConfigList, ConfigListScreen
from Components.config import *

from Tools.Directories import resolveFilename, fileExists, pathExists, createDir, SCOPE_MEDIA
from Components.FileList import FileList
from Components.AVSwitch import AVSwitch
from Screens.InfoBar import MoviePlayer
from Plugins.Plugin import PluginDescriptor

from GlobalFunctions import MC_FolderOptions, MC_FavoriteFolders, MC_FavoriteFolderAdd, MC_FavoriteFolderEdit, MC_VideoInfoView

import os
from os import path as os_path

if QBOXHD:
	from MC_MoviePlayer import MC_MoviePlayer

config.plugins.mc_vp = ConfigSubsection()
config.plugins.mc_vp.showPreview = ConfigYesNo(default=True)
config.plugins.mc_vp.preview_delay = ConfigInteger(default=3, limits=(1, 99))
config.plugins.mc_vp.lastDir = ConfigText(default=resolveFilename(SCOPE_MEDIA))

def getAspect():
	val = AVSwitch().getAspectRatioSetting()
	return val/2

#------------------------------------------------------------------------------------------

class MC_VideoPlayer(Screen, HelpableScreen):
	def __init__(self, session):
		Screen.__init__(self, session)
		HelpableScreen.__init__(self)

		self.isVisible = True
		self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
		self.session.nav.stopService()

		# Show Background MVI
		if not QBOXHD:
			os.system("/usr/bin/showiframe /usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/background.mvi &")

		self["key_red"] = Button(_("Favorites"))
		self["key_green"] = Button(_("Preview"))
		self["key_yellow"] = Button("")
		self["key_blue"] = Button(_("Settings"))

		self["currentfolder"] = Label("")
		self["currentfavname"] = Label("")
		self.curfavfolder = -1

		self["actions"] = HelpableActionMap(self, "MC_VideoPlayerActions",
			{
				"ok": (self.KeyOk, "Play selected file"),
				"cancel": (self.Exit, "Exit Video Player"),
				"left": (self.leftUp, "List Top"),
				"right": (self.rightDown, "List Bottom"),
				"up": (self.up, "List up"),
				"down": (self.down, "List down"),
				"menu": (self.KeyMenu, "File / Folder Options"),
				"video": (self.visibility, "Show / Hide Player"),
				"info": (self.showFileInfo, "Show File Info"),
				"nextBouquet": (self.NextFavFolder, "Next Favorite Folder"),
				"prevBouquet": (self.PrevFavFolder, "Previous Favorite Folder"),
				"stop": (self.StopPlayback, "Stop Playback"),
				"red": (self.FavoriteFolders, "Favorite Folders"),
				"green": (self.showPreview, "Preview"),
				"blue": (self.KeySettings, "Settings"),
			}, -2)

		self.aspect = getAspect()
		currDir = config.plugins.mc_vp.lastDir.value
		if not pathExists(currDir):
			currDir = "/"

		self["currentfolder"].setText(str(currDir))

		if not QBOXHD:
			self.filelist = FileList(currDir, useServiceRef = True, showDirectories = True, showFiles = True, matchingPattern = "(?i)^.*\.(ts|vob|mpg|mpeg|avi|mkv|dat|iso|mp4|divx)")
		else:
			self.filelist = FileList(currDir, useServiceRef = True, showDirectories = True, showFiles = True, matchingPattern = "(?i)^.*\.(ts|mpg|mpeg|avi|mkv|mp4|divx)")
		self["filelist"] = self.filelist
		self["thumbnail"] = Pixmap()

		self.ThumbTimer = eTimer()
		self.ThumbTimer.callback.append(self.showThumb)
		#self.ThumbTimer.start(500, True)

		self.__event_tracker = ServiceEventTracker(screen=self, eventmap=
			{
				iPlayableService.evUser+11: self.__evDecodeError,
				iPlayableService.evUser+12: self.__evPluginError
			})

	def up(self):
		self["filelist"].up()
		self.ThumbTimer.start(config.plugins.mc_vp.preview_delay.getValue() * 1000, True)

	def down(self):
		self["filelist"].down()
		self.ThumbTimer.start(config.plugins.mc_vp.preview_delay.getValue() * 1000, True)

	def leftUp(self):
		self["filelist"].pageUp()
		self.ThumbTimer.start(config.plugins.mc_vp.preview_delay.getValue() * 1000, True)

	def rightDown(self):
		self["filelist"].pageDown()
		self.ThumbTimer.start(config.plugins.mc_vp.preview_delay.getValue() * 1000, True)

	def NextFavFolder(self):
		if self.curfavfolder + 1 < config.plugins.mc_favorites.foldercount.value:
			self.curfavfolder += 1
			self.favname = config.plugins.mc_favorites.folders[self.curfavfolder].name.value
			self.folder = config.plugins.mc_favorites.folders[self.curfavfolder].basedir.value
			self["currentfolder"].setText(("%s") % (self.folder))
			self["currentfavname"].setText(("%s") % (self.favname))
			if os.path.exists(self.folder) == True:
				self["filelist"].changeDir(self.folder)
		else:
			return

	def PrevFavFolder(self):
		if self.curfavfolder <= 0:
			return
		else:
			self.curfavfolder -= 1
			self.favname = config.plugins.mc_favorites.folders[self.curfavfolder].name.value
			self.folder = config.plugins.mc_favorites.folders[self.curfavfolder].basedir.value
			self["currentfolder"].setText(("%s") % (self.folder))
			self["currentfavname"].setText(("%s") % (self.favname))
			if os.path.exists(self.folder) == True:
				self["filelist"].changeDir(self.folder)

	def showPreview(self):
		if self["filelist"].canDescent():
			return
		else:
			if self["filelist"].getServiceRef() is not None:
				self.session.nav.stopService()
				print "showPreview <----------------------"
				res = self.session.nav.playService(self["filelist"].getServiceRef())
				if res:
					#self.__evPluginError()
					self.session.open(MessageBox, ("libeplayer2 Error: Avi file too large"), type = MessageBox.TYPE_INFO,timeout = 10 )

	def showThumb(self):
		if config.plugins.mc_vp.showPreview.getValue() == False:
			return

		if self["filelist"].canDescent():
			return
		else:
			if self["filelist"].getServiceRef() is not None:
				#self.ShowFileInfoTimer.start(3000, True)
				self.session.nav.stopService()
				print "showThumb <---------------------------------"
				#self.session.nav.playService(self["filelist"].getServiceRef())

				res = self.session.nav.playService(self["filelist"].getServiceRef())
				print "showThumb: res=%d <---------------------------------" % res
				if res:
					#self.__evPluginError()
					self.session.open(MessageBox, ("libeplayer2 Error: Avi file too large"), type = MessageBox.TYPE_INFO,timeout = 10 )


	def showFileInfo(self):
		if self["filelist"].canDescent():
			return
		else:
			self.session.open(MC_VideoInfoView, self["filelist"].getCurrentDirectory() + self["filelist"].getFilename() , self["filelist"].getFilename(), self["filelist"].getServiceRef())


	def isFilePlayable(self, file):
		#check if avi
		res = str.rfind(str(file), ".avi")
		if res!=-1:
			#check MAX size
			print "isFilePlayable file=%s" % (str(file))
			statInfo = os.stat(str(file))
			if(statInfo.st_size > 2048000000): #MAX size = 2GB
				return False
		return True

	def KeyOk(self):
		if self.isVisible == False:
			self.visibility()
			return

		self.ThumbTimer.stop()

		if self.filelist.canDescent():
			self.filelist.descent()
		else:
			if QBOXHD:
				#check if file is playable
				if self.isFilePlayable(self["filelist"].getFilename()):
					self.session.open(MC_MoviePlayer, self["filelist"].getServiceRef())
				else:
					#self.__evPluginError()
					self.session.open(MessageBox, ("libeplayer2 Error: Avi file too large"), type = MessageBox.TYPE_INFO,timeout = 10 )
			else:
				self.session.open(MoviePlayer, self["filelist"].getServiceRef())


	def KeyMenu(self):
		self.ThumbTimer.stop()
		if self["filelist"].canDescent():
			if self.filelist.getCurrent()[0][1]:
				self.currentDirectory = self.filelist.getCurrent()[0][0]
				if self.currentDirectory is not None:
					foldername = self.currentDirectory.split('/')
					foldername = foldername[-2]
					self.session.open(MC_FolderOptions,self.currentDirectory, foldername)


	def StartThumb(self):
		self.session.openWithCallback(self.returnVal, ThumbView, self.filelist.getFileList(), self.filelist.getFilename(), self.filelist.getCurrentDirectory())

	def returnVal(self, val=0):
		if val > 0:
			for x in self.filelist.getFileList():
				if x[0][1] == True:
					val += 1
			self.filelist.moveToIndex(val)

	def StartExif(self):
		if not self.filelist.canDescent():
			self.session.open(ExifView, self.filelist.getCurrentDirectory() + self.filelist.getFilename(), self.filelist.getFilename())

	def visibility(self, force=1):
		if self.isVisible == True:
			self.isVisible = False
			self.hide()
		else:
			self.isVisible = True
			self.show()
			#self["list"].refresh()

	def StopPlayback(self):
		self.ThumbTimer.stop()
		self.session.nav.stopService()
		print "StopPlayback <---------------------------------"
		self.session.nav.playService(self.oldService)

		if self.isVisible == False:
			self.show()
			self.isVisible = True

	def JumpToFolder(self, jumpto = None):
		if jumpto is None:
			return
		else:
			self["filelist"].changeDir(jumpto)
			self["currentfolder"].setText(("%s") % (jumpto))

	def FavoriteFolders(self):
		self.session.openWithCallback(self.JumpToFolder, MC_FavoriteFolders)

	def KeySettings(self):
		self.session.open(VideoPlayerSettings)

	def KeyScreenAdjust(self):
		if self.isVisible == True:
			return

		os.system("echo " + hex(config.plugins.mc_globalsettings.dst_top.value)[2:] + " > /proc/stb/vmpeg/0/dst_top")
		os.system("echo " + hex(config.plugins.mc_globalsettings.dst_left.value)[2:] + " > /proc/stb/vmpeg/0/dst_left")
		os.system("echo " + hex(config.plugins.mc_globalsettings.dst_width.value)[2:] + " > /proc/stb/vmpeg/0/dst_width")
		os.system("echo " + hex(config.plugins.mc_globalsettings.dst_height.value)[2:] + " > /proc/stb/vmpeg/0/dst_height")

	def __evDecodeError(self):
		currPlay = self.session.nav.getCurrentService()
		sVideoType = currPlay.info().getInfoString(iServiceInformation.sVideoType)
		print "[__evDecodeError] video-codec %s can't be decoded by hardware" % (sVideoType)
		#self.session.open(MessageBox, _("This Dreambox can't decode %s video streams!") % sVideoType, type = MessageBox.TYPE_INFO,timeout = 10 )
		self.session.open(MessageBox, _("This QBoxHD can't decode %s video streams!") % sVideoType, type = MessageBox.TYPE_INFO,timeout = 10 )

	def __evPluginError(self):
		currPlay = self.session.nav.getCurrentService()
		message = currPlay.info().getInfoString(iServiceInformation.sUser+12)
		print "[__evPluginError]" , message
		#self.session.open(MessageBox, ("GStreamer Error: missing %s") % message, type = MessageBox.TYPE_INFO,timeout = 20 )
		self.session.open(MessageBox, _("libeplayer2 Error: missing %s") % message, type = MessageBox.TYPE_INFO,timeout = 20 )

	def Exit(self):
		if self.isVisible == False:
			self.visibility()
			return

		if self.filelist.getCurrentDirectory() is None:
			config.plugins.mc_vp.lastDir.value = "/"
		else:
			config.plugins.mc_vp.lastDir.value = self.filelist.getCurrentDirectory()

		self.session.nav.stopService()
		#self.session.nav.playService(self.oldService)

		config.plugins.mc_vp.save()
		self.close()



#------------------------------------------------------------------------------------------

class VideoPlayerSettings(Screen, ConfigListScreen):
	skin = """
		<screen position="160,220" size="400,120" title="Media Center - VideoPlayer Settings" >
			<widget name="config" position="10,10" size="380,100" />
		</screen>"""

	def __init__(self, session):
		Screen.__init__(self, session)

		self["actions"] = NumberActionMap(["SetupActions","OkCancelActions"],
		{
			"ok": self.keyOK,
			"cancel": self.keyOK
		}, -1)

		self.list = []
		self.list.append(getConfigListEntry(_("Autoplay Enable"), config.plugins.mc_vp.showPreview))
		self.list.append(getConfigListEntry(_("Autoplay Delay"), config.plugins.mc_vp.preview_delay))

		ConfigListScreen.__init__(self, self.list, session)

	def keyOK(self):
		config.plugins.mc_vp.save()
		self.close()

