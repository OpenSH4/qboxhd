from Components.Sources.Source import Source
from enigma import eEPGCache
import _enigma
import new

class PowerState(Source):
	def __init__(self, session):
		self.cmd = None
		self.session = session
		Source.__init__(self)

	def handleCommand(self, cmd):
		self.cmd = cmd

	def getStandby(self):
		from Screens.Standby import inStandby
		if inStandby == None:
			return "false"
		else:
			return "true"

	def getText(self):
		if self.cmd == "" or self.cmd is None:
			return self.getStandby()

		#-1: get current state
		# 0: toggle standby
		# 1: poweroff/deepstandby
		# 2: rebootdreambox
		# 3: rebootenigma
		# 10 : reload EPGCache /hdd/epg.dat
		try:
			type = int(self.cmd)
			if type == -1:
				return self.getStandby()

			elif type == 0:
				print "[PowerState.py] Standby 0"
				from Screens.Standby import inStandby
				if inStandby == None:
					from Screens.Standby import Standby
					self.session.open(Standby)
					return "true"
				else:
					inStandby.Power()
					return "false"

			elif 0 < type < 4:
				print "[PowerState.py] TryQuitMainloop"
				from Screens.Standby import TryQuitMainloop
				self.session.open(TryQuitMainloop, type)
				return "true"

			elif type == 10:
				print "[PowerState.py] Reload EPG /hdd/epg.dat 10"
				try:
					self.epgpatch = new.instancemethod(_enigma.eEPGCache_load,None,eEPGCache)
					print "[PowerState.py] patch epgcache.load() found"
					self.epgpatch(eEPGCache.getInstance())
					print "[PowerState.py] EPGCache reloaded"
					return "true"
				except Exception, e:
					self.epgpatch = None
					print "[PowerState.py] patch epgcache.load() not found"
					return "false"
				return "error"

			else:
				print "[PowerState.py] cmd unknown" % type
				return "error"
		except ValueError:
			return "error"

	text = property(getText)
