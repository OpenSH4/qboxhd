from qboxhd import QBOXHD, QBOXHD_MINI
from enigma import eTimer
from Components.config import config, ConfigSelection, ConfigSubDict, ConfigYesNo

from Tools.CList import CList

if QBOXHD:
	from enigma import set_scart_aspect

# The "VideoHardware" is the interface to /etc/enigma2/stb/video.
# It generates hotplug events, and gives you the list of
# available and preferred modes, as well as handling the currently
# selected mode. No other strict checking is done.
class VideoHardware:
	rates = { } # high-level, use selectable modes.

	modes = { }  # a list of (high-level) modes for a certain port.
	if QBOXHD:
		rates["PAL"] =			{ "50Hz":		{ 50: "pal" } }

		rates["NTSC"] =			{ "60Hz": 	{ 60: "ntsc" } }

#		rates["480i"] =			{ "60Hz": 	{ 60: "480i" } }

#		rates["576i"] =			{ "50Hz": 	{ 50: "576i" } }

		rates["480p"] =			{ "60Hz": 	{ 60: "480p" } }

		rates["576p"] =			{ "50Hz": 	{ 50: "576p" } }

		rates["720p"] =			{ "50Hz": 	{ 50: "720p50" },
						  "60Hz": 	{ 60: "720p" } }

		rates["1080i"] =		{ "50Hz":		{ 50: "1080i50" },
						"60Hz":		{ 60: "1080i" } }

		modes["Scart"] = ["PAL", "NTSC"]
	else:
		rates["PAL"] =			{ "50Hz":		{ 50: "pal" },
									"60Hz":		{ 60: "pal60" },
									"multi":	{ 50: "pal", 60: "pal60" } }

		rates["NTSC"] =			{ "60Hz": 	{ 60: "ntsc" } }

		rates["Multi"] =		{ "multi": 	{ 50: "pal", 60: "ntsc" } }

		rates["480i"] =			{ "60Hz": 	{ 60: "480i" } }

		rates["576i"] =			{ "50Hz": 	{ 50: "576i" } }

		rates["480p"] =			{ "60Hz": 	{ 60: "480p" } }

		rates["576p"] =			{ "50Hz": 	{ 50: "576p" } }

		rates["720p"] =			{ "50Hz": 	{ 50: "720p50" },
									"60Hz": 	{ 60: "720p" },
									"multi": 	{ 50: "720p50", 60: "720p" } }

		rates["1080i"] =		{ "50Hz":		{ 50: "1080i50" },
									"60Hz":		{ 60: "1080i" },
									"multi":	{ 50: "1080i50", 60: "1080i" } }

		rates["PC"] = {
			"1024x768": { 60: "1024x768" }, # not possible on DM7025
			"800x600" : { 60: "800x600" },  # also not possible
			"720x480" : { 60: "720x480" },
			"720x576" : { 60: "720x576" },
			"1280x720": { 60: "1280x720" },
			"1280x720 multi": { 50: "1280x720_50", 60: "1280x720" },
			"1920x1080": { 60: "1920x1080"},
			"1920x1080 multi": { 50: "1920x1080", 60: "1920x1080_50" },
			"1280x1024" : { 60: "1280x1024"},
			"1366x768" : { 60: "1366x768"},
			"1366x768 multi" : { 50: "1366x768", 60: "1366x768_50" },
			"1280x768": { 60: "1280x768" },
			"640x480" : { 60: "640x480" }
		}

		modes["Scart"] = ["PAL", "NTSC", "Multi"]
		
	if QBOXHD:
		if not QBOXHD_MINI:
			modes["YPbPr"] = ["720p", "1080i", "576p", "480p"]
		modes["HDMI"] = ["720p", "1080i", "576p", "480p" ]
		widescreen_modes = set(["576p","576i", "720p", "1080i", "1080p"])
	else:
		modes["YPbPr"] = ["720p", "1080i", "576p", "480p", "576i", "480i"]
		modes["HDMI"] = ["720p", "1080i", "576p", "480p", "576i", "480i"]
		modes["HDMI-PC"] = ["PC"]
		widescreen_modes = set(["576i", "576p", "720p", "1080i", "1080p"])

	def getOutputAspect(self):
		ret = (16,9)
		port = config.av.videoport.value
		if port not in config.av.videomode:
			print "current port not available in getOutputAspect!!! force 16:9"
		else:
			mode = config.av.videomode[port].value
			force_widescreen = self.isWidescreenMode(port, mode)
			is_widescreen = force_widescreen or config.av.aspect.value in ("16_9", "16_10")
			is_auto = config.av.aspect.value == "auto"
			
			if is_widescreen:
				if force_widescreen:
					if QBOXHD:
						try:
							aspect_str = open("/proc/stb/vmpeg/0/aspect", "r").read()
							print "aspect_str=%s" % str(aspect_str)
							if aspect_str.find( "0" ) > -1: # 4:3
								ret = (4,3)
							elif aspect_str.find( "1" ) > -1: # 16:9
								ret = (16,9)
						except IOError:
							pass	
					else:
						pass
				else:
					aspect = {"16_9": "16:9", "16_10": "16:10"}[config.av.aspect.value]
					if aspect == "16:10":
						ret = (16,10)
			elif is_auto:
				try:
					aspect_str = open("/proc/stb/vmpeg/0/aspect", "r").read()
					if QBOXHD:
						if aspect_str.find( "0" ) > -1: # 4:3
							ret = (4,3)
						elif  aspect_str.find( "1" ) > -1: # 16:9
							ret = (16,9)
					else:
						if aspect_str == "1": # 4:3
							ret = (4,3)
				except IOError:
					pass
			else:  # 4:3
				ret = (4,3)
		return ret

	def __init__(self):
		self.last_modes_preferred =  [ ]
		self.on_hotplug = CList()
		self.standby = False
		self.current_mode = None
		self.current_port = None

		self.readAvailableModes()

		self.createConfig()
