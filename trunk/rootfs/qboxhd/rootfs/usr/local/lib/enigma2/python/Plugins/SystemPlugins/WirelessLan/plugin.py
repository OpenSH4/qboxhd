# for localized messages
from qboxhd import QBOXHD, QBOXHD_MINI
from __init__ import _

from enigma import eTimer
from Screens.Screen import Screen
from Components.ActionMap import ActionMap, NumberActionMap
from Components.Pixmap import Pixmap
from Components.Label import Label
from Components.MenuList import MenuList
from Components.config import config, getConfigListEntry, ConfigYesNo, NoSave, ConfigSubsection, ConfigText, ConfigSelection, ConfigPassword
from Components.ConfigList import ConfigListScreen
from Components.Network import iNetwork
from Components.Console import Console
from Plugins.Plugin import PluginDescriptor
from os import system, path as os_path, listdir
from Wlan import * #Wlan, WlanList, iNIC_MII_WiFi_Module
from Wlan import Status, iStatus

if QBOXHD:
	plugin_path = "/usr/local/lib/enigma2/python/Plugins/SystemPlugins/WirelessLan"
else:
	plugin_path = "/usr/lib/enigma2/python/Plugins/SystemPlugins/WirelessLan"

class WlanStatus(Screen):
	skin = """
	<screen position="center,center" size="550,300" title="Wireless Network State" >
		<widget name="LabelBSSID" position="10,10" size="250,25" valign="left" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="LabelESSID" position="10,38" size="250,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="LabelQuality" position="10,66" size="250,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="LabelSignal" position="10,94" size="250,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="LabelBitrate" position="10,122" size="250,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="LabelEnc" position="10,150" size="250,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="LabelFrequency" position="10,182" size="250,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		
		
		<widget name="BSSID" position="320,10" size="200,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="ESSID" position="320,38" size="200,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="quality" position="320,66" size="200,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="signal" position="320,94" size="200,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="bitrate" position="320,122" size="200,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="enc" position="320,150" size="200,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="frequency" position="320,182" size="200,25" valign="center" font="Regular;20" transparent="1" foregroundColor="#FFFFFF" />
		
		<widget name="BottomBG" pixmap="skin_default/bottombar.png" position="5,210" size="540,120" zPosition="1" transparent="1" alphatest="on" />
		<widget name="IFtext" position="20,225" size="100,21" zPosition="10" font="Regular;19" transparent="1" />
		<widget name="IF" position="110,225" size="300,21" zPosition="10" font="Regular;19" transparent="1" />
		<widget name="Statustext" position="20,250" size="115,21" zPosition="10" font="Regular;19" transparent="1"/>
		<widget name="statuspic_inactive" pixmap="skin_default/buttons/button_green_off.png" position="120,250" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		<widget name="statuspic_active" pixmap="skin_default/buttons/button_green.png" position="120,250" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		<widget name="ButtonRedtext" position="430,225" size="120,21" zPosition="10" font="Regular;21" transparent="1" />
		<widget name="ButtonRed" pixmap="skin_default/buttons/button_red.png" position="410,225" zPosition="10" size="15,16" transparent="1" alphatest="on" />

	</screen>
	"""
	
	
	def __init__(self, session, iface):
		
		Screen.__init__(self, session)
		
		self.session = session
		
		if (iface == 'eth0'):
			self.iface = 'ra0'
		else:
			self.iface = iface

		self.skin = WlanStatus.skin
				    
		self["LabelBSSID"] = Label(_('Accesspoint:'))
		self["LabelESSID"] = Label(_('SSID:'))
		self["LabelQuality"] = Label(_('Link Quality:'))
		self["LabelSignal"] = Label(_('Signal Strength:'))
		self["LabelBitrate"] = Label(_('Bitrate:'))
		self["LabelEnc"] = Label(_('Encryption:'))
		self["LabelFrequency"] = Label(_('Frequency:'))
			
		self["BSSID"] = Label()
		self["ESSID"] = Label()
		self["quality"] = Label()
		self["signal"] = Label()
		self["bitrate"] = Label()
		self["enc"] = Label()
		self["frequency"] = Label()

		self["IFtext"] = Label()
		self["IF"] = Label()
		self["Statustext"] = Label()
		self["statuspic_active"] = Pixmap()
		self["statuspic_active"].hide()
		self["statuspic_inactive"] = Pixmap()
		self["statuspic_inactive"].hide()
		self["BottomBG"] = Pixmap()
		self["ButtonRed"] = Pixmap()
		self["ButtonRedtext"] = Label(_("Close"))

		self.resetList()
		self.updateStatusbar()
		
		self["actions"] = NumberActionMap(["WizardActions", "InputActions", "EPGSelectActions", "ShortcutActions"],
		{
			"ok": self.exit,
			"back": self.exit,
			"red": self.exit,
		}, -1)
		self.timer = eTimer()
		self.timer.timeout.get().append(self.resetList) 
		self.onShown.append(lambda: self.timer.start(3500))
		self.onLayoutFinish.append(self.layoutFinished)
		self.onClose.append(self.cleanup)

	def cleanup(self):
		iStatus.stopWlanConsole()
		
	def layoutFinished(self):
		self.setTitle(_("Wireless Network State"))
		
	def resetList(self):
		print "self.iface im resetlist",self.iface
		iStatus.getDataForInterface(self.iface,self.getInfoCB)
		
	def getInfoCB(self,data,status):
		if data is not None:
			if data is True:
				if status is not None:
					self["BSSID"].setText(status[self.iface]["acesspoint"])
					self["ESSID"].setText(status[self.iface]["essid"])
					self["quality"].setText(status[self.iface]["quality"]+"%")
					self["signal"].setText(status[self.iface]["signal"])
					self["bitrate"].setText(status[self.iface]["bitrate"])
					self["enc"].setText(status[self.iface]["encryption"])
					self["frequency"].setText(status[self.iface]["frequency"]+" GHz")
					self.updateStatusLink(status)

	def exit(self):
		self.timer.stop()
		self.close()	

	def updateStatusbar(self):
		print "self.iface im updateStatusbar",self.iface
		self["BSSID"].setText(_("Please wait..."))
		self["ESSID"].setText(_("Please wait..."))
		self["quality"].setText(_("Please wait..."))
		self["signal"].setText(_("Please wait..."))
		self["bitrate"].setText(_("Please wait..."))
		self["enc"].setText(_("Please wait..."))
		self["frequency"].setText(_("Please wait..."))
		self["IFtext"].setText(_("Network:"))
		self["IF"].setText(iNetwork.getFriendlyAdapterName(self.iface))
		self["Statustext"].setText(_("Link:"))

	def updateStatusLink(self,status):
		
		if status is not None:
			if status[self.iface]["acesspoint"] == "No Connection" or status[self.iface]["acesspoint"] == "Not-Associated" or status[self.iface]["acesspoint"] == False:
				self["statuspic_active"].hide()
				self["statuspic_inactive"].show()
			else:
				self["statuspic_active"].show()
				self["statuspic_inactive"].hide()		



