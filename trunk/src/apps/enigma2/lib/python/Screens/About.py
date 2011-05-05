from qboxhd import QBOXHD, QBOXHD_MINI
from Screen import Screen
from Components.ActionMap import ActionMap
from Components.Sources.StaticText import StaticText
from Components.Harddisk import harddiskmanager
from Components.NimManager import nimmanager
from Components.About import about
if QBOXHD:
	from Components.iDevsDevice import idevsdevicemanager
	from Screens.MessageBox import MessageBox
	from enigma import eTimer


from Tools.DreamboxHardware import getFPVersion

class About(Screen):
	def __init__(self, session):
		Screen.__init__(self, session)
		
		if QBOXHD:
			self.fist_time = True
			if QBOXHD_MINI:
				self["Title"] = StaticText(_("Technical Information about QBox Mini"))
			else:
				self["Title"] = StaticText(_("Technical Information about QBoxHD"))
			
			self["EnigmaVersion"] = StaticText(_("Enigma: unknown"))
			self["ImageVersion"] = StaticText(("Filesystem: unknown"))
			self["TunerHeader"] = StaticText(_("Detected NIMs:"))
			self["FPVersion"] = StaticText(_("FP Info: unknown"))

			for count in (0, 1, 2, 3):
				self["Tuner" + str(count)] = StaticText("")
	
			self["HDDHeader"] = StaticText(_("Detected HDD:"))
			self["hddA"] = StaticText(_("none"))
				
			self["KernelInfo"] = StaticText(_("Kernel: unknown"))
			self["RamfsInfo"] = StaticText(_("Ramfs: unknown"))
			self["BSinfo"] = StaticText(_("BS Info: unknown"))
			self["IPInfo"] = StaticText(_("IP: unknown"))
			self["MouseInfo"] = StaticText(_("unknown"))
			self["KeyboardInfo"] = StaticText(_("unknown"))
			self["IPhoneInfo"] = StaticText(_("No iDevs connected"))
			
			self.onShown.append(self.onCreate)
		else:
			self["EnigmaVersion"] = StaticText("Enigma: " + about.getEnigmaVersionString())
			self["ImageVersion"] = StaticText("Filesystem: " + about.getImageVersionString())
	
			self["TunerHeader"] = StaticText(_("Detected NIMs:"))
	
			fp_version = getFPVersion()
			if fp_version is not None:
				fp_version = _("FP Info: %d") % fp_version
			else:
				fp_version = _("FP Info: unknown")
	
			self["FPVersion"] = StaticText(fp_version)
	
			nims = nimmanager.nimList()
			for count in (0, 1, 2, 3):
				if count < len(nims):
					self["Tuner" + str(count)] = StaticText(nims[count])
				else:
					self["Tuner" + str(count)] = StaticText("")
	
			self["HDDHeader"] = StaticText(_("Detected HDD:"))
			hddlist = harddiskmanager.HDDList()
			hdd = hddlist and hddlist[0][1] or None
			if hdd is not None and hdd.model() != "":
				self["hddA"] = StaticText(_("%s\n(%s, %d MB free)") % (hdd.model(), hdd.capacity(),hdd.free()))
			else:
				self["hddA"] = StaticText(_("none"))
				
		self["actions"] = ActionMap(["SetupActions", "ColorActions"], 
			{
				"cancel": self.close,
				"ok": self.close,
				"green": self.showTranslationInfo
			})
			
	def createAbout(self):
		self.Timer.stop()
		
		self["EnigmaVersion"].setText("Enigma: " + about.getEnigmaVersionString())
		self["ImageVersion"].setText("Filesystem: " + about.getFilesystemInfo())

		fp_version = getFPVersion()
		if fp_version is not None:
			fp_version = _("FP Info: %d") % fp_version
		else:
			fp_version = _("FP Info: unknown")

		self["FPVersion"].setText(fp_version)

		nims = nimmanager.nimList()
		for count in (0, 1, 2, 3):
			if count < len(nims):
				self["Tuner" + str(count)].setText(nims[count])
			else:
				self["Tuner" + str(count)].setText("")

		hddlist = harddiskmanager.HDDList()
		hdd = hddlist and hddlist[0][1] or None
		if hdd is not None and hdd.model() != "":
			self["hddA"].setText(_("%s\n(%s, %d MB free)") % (hdd.model(), hdd.capacity(),hdd.free()))
		else:
			self["hddA"].setText(_("none"))
			
		#Kernel Info
		self["KernelInfo"].setText("Kernel: " + about.getKernelInfo())
		#Ramfs Info
		self["RamfsInfo"].setText("Ramfs: " + about.getRamfsInfo())
		#BitStream Info
		self["BSinfo"].setText("BS Info: " + about.getBitstreamInfo())
		# IP Information
		self["IPInfo"].setText("IP: " + about.getEth0IpAddress())
		# Mouse Info	
		self["MouseInfo"].setText(about.getMouseInfo())
		# Keyboard Info
		self["KeyboardInfo"].setText(about.getKeyBoardInfo())
		
		iphonedescr = _("No iDevs connected")
		if (idevsdevicemanager.getiPodCount() > 0 or idevsdevicemanager.getiPhoneCount() > 0 or idevsdevicemanager.getiPadCount() > 0):
			iphonedescr = _("%d iDevs connected") % (idevsdevicemanager.getiPodCount() + idevsdevicemanager.getiPhoneCount() + idevsdevicemanager.getiPadCount())
			
		self["IPhoneInfo"].setText(iphonedescr)
		
		self.waitMsg.close(True)

	def onCreate(self):
		if not self.fist_time:
			return
		else:
			self.fist_time = False
			
			self.waitMsg = self.session.open( MessageBox, _("Getting system information..."), type = MessageBox.TYPE_INFO, enable_input = False)
			
			self.Timer = eTimer()
			self.Timer.callback.append(self.createAbout)
			self.Timer.start(100, True)
			


	def showTranslationInfo(self):
		self.session.open(TranslationInfo)

class TranslationInfo(Screen):
	def __init__(self, session):
		Screen.__init__(self, session)
		# don't remove the string out of the _(), or it can't be "translated" anymore.

		# TRANSLATORS: Add here whatever should be shown in the "translator" about screen, up to 6 lines (use \n for newline)
		info = _("TRANSLATOR_INFO")

		if info == "TRANSLATOR_INFO":
			info = "(N/A)"

		infolines = _("").split("\n")
		infomap = {}
		for x in infolines:
			l = x.split(': ')
			if len(l) != 2:
				continue
			(type, value) = l
			infomap[type] = value
		print infomap

		self["TranslationInfo"] = StaticText(info)

		translator_name = infomap.get("Language-Team", "none")
		if translator_name == "none":
			translator_name = infomap.get("Last-Translator", "")

		self["TranslatorName"] = StaticText(translator_name)

		self["actions"] = ActionMap(["SetupActions"], 
			{
				"cancel": self.close,
				"ok": self.close,
			})
