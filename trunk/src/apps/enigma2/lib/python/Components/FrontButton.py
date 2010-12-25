from qboxhd import QBOXHD
from config import config, ConfigSubsection, ConfigSlider, ConfigYesNo, ConfigNothing, ConfigOnOff, ConfigEnableDisable
from enigma import eQBOXFrontButton
from Components.SystemInfo import SystemInfo
from Components.config import configfile

class FrontButton:
	def __init__(self):
		self.R = 0
		self.G = 0
		self.B = 0
		self.R_standby = 0
		self.G_standby = 0
		self.B_standby = 0
		self.R_deepstandby = 0
		self.G_deepstandby = 0
		self.B_deepstandby = 0
			
			
	def setFrontButtonLed(self, R, G, B):
		if R > 0:
			self.R = R
		else:
			self.R = 0
			
		if G > 0:
			self.G = G
		else:
			self.G = 0
			
		if B > 0:
			self.B = B
		else:
			self.B = 0
		eQBOXFrontButton.getInstance().setLed(self.R, self.G, self.B)
			
			
	def setRedFrontButtonLed(self, value):
		if value > 0:
			self.R = value
		else:
			self.R = 0
		eQBOXFrontButton.getInstance().setLed(self.R, self.G, self.B)
			
			
	def setGreenFrontButtonLed(self, value):
		if value > 0:
			self.G = value
		else:
			self.G = 0
		eQBOXFrontButton.getInstance().setLed(self.R, self.G, self.B)
		
		
	def setBlueFrontButtonLed(self, value):
		if value > 0:
			self.B = value
		else:
			self.B = 0
		eQBOXFrontButton.getInstance().setLed(self.R, self.G, self.B)
		
	# STAND-BY
	def setStandbyRedFrontButtonLed(self, value):
		if value > 0:
			self.R_standby = value
		else:
			self.R_standby = 0
		eQBOXFrontButton.getInstance().setLed(self.R_standby, self.G_standby, self.B_standby)
			
			
	def setStandbyGreenFrontButtonLed(self, value):
		if value > 0:
			self.G_standby = value
		else:
			self.G_standby = 0
		eQBOXFrontButton.getInstance().setLed(self.R_standby, self.G_standby, self.B_standby)
		
		
	def setStandbyBlueFrontButtonLed(self, value):
		if value > 0:
			self.B_standby = value
		else:
			self.B_standby = 0
		eQBOXFrontButton.getInstance().setLed(self.R_standby, self.G_standby, self.B_standby)
		
		
	def setStandbyFrontButtonLed( self, R, G, B ):
		if R > 0:
			self.R_standby = R
		else:
			self.R_standby = 0
			
		if G > 0:
			self.G_standby = G
		else:
			self.G_standby = 0
			
		if B > 0:
			self.B_standby = B
		else:
			self.B_standby = 0
		eQBOXFrontButton.getInstance().setLed(self.R_standby, self.G_standby, self.B_standby)
		
		
	def enterStandby(self):
		print "[FrontButton] enter Standby.\n"
		eQBOXFrontButton.getInstance().enterStandby()
		
		self.setStandbyFrontButtonLed ( self.R_standby, self.G_standby, self.B_standby )
			
		
	def leaveStandby(self):
		print "[FrontButton] leave Standby.\n"
		eQBOXFrontButton.getInstance().leaveStandby()
		
		self.setFrontButtonLed ( self.R, self.G, self.B )
		
	# DEEP STAND-BY
	def setDeepStandbyRedFrontButtonLed(self, value):
		if value > 0:
			self.R_deepstandby = value
		else:
			self.R_deepstandby = 0
		eQBOXFrontButton.getInstance().setLed(self.R_deepstandby, self.G_deepstandby, self.B_deepstandby)
			
			
	def setDeepStandbyGreenFrontButtonLed(self, value):
		if value > 0:
			self.G_deepstandby = value
		else:
			self.G_deepstandby = 0
		eQBOXFrontButton.getInstance().setLed(self.R_deepstandby, self.G_deepstandby, self.B_deepstandby)
		
		
	def setDeepStandbyBlueFrontButtonLed(self, value):
		if value > 0:
			self.B_deepstandby = value
		else:
			self.B_deepstandby = 0
		eQBOXFrontButton.getInstance().setLed(self.R_deepstandby, self.G_deepstandby, self.B_deepstandby)
		
		
	def setDeepStandbyFrontButtonLed( self, R, G, B ):
		if R > 0:
			self.R_deepstandby = R
		else:
			self.R_deepstandby = 0
			
		if G > 0:
			self.G_deepstandby = G
		else:
			self.G_deepstandby = 0
			
		if B > 0:
			self.B_deepstandby = B
		else:
			self.B_deepstandby = 0
		eQBOXFrontButton.getInstance().setLed(self.R_deepstandby, self.G_deepstandby, self.B_deepstandby)
		
		
	def enterDeepStandby(self):
		print "[FrontButton] enter DEEP Standby.\n"
		eQBOXFrontButton.getInstance().enterDeepStandby()
		
		self.setDeepStandbyFrontButtonLed ( self.R_deepstandby, self.G_deepstandby, self.B_deepstandby )
		
		
