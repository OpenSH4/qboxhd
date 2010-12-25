from qboxhd import QBOXHD, QBOXHD_MINI
from enigma import eTimer, eWidget, eRect, eServiceReference, iServiceInformation, iPlayableService, ePicLoad, getPrevAsciiCode
from Screens.Screen import Screen
from Screens.ServiceInfo import ServiceInfoList, ServiceInfoListEntry
from Components.ActionMap import ActionMap, NumberActionMap, HelpableActionMap
from Components.Pixmap import Pixmap, MovingPixmap
from Components.Label import Label
from Components.Button import Button

from Tools.NumericalTextInput import NumericalTextInput

from Components.Sources.List import List
from Screens.MessageBox import MessageBox
from Screens.HelpMenu import HelpableScreen

from Components.ServicePosition import ServicePositionGauge
from Components.ServiceEventTracker import ServiceEventTracker
from Components.Playlist import PlaylistIOInternal, PlaylistIOM3U, PlaylistIOPLS

from Components.ConfigList import ConfigList, ConfigListScreen
from Components.config import *

from Tools.Directories import resolveFilename, fileExists, pathExists, createDir, SCOPE_MEDIA, SCOPE_PLAYLIST, SCOPE_SKIN_IMAGE
from Components.iDevsDeviceFileList import iDevsDeviceFileList, IDEVS_NO_SPECIFIC_ENTRY, IDEVSWAIT_ENTRY, IDEVSEMPTY_ENTRY, IDEVSDEVICE_ENTRY, IDEVSDEVICE_AUDIO_MENU_ENTRY, IDEVSDEVICE_VIDEO_MENU_ENTRY, IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY, IDEVSDEVICE_AUDIO_ARTIST_ENTRY, IDEVSDEVICE_AUDIO_ALBUM_ENTRY, IDEVSDEVICE_AUDIO_GENRE_ENTRY, IDEVSDEVICE_AUDIO_TRACK_ENTRY, IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY, IDEVSDEVICE_VIDEO_TRACK_ENTRY, OP_SELECT_TRACK_ENTRY, OP_DESELECT_TRACK_ENTRY, OP_SELECT_ALL_TRACK_ENTRY, OP_DESELECT_ALL_TRACK_ENTRY
from Components.AVSwitch import AVSwitch
#from Screens.InfoBar import MoviePlayer
from MC_MoviePlayer import MC_MoviePlayer

from Plugins.Plugin import PluginDescriptor

from GlobalFunctions import MC_FolderOptions, MC_FavoriteFolders, MC_FavoriteFolderAdd, MC_FavoriteFolderEdit, MC_AudioInfoView

import os
from os import path as os_path

from Components.iDevsDevice import idevsdevicemanager


PLAY_SINGLE 	= 0
PLAY_ALL 		= 1
PLAY_SELECTED 	= 2


def getAspect():
	val = AVSwitch().getAspectRatioSetting()
	return val/2

#------------------------------------------------------------------------------------------

class MC_iDevsPlayer(Screen, HelpableScreen):
	def __init__(self, session):
		Screen.__init__(self, session)
		HelpableScreen.__init__(self)
		
		self.isVisible = True
		self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
		#self.session.nav.stopService()

		self.coverArtFileName = ""
		
		self["PositionGauge"] = ServicePositionGauge(self.session.nav)
		
		self["key_red"] = Button(_("Exit"))
		self["key_green"] = Button("")
		self["key_yellow"] = Button("")
		self["key_blue"] = Button("")
		
		self["fileinfo"] = Label()
