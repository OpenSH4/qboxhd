from qboxhd import QBOXHD
from Source import Source
from Components.Element import cached
from enigma import eDelayerManager
from Components.NimManager import nimmanager

class DelayerState(Source):
	def __init__(self, session):
		Source.__init__(self)
		self.session = session
		self.initDelayerStatus()
		eDelayerManager.getInstance().delayerStateChanged.get().append(self.delayerStateChanged)
		
	def initDelayerStatus(self):
		adapter = 0
		self.delayer_active = []
		for nim in nimmanager.nimActivesList():
			self.delayer_active.append( False )
			adapter += 1

	def delayerStateChanged(self, adapter, event):
		if event == eDelayerManager.evDelayActive:
			self.delayer_active[adapter] = True
		else:
			self.delayer_active[adapter] = False
			
		print "[DELAYER STATECHANGED] adapter=%d state=%s\n" % (adapter, str(self.delayer_active[adapter]))
		
		self.changed((self.CHANGED_ALL,))

	def destroy(self):
		eDelayerManager.getInstance().delayerStateChanged.get().remove(self.delayerStateChanged)
		Source.destroy(self)

	@cached
	def getBoolean(self):
		service = self.session.nav.getCurrentService()
		if service is None:
			del service
			return False
		else:
			feinfo = None
			frontendData = None
			if service is not None:
				feinfo = service.frontendInfo()
				frontendData = feinfo and feinfo.getAll(True)
				adapter = frontendData["adapter_number"]
				
				del frontendData
				del feinfo
				del service
					
				return self.delayer_active[adapter] and True or False
			else:
				del service
				return False
			
	boolean = property(getBoolean)

	@cached
	def getValue(self):
		service = self.session.nav.getCurrentService()
		if service is None:
			del service
			return False
		else:
			feinfo = None
			frontendData = None
			if service is not None:
				feinfo = service.frontendInfo()
				frontendData = feinfo and feinfo.getAll(True)
				adapter = frontendData["adapter_number"]
				
				delayer_value = self.delayer_active[adapter]
				del frontendData
				del feinfo
				del service
			else:
				delayer_value = False
		
		return delayer_value
	value = property(getValue)
