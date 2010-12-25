from qboxhd import QBOXHD
from os import popen
from Components.Console import Console
from enigma import eConsoleAppContainer
from os import path as os_path, remove as os_remove, listdir as os_listdir
from time import strftime, sleep
from enigma import iPlayableService, eTimer, eServiceCenter, iServiceInformation
from Screens.Screen import Screen
from Screens.MessageBox import MessageBox
from Screens.InputBox import InputBox
from Components.ActionMap import NumberActionMap, HelpableActionMap
from Components.Label import Label
from Components.Pixmap import Pixmap,MultiPixmap
from Components.Label import Label
from Components.FileList import FileList
from Tools.Directories import resolveFilename, SCOPE_CONFIG, SCOPE_PLAYLIST, SCOPE_SKIN_IMAGE
from Components.ServicePosition import ServicePositionGauge
from Components.ServiceEventTracker import ServiceEventTracker, InfoBarBase
from Components.Playlist import PlaylistIOInternal, PlaylistIOM3U, PlaylistIOPLS
from Screens.InfoBarGenerics import InfoBarSeek, InfoBarAudioSelection, InfoBarCueSheetSupport, InfoBarNotifications, InfoBarSubtitleSupport
from ServiceReference import ServiceReference
from Screens.ChoiceBox import ChoiceBox
from Screens.HelpMenu import HelpableScreen
from Components.Harddisk import harddiskmanager
from Tools.Directories import fileExists, pathExists
import random
import os
import time