#		self["coverArt"] = MediaPixmap()
		
		self["play"] = Pixmap()
		self["stop"] = Pixmap()
		
		self["curmodality"] = Label(_("Audio :"))

		self["curplayingtitle"] = Label()
		self.currPlaying = 0
		self.PlayMode = PLAY_SINGLE
		
		self.currentState = None

		self.__event_tracker = ServiceEventTracker(screen=self, eventmap=
			{
				iPlayableService.evEOF: self.doEOF,
				iPlayableService.evStopped: self.StopPlayback,
				iPlayableService.evUser+11: self.__evDecodeError,
				iPlayableService.evUser+12: self.__evPluginError,
#				iPlayableService.evUser+13: self["coverArt"].embeddedCoverArt
			})
			
		self["actions"] = NumberActionMap(["MC_iDevsPlayerActions", "NumberActions", "InputAsciiActions"],
			{
				"ok": self.KeyEnter,
				"cancel": self.Exit,
				"left": self.leftUp,
				"right": self.rightDown,
				"up": self.up,
				"down": self.down,
				"audio":self.switchToAudio,
				"video":self.switchToVideo,
				"stop": self.StopPlayback,
				"red" : self.redButtonHandler,
				"green": self.greenButtonHandler,
				"yellow": self.yellowButtonHandler,
				"blue": self.blueButtonHandler,
				"nextSong": self.nextSong,
				"prevSong": self.prevSong,
				"gotAsciiCode": self.keyAsciiCode,
				"clearSelection":self.clearSelection,
				"1": self.keyNumberGlobal,
				"2": self.keyNumberGlobal,
				"3": self.keyNumberGlobal,
				"4": self.keyNumberGlobal,
				"5": self.keyNumberGlobal,
				"6": self.keyNumberGlobal,
				"7": self.keyNumberGlobal,
				"8": self.keyNumberGlobal,
				"9": self.keyNumberGlobal,
				"0": self.keyNumberGlobal
			}, -2)

		self.aspect = getAspect()
		
		self.idevsdevicefilelist = iDevsDeviceFileList(entryEventCB = self.changeEntryEvent)
		self["idevsdevicefilelist"] = self.idevsdevicefilelist
		
		self["thumbnail"] = Pixmap()
		
		print "Number of iDevsDevice %d" % idevsdevicemanager.getiDevsDeviceCount()
		
		self.ThumbTimer = eTimer()
		self.ThumbTimer.callback.append(self.showThumb)
		
		self.BlinkingPlayIconTimer = eTimer()
		self.BlinkingPlayIconTimer.callback.append(self.BlinkingPlayIcon)
		self.blinking=False

		self.StepPlaySong = eTimer()
		self.StepPlaySong.callback.append(self.stepPlaySong)

		self.FileInfoTimer = eTimer()
		self.FileInfoTimer.callback.append(self.updateFileInfo)
		
		self.numericalTextInput = NumericalTextInput()
		self.numericalTextInput.setUseableChars(u'1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ')
		
		self.onClose.append(self.__onClose)
		
		
	def __onClose(self):
		self["idevsdevicefilelist"].close()
		
		
	def clearSelection(self):
		self["idevsdevicefilelist"].clearSelectedTrack()
		
		
	def redButtonHandler(self):
		if self.currentState is None:
			return
		
		if self.currentState == IDEVSWAIT_ENTRY:
			return
		else:
			self.Terminate()
	
	
	def greenButtonHandler(self):
		if self.currentState is None:
			return
		
		if self.currentState == IDEVSWAIT_ENTRY or self.currentState == IDEVSEMPTY_ENTRY:
			return
		else:
			if self.currentState == IDEVSDEVICE_VIDEO_TRACK_ENTRY:
				self.KeyPreview()
			else:
				self.KeyPlayAll()
	
	
	def yellowButtonHandler(self):
		if self.currentState is None:
			return
		
		if self.currentState == IDEVSWAIT_ENTRY:
			return
		
		if self.currentState == IDEVSDEVICE_AUDIO_TRACK_ENTRY or self.currentState == IDEVSDEVICE_VIDEO_TRACK_ENTRY:
			if self["idevsdevicefilelist"].isTrack():
				if self["idevsdevicefilelist"].isTrackSelected():
					self["idevsdevicefilelist"].deselectTrack()
				else:
					self["idevsdevicefilelist"].selectTrack()
		else:
			return
	
	
	def blueButtonHandler(self):
		if self.currentState is None:
			return
		
		if self["idevsdevicefilelist"].getSelectedTrackNumber() > 0:
			self.KeyPlaySelected()
		
		
		
	def changeEntryEvent(self, typeEntry, typeOperation):
		if typeEntry == IDEVSWAIT_ENTRY:
			self.currentState = typeEntry
			self["key_red"].hide()
			self["key_green"].hide()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSEMPTY_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show()
			self["key_green"].hide()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSDEVICE_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() # Exit
			self["key_green"].setText(_("Enter"))
			self["key_green"].show()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSDEVICE_AUDIO_MENU_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() #Exit
			self["key_green"].setText(_("Enter"))
			self["key_green"].show()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSDEVICE_VIDEO_MENU_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() #Exit
			self["key_green"].setText(_("Enter"))
			self["key_green"].show()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() #Exit
			self["key_green"].setText(_("Enter"))
			self["key_green"].show()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSDEVICE_AUDIO_ARTIST_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() #Exit
			self["key_green"].setText(_("Enter"))
			self["key_green"].show()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSDEVICE_AUDIO_ALBUM_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() #Exit
			self["key_green"].setText(_("Enter"))
			self["key_green"].show()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSDEVICE_AUDIO_GENRE_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() #Exit
			self["key_green"].setText(_("Enter"))
			self["key_green"].show()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() #Exit
			self["key_green"].setText(_("Enter"))
			self["key_green"].show()
			self["key_yellow"].hide()
			self["key_blue"].hide()
		elif typeEntry == IDEVSDEVICE_AUDIO_TRACK_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() #Exit
			self["key_green"].setText(_("Play All"))
			self["key_green"].show()
			self["key_yellow"].setText(_("Select"))
			self["key_yellow"].show()
		elif typeEntry == IDEVSDEVICE_VIDEO_TRACK_ENTRY:
			self.currentState = typeEntry
			self["key_red"].show() #Exit
			self["key_green"].setText(_("Preview"))
			self["key_green"].show()
			self["key_yellow"].setText(_("Select"))
			self["key_yellow"].show()
			
		if self["idevsdevicefilelist"].getSelectedTrackNumber() > 0:
			self["key_blue"].setText(_("Play selected"))
			self["key_blue"].show()
		else:
			self["key_blue"].hide()
		
		
	def keyNumberGlobal(self, number):
		unichar = self.numericalTextInput.getKey(number)
		charstr = unichar.encode("utf-8")
		if len(charstr) == 1:
			self.idevsdevicefilelist.moveToChar(charstr[0])


	def keyAsciiCode(self):
		unichar = unichr(getPrevAsciiCode())
		charstr = unichar.encode("utf-8")
		if len(charstr) == 1:
			self.idevsdevicefilelist.moveToChar(charstr[0])
		
		
	def up(self):
		self["idevsdevicefilelist"].up()

	def down(self):
		self["idevsdevicefilelist"].down()
		
	def leftUp(self):
		self["idevsdevicefilelist"].pageUp()
		
	def rightDown(self):
		self["idevsdevicefilelist"].pageDown()
		
	def goParent(self):
		self["idevsdevicefilelist"].gotoParent()
		
	def switchToAudio(self):
		self["curmodality"].setText(_("Audio :"))
		self["idevsdevicefilelist"].showAudioMenu()
		
	def switchToVideo(self):
		self["curmodality"].setText(_("Video :"))
		self["idevsdevicefilelist"].showVideoMenu()
	
	def KeyPreview(self):
		if self.PlayMode == PLAY_SELECTED:
			item = self["idevsdevicefilelist"].getSelectedServiceRef()
			if item is None:
				return
			
			self.session.nav.playService(item)