#		self.on_hotplug.append(self.createConfig)

		self.readPreferredModes()

		# take over old AVSwitch component :)
		from Components.AVSwitch import AVSwitch
#		config.av.colorformat.notifiers = [ ]
		config.av.aspectratio.notifiers = [ ]
		config.av.tvsystem.notifiers = [ ]
		config.av.wss.notifiers = [ ]
		AVSwitch.setInput = self.AVSwitchSetInput

		config.av.colorformat_hdmi = ConfigSelection(choices = {"hdmi_rgb": _("RGB"), "hdmi_yuv": _("YUV"), "hdmi_422": _("422")}, default="hdmi_rgb")
		config.av.colorformat_yuv = ConfigSelection(choices = {"yuv": _("YUV")}, default="yuv")
#		config.av.hdmi_audio_source = ConfigSelection(choices = {"pcm": _("PCM"), "spdif": _("SPDIF"), "8ch": _("8Ch"), "none": _("None")}, default="pcm")
		config.av.hdmi_audio_source = ConfigSelection(choices = {"pcm": _("PCM"), "spdif": _("SPDIF")}, default="pcm")

		config.av.colorformat_hdmi.addNotifier(self.setHDMIColor)
		config.av.colorformat_yuv.addNotifier(self.setYUVColor)
		config.av.hdmi_audio_source.addNotifier(self.setHDMIAudioSource)
		config.av.aspect.addNotifier(self.updateAspect)
		config.av.wss.addNotifier(self.updateAspect)
		config.av.policy_169.addNotifier(self.updateAspect)
		config.av.policy_43.addNotifier(self.updateAspect)

		# until we have the hotplug poll socket
#		self.timer = eTimer()
#		self.timer.callback.append(self.readPreferredModes)
#		self.timer.start(1000)
		if QBOXHD:
			config.av.colorformat.addNotifier(self.updateFastblank)

			fp = open('/etc/fb.modes.supp', 'r')
			result=fp.readline().strip()
			fp.close()

			names=result.split('@')

			if len(names) > 2:
				port = names[2]
				#print "port:",port
				mode = names[0]
				#print "mode:",mode
				rate = names[1]
				#print "rates:",rate
				#print ""
				self.saveMode(port,mode,rate)

	def AVSwitchSetInput(self, mode):
		self.standby = mode == "SCART"
		self.updateStandby()


	def readAvailableModes(self):
		try:
			modes = open("/proc/stb/video/videomode_choices").read()[:-1]
		except IOError:
			print "couldn't read available videomodes."
			self.modes_available = [ ]
			return
		self.modes_available = modes.split(' ')
		print self.modes_available

	def readPreferredModes(self):
		try:
			modes = open("/proc/stb/video/videomode_preferred").read()[:-1]
			self.modes_preferred = modes.split(' ')
		except IOError:
			print "reading preferred modes failed, using all modes"
			self.modes_preferred = self.modes_available

#		if self.modes_preferred != self.last_modes_preferred:
#			self.last_modes_preferred = self.modes_preferred
#			print "hotplug on hdmi"
#			self.on_hotplug("HDMI") # must be DVI

	# check if a high-level mode with a given rate is available.
	def isModeAvailable(self, port, mode, rate):
		rate = self.rates[mode][rate]
		for mode in rate.values():
			# DVI modes must be in "modes_preferred"
			if port == "HDMI-PC":
				return True
