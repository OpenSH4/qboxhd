from qboxhd import QBOXHD
from Tools.Profile import profile
from Screens.Screen import Screen
from enigma import iPlayableService

from Screens.InfoBarGenerics import InfoBarShowHide, InfoBarMenu, \
		InfoBarSeek, InfoBarAudioSelection, InfoBarNotifications, \
		InfoBarServiceNotifications, InfoBarPVRState, InfoBarCueSheetSupport, InfoBarSimpleEventView, \
		InfoBarMoviePlayerSummarySupport, InfoBarSubtitleSupport, InfoBarServiceErrorPopupSupport

from Components.ActionMap import HelpableActionMap
from Components.config import config
from Components.ServiceEventTracker import ServiceEventTracker, InfoBarBase

from Screens.HelpMenu import HelpableScreen

class MC_MoviePlayer(Screen, InfoBarBase, InfoBarShowHide, \
		InfoBarMenu, InfoBarSeek, \
		InfoBarAudioSelection, HelpableScreen, InfoBarNotifications, \
		InfoBarServiceNotifications, InfoBarPVRState, InfoBarCueSheetSupport, InfoBarSimpleEventView, \
		InfoBarMoviePlayerSummarySupport, InfoBarSubtitleSupport, \
		InfoBarServiceErrorPopupSupport):

	ENABLE_RESUME_SUPPORT = True
	ALLOW_SUSPEND = True
		
	def __init__(self, session, service, closeCB = None, no_menudelete = False):
		Screen.__init__(self, session)
		
		self["actions"] = HelpableActionMap(self, "MC_MoviePlayerActions",
			{
				"leavePlayer": (self.leavePlayer, _("leave movie player..."))
			})
		
		for x in HelpableScreen, InfoBarShowHide, InfoBarMenu, \
				InfoBarBase, InfoBarSeek, \
				InfoBarAudioSelection, InfoBarNotifications, InfoBarSimpleEventView, \
				InfoBarServiceNotifications, InfoBarPVRState, InfoBarCueSheetSupport, \
				InfoBarMoviePlayerSummarySupport, InfoBarSubtitleSupport, \
				InfoBarServiceErrorPopupSupport:
			x.__init__(self)

		self.lastservice = session.nav.getCurrentlyPlayingServiceReference()
		session.nav.playService(service)
		self.returning = False
		self.closeCB = closeCB
		self.no_menudelete = no_menudelete
		self.onClose.append(self.__onClose)

	def __onClose(self):
		self.session.nav.playService(self.lastservice)
		if self.closeCB is not None:
			self.closeCB()

	def handleLeave(self, how):
		self.is_closing = True
		if how == "ask":
			if config.usage.setup_level.index < 2: # -expert
				list = (
					(_("Yes"), "quit"),
					(_("No"), "continue")
				)
			else:
				if self.no_menudelete:
					list = (
						(_("Yes"), "quit"),
						(_("No"), "continue"),
						(_("No, but restart from begin"), "restart")
					)
					
				else:
					list = (
						(_("Yes"), "quit"),
						(_("Yes, and delete this movie"), "quitanddelete"),
						(_("No"), "continue"),
						(_("No, but restart from begin"), "restart")
					)

			from Screens.ChoiceBox import ChoiceBox
			self.session.openWithCallback(self.leavePlayerConfirmed, ChoiceBox, title=_("Stop playing this movie?"), list = list)
		else:
			self.leavePlayerConfirmed([True, how])

	def leavePlayer(self):
		self.handleLeave(config.usage.on_movie_stop.value)

	def deleteConfirmed(self, answer):
		if answer:
			self.leavePlayerConfirmed((True, "quitanddeleteconfirmed"))

	def leavePlayerConfirmed(self, answer):
		answer = answer and answer[1]

		if answer in ("quitanddelete", "quitanddeleteconfirmed"):
			ref = self.session.nav.getCurrentlyPlayingServiceReference()
			from enigma import eServiceCenter
			serviceHandler = eServiceCenter.getInstance()
			info = serviceHandler.info(ref)
			name = info and info.getName(ref) or _("this recording")

			if answer == "quitanddelete":
				from Screens.MessageBox import MessageBox
				self.session.openWithCallback(self.deleteConfirmed, MessageBox, _("Do you really want to delete %s?") % name)
				return

			elif answer == "quitanddeleteconfirmed":
				offline = serviceHandler.offlineOperations(ref)
				if offline.deleteFromDisk(0):
					from Screens.MessageBox import MessageBox
					self.session.openWithCallback(self.close, MessageBox, _("You cannot delete this!"), MessageBox.TYPE_ERROR)
					return

		if answer in ("quit", "quitanddeleteconfirmed"):
			config.movielist.last_videodir.cancel()
			self.close()
		elif answer == "restart":
			self.doSeek(0)

	def doEofInternal(self, playing):
		print " MC_MoviePlayer::*************************** doEofInternal\n"
		if not self.execing:
			return
		if not playing:
			return
		if QBOXHD:
			self.close()
		else:
			self.handleLeave(config.usage.on_movie_eof.value)