class DivXPlayer(Screen, InfoBarBase, InfoBarSeek, InfoBarAudioSelection, InfoBarCueSheetSupport, InfoBarNotifications, InfoBarSubtitleSupport, HelpableScreen):
	ALLOW_SUSPEND = True
	ENABLE_RESUME_SUPPORT = True

	SEEK_STATE_FWD = (1, 0, 0, ">>")
	SEEK_STATE_BWD = (1, 0, 0, "<<")

	def __init__(self, session, args = None):
		Screen.__init__(self, session)
		InfoBarAudioSelection.__init__(self)
		InfoBarCueSheetSupport.__init__(self, actionmap = "DivXPlayerCueSheetActions")
		InfoBarNotifications.__init__(self)
		InfoBarBase.__init__(self)
		InfoBarSubtitleSupport.__init__(self)
		HelpableScreen.__init__(self)
		
		self.container = eConsoleAppContainer()
		
		self.summary = None
		self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
		self.session.nav.stopService()

		# 'None' is magic to start at the list of mountpoints
		self.filelist = FileList(None, matchingPattern = "(?i)^.*\.(avi)", useServiceRef = True, additionalExtensions = "4098:m3u 4098:e2pls 4098:pls")
		self["filelist"] = self.filelist

		self.is_closing = False
		self.delname = ""
		
		self.next_operation = ""
		self.lastServicePlayed = None
		self.current_service = None

		self["PositionGauge"] = ServicePositionGauge(self.session.nav)
		
		self["currenttext"] = Label("")

		self.repeat = False
		self.seek_target = None

		class DivXPlayerActionMap(NumberActionMap):
			def __init__(self, player, contexts = [ ], actions = { }, prio=0):
				NumberActionMap.__init__(self, contexts, actions, prio)
				self.player = player

			def action(self, contexts, action):
				self.player.show()
				return NumberActionMap.action(self, contexts, action)


		self["OkCancelActions"] = HelpableActionMap(self, "OkCancelActions", 
			{
				"ok": (self.ok, _("play divx file")),
				"cancel": (self.exit, _("exit divxplayer")),
			}, -2)

		self["DivXPlayerActions"] = HelpableActionMap(self, "DivXPlayerActions",
			{
				"play": (self.playEntry, _("play entry")),
				"pause": (self.pauseEntry, _("pause")),
				"stop": (self.stopEntry, _("stop entry")),
				"forward": (self.forwardEntry, _("forward entry")),
				"backward": (self.doNothing, _("backward entry")),
				"menu": (self.showMenu, _("menu")),
			}, -2)

		self["actions"] = DivXPlayerActionMap(self, ["DirectionActions"], 
		{
			"right": self.right,
			"left": self.left,

			"up": self.up,
			"upRepeated": self.up,
			"upUp": self.doNothing,
			"down": self.down,
			"downRepeated": self.down,
			"downUp": self.doNothing,
		}, -2)

		InfoBarSeek.__init__(self, actionmap = "DivXPlayerSeekActions")

		self.onClose.append(self.__onClose)

		self.__event_tracker = ServiceEventTracker(screen=self, eventmap=
			{
				iPlayableService.evUser+11: self.__evDecodeError,
				iPlayableService.evUser+12: self.__evPluginError
			})

	def doNothing(self):
		pass

	def createSummary(self):
		return DivXPlayerLCDScreen
	

	def exit(self):
		self.hide()
		self.session.openWithCallback(self.exitCB, MessageBox, _("Do you really want to exit?"), timeout=5)

	
	def exitCB(self, answer):
		if answer == True:
			self.close()

	def doEofInternal(self, playing):
		self.show()

	def __onClose(self):
		self.stopDivXService()
		self.session.nav.playService(self.oldService)

	def __evDecodeError(self):
		currPlay = self.session.nav.getCurrentService()
		sVideoType = currPlay.info().getInfoString(iServiceInformation.sVideoType)
		print "[__evDecodeError] video-codec %s can't be decoded by hardware" % (sVideoType)
		self.session.open(MessageBox, _("This QBoxHD can't decode %s video streams!") % sVideoType, type = MessageBox.TYPE_INFO,timeout = 20 )

	def __evPluginError(self):
		currPlay = self.session.nav.getCurrentService()
		message = currPlay.info().getInfoString(iServiceInformation.sUser+12)
		print "[__evPluginError]" , message
		self.session.open(MessageBox, message, type = MessageBox.TYPE_INFO,timeout = 20 )

	def hideAfterResume(self):
		self.hide()

	def getIdentifier(self, ref):
		text = ref.getPath()
		return text.split('/')[-1]

	# FIXME: maybe this code can be optimized 
	def updateCurrentInfo(self):
		text = ""
		idx = self.filelist.getSelectionIndex()
		r = self.filelist.list[idx]
		text = r[1][7]
					
		if r[0][1] == True:
			if len(text) < 2:
				text += " "
			if text[:2] != "..":
				text = "/" + text
				
			self.summaries.setText(text,1)
			idx += 1
		if idx < len(self.filelist.list):
			r = self.filelist.list[idx]
			text = r[1][7]
			if r[0][1] == True:
				text = "/" + text
			self.summaries.setText(text,3)
		else:
			self.summaries.setText(" ",3)
	
	def ok(self):
		selection = self["filelist"].getSelection()
		print "[DivX Player] %s\n" % str(selection[0])
		
		if selection[1] == True: # isDir
			self["filelist"].changeDir(selection[0])
		else:
			r = self.filelist.getServiceRef()
			if r is None:
				return
			text = r.getPath()
			
			self.playEntry()
			self.hide()


	def showMenu(self):
		menu = []
		menu.append((_("hide player"), "hide"));
		self.session.openWithCallback(self.menuCallback, ChoiceBox, title="", list=menu)


	def menuCallback(self, choice):
		if choice is None:
			return
		
		elif choice[1] == "hide":
			self.hide()

	def left(self):
		self["filelist"].pageUp()
		self.updateCurrentInfo()

	def right(self):
		self["filelist"].pageDown()
		self.updateCurrentInfo()
		
	def up(self):
		self["filelist"].up()
		self.updateCurrentInfo()

	def down(self):
		self["filelist"].down()
		self.updateCurrentInfo()

	def IsPlayingDivXService(self):
		return self.container.running()
				
				
	def performNextOperation(self):
		if len(self.next_operation) > 0:
			print "[DivX Player] NEXT OPERATION %s" % self.next_operation
			
			if self.next_operation == "PLAY_DIVX":
				print "[DivX Player] Play new %s" % self.current_service.getPath()
			
				self.lastServicePlayed = self.current_service
				self.playDivXService( self.current_service )
				self.seekstate = self.SEEK_STATE_PLAY
			
		self.next_operation = ""
				
				
				
	def playDivXService(self, currref):
		if not self.IsPlayingDivXService():
			text = currref.getPath()
			print "[DivX Player] Playing %s ..." % str(text)
			cmd = "/usr/local/bin/dvbtest -i -w auto -W auto \"%s\"" % str(text)
			print cmd
			self.container.appClosed.append(self.divxPlayFinish)
			self.container.execute(cmd)
			
			
	def timeStampDivXService(self):
		if self.IsPlayingDivXService():
			self.container.write( "t", 1 )
			print "[DivX Player] show TimeStamp"
		
	def forwardDivXService(self):
		if self.IsPlayingDivXService():
			self.container.write( "+", 1 )
			print "[DivX Player] Forward Speed"
				
		
	def stopDivXService(self):
		if self.IsPlayingDivXService():
			self.container.write( "x", 1 )
			print "[DivX Player] Stopped"
			
		
	def pauseDivXService(self):
		if self.IsPlayingDivXService():
			self.container.write( "z", 1 )
			print "[DivX Player] Paused"
			
			
	def resumeDivXPlay(self):
		if self.IsPlayingDivXService():
			print "[DivX Player] Resume Play"
			self.container.write( "c", 1 )
					
					
	def divxPlayFinish(self, retval):
		self.lastServicePlayed = None
		self.container.appClosed.remove(self.divxPlayFinish)
		print "[DivX Player] Killed"
		
		self.performNextOperation()
	
	
	def playEntry(self):
		
		selection = self["filelist"].getSelection()
		if selection[1] == True: # isDir
			return
		
		self.current_service = self.filelist.getServiceRef()
		
		self.next_operation = "PLAY_DIVX"
		
		#if last service played stop it ...
		if self.lastServicePlayed is not None and self.lastServicePlayed != self.current_service:
			self.stopDivXService()
		else:
			if (self.seekstate == self.SEEK_STATE_PAUSE) or (self.seekstate == self.SEEK_STATE_BWD) or (self.seekstate == self.SEEK_STATE_FWD):
				print "resumeDivXPlay"
				self.resumeDivXPlay()
			else:
				print "playDivXService"
				self.lastServicePlayed = self.current_service
				self.playDivXService( self.current_service )
				
			self.seekstate = self.SEEK_STATE_PLAY


	def pauseEntry(self):
		self.pauseDivXService()
		self.seekstate = self.SEEK_STATE_PAUSE
		self.hide()


	def stopEntry(self):
		self.next_operation = ""
		self.stopDivXService()
		self.show()
		
		
	def forwardEntry(self):
		self.forwardDivXService()
		self.seekstate = self.SEEK_STATE_FWD

		
	def subtitleSelection(self):
		from Screens.Subtitles import Subtitles
		self.session.open(Subtitles)