#			self["coverArt"].updateCoverArt(str(self["idevsdevicefilelist"].getSelectedTrackFilename()))
		else:
			self.session.nav.playService(self["idevsdevicefilelist"].getServiceRef())
#			self["coverArt"].updateCoverArt(str(self["idevsdevicefilelist"].getTrackFilename()))

		self.FileInfoTimer.start(2000, True)
			
		self["play"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/play_enabled.png")
		self["stop"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/stop_disabled.png")


	def KeyEnter(self):
		self.ThumbTimer.stop()
		if self["idevsdevicefilelist"].canDescent():
			self.idevsdevicefilelist.descent()
		else:
			self.KeyPlaySingle()


	def KeyPlaySingle(self):
		self.ThumbTimer.stop()
		if self["idevsdevicefilelist"].canDescent():
			self.idevsdevicefilelist.descent()
		else:
			self.PlayMode = PLAY_SINGLE
			self.PlayService()
			self.BlinkingPlayIconTimer.stop()


	def KeyPlayAll(self):
		self.ThumbTimer.stop()
		if not self["idevsdevicefilelist"].canDescent():
			self.PlayMode = PLAY_ALL
			self.PlayService()
			self.BlinkingPlayIconTimer.start(1000, True)
			
			
	def KeyPlaySelected(self):
		self.ThumbTimer.stop()
		if self["idevsdevicefilelist"].getSelectedTrackNumber() > 0:
			self.PlayMode = PLAY_SELECTED
			self["idevsdevicefilelist"].gotoFirstSelectedTracks()
			self.PlayService()
			self.BlinkingPlayIconTimer.start(1000, True)


	def PlayService(self):
		self.currPlaying = 1
		if self.PlayMode == PLAY_SELECTED:
			item = self["idevsdevicefilelist"].getSelectedServiceRef()
			if item is None:
				return
			
			media_type = self["idevsdevicefilelist"].getSelectedTrackMediaTypes()
			
			if media_type == "VIDEO":
				self.session.open(MC_MoviePlayer, item, closeCB = self.closeMediaPlay ,no_menudelete = True)
			elif media_type == "AUDIO":
				self.session.nav.playService(item)
				#self["coverArt"].updateCoverArt(str(self["idevsdevicefilelist"].getSelectedTrackFilename()))
		else:
			
			media_type = self["idevsdevicefilelist"].getTrackMediaTypes()
			
			if media_type == "VIDEO":
				self.session.open(MC_MoviePlayer, self["idevsdevicefilelist"].getServiceRef(), closeCB = self.closeMediaPlay, no_menudelete = True)
			elif media_type == "AUDIO":
				self.session.nav.playService(self["idevsdevicefilelist"].getServiceRef())
				#self["coverArt"].updateCoverArt(str(self["idevsdevicefilelist"].getTrackFilename()))
					
		self.FileInfoTimer.start(2000, True)
			
		self["play"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/play_enabled.png")
		self["stop"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/stop_disabled.png")
		
					
	def showThumb(self):
		if self["idevsdevicefilelist"].canDescent():
			return
		else:
			if self.PlayMode == PLAY_SELECTED:
				if self["idevsdevicefilelist"].getSelectedServiceRef() is not None:
					self.session.nav.stopService()
					self.session.nav.playService(self["idevsdevicefilelist"].getSelectedServiceRef())
					
			else:
				if self["idevsdevicefilelist"].getServiceRef() is not None:
					self.session.nav.stopService()
					self.session.nav.playService(self["idevsdevicefilelist"].getServiceRef())
				
			self.currPlaying = 1
			self["play"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/play_enabled.png")
			self["stop"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/stop_disabled.png")
		
			self.FileInfoTimer.start(2000, True)
					
					
	def visibility(self, force=1):
		if self.isVisible == True:
			self.isVisible = False
			self.hide()
		else:
			self.isVisible = True
			self.show()


	def BlinkingPlayIcon(self):
		if self.blinking:
			self.blinking=False
			self["play"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/play_disabled.png")
			self.BlinkingPlayIconTimer.start(1000, True)
		else:
			self.blinking=True
			self["play"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/play_enabled.png")
			self.BlinkingPlayIconTimer.start(1000, True)
			
			
	def closeMediaPlay(self):
		print "closeMediaPlay"
		self["play"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/play_disabled.png")
		self["stop"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/stop_enabled.png")

		self.currPlaying = 0
		self.BlinkingPlayIconTimer.stop()
			
			
	def StopPlayback(self):
		self.ThumbTimer.stop()

		if self.isVisible == False:
			self.show()
			self.isVisible = True
		
		if self.session.nav.getCurrentService() is None:
			return
		
		else:
			self.session.nav.stopService()
			
			self["play"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/play_disabled.png")
			self["stop"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/stop_enabled.png")

			self.currPlaying = 0
			self.BlinkingPlayIconTimer.stop()


	def updateFileInfo(self):
		if self.PlayMode == PLAY_SELECTED:
			currTrack = self["idevsdevicefilelist"].getcurrSelectedTrack()
			if currTrack is None:
				return
		else:
			if self["idevsdevicefilelist"].canDescent():
				return
			currTrack = self["idevsdevicefilelist"].getcurrTrack()
		
		if currTrack is not None:
			sTitle = str(currTrack.title)
			sArtist = str(currTrack.artist)
			sAlbum = str(currTrack.album)
			sGenre = str(currTrack.genre)
			sPlayCount = str(currTrack.playcount)
			sBitrate = str(currTrack.bitrate)
			sComment = str(currTrack.comment)
			
			self["fileinfo"].setText(_("Title: ") + sTitle + _("\nArtist: ") +  sArtist + _("\nAlbum: ") + sAlbum + _("\nGenre: ") + sGenre + _("\nComment: ") + sComment +_("\nBitrate: ") + sBitrate)
			self["curplayingtitle"].setText(sTitle)
				
				
	def doEOF(self):
		print "MediaCenter: EOF Event ..."

		if self.PlayMode == PLAY_ALL:
			print "Play Next File ..."
			self.down()
			self.PlayService()
			self.ThumbTimer.stop()
			
		elif self.PlayMode == PLAY_SELECTED:
			print "Play Next Selected File ..."
			if self["idevsdevicefilelist"].getNextSelectedTracks():
				self.PlayService()
			self.ThumbTimer.stop()
			
		elif self.PlayMode == PLAY_SINGLE:
			print "Stop Playback ..."
			self.StopPlayback()
			
			
	def stepPlaySong(self):
		self.StopPlayback()
		self.PlayService()
		self.ThumbTimer.stop()
			
			
	def prevSong(self):
		self.StepPlaySong.stop()
		print "MediaCenter: previous Song Event ..."
		print "Play Previous File ..."
		if self.PlayMode == PLAY_SELECTED:
			if self["idevsdevicefilelist"].getPreviousSelectedTracks():
				self.StepPlaySong.start(1000, True)
		else:
			self.up()
			self.StepPlaySong.start(1000, True)
			
			
	def nextSong(self):
		self.StepPlaySong.stop()
		print "MediaCenter: next Song Event ..."
		print "Play Next File ..."
		if self.PlayMode == PLAY_SELECTED:
			if self["idevsdevicefilelist"].getNextSelectedTracks():
				self.StepPlaySong.start(1000, True)
		else:
			self.down()
			self.StepPlaySong.start(1000, True)
		

	def __evDecodeError(self):
		if self["idevsdevicefilelist"].canDescent():
			return

		currTrack = self["idevsdevicefilelist"].getcurrTrack()
		
		if currTrack is not None:
			currPlay = self.session.nav.getCurrentService()
			sVideoType = currTrack.filetype
			print "[__evDecodeError] video-codec %s can't be decoded by hardware" % (sVideoType)
			self.session.open(MessageBox, _("This QBoxHD can't decode %s video streams!") % sVideoType, type = MessageBox.TYPE_INFO,timeout = 20 )

	def __evPluginError(self):
		pass
	
	
	def Terminate(self):
		if self.isVisible == False:
			self.visibility()
			return
			
		self.ThumbTimer.stop()
		self.FileInfoTimer.stop()
		
#		del self["coverArt"].picload
		
		self.session.nav.stopService()
		self.close()


	def Exit(self):
		if self.idevsdevicefilelist.isTop:
			self.Terminate()
		else:
			self.goParent()


#------------------------------------------------------------------------------------------

class MediaPixmap(Pixmap):
	def __init__(self):
		Pixmap.__init__(self)
		self.coverArtFileName = ""
		self.picload = ePicLoad()
		self.picload.PictureData.get().append(self.paintCoverArtPixmapCB)
		self.coverFileNames = ["folder.png", "folder.jpg"]

	def applySkin(self, desktop, screen):
		from Tools.LoadPixmap import LoadPixmap
		noCoverFile = None
		if self.skinAttributes is not None:
			for (attrib, value) in self.skinAttributes:
				if attrib == "pixmap":
					noCoverFile = value
					break
		if noCoverFile is None:
			noCoverFile = resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/no_coverArt.png")
		self.noCoverPixmap = LoadPixmap(noCoverFile)
		return Pixmap.applySkin(self, desktop, screen)

	def onShow(self):
		Pixmap.onShow(self)
		sc = AVSwitch().getFramebufferScale()
		#0=Width 1=Height 2=Aspect 3=use_cache 4=resize_type 5=Background(#AARRGGBB)
		self.picload.setPara((self.instance.size().width(), self.instance.size().height(), sc[0], sc[1], False, 1, "#00000000"))

	def paintCoverArtPixmapCB(self, picInfo=None):
		ptr = self.picload.getData()
		if ptr != None:
			self.instance.setPixmap(ptr.__deref__())

	def updateCoverArt(self, path):
		while not path.endswith("/"):
			path = path[:-1]
		new_coverArtFileName = None
		for filename in self.coverFileNames:
			if fileExists(path + filename):
				new_coverArtFileName = path + filename
		if self.coverArtFileName != new_coverArtFileName:
			self.coverArtFileName = new_coverArtFileName
			if new_coverArtFileName:
				self.picload.startDecode(self.coverArtFileName)
			else:
				self.showDefaultCover()

	def showDefaultCover(self):
		self.instance.setPixmap(self.noCoverPixmap)

	def embeddedCoverArt(self):
		print "[embeddedCoverArt] found"
		self.coverArtFileName = "/tmp/.id3coverart"
		self.picload.startDecode(self.coverArtFileName)