class WlanAntennaPositionStatus(Screen):
	skin = """
	<screen position="center,center" size="550,300" title="Wireless Antenna Position State" >
		<widget name="DecoderPhoto" pixmap="skin_default/front1.png" position="70,25" size="400,183" zPosition="1" transparent="1" alphatest="on" />
	
		<widget name="RSSI_A" position="330,5" size="200,25" valign="center" font="Regular;17" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="RSSI_B" position="150,5" size="200,25" valign="center" font="Regular;17" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="RSSI_C" position="250,5" size="200,25" valign="center" font="Regular;17" transparent="1" foregroundColor="#FFFFFF" />
		
		<widget name="statuspic_antenna_a_inactive" pixmap="skin_default/buttons/button_red.png" position="330,30" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		<widget name="statuspic_antenna_a_partial" pixmap="skin_default/buttons/button_yellow.png" position="330,30" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		<widget name="statuspic_antenna_a_active" pixmap="skin_default/buttons/button_green.png" position="330,30" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		
		<widget name="statuspic_antenna_b_inactive" pixmap="skin_default/buttons/button_red.png" position="160,30" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		<widget name="statuspic_antenna_b_partial" pixmap="skin_default/buttons/button_yellow.png" position="160,30" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		<widget name="statuspic_antenna_b_active" pixmap="skin_default/buttons/button_green.png" position="160,30" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		
		<widget name="statuspic_antenna_c_inactive" pixmap="skin_default/buttons/button_red.png" position="260,30" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		<widget name="statuspic_antenna_c_partial" pixmap="skin_default/buttons/button_yellow.png" position="260,30" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		<widget name="statuspic_antenna_c_active" pixmap="skin_default/buttons/button_green.png" position="260,30" zPosition="10" size="15,16" transparent="1" alphatest="on"/>
		
		<widget name="LabelBitRate" position="25,225" size="120,21" zPosition="10" font="Regular;21" transparent="1" valign="center" />
		
		<widget name="BitRate" position="125,225" size="120,21" zPosition="10" font="Regular;21" transparent="1" foregroundColor="#FFA323"/>
		
		<widget name="BottomBG" pixmap="skin_default/bottombar.png" position="5,210" size="540,120" zPosition="1" transparent="1" alphatest="on" />
		<widget name="ButtonRedtext" position="430,225" size="120,21" zPosition="10" font="Regular;21" transparent="1" />
		<widget name="ButtonRed" pixmap="skin_default/buttons/button_red.png" position="410,225" zPosition="10" size="15,16" transparent="1" alphatest="on" />

	</screen>
	"""
	
	
	def __init__(self, session, iface):
		
		Screen.__init__(self, session)
		
		self.session = session
		
		if (iface == 'eth0'):
			self.iface = 'ra0'
		else:
			self.iface = iface

		self.skin = WlanAntennaPositionStatus.skin
		
		self["DecoderPhoto"] = Pixmap()
		
		self["RSSI_A"] = Label()
		self["RSSI_B"] = Label()
		self["RSSI_C"] = Label()
		self["LabelBitRate"] = Label(_("Bit rate: "))
		self["BitRate"] = Label()
		
		self["statuspic_antenna_a_active"] = Pixmap()
		self["statuspic_antenna_a_active"].hide()
		self["statuspic_antenna_a_partial"] = Pixmap()
		self["statuspic_antenna_a_partial"].hide()
		self["statuspic_antenna_a_inactive"] = Pixmap()
		self["statuspic_antenna_a_inactive"].hide()
		
		self["statuspic_antenna_b_active"] = Pixmap()
		self["statuspic_antenna_b_active"].hide()
		self["statuspic_antenna_b_partial"] = Pixmap()
		self["statuspic_antenna_b_partial"].hide()
		self["statuspic_antenna_b_inactive"] = Pixmap()
		self["statuspic_antenna_b_inactive"].hide()
		
		self["statuspic_antenna_c_active"] = Pixmap()
		self["statuspic_antenna_c_active"].hide()
		self["statuspic_antenna_c_partial"] = Pixmap()
		self["statuspic_antenna_c_partial"].hide()
		self["statuspic_antenna_c_inactive"] = Pixmap()
		self["statuspic_antenna_c_inactive"].hide()
		
		self["BottomBG"] = Pixmap()
		self["ButtonRed"] = Pixmap()
		self["ButtonRedtext"] = Label(_("Close"))

		self.resetList()
		self.updateStatusbar()
		
		self["actions"] = NumberActionMap(["WizardActions", "InputActions", "EPGSelectActions", "ShortcutActions"],
		{
			"ok": self.exit,
			"back": self.exit,
			"red": self.exit,
		}, -1)
		self.timer = eTimer()
		self.timer.timeout.get().append(self.resetList)
		self.onShown.append(lambda: self.timer.start(2500))
		self.onLayoutFinish.append(self.layoutFinished)
		self.onClose.append(self.cleanup)

	def cleanup(self):
		iStatus.stopWlanConsole()
		
	def layoutFinished(self):
		self.setTitle(_("Wireless Antennas Position"))
		
	def resetList(self):
		print "self.iface im resetlist",self.iface
		iStatus.getAntennasForInterface(self.iface,self.getInfoCBAntennas)
		
	def getInfoCBAntennas(self,data,status):
		if data is not None:
			if data is True:
				if status is not None:
					self["RSSI_A"].setText(status[self.iface]["RSSI_A"] + " dBm")
					self["RSSI_B"].setText(status[self.iface]["RSSI_B"] + " dBm")
					self["RSSI_C"].setText(status[self.iface]["RSSI_C"] + " dBm")
					self["BitRate"].setText(status[self.iface]["Bitrate"] )
					self.updateStatusLink(status)

	def exit(self):
		self.timer.stop()
		self.close()	

	def updateStatusbar(self):
		print "self.iface im updateStatusbar",self.iface
		self["RSSI_A"].setText(_("..."))
		self["RSSI_B"].setText(_("..."))
		self["RSSI_C"].setText(_("..."))
		self["BitRate"].setText(_("..."))
		

	def updateStatusLink(self,status):
		
		if status is not None:
			if int(status[self.iface]["RSSI_A"]) <= -90 :
				self["statuspic_antenna_a_active"].hide()
				self["statuspic_antenna_a_partial"].hide()
				self["statuspic_antenna_a_inactive"].show()
			elif (int(status[self.iface]["RSSI_A"]) > -90) and (int(status[self.iface]["RSSI_A"]) <= -50) :
				self["statuspic_antenna_a_partial"].show()
				self["statuspic_antenna_a_inactive"].hide()
				self["statuspic_antenna_a_active"].hide()
			else:
				self["statuspic_antenna_a_active"].show()
				self["statuspic_antenna_a_inactive"].hide()
				self["statuspic_antenna_a_partial"].hide()
				
			if int(status[self.iface]["RSSI_B"]) <= -90 :
				self["statuspic_antenna_b_active"].hide()
				self["statuspic_antenna_b_partial"].hide()
				self["statuspic_antenna_b_inactive"].show()
			elif (int(status[self.iface]["RSSI_B"]) > -90) and (int(status[self.iface]["RSSI_B"]) <= -50) :
				self["statuspic_antenna_b_partial"].show()
				self["statuspic_antenna_b_inactive"].hide()
				self["statuspic_antenna_b_active"].hide()
			else:
				self["statuspic_antenna_b_active"].show()
				self["statuspic_antenna_b_inactive"].hide()
				self["statuspic_antenna_b_partial"].hide()
				
			if int(status[self.iface]["RSSI_C"]) <= -90 :
				self["statuspic_antenna_c_active"].hide()
				self["statuspic_antenna_c_partial"].hide()
				self["statuspic_antenna_c_inactive"].show()
			elif (int(status[self.iface]["RSSI_C"]) > -90) and (int(status[self.iface]["RSSI_C"]) <= -50) :
				self["statuspic_antenna_c_partial"].show()
				self["statuspic_antenna_c_inactive"].hide()
				self["statuspic_antenna_c_active"].hide()
			else:
				self["statuspic_antenna_c_active"].show()
				self["statuspic_antenna_c_inactive"].hide()
				self["statuspic_antenna_c_partial"].hide()