class DivXPlayerLCDScreen(Screen):
# 2 lines for each file/directory name
	skin = """
	<screen position="0,0" size="320,240" title="LCD Text">
		<widget name="text1" position="20,5" size="320,65" font="Regular;32"/>
		<widget name="text3" position="20,85" size="320,61" font="Regular;30"/>
		<widget name="text4" position="20,160" size="320,61" font="Regular;30"/>
	</screen>"""

	def __init__(self, session, parent):
		Screen.__init__(self, session)
		self["text1"] = Label("DivXplayer")
		self["text3"] = Label("")
		self["text4"] = Label("")

	def setText(self, text, line):
		print "lcd set text:", text, line
		if len(text) > 10:
			if text[-4:] == ".mp3":
				text = text[:-4]
		textleer = "    "
		text = text + textleer*10
		if line == 1:
			self["text1"].setText(text)
		elif line == 3:
			self["text3"].setText(text)
		elif line == 4:
			self["text4"].setText(text)

def main(session, **kwargs):
	session.open(DivXPlayer)

def menu(menuid, **kwargs):
	if menuid == "mainmenu":
		return [(_("DivX player"), main, "divx_player", 45)]
	return []

def filescan_open(list, session, **kwargs):
	from enigma import eServiceReference

	mp = session.open(DivXPlayer)
	mp.playlist.clear()
	mp.savePlaylistOnExit = False

	for file in list:
		if file.mimetype == "video/MP2T":
			stype = 1
		else:
			stype = 4097
		ref = eServiceReference(stype, 0, file.path)
		mp.playlist.addFile(ref)

	mp.changeEntry(0)
	mp.switchToPlayList()

def filescan(**kwargs):
	from Components.Scanner import Scanner, ScanPath
	mediatypes = [
		Scanner(mimetypes = ["video/divx3", "video/divx4", "video/divx5"],
			paths_to_scan =
				[
					ScanPath(path = "", with_subdirs = False),
				],
			name = "DivX Movie",
			description = "View DivX Movies...",
			openfnc = filescan_open,
		)]
	return mediatypes

from Plugins.Plugin import PluginDescriptor
def Plugins(**kwargs):
	return [
		PluginDescriptor(name = "DivXPlayer", description = "Play back divx media files", where = PluginDescriptor.WHERE_MENU, fnc = menu),
		PluginDescriptor(name = "DivXPlayer", where = PluginDescriptor.WHERE_FILESCAN, fnc = filescan)
	]
