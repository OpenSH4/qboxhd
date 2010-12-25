from qboxhd import QBOXHD, QBOXHD_MINI
from Screen import Screen
from Components.ActionMap import ActionMap
from Components.config import config
from Components.AVSwitch import AVSwitch
from Components.SystemInfo import SystemInfo
from enigma import eDVBVolumecontrol

#QBOXHD
if QBOXHD:
	if QBOXHD_MINI:
		from Components.FrontButton import ifrontbutton
	else:
		from Components.SenseWheel import isensewheel
		
	from enigma import eDVBCIInterfaces

inStandby = None

class Standby(Screen):
	def Power(self):
		print "leave standby"

		if QBOXHD:
			if not QBOXHD_MINI:
				#enable CI modules
				eDVBCIInterfaces.getInstance().rst_all_modules(0)

		#set input to encoder
		self.avswitch.setInput("ENCODER")
		#restart last played service
		#unmute adc
		self.leaveMute()
		#set brightness of lcd
		config.lcd.bright.apply()
		if QBOXHD:
			if QBOXHD_MINI:
				#set frontbutton standby mode
				ifrontbutton.leaveStandby()
			else:
				#set sensewheel standby mode
				isensewheel.leaveStandby()
		#kill me
		self.close(True)

		if QBOXHD:
			#enable video and audio output
			self.avswitch.disable_scart_hdmi(0)

	def setMute(self):
		if (eDVBVolumecontrol.getInstance().isMuted()):
			self.wasMuted = 1
			print "mute already active"
		else:
			self.wasMuted = 0
			eDVBVolumecontrol.getInstance().volumeToggleMute()

	def leaveMute(self):
		if self.wasMuted == 0:
			eDVBVolumecontrol.getInstance().volumeToggleMute()

	def __init__(self, session):
		Screen.__init__(self, session)
		if QBOXHD:
			global inStandby
			inStandby = self

			print "enter standby"

		self.avswitch = AVSwitch()

		self["actions"] = ActionMap( [ "StandbyActions" ],
		{
			"power": self.Power
		}, -1)

		#mute adc
		self.setMute()
		#get currently playing service reference
		self.prev_running_service = self.session.nav.getCurrentlyPlayingServiceReference()
		#stop actual played dvb-service
		self.session.nav.stopService()

		if QBOXHD:
			if not QBOXHD_MINI:
				#disable CI modules
				eDVBCIInterfaces.getInstance().rst_all_modules(1)

		#set input to vcr scart
		if SystemInfo["ScartSwitch"]:
			self.avswitch.setInput("SCART")
		else:
			self.avswitch.setInput("AUX")
		#set lcd brightness to standby value
		config.lcd.standby.apply()

		if QBOXHD:
			if QBOXHD_MINI:
				#set frontbutton standby mode
				ifrontbutton.enterStandby()
			else:
				#set sensewheel standby mode
				isensewheel.enterStandby()

			#disable video and audio output
			self.avswitch.disable_scart_hdmi(1)

		self.onShow.append(self.__onShow)
		self.onHide.append(self.__onHide)
		self.onClose.append(self.__onClose)

	def __onClose(self):
		if self.prev_running_service:
			self.session.nav.playService(self.prev_running_service)

		if QBOXHD:
			global inStandby
			inStandby = None

	def createSummary(self):
		return StandbySummary

	def __onShow(self):
		if not QBOXHD:
			self.session.screen["Standby"].boolean = True
		else :
			pass

	def __onHide(self):
		if not QBOXHD:
			self.session.screen["Standby"].boolean = False
		else :
			pass


class StandbySummary(Screen):
	skin = """
	<screen name="StandbySummary" position="0,0" size="320,240">
		<widget source="global.CurrentTime" render="Label" position="6,0" size="320,240" font="Regular;100" halign="center" valign="center">
			<convert type="ClockToText">Format:%H:%M</convert>
		</widget>
		<widget source="session.RecordState" render="FixedLabel" position="6,0" zPosition="1" size="320,240" text=" ">
			<convert type="ConfigEntryTest">config.usage.blinking_display_clock_during_recording,True,CheckSourceBoolean</convert>
			<convert type="ConditionalShowHide">Blink</convert>
		</widget>
	</screen>"""

from enigma import quitMainloop, iRecordableService
from Screens.MessageBox import MessageBox
from time import time
from Components.Task import job_manager

inTryQuitMainloop = False

class TryQuitMainloop(MessageBox):
	def __init__(self, session, retvalue=1, timeout=-1, default_yes = True):
		self.retval=retvalue
		recordings = session.nav.getRecordings()
		jobs = len(job_manager.getPendingJobs())
		self.connected = False
		reason = ""
		next_rec_time = -1
		if not recordings:
			next_rec_time = session.nav.RecordTimer.getNextRecordingTime()
		if recordings or (next_rec_time > 0 and (next_rec_time - time()) < 360):
			reason = _("Recording(s) are in progress or coming up in few seconds!") + '\n'
		if jobs:
			if jobs == 1:
				job = job_manager.getPendingJobs()[0]
				reason += "%s: %s (%d%%)\n" % (job.getStatustext(), job.name, int(100*job.progress/float(job.end)))
			else:
				reason += (_("%d jobs are running in the background!") % jobs) + '\n'
		if reason:
			if retvalue == 1:
				MessageBox.__init__(self, session, reason+_("Really shutdown now?"), type = MessageBox.TYPE_YESNO, timeout = timeout, default = default_yes)
			elif retvalue == 2:
				MessageBox.__init__(self, session, reason+_("Really reboot now?"), type = MessageBox.TYPE_YESNO, timeout = timeout, default = default_yes)
			elif retvalue == 4:
				pass
			else:
				MessageBox.__init__(self, session, reason+_("Really restart now?"), type = MessageBox.TYPE_YESNO, timeout = timeout, default = default_yes)
			self.skinName = "MessageBox"
			session.nav.record_event.append(self.getRecordEvent)
			self.connected = True
			self.onShow.append(self.__onShow)
			self.onHide.append(self.__onHide)
		else:
			self.skin = """<screen position="0,0" size="0,0"/>"""
			Screen.__init__(self, session)
			self.close(True)

	def getRecordEvent(self, recservice, event):
		if event == iRecordableService.evEnd:
			recordings = self.session.nav.getRecordings()
			if not recordings: # no more recordings exist
				rec_time = self.session.nav.RecordTimer.getNextRecordingTime()
				if rec_time > 0 and (rec_time - time()) < 360:
					self.initTimeout(360) # wait for next starting timer
					self.startTimer()
				else:
					self.close(True) # immediate shutdown
		elif event == iRecordableService.evStart:
			self.stopTimer()

	def close(self, value):
		if self.connected:
			self.conntected=False
			self.session.nav.record_event.remove(self.getRecordEvent)
		if value:
			print "ENIGMA EXITCODE: ", str(self.retval)
			if QBOXHD:
				if (self.retval == 1):
					if QBOXHD_MINI:
						ifrontbutton.enterDeepStandby()
					else:
						#When ShutDown i power off all leds
						isensewheel.setPanelLedsEnable( False )
						isensewheel.setBoardLedsEnable( False )
						isensewheel.disableSense()
						
					config.lcd.bright.value = 0
					config.lcd.bright.apply()

			quitMainloop(self.retval)
		else:
			MessageBox.close(self, True)

	def __onShow(self):
		global inTryQuitMainloop
		inTryQuitMainloop = True

	def __onHide(self):
		global inTryQuitMainloop
		inTryQuitMainloop = False