class WlanScan(Screen):
	skin = """
	<screen position="center,center" size="600,400" title="Choose a Wireless Network" >
		<widget name="info" position="10,10" size="580,30" font="Regular;24" transparent="1" foregroundColor="#FFFFFF" />
		<widget name="list" position="10,50" size="580,240" scrollbarMode="showOnDemand" />

		<ePixmap pixmap="skin_default/bottombar.png" position="30,310" size="540,120" zPosition="1" transparent="1" alphatest="on" />
		<ePixmap pixmap="skin_default/buttons/button_red.png" position="430,325" zPosition="10" size="15,16" transparent="1" alphatest="on" />
		<widget name="canceltext" position="450,325" size="120,21" zPosition="10" font="Regular;21" transparent="1" />
		<ePixmap pixmap="skin_default/buttons/button_green.png" position="50,325" zPosition="10" size="15,16" transparent="1" alphatest="on" />
		<widget name="selecttext" position="80,325" size="150,21" zPosition="10" font="Regular;21" transparent="1" />
		<ePixmap pixmap="skin_default/buttons/button_yellow.png" position="50,355" zPosition="10" size="15,16" transparent="1" alphatest="on" />
		<widget name="rescantext" position="80,355" size="150,21" zPosition="10" font="Regular;21" transparent="1" />
	</screen>
	"""

	
	def __init__(self, session, iface):
	
		Screen.__init__(self, session)
		self.session = session
		
		if (iface == 'eth0'):
			self.iface = 'ra0'
		else:
			self.iface = iface
			
		self.skin = WlanScan.skin
		self.skin_path = plugin_path
		self.oldInterfaceState = iNetwork.getAdapterAttribute(self.iface, "up")
		
		self["info"] = Label(" Searching Wireless Network(s). Please wait ...")
		
		self.list = []	