#				if mode not in self.modes_preferred and not config.av.edid_override.value:
#					print "no, not preferred"
#					return False
			if mode not in self.modes_available:
				#print "isModeAvailable", mode, "false"
				return False
		#print "isModeAvailable", mode, "true"
		return True

	def isWidescreenMode(self, port, mode):
		return mode in self.widescreen_modes

	def setMode(self, port, mode, rate, force = None):
		print "setMode - port:", port, "mode:", mode, "rate:", rate
		# we can ignore "port"
		self.current_mode = mode
		self.current_port = port
		modes = self.rates[mode][rate]

		mode_50 = modes.get(50)
		mode_60 = modes.get(60)
		if mode_50 is None or force == 60:
			mode_50 = mode_60
		if mode_60 is None or force == 50:
			mode_60 = mode_50

		from enigma import fbClass
		if QBOXHD:
			port_res = mode_50+"@"+port
			fbClass.getInstance().SetStretchMode(port_res)
		else:
			fbClass.getInstance().SetStretchMode(mode_50)

		try:
			open("/proc/stb/video/videomode_50hz", "w").write(mode_50)
			open("/proc/stb/video/videomode_60hz", "w").write(mode_60)
		except IOError:
			try:
				# fallback if no possibility to setup 50/60 hz mode
				open("/proc/stb/video/videomode", "w").write(mode_50)
			except IOError:
				print "setting videomode failed."

		try:
			open("/etc/videomode", "w").write(mode_50) # use 50Hz mode (if available) for booting
		except IOError:
			print "writing initial videomode to /etc/videomode failed."
		#call setResolution() with -1,-1 to read the new scrren dimesions without changing the framebuffer resolution
