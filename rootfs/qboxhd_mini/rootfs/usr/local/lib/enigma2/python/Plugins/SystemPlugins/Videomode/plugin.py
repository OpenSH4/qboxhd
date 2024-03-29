from qboxhd import QBOXHD
from Screens.Screen import Screen
from Plugins.Plugin import PluginDescriptor
from Components.SystemInfo import SystemInfo
from Components.ConfigList import ConfigListScreen
from Components.config import getConfigListEntry, config, ConfigBoolean

from VideoHardware import video_hw
#QBOXHD
from os import path , system , unlink
	
config.misc.videowizardenabled = ConfigBoolean(default = True)

class VideoSetup(Screen, ConfigListScreen):
	def __init__(self, session, hw):
		Screen.__init__(self, session)
		if not QBOXHD:
			self.skinName = "Setup"
		self.setup_title = "Videomode Setup"
		self.hw = hw
		self.onChangedEntry = [ ]

		# handle hotplug by re-creating setup
		self.onShow.append(self.startHotplug)
		self.onHide.append(self.stopHotplug)

		self.list = [ ]
		ConfigListScreen.__init__(self, self.list, session = session, on_change = self.changedEntry)

		from Components.ActionMap import ActionMap
		if QBOXHD:
			self["actions"] = ActionMap(["SetupActions", "InputActions"],
				{
					"ok": self.keyOK,
					"cancel": self.keyCancel,
					"save": self.apply,
					"left": self.keyLeft,
					"right": self.keyRight,
				}, -2)
		else:
			self["actions"] = ActionMap(["SetupActions", "InputActions"],
				{
					"cancel": self.keyCancel,
					"save": self.apply,
					"left": self.keyLeft,
					"right": self.keyRight,
				}, -2)

		from Components.Label import Label
		self["title"] = Label(_("A/V Settings"))

		self["oktext"] = Label(_("OK"))
		self["canceltext"] = Label(_("Cancel"))

		from Components.Pixmap import Pixmap
		self["ok"] = Pixmap()
		self["cancel"] = Pixmap()

		self.createSetup()
		self.grabLastGoodMode()

	def startHotplug(self):
		self.hw.on_hotplug.append(self.createSetup)

	def stopHotplug(self):
		self.hw.on_hotplug.remove(self.createSetup)

	def createSetup(self):
		level = config.usage.setup_level.index
		
		self.list = [
			getConfigListEntry(_("Video Output"), config.av.videoport)
		]

		# if we have modes for this port:
		if config.av.videoport.value in config.av.videomode:
			# add mode- and rate-selection:
			self.list.append(getConfigListEntry(_("Mode"), config.av.videomode[config.av.videoport.value]))
			if config.av.videomode[config.av.videoport.value].value == 'PC':
				self.list.append(getConfigListEntry(_("Resolution"), config.av.videorate[config.av.videomode[config.av.videoport.value].value]))
			else:
				self.list.append(getConfigListEntry(_("Refresh Rate"), config.av.videorate[config.av.videomode[config.av.videoport.value].value]))

		port = config.av.videoport.value
		if port not in config.av.videomode:
			mode = None
		else:
			mode = config.av.videomode[port].value

		# some modes (720p, 1080i) are always widescreen. Don't let the user select something here, "auto" is not what he wants.
		force_wide = self.hw.isWidescreenMode(port, mode)

		if not force_wide:
			self.list.append(getConfigListEntry(_("Aspect Ratio"), config.av.aspect))
		
		if force_wide or config.av.aspect.value in ("16_9", "16_10"):
			self.list.extend((
				getConfigListEntry(_("Display 4:3 content as"), config.av.policy_43),
				getConfigListEntry(_("Display >16:9 content as"), config.av.policy_169)
			))
		elif config.av.aspect.value == "4_3":
			self.list.append(getConfigListEntry(_("Display 16:9 content as"), config.av.policy_169))

		if config.av.videoport.value == "Component":
			self.list.append(getConfigListEntry(_("Color Format"), config.av.colorformat_yuv))
			
		if config.av.videoport.value == "HDMI":
			if QBOXHD:
				self.list.append(getConfigListEntry(_("Color Format"), config.av.colorformat_hdmi))
			else:
				self.list.append(getConfigListEntry(_("Color Format"), config.av.colorformat))
			self.list.append(getConfigListEntry(_("Audio Source"), config.av.hdmi_audio_source))
		
		if config.av.videoport.value == "Scart":
			self.list.append(getConfigListEntry(_("Color Format"), config.av.colorformat))
			if level >= 1:
				self.list.append(getConfigListEntry(_("WSS on 4:3"), config.av.wss))
				if not QBOXHD:
					if SystemInfo["ScartSwitch"]:
						self.list.append(getConfigListEntry(_("Auto scart switching"), config.av.vcrswitch))

		if level >= 1:
			self.list.append(getConfigListEntry(_("AC3 default"), config.av.defaultac3))
			if SystemInfo["CanDownmixAC3"]:
				self.list.append(getConfigListEntry(_("AC3 downmix"), config.av.downmix_ac3))
			if not QBOXHD:
				self.list.extend((
					getConfigListEntry(_("General AC3 Delay"), config.av.generalAC3delay),
					getConfigListEntry(_("General PCM Delay"), config.av.generalPCMdelay)
					))
		
		if SystemInfo["CanChangeOsdAlpha"]:
			self.list.append(getConfigListEntry(_("OSD visibility"), config.av.osd_alpha))

		self["config"].list = self.list
		self["config"].l.setList(self.list)
		
		if QBOXHD:
			############
			print""
			print "port:",port
			print "mode:",mode
			#print "rates:",rate
			print ""
			############

	def keyLeft(self):
		ConfigListScreen.keyLeft(self)
		self.createSetup()

	def keyRight(self):
		ConfigListScreen.keyRight(self)
		self.createSetup()

	def confirm(self, confirmed):
		if not confirmed:
			config.av.videoport.value = self.last_good[0]
			config.av.videomode[self.last_good[0]].value = self.last_good[1]
			config.av.videorate[self.last_good[1]].value = self.last_good[2]
			if QBOXHD:
				############
				self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
				self.session.nav.stopService()
				############
			self.hw.setMode(*self.last_good)
			if QBOXHD:
				############
				self.session.nav.playService(self.oldService)
				############
		else:
			if QBOXHD:
				############
				port = config.av.videoport.value
				mode = config.av.videomode[port].value
				rate = config.av.videorate[mode].value
				fp = open('/etc/fb.modes.supp', 'w+')
				fp.write(str(mode)+"@"+str(rate)+"@"+str(port)+"@SAVED\n")
				fp.close()
				############
			self.keySave()

	def grabLastGoodMode(self):
		port = config.av.videoport.value
		mode = config.av.videomode[port].value
		rate = config.av.videorate[mode].value
		self.last_good = (port, mode, rate)
		
	def keyOK(self):
		self.apply()

	def apply(self):
		port = config.av.videoport.value
		mode = config.av.videomode[port].value
		rate = config.av.videorate[mode].value
		
	
		if (port, mode, rate) != self.last_good:
			if QBOXHD:
				############
				self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
				self.session.nav.stopService()
				############
			self.hw.setMode(port, mode, rate)
			if QBOXHD:
				############
				self.session.nav.playService(self.oldService)
				############
			from Screens.MessageBox import MessageBox
			self.session.openWithCallback(self.confirm, MessageBox, "Is this videomode ok?", MessageBox.TYPE_YESNO, timeout = 20, default = False)
		else:
			self.keySave()

	# for summary:
	def changedEntry(self):
		for x in self.onChangedEntry:
			x()

	def getCurrentEntry(self):
		return self["config"].getCurrent()[0]

	def getCurrentValue(self):
		return str(self["config"].getCurrent()[1].getText())

	def createSummary(self):
		from Screens.Setup import SetupSummary
		return SetupSummary