#		self["list"] = WlanList(self.session, self.iface)
#		self.setInfo()
		
		self["list"] = WlanList(self.session, self.iface, self.setInfo)

		self["canceltext"] = Label(_("Close"))
		self["selecttext"] = Label(_("Connect"))
		self["rescantext"] = Label(_("Refresh"))
			
		self["actions"] = NumberActionMap(["WizardActions", "InputActions", "EPGSelectActions"],
		{
			"ok": self.select,
			"back": self.cancel,
		}, -1)
		
		self["shortcuts"] = ActionMap(["ShortcutActions"],
		{
		 	"red": self.cancel,
			"green": self.select,
			"yellow": self.rescan,
		})
		self.onLayoutFinish.append(self.layoutFinished)
		
	def layoutFinished(self):
		self.setTitle(_("Choose a Wireless Network"))
	
	def select(self):
		cur = self["list"].getCurrent()
		#print "CURRENT",cur
		if cur is not None:
			if cur[1] is not None:
				essid = cur[0]
				if essid == '':
					essid = cur[1]
				encrypted = cur[2]
				self.close(essid,self["list"].getList())
			else:
				self.close(None,None)
		else:
			self.close(None,None)
	
	def WlanSetupClosed(self, *ret):
		if ret[0] == 2:
			self.close(None)
			
	def rescan(self):
		self["info"].setText(_(" Searching Wireless Network(s). Please wait ..."))
		self["list"].reload()
