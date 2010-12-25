from qboxhd import QBOXHD
from config import config, ConfigSubsection, ConfigSlider, ConfigYesNo, ConfigNothing
from enigma import eDBoxLCD
from Components.SystemInfo import SystemInfo

class LCD:
	def __init__(self):
		pass

	def setBright(self, value):
		if value > 63:
			value = 63

		if value < 0:
			value = 0

		eDBoxLCD.getInstance().setLCDBrightness(value)

def InitLcd():
	detected = eDBoxLCD.getInstance().detected()
	SystemInfo["Display"] = detected
	config.lcd = ConfigSubsection();
	if detected:
		def setLCDbright(configElement):
			ilcd.setBright(configElement.value);

		ilcd = LCD()

		config.lcd.standby = ConfigSlider(default=0, limits=(0, 63))
		config.lcd.standby.addNotifier(setLCDbright);
		config.lcd.standby.apply = lambda : setLCDbright(config.lcd.standby)

		config.lcd.bright = ConfigSlider(default=40, limits=(0, 63))
		config.lcd.bright.addNotifier(setLCDbright);
		config.lcd.bright.apply = lambda : setLCDbright(config.lcd.bright)
		config.lcd.bright.callNotifiersOnSaveAndCancel = True
		
	else:
		def doNothing():
			pass
		config.lcd.bright = ConfigNothing()
		config.lcd.standby = ConfigNothing()
		config.lcd.bright.apply = lambda : doNothing()
		config.lcd.standby.apply = lambda : doNothing()