ifrontbutton = FrontButton()

			
def InitFrontButton():
	detected = eQBOXFrontButton.getInstance().detected()
	SystemInfo["FrontButton"] = detected
	
	def setRedFrontButton(configElement):
		ifrontbutton.setRedFrontButtonLed(configElement.value)

	def setGreenFrontButton(configElement):
		ifrontbutton.setGreenFrontButtonLed(configElement.value)


	def setBlueFrontButton(configElement):
		ifrontbutton.setBlueFrontButtonLed(configElement.value)


	def setStandbyRedFrontButton(configElement):
		ifrontbutton.setStandbyRedFrontButtonLed( configElement.value )
		
		
	def setStandbyGreenFrontButton(configElement):
		ifrontbutton.setStandbyGreenFrontButtonLed( configElement.value )
		
		
	def setStandbyBlueFrontButton(configElement):
		ifrontbutton.setStandbyBlueFrontButtonLed( configElement.value )
		
		
	def setDeepStandbyRedFrontButton(configElement):
		ifrontbutton.setDeepStandbyRedFrontButtonLed( configElement.value )
		
		
	def setDeepStandbyGreenFrontButton(configElement):
		ifrontbutton.setDeepStandbyGreenFrontButtonLed( configElement.value )
		
		
	def setDeepStandbyBlueFrontButton(configElement):
		ifrontbutton.setDeepStandbyBlueFrontButtonLed( configElement.value )
		
		
	def saveAndExitFromFrontButtonPlugin(configElement):
		ifrontbutton.setFrontButtonLed( config.frontbutton.Red.value, config.frontbutton.Green.value, config.frontbutton.Blue.value )
		configfile.save()
	
	def reloadAndExitFromFrontButtonPlugin(configElement):
		ifrontbutton.setFrontButtonLed( config.frontbutton.Red.value, config.frontbutton.Green.value, config.frontbutton.Blue.value )
			

	config.frontbutton = ConfigSubsection()
	config.frontbutton.Red = ConfigSlider(default=16, increment=4, limits=(0, 31))
	config.frontbutton.Green = ConfigSlider(default=16, increment=4, limits=(0, 31))
	config.frontbutton.Blue = ConfigSlider(default=16, increment=4, limits=(0, 31))
	
	config.frontbutton_standby = ConfigSubsection()
	config.frontbutton_standby.Red = ConfigSlider(default=16, increment=4, limits=(0, 31))
	config.frontbutton_standby.Green = ConfigSlider(default=16, increment=4, limits=(0, 31))
	config.frontbutton_standby.Blue = ConfigSlider(default=16, increment=4, limits=(0, 31))
			
	config.frontbutton_deepstandby = ConfigSubsection()
	config.frontbutton_deepstandby.Red = ConfigSlider(default=16, increment=4, limits=(0, 31))
	config.frontbutton_deepstandby.Green = ConfigSlider(default=16, increment=4, limits=(0, 31))
	config.frontbutton_deepstandby.Blue = ConfigSlider(default=16, increment=4, limits=(0, 31))
			
	config.frontbutton.Red.addNotifier(setRedFrontButton)
	config.frontbutton.Red.apply = lambda : setRedFrontButton(config.frontbutton.Red)
	config.frontbutton.Red.addNotifierLoad(reloadAndExitFromFrontButtonPlugin)
	config.frontbutton.Red.addNotifierSave(saveAndExitFromFrontButtonPlugin)
	
	config.frontbutton.Blue.addNotifier(setBlueFrontButton)
	config.frontbutton.Blue.apply = lambda : setBlueFrontButton(config.frontbutton.Blue)
	config.frontbutton.Blue.addNotifierLoad(reloadAndExitFromFrontButtonPlugin)
	config.frontbutton.Blue.addNotifierSave(saveAndExitFromFrontButtonPlugin)
	
	config.frontbutton.Green.addNotifier(setGreenFrontButton)
	config.frontbutton.Green.apply = lambda : setGreenFrontButton(config.frontbutton.Green)
	config.frontbutton.Green.addNotifierLoad(reloadAndExitFromFrontButtonPlugin)
	config.frontbutton.Green.addNotifierSave(saveAndExitFromFrontButtonPlugin)
	
	
	#STANDBY
	config.frontbutton_standby.Red.addNotifier(setStandbyRedFrontButton)
	config.frontbutton_standby.Red.apply = lambda : setStandbyRedFrontButton(config.frontbutton_standby.Red)
	config.frontbutton_standby.Red.addNotifierLoad(reloadAndExitFromFrontButtonPlugin)
	config.frontbutton_standby.Red.addNotifierSave(saveAndExitFromFrontButtonPlugin)
	
	config.frontbutton_standby.Green.addNotifier(setStandbyGreenFrontButton)
	config.frontbutton_standby.Green.apply = lambda : setStandbyGreenFrontButton(config.frontbutton_standby.Green)
	config.frontbutton_standby.Green.addNotifierLoad(reloadAndExitFromFrontButtonPlugin)
	config.frontbutton_standby.Green.addNotifierSave(saveAndExitFromFrontButtonPlugin)
	
	config.frontbutton_standby.Blue.addNotifier(setStandbyBlueFrontButton)
	config.frontbutton_standby.Blue.apply = lambda : setStandbyBlueFrontButton(config.frontbutton_standby.Blue)
	config.frontbutton_standby.Blue.addNotifierLoad(reloadAndExitFromFrontButtonPlugin)
	config.frontbutton_standby.Blue.addNotifierSave(saveAndExitFromFrontButtonPlugin)
	
	#DEEP STANDBY
	config.frontbutton_deepstandby.Red.addNotifier(setDeepStandbyRedFrontButton)
	config.frontbutton_deepstandby.Red.apply = lambda : setDeepStandbyRedFrontButton(config.frontbutton_deepstandby.Red)
	config.frontbutton_deepstandby.Red.addNotifierLoad(reloadAndExitFromFrontButtonPlugin)
	config.frontbutton_deepstandby.Red.addNotifierSave(saveAndExitFromFrontButtonPlugin)
	
	config.frontbutton_deepstandby.Green.addNotifier(setDeepStandbyGreenFrontButton)
	config.frontbutton_deepstandby.Green.apply = lambda : setDeepStandbyGreenFrontButton(config.frontbutton_deepstandby.Green)
	config.frontbutton_deepstandby.Green.addNotifierLoad(reloadAndExitFromFrontButtonPlugin)
	config.frontbutton_deepstandby.Green.addNotifierSave(saveAndExitFromFrontButtonPlugin)
	
	config.frontbutton_deepstandby.Blue.addNotifier(setDeepStandbyBlueFrontButton)
	config.frontbutton_deepstandby.Blue.apply = lambda : setDeepStandbyBlueFrontButton(config.frontbutton_deepstandby.Blue)
	config.frontbutton_deepstandby.Blue.addNotifierLoad(reloadAndExitFromFrontButtonPlugin)
	config.frontbutton_deepstandby.Blue.addNotifierSave(saveAndExitFromFrontButtonPlugin)	