class VideomodeHotplug:
	def __init__(self, hw):
		self.hw = hw

	def start(self):
		self.hw.on_hotplug.append(self.hotplug)

	def stop(self):
		self.hw.on_hotplug.remove(self.hotplug)

	def hotplug(self, what):
		print "hotplug detected on port '%s'" % (what)
		port = config.av.videoport.value
		mode = config.av.videomode[port].value
		rate = config.av.videorate[mode].value

		if not self.hw.isModeAvailable(port, mode, rate):
			print "mode %s/%s/%s went away!" % (port, mode, rate)
			modelist = self.hw.getModeList(port)
			if not len(modelist):
				print "sorry, no other mode is available (unplug?). Doing nothing."
				return
			mode = modelist[0][0]
			rate = modelist[0][1]
			print "setting %s/%s/%s" % (port, mode, rate)
			if QBOXHD:
				############
				self.oldService = self.session.nav.getCurrentlyPlayingServiceReference()
				self.session.nav.stopService()
				############
			self.hw.setMode(port, mode, rate)
			if QBOXHD:
				############
				self.session.nav.playService(self.oldService)
				############

hotplug = None

def startHotplug():
	global hotplug, video_hw
	hotplug = VideomodeHotplug(video_hw)
	hotplug.start()

def stopHotplug():
	global hotplug
	hotplug.stop()


def autostart(reason, session = None, **kwargs):
	if session is not None:
		global my_global_session
		my_global_session = session
		return

	if reason == 0:
		startHotplug()
	elif reason == 1:
		stopHotplug()

def videoSetupMain(session, **kwargs):
	session.open(VideoSetup, video_hw)

def startSetup(menuid):
	if QBOXHD:
		if menuid != "user_preferences":
			return [ ]
		
		return [(_("TV Settings") + "...", videoSetupMain, "av_setup", 40)]
	else:
		if menuid != "system":
			return [ ]

		return [(_("A/V Settings") + "...", videoSetupMain, "av_setup", 40)]

def VideoWizard(*args, **kwargs):
	from VideoWizard import VideoWizard
	return VideoWizard(*args, **kwargs)

def Plugins(**kwargs):
	list = [
#		PluginDescriptor(where = [PluginDescriptor.WHERE_SESSIONSTART, PluginDescriptor.WHERE_AUTOSTART], fnc = autostart),
		PluginDescriptor(name=_("Video Setup"), description=_("Advanced Video Setup"), where = PluginDescriptor.WHERE_MENU, fnc=startSetup)
	]
	if config.misc.videowizardenabled.value:
		list.append(PluginDescriptor(name=_("Video Wizard"), where = PluginDescriptor.WHERE_WIZARD, fnc=(0, VideoWizard)))
 	return list