#		self.setInfo()
	
	def cancel(self):
		if self.oldInterfaceState is False:
			iNetwork.deactivateInterface(self.iface)
			
		self.close(None)

	def deactivateInterfaceCB(self,data):
		if data is not None:
			if data is True:
				iNetwork.getInterfaces(self.cancelCB)
	
	def cancelCB(self,data):
		if data is not None:
			if data is True:
				self.close(None)

	def setInfo(self):
		length = self["list"].getLength()
		
		if length == 0:
			length = _("No") 
		self["info"].setText(str(length)+_(" Wireless Network(s) found!"))


def WlanStatusScreenMain(session, iface):
	session.open(WlanStatus, iface)


def callActiveFunction(iface):
	if QBOXHD:
		if not QBOXHD_MINI and (iface == 'eth0' and iNetwork.check_iNIC_WIFI_Module_active() ):
			return WlanStatusScreenMain
		elif iface == 'wlan0':
			return WlanStatusScreenMain
	else:
		return None

def callFunction(iface):
	if QBOXHD:
		if not QBOXHD_MINI and (iface == 'eth0' and iNetwork.check_iNIC_WIFI_Module_present() ):
			return WlanStatusScreenMain
		elif iface == 'wlan0':
			return WlanStatusScreenMain
	else:
		w = Wlan(iface)
		i = w.getWirelessInterfaces()
		if i:
			if iface in i:
				return WlanStatusScreenMain
	
	return None
	
	
def configStrings(iface):
	driver = iNetwork.detectWlanModule(iface)
	print "WLAN-MODULE",driver
	
	if iface == "wlan0":
		if driver == 'rt73':
			mystr = "\tdown /etc/init.d/wifi_usb stop\n"
		elif driver == 'zd1211rw':
			mystr = "\tpre-up /usr/sbin/wpa_cli terminate;/usr/sbin/wpa_supplicant -i"+iface+" -c/etc/wpa_supplicant.conf -B -dd -Dwext\n\tpost-down /usr/sbin/wpa_cli terminate\n"
		else:
			return ""
			
		if iNetwork.ifaces[iface]['dhcp'] == False:
			mystr = "%s%s %s\n" % ( mystr, "\tup /etc/init.d/wifi_usb start", driver )
		else:
			mystr = "%s%s %s\n" % ( mystr, "\tup /etc/init.d/wifi_usb start_with_dhcp", driver )
		
		return mystr

	elif not QBOXHD_MINI and iface == "eth0" and driver == 'iNIC_Module':
		mystr = "\tdown /etc/init.d/wifi stop\n"
		if config.plugins.wlan.active.value:
			if iNetwork.ifaces[iface]['dhcp'] == False:
				mystr = "%s%s" % ( mystr, "\tup /etc/init.d/wifi start\n" )
			else:
				mystr = "%s%s" % ( mystr, "\tup /etc/init.d/wifi start_with_dhcp\n" )
		
		return mystr

	else:
		return ""


def Plugins(**kwargs):
	return PluginDescriptor(name=_("Wireless LAN"), description=_("Connect to a Wireless Network"), where = PluginDescriptor.WHERE_NETWORKSETUP, fnc={"ifaceSupported": callFunction, "ifaceActive": callActiveFunction, "configStrings": configStrings, "WlanPluginEntry": lambda x: "Wireless Network Configuration..."})
	
