from qboxhd import QBOXHD, QBOXHD_MINI
from config import config, ConfigSubsection, ConfigOnOff, ConfigSlider, configfile
from enigma import eDelayerManager
from Components.NimManager import nimmanager
from Components.SystemInfo import SystemInfo


class DELAYER:
	def __init__(self):
		adapter = 0
		for nim in nimmanager.nimActivesList():
			eDelayerManager.getInstance().addDelayerAdapter( adapter, 0 )
			adapter += 1


	def setDelayTime( self, adapter, delay_time ):
		delayer_adapter = eDelayerManager.getInstance().getDelayerAdapter(adapter)
		if delayer_adapter is None:
			return False
		
		return delayer_adapter.setDelayTime( delay_time )
	
	
	def isDelayerActive(self, adapter):
		delayer_adapter = eDelayerManager.getInstance().getDelayerAdapter(adapter)
		if delayer_adapter is None:
			return False
		
		return delayer_adapter.isActive()
	
	
	def enableDelayerForAllAdapter(self):
		config.delayer.delay_time.value = 50000
		config.delayer.delay_time.save()
		configfile.save()
		return True
	
	
	def disableDelayerForAllAdapter(self):
		config.delayer.delay_time.value = 0
		config.delayer.delay_time.save()
		configfile.save()
		return True
			
			
idelayer = DELAYER()


def InitDelayer():
	
	def setDelayTime(configElement):
		print "[InitDelayer] Setting DelayTime to %d" % configElement.value
		adapter = 0 
		for nim in nimmanager.nimActivesList():
			idelayer.setDelayTime(adapter, configElement.value)
			adapter += 1

	config.delayer = ConfigSubsection()
	
	config.delayer.delay_time = ConfigSlider(default=0, increment=1000, limits=(0, 50000))
	config.delayer.delay_time.addNotifier(setDelayTime)
	config.delayer.delay_time.apply = lambda : setDelayTime(config.delayer.delay_time)
			
