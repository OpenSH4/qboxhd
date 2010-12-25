from Components.Converter.Converter import Converter
from enigma import iServiceInformation, iPlayableService
from Components.Element import cached
from Components.config import config

class ServiceInfo(Converter, object):
	HAS_TELETEXT = 0
	IS_MULTICHANNEL = 1
	IS_CRYPTED = 2
	IS_WIDESCREEN = 3
	SUBSERVICES_AVAILABLE = 4
	XRES = 5
	YRES = 6
	IS_HD = 7
	IS_FULL_HD = 8
	
	def __init__(self, type):
		Converter.__init__(self, type)
		self.type = {
				"HasTelext": self.HAS_TELETEXT,
				"IsMultichannel": self.IS_MULTICHANNEL,
				"IsCrypted": self.IS_CRYPTED,
				"IsWidescreen": self.IS_WIDESCREEN,
				"SubservicesAvailable": self.SUBSERVICES_AVAILABLE,
				"VideoWidth": self.XRES,
				"VideoHeight": self.YRES,
				"IsHD": self.IS_HD,
				"IsFullHD": self.IS_FULL_HD,
			}[type]

		self.interesting_events = {
				self.HAS_TELETEXT: [iPlayableService.evUpdatedInfo],
				self.IS_MULTICHANNEL: [iPlayableService.evUpdatedInfo],
				self.IS_CRYPTED: [iPlayableService.evUpdatedInfo],
				self.IS_WIDESCREEN: [iPlayableService.evVideoSizeChanged],
				self.SUBSERVICES_AVAILABLE: [iPlayableService.evUpdatedEventInfo],
				self.XRES: [iPlayableService.evVideoSizeChanged],
				self.YRES: [iPlayableService.evVideoSizeChanged],
				self.IS_HD: [iPlayableService.evVideoSizeChanged],
				self.IS_FULL_HD: [iPlayableService.evVideoSizeChanged],
			}[self.type]

	def getServiceInfoString(self, info, what):
		v = info.getInfo(what)
		if v == -1:
			return "N/A"
		if v == -2:
			return info.getInfoString(what)
		return "%d" % v

	@cached
	def getBoolean(self):
		service = self.source.service
		info = service and service.info()
		if not info:
			return False
		
		if self.type == self.HAS_TELETEXT:
			tpid = info.getInfo(iServiceInformation.sTXTPID)
			return tpid != -1
		elif self.type == self.IS_MULTICHANNEL:
			# FIXME. but currently iAudioTrackInfo doesn't provide more information.
			audio = service.audioTracks()
			if audio:
				n = audio.getNumberOfTracks()
				for x in range(n):
					i = audio.getTrackInfo(x)
					description = i.getDescription();
					if description.find("AC3") != -1 or description.find("DTS") != -1:
						return True
			return False
		elif self.type == self.IS_CRYPTED:
			if "simple" != config.usage.setup_level.value: 
				return info.getInfo(iServiceInformation.sIsCrypted) == 1
			else:
				return False
			
		elif self.type == self.IS_WIDESCREEN:
			return info.getInfo(iServiceInformation.sAspect) in [3, 4, 7, 8, 0xB, 0xC, 0xF, 0x10]
		elif self.type == self.SUBSERVICES_AVAILABLE:
			subservices = service.subServices()
			return subservices and subservices.getNumberOfSubservices() > 0
		elif self.type == self.IS_HD:
			yresol = info.getInfo(iServiceInformation.sVideoHeight)
			if (yresol > 700):
				return True
			else:
				return False
		elif ((self.type == self.IS_FULL_HD) and (config.usage.setup_level.value != "simple")):
			yresol = info.getInfo(iServiceInformation.sVideoHeight)
			if (yresol > 1079):
				return True
			else:
				return False
		return False

	boolean = property(getBoolean)
	
	@cached
	def getText(self):
		service = self.source.service
		info = service and service.info()
		if not info:
			return ""

		if self.type == self.XRES:
			return self.getServiceInfoString(info, iServiceInformation.sVideoWidth)
		if ((self.type == self.YRES)  and (config.usage.setup_level.value != "simple")):
			yresol = info.getInfo(iServiceInformation.sVideoHeight)
			if (yresol > 1080):
				return "1080"
			else:
				return self.getServiceInfoString(info, iServiceInformation.sVideoHeight)
		return ""

	text = property(getText)

	@cached
	def getValue(self):
		service = self.source.service
		info = service and service.info()
		if not info:
			return -1

		if self.type == self.XRES:
			return info.getInfo(iServiceInformation.sVideoWidth)
		if self.type == self.YRES:
			return info.getInfo(iServiceInformation.sVideoHeight)


		return -1

	value = property(getValue)


	boolean = property(getBoolean)

	def changed(self, what):
		if what[0] != self.CHANGED_SPECIFIC or what[1] in self.interesting_events:
			Converter.changed(self, what)