#		from enigma import gFBDC
#		gFBDC.getInstance().setResolution(-1, -1)
		self.updateAspect(None)
		self.updateColor(port)


	def saveMode(self, port, mode, rate):
		print "saveMode", port, mode, rate
		config.av.videoport.value = port
		config.av.videoport.save()
		config.av.videomode[port].value = mode
		config.av.videomode[port].save()
		config.av.videorate[mode].value = rate
		config.av.videorate[mode].save()

	def isPortAvailable(self, port):
		# fixme
		return True

	def isPortUsed(self, port):
		if port == "HDMI":
			self.readPreferredModes()
			return len(self.modes_preferred) != 0
		else:
			return True

	def getPortList(self):
		return [port for port in self.modes if self.isPortAvailable(port)]

	# get a list with all modes, with all rates, for a given port.
	def getModeList(self, port):
		print "getModeList for port", port
		res = [ ]
		for mode in self.modes[port]:
			# list all rates which are completely valid
			rates = [rate for rate in self.rates[mode] if self.isModeAvailable(port, mode, rate)]

			# if at least one rate is ok, add this mode
			if len(rates):
				res.append( (mode, rates) )
		return res

	def createConfig(self, *args):
		# create list of output ports
		portlist = self.getPortList()

		# create list of available modes
		config.av.videoport = ConfigSelection(choices = [(port, _(port)) for port in portlist])
		config.av.videomode = ConfigSubDict()
		config.av.videorate = ConfigSubDict()

		for port in portlist:
			modes = self.getModeList(port)
			if len(modes):
				config.av.videomode[port] = ConfigSelection(choices = [mode for (mode, rates) in modes])
			for (mode, rates) in modes:
				config.av.videorate[mode] = ConfigSelection(choices = rates)

	def setConfiguredMode(self):
		port = config.av.videoport.value
		if port not in config.av.videomode:
			print "current port not available, not setting videomode"
			return

		mode = config.av.videomode[port].value

		if mode not in config.av.videorate:
			print "current mode not available, not setting videomode"
			return

		rate = config.av.videorate[mode].value
		self.setMode(port, mode, rate)
		

	def updateAspect(self, cfgelement):
		# determine aspect = {any,4:3,16:9,16:10}
		# determine policy = {bestfit,letterbox,panscan,nonlinear}

		# based on;
		#   config.av.videoport.value: current video output device
		#     Scart:
		#   config.av.aspect:
		#     4_3:            use policy_169
		#     16_9,16_10:     use policy_43
		#     auto            always "bestfit"
		#   config.av.policy_169
		#     letterbox       use letterbox
		#     panscan         use panscan
		#     scale           use bestfit
		#   config.av.policy_43
		#     pillarbox       use panscan
		#     panscan         use letterbox  ("panscan" is just a bad term, it's inverse-panscan)
		#     nonlinear       use nonlinear
		#     scale           use bestfit

		port = config.av.videoport.value
		if port not in config.av.videomode:
			print "current port not available, not setting videomode"
			return
		mode = config.av.videomode[port].value

		force_widescreen = self.isWidescreenMode(port, mode)
		is_widescreen = force_widescreen or config.av.aspect.value in ["16_9", "16_10"]
		is_auto = config.av.aspect.value == "auto"
		policy2 = "policy" # use main policy
		
		if is_widescreen:
			if force_widescreen:
				aspect = "16:9"
			else:
				aspect = {"16_9": "16:9", "16_10": "16:10"}[config.av.aspect.value]
			policy = {"pillarbox": "letterbox", "panscan": "panscan", "nonlinear": "nonlinear", "scale": "bestfit"}[config.av.policy_43.value]
			policy2 = {"letterbox": "letterbox", "panscan": "panscan", "scale": "bestfit"}[config.av.policy_169.value]
				
		elif is_auto:
			aspect = "any"
			policy = "bestfit"
			if QBOXHD:
				policy2 = "bestfit"
		else:
			aspect = "4:3"
			if QBOXHD:
				policy = {"pillarbox": "letterbox", "panscan": "panscan", "nonlinear": "nonlinear", "scale": "bestfit"}[config.av.policy_43.value]
				policy2 = {"letterbox": "letterbox", "panscan": "panscan", "scale": "bestfit"}[config.av.policy_169.value]
			else:
				policy = {"letterbox": "letterbox", "panscan": "panscan", "scale": "bestfit"}[config.av.policy_169.value]

		if not config.av.wss.value:
			wss = "auto(4:3_off)"
		else:
			wss = "auto"

		print "-> setting aspect, policy, policy2, wss", aspect, policy, policy2, wss
		open("/proc/stb/video/aspect", "w").write(aspect)
		open("/proc/stb/video/policy", "w").write(policy)
		open("/proc/stb/denc/0/wss", "w").write(wss)

		if QBOXHD:
			if self.current_port == "Scart" and port == "Scart":
				if aspect == "16:9" or aspect == "16:10":
					set_scart_aspect(169)	#16_9 or 16_10
				else:
					set_scart_aspect(43)	#4_3 or any

		try:
			open("/proc/stb/video/policy2", "w").write(policy2)
		except IOError:
			pass

		self.updateSlowblank()
		self.updateFastblank()

	def updateSlowblank(self):
		if self.standby:
			from Components.SystemInfo import SystemInfo
			if SystemInfo["ScartSwitch"]:
				input = "scart"
				sb = "vcr"
			else:
				input = "off"
				sb = "0"
		else:
			input = "encoder"
			sb = "auto"

		open("/proc/stb/avs/0/sb", "w").write(sb)
		open("/proc/stb/avs/0/input", "w").write(input)

	def updateStandby(self):
		self.updateSlowblank()
		self.updateFastblank()

	def updateFastblank(self, *args):
		if self.standby:
			from Components.SystemInfo import SystemInfo
			if SystemInfo["ScartSwitch"]:
				fb = "vcr"
			else:
				fb = "low"
		else:
			if self.current_port == "Scart" and config.av.colorformat.value == "rgb":
				fb = "high"
			else:
				fb = "low"
			open("/proc/stb/avs/0/fb", "w").write(fb)

	def setHDMIColor(self, configElement):
		map = {"hdmi_rgb": 0, "hdmi_yuv": 1, "hdmi_422": 2}
		open("/proc/stb/avs/0/colorformat", "w").write(configElement.value)

	def setYUVColor(self, configElement):
		map = {"yuv": 0}
		open("/proc/stb/avs/0/colorformat", "w").write(configElement.value)

	def setHDMIAudioSource(self, configElement):
		open("/proc/stb/hdmi/audio_source", "w").write(configElement.value)
		
	def updateColor(self, port):
		print "updateColor: ", port
		if port == "HDMI":
			self.setHDMIColor(config.av.colorformat_hdmi)
		elif port == "Component":
			self.setYUVColor(config.av.colorformat_yuv)
		elif port == "Scart":
			map = {"cvbs": 0, "rgb": 1, "svideo": 2, "yuv": 3}
			from enigma import eAVSwitch
			eAVSwitch.getInstance().setColorFormat(map[config.av.colorformat.value])


config.av.edid_override = ConfigYesNo(default = False)
video_hw = VideoHardware()
video_hw.setConfiguredMode()
