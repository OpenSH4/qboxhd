# for localized messages
# version stone 1.0.7 and 1.0.10 and 1.0.12
# version stone 0.0.50 and 0.0.54 and 0.0.56

from qboxhd import QBOXHD, QBOXHD_MINI
from __init__ import _

from enigma import eTimer, eListboxPythonMultiContent, eListbox, gFont, RT_HALIGN_LEFT, RT_HALIGN_RIGHT, RT_HALIGN_CENTER
from Components.MultiContent import MultiContentEntryText
from Components.GUIComponent import GUIComponent
from Components.HTMLComponent import HTMLComponent
from Components.config import config, ConfigYesNo, NoSave, ConfigSubsection, ConfigText, ConfigSelection, ConfigPassword
from Components.Console import Console
from Components.Network import iNetwork

from os import system
from string import maketrans, strip
import sys
import types
from re import compile as re_compile, search as re_search
from iwlibs import getNICnames, Wireless, Iwfreq

list = []
list.append("WEP 64")
list.append("WEP 128")
list.append("WPA")
list.append("WPA2")

weplist = []
weplist.append("ASCII")
weplist.append("HEX")

config.plugins.wlan = ConfigSubsection()
if not QBOXHD_MINI:
  config.plugins.wlan.active = NoSave(ConfigYesNo(default = True))
config.plugins.wlan.essid = NoSave(ConfigText(default = "--NONE--", fixed_size = False))
config.plugins.wlan.hiddenessid = NoSave(ConfigText(default = "--NONE--", fixed_size = False))
config.plugins.wlan.essidscan = NoSave(ConfigYesNo(default = True))

config.plugins.wlan.encryption = ConfigSubsection()
config.plugins.wlan.encryption.enabled = NoSave(ConfigYesNo(default = False))
config.plugins.wlan.encryption.type = NoSave(ConfigSelection(list, default = "WEP 64"))
config.plugins.wlan.encryption.wepkeytype = NoSave(ConfigSelection(weplist, default = "ASCII"))
config.plugins.wlan.encryption.psk = NoSave(ConfigPassword(default = "passd", fixed_size = False))

class Wlan:
	def __init__(self, iface):
		a = ''; b = ''
		
		for i in range(0, 255):
		    a = a + chr(i)
		    if i < 32 or i > 127:
			b = b + ' '
		    else:
			b = b + chr(i)
		
		if not QBOXHD_MINI and (iface == 'eth0') and iNetwork.check_iNIC_WIFI_Module_active():
			self.iface = 'ra0'
		else:
			self.iface = iface
		
		self.wlaniface = {}
		self.WlanConsole = Console()
		self.asciitrans = maketrans(a, b)
		

	def stopWlanConsole(self):
		if self.WlanConsole is not None:
			print "killing self.WlanConsole"
			self.WlanConsole = None
			del self.WlanConsole
			
	def getDataForInterface(self, callback = None):
		#get ip out of ip addr, as avahi sometimes overrides it in ifconfig.
		print "self.iface im getDataForInterface",self.iface
		if len(self.WlanConsole.appContainers) == 0:
			self.WlanConsole = Console()
			cmd = "iwconfig " + self.iface
			self.WlanConsole.ePopen(cmd, self.iwconfigFinished, callback)

	def iwconfigFinished(self, result, retval, extra_args):
		print "self.iface im iwconfigFinished",self.iface
		callback = extra_args
		data = { 'essid': False, 'frequency': False, 'acesspoint': False, 'bitrate': False, 'encryption': False, 'quality': False, 'signal': False }
		#print "result im iwconfigFinished",result
		
		for line in result.splitlines():
			#print "line",line
			line = line.strip()
			if "ESSID" in line:
				if "off/any" in line:
					ssid = _("No Connection")
				else:
					tmpssid=(line[line.index('ESSID')+7:len(line)-1])
					if tmpssid == '':
						ssid = _("Hidden networkname")
					elif tmpssid ==' ':
						ssid = _("Hidden networkname")
					else:
					    ssid = tmpssid
				#print "SSID->",ssid
				if ssid is not None:
					data['essid'] = ssid
			if 'Frequency' in line:
				frequency = line[line.index('Frequency')+10 :line.index(' GHz')]
				#print "Frequency",frequency   
				if frequency is not None:
					data['frequency'] = frequency
			if "Access Point" in line:
				ap=line[line.index('Access Point')+14:line.index('Access Point')+14+17]
				#print "AP",ap
				if ap is not None:
					data['acesspoint'] = ap
			if "Bit Rate" in line:
				br = line[line.index('Bit Rate')+9 :line.index(' Mb/s')]
				#print "Bitrate",br
				if br is not None:
					data['bitrate'] = br
			if 'Encryption key' in line:
				if ":off" in line:
				    enc = _("Disabled")
				else:
				    enc = line[line.index('Encryption key')+15 :len(line)-1] #line.index('   Security')]
				#print "Encryption key",enc 
				if enc is not None:
					data['encryption'] = _("Enabled")
			if 'Quality' in line:
				if "/100" in line:
					qual = line[line.index('Quality')+8:line.index('/100')]
				else:
					qual = line[line.index('Quality')+8:line.index('Sig')]
				#print "Quality",qual
				if qual is not None:
					data['quality'] = qual
			if 'Signal level' in line:
				signal = line[line.index('Signal level')+13 :line.index(' dBm')]
				#print "Signal level",signal		
				if signal is not None:
					data['signal'] = signal

		self.wlaniface[self.iface] = data
		
		if len(self.WlanConsole.appContainers) == 0:
			print "self.wlaniface after loading:", self.wlaniface
			self.WlanConsole = None
			if callback is not None:
				callback(True,self.wlaniface)


	def getAdapterAttribute(self, attribute):
		print "im getAdapterAttribute"
		if self.wlaniface.has_key(self.iface):
			print "self.wlaniface.has_key",self.iface
			if self.wlaniface[self.iface].has_key(attribute):
				return self.wlaniface[self.iface][attribute]
		return None
		
	def asciify(self, str):
		return str.translate(self.asciitrans)

	
	def getWirelessInterfaces(self):
		iwifaces = None
		try:
			iwifaces = getNICnames()
		except:
			print "[Wlan.py] No Wireless Networkcards could be found"
		
		return iwifaces
		
		
	def getNetworkList(self):
		system("ifconfig "+self.iface+" up")
		ifobj = Wireless(self.iface) # a Wireless NIC Object
		
		#Association mappings
		try:
			stats, quality, discard, missed_beacon = ifobj.getStatistics()
			snr = quality.signallevel - quality.noiselevel
		except:
			print "[Wlan.py] No Wireless Networks could be found"
			return None

		try:
			scanresults = ifobj.scan()
		except:
			scanresults = None
			print "[Wlan.py] No Wireless Networks could be found"
		
		if scanresults is not None:
			aps = {}
			for result in scanresults:
			
				bssid = result.bssid
		
				encryption = map(lambda x: hex(ord(x)), result.encode)
		
				if encryption[-1] == "0x8":
					encryption = True
				else:
					encryption = False
		
				extra = []
				for element in result.custom:
					element = element.encode()
					extra.append( strip(self.asciify(element)) )
				
				if result.quality.sl is 0 and len(extra) > 0:
					begin = extra[0].find('SignalStrength=')+15
									
					done = False
					end = begin+1
					
					while not done:
						if extra[0][begin:end].isdigit():
							end += 1
						else:
							done = True
							end -= 1
					
					signal = extra[0][begin:end]
					
					if QBOXHD:
						driver = iNetwork.detectWlanModule(self.iface)
						if driver == 'zd1211rw' and self.iface == 'wlan0':
							signal = "%d" % (result.quality.signal_level)

				else:
					signal = "%d" % (result.quality.signal_level)
					
				fq = Iwfreq()
				try:
					channel = str(fq.getChannel(str(result.frequency.getFrequency()[0:-3])))
				except:
					channel = 0
			
				aps[bssid] = {
					'active' : True,
					'bssid': result.bssid,
					'channel': channel,
					'encrypted': encryption,
					'essid': strip(self.asciify(result.essid)),
					'iface': self.iface,
					'maxrate' : result.rate[-1],
					'noise' : result.quality.noise_level, #result.quality.getNoiselevel(),
					'quality' : str(result.quality.quality),
					'signal' : signal,
					'custom' : extra,
				}
				print aps[bssid]
			return aps
		
		
	def getStatus(self):
		ifobj = Wireless(self.iface)
		fq = Iwfreq()
		try:
			self.channel = str(fq.getChannel(str(ifobj.getFrequency()[0:-3])))
		except:
			self.channel = 0
		#print ifobj.getStatistics()
		status = {
				  'BSSID': str(ifobj.getAPaddr()),
				  'ESSID': str(ifobj.getEssid()),
				  'quality': str(ifobj.getStatistics()[1].quality),
				  'signal': str(ifobj.getStatistics()[1].sl),
				  'bitrate': str(ifobj.getBitrate()),
				  'channel': str(self.channel),
				  #'channel': str(fq.getChannel(str(ifobj.getFrequency()[0:-3]))),
		}
		
		for (key, item) in status.items():
			if item is "None" or item is "":
					status[key] = _("N/A")
				
		return status



class WlanList(HTMLComponent, GUIComponent):
	def __init__(self, session, iface, callback):
		
		GUIComponent.__init__(self)

		if not QBOXHD_MINI and (iface == 'eth0') and iNetwork.check_iNIC_WIFI_Module_active() :
			self.iface = 'ra0'
		else:
			self.iface = iface
			
		self.w = Wlan(self.iface)
		
		self.length = 0
		self.callback = callback
		self.aplist = None
		self.list = None
		self.l = None
		self.l = eListboxPythonMultiContent()
		
		self.l.setFont(0, gFont("Regular", 24)) # Original ("Regular", 32)
		self.l.setFont(1, gFont("Regular", 18))
		self.l.setFont(2, gFont("Regular", 16))
		self.l.setBuildFunc(self.buildWlanListEntry)
				
		if QBOXHD:
			self.timerNL = eTimer()
			self.timerNL.callback.append(self.__getNetworkListCB)

		self.reload()
		
		
	def buildWlanListEntry(self, essid, bssid, encryption, wlanmode, iface, signal, channel):
		
		res = [ (essid, encryption, iface) ]
		
		if essid == "":
			essid = bssid
		
		res.append( MultiContentEntryText(pos=(0, 0), size=(470, 35), font=0, flags=RT_HALIGN_LEFT, text=essid) )
	        #res.append( MultiContentEntryText(pos=(350, 0), size=(80, 20), font=1, flags=RT_HALIGN_LEFT, text=_("Quality"))) 
                res.append( MultiContentEntryText(pos=(385, 0), size=(80, 20), font=1, flags=RT_HALIGN_LEFT, text=_("Quality")))
		if int(signal) > 0:
                        #res.append( MultiContentEntryText(pos=(420, 0), size=(80, 35), font=0, flags=RT_HALIGN_RIGHT, text=("%s" %signal) + "%" )) 
			res.append( MultiContentEntryText(pos=(460, 0), size=(80, 35), font=0, flags=RT_HALIGN_RIGHT, text=("%s" %signal) + "%" ))
		else:
                        #res.append( MultiContentEntryText(pos=(420, 0), size=(120, 35), font=0, flags=RT_HALIGN_RIGHT, text=("%s" %signal) + " dBm" )) 
			res.append( MultiContentEntryText(pos=(460, 0), size=(120, 35), font=0, flags=RT_HALIGN_RIGHT, text=("%s" %signal) + " dBm" ))
		res.append( MultiContentEntryText(pos=(0, 40), size=(180, 20), font=1, color = 0xFFA323, color_sel = 0xFFA323, flags=RT_HALIGN_LEFT, text=_("Mode: %s") %wlanmode ))
		res.append( MultiContentEntryText(pos=(140, 40), size=(330, 20), font=1, color = 0xFFA323, color_sel = 0xFFA323, flags=RT_HALIGN_LEFT, text=_("Encryption: %s") % (encryption) ))
		if channel != None:
			res.append( MultiContentEntryText(pos=(430, 40), size=(110, 20), font=1, color = 0xFFA323, color_sel = 0xFFA323, flags=RT_HALIGN_RIGHT, text=_("Channel: %s") %channel ))
		return res
		
		
	def getInfoCBNetworks(self, data, apslist):
		list = []
		try:
			if data is not None:
				if data is True:
					if apslist is not None:
						print "[Wlan.py] got Accespoints!"
						for aps in apslist:
							ap = apslist[aps]
							for a in ap:
								mya = ap[a]
								if mya['active']:
									if mya['essid'] == "":
										mya['essid'] = mya['bssid']
							
									#list.append( (mya['essid'], mya['bssid'], mya['auth'], mya['encryption'], mya['wlanmode'], mya['iface'], mya['signal'], mya['channel'] ) )
									list.append( (mya['essid'], mya['bssid'], mya['encryption'], mya['wlanmode'], mya['iface'], mya['signal'], mya['channel'] ) )
									self.aplist.append( mya['essid'] )
					
		except:
			print "[Wlan.py] Error parse iwpriv get_site_survey"
			list = []
			self.aplist = []
			self.length = len(list)
			self.l.setList([])
			self.l.setList(list)
			
		self.length = len(list)
		self.l.setList([])
		self.l.setList(list)
		
		if self.callback is not None:
			self.callback()


	def __getNetworkListCB(self):
		self.timerNL.stop()
		aps = self.w.getNetworkList()

		list = []
		self.aplist = []
		if aps is not None:
			print "[Wlan.py] got Accespoints!"
			for ap in aps:
				a = aps[ap]
				if a['active']:
					if a['essid'] == '':
						a['essid'] = a['bssid']
					list.append( (a['essid'], a['bssid'], a['encrypted'], a['maxrate'], a['iface'], a['signal'], None) )
		
		if len(list):
			for entry in list:
				self.aplist.append( entry[0])
		self.length = len(list)
		self.l.setList([])
		self.l.setList(list)
		
		if self.callback is not None:
			self.callback()

			
	def reload(self):
		self.length = 0
		self.aplist = []
		self.l.setList([])
		
		driver = iNetwork.detectWlanModule(self.iface)
		print ">>>>>>>>>>>>>>>>>>>>> driver=%s" % driver
		if driver == 'zd1211rw' and self.iface == 'wlan0':
			self.timerNL.start(100)
		else:
			iStatus.getWNetwork(self.iface, self.getInfoCBNetworks)

	GUI_WIDGET = eListbox

	def getCurrent(self):
		return self.l.getCurrentSelection()
	
	
	def postWidgetCreate(self, instance):
		instance.setContent(self.l)
		instance.setItemHeight(60)
	
	
	def getLength(self):
		return self.length
	
	def getList(self):
		return self.aplist


class wpaSupplicant:
	def __init__(self):
		pass
	
		
	def writeConfig(self):	
			
			essid = config.plugins.wlan.essid.value
			essidscan = config.plugins.wlan.essidscan.value
			hiddenessid = config.plugins.wlan.hiddenessid.value
			encrypted = config.plugins.wlan.encryption.enabled.value
			encryption = config.plugins.wlan.encryption.type.value
			wepkeytype = config.plugins.wlan.encryption.wepkeytype.value
			psk = config.plugins.wlan.encryption.psk.value
			fp = file('/etc/wpa_supplicant.conf', 'w')
			fp.write('#WPA Supplicant Configuration by enigma2\n')
			fp.write('ctrl_interface=/var/run/wpa_supplicant\n')
			fp.write('eapol_version=1\n')
			fp.write('fast_reauth=1\n')	
			if essid == 'hidden...':
				fp.write('ap_scan=2\n')
			else:
				fp.write('ap_scan=1\n')
			fp.write('network={\n')
			if essid == 'hidden...':
				fp.write('\tssid="'+hiddenessid+'"\n')
			else:
				fp.write('\tssid="'+essid+'"\n')
			if essidscan:
				fp.write('\tscan_ssid=1\n')
			else:
				fp.write('\tscan_ssid=0\n')
			if encrypted:
				if encryption == 'WPA' or encryption == 'WPA2':
					fp.write('\tkey_mgmt=WPA-PSK\n')
					
					if encryption == 'WPA':
						fp.write('\tproto=WPA\n')
						fp.write('\tpairwise=TKIP\n')
						fp.write('\tgroup=TKIP\n')
						
					elif encryption == 'WPA2':
						fp.write('\tproto=WPA RSN\n')
						fp.write('\tpairwise=CCMP TKIP\n')
						fp.write('\tgroup=CCMP TKIP\n')
					fp.write('\tpsk="'+psk+'"\n')
						
				elif encryption == 'WEP 64' or encryption == 'WEP 128':
					fp.write('\tkey_mgmt=NONE\n')
					#FIXME: This is a workaround for a bug when ST wpa_supplicant start. 
					#		if there aren't these lines below, wpa_supplicant doesn't start.
					fp.write('\tproto=WPA RSN\n')
					fp.write('\tpsk="dummypassword"\n')
					##########################################################################
					
					if wepkeytype == 'ASCII':
						fp.write('\twep_key0="'+psk+'"\n')
					else:
						fp.write('\twep_key0='+psk+'\n')
					fp.write('\twep_tx_keyidx=0\n')
					
			else:
				fp.write('\tkey_mgmt=NONE\n')			
			fp.write('}')
			fp.write('\n')
			fp.close()
			system("cat /etc/wpa_supplicant.conf")
		
	def loadConfig(self):
		try:
			#parse the wpasupplicant configfile
			fp = file('/etc/wpa_supplicant.conf', 'r')
			supplicant = fp.readlines()
			fp.close()
			ap_scan = False
			scan_ssid = False
			essid = None

			for s in supplicant:
				split = s.strip().split('=',1)
				if split[0] == 'ap_scan':
					print "[Wlan.py] Got Hidden SSID Scan  Value "+split[1]
					if split[1] == '2':
						ap_scan = True
					else:
						ap_scan = False
						
				elif split[0] == 'ssid':
					print "[Wlan.py] Got SSID "+split[1][1:-1]
					essid = split[1][1:-1]
					
				elif split[0] == 'scan_ssid':
					print "[Wlan.py] Got scan_ssid "+split[1][1:-1]
					scan_ssid = split[1][1:-1]
					config.plugins.wlan.essidscan.value = scan_ssid
					
				elif split[0] == 'proto':
					print "split[1]",split[1]
					config.plugins.wlan.encryption.enabled.value = True
					if split[1] == "WPA" :
						mode = 'WPA'
					if split[1] == "WPA RSN" :
						mode = 'WPA2'
					config.plugins.wlan.encryption.type.value = mode
					print "[Wlan.py] Got Encryption: "+mode
					
				#currently unused !
				#elif split[0] == 'key_mgmt':
				#	print "split[1]",split[1]
				#	if split[1] == "WPA-PSK" :
				#		config.plugins.wlan.encryption.enabled.value = True
				#		config.plugins.wlan.encryption.type.value = "WPA/WPA2"
				#	print "[Wlan.py] Got Encryption: "+ config.plugins.wlan.encryption.type.value
					
				elif split[0] == 'wep_key0':
					mode = 'WEP 64'
					config.plugins.wlan.encryption.enabled.value = True
					if split[1].startswith('"') and split[1].endswith('"'):
						config.plugins.wlan.encryption.wepkeytype.value = 'ASCII'
						config.plugins.wlan.encryption.psk.value = split[1][1:-1]
						
						#validate ASCII key
						if len(config.plugins.wlan.encryption.psk.value) == 5:
							mode = 'WEP 64'
						elif len(config.plugins.wlan.encryption.psk.value) == 13:
							mode = 'WEP 128'
					else:
						config.plugins.wlan.encryption.wepkeytype.value = 'HEX'
						config.plugins.wlan.encryption.psk.value = split[1]
						
						#validate HEX key
						if len(config.plugins.wlan.encryption.psk.value) == 10:
							mode = 'WEP 64'
						elif len(config.plugins.wlan.encryption.psk.value) == 26:
							mode = 'WEP 128'
						
					config.plugins.wlan.encryption.type.value = mode
					print "[Wlan.py] Got Encryption: WEP - keytype is: "+config.plugins.wlan.encryption.wepkeytype.value
					print "[Wlan.py] Got Encryption: WEP - key0 is: "+config.plugins.wlan.encryption.psk.value
					
				elif split[0] == 'psk':
					config.plugins.wlan.encryption.psk.value = split[1][1:-1]
					print "[Wlan.py] Got PSK: "+split[1][1:-1]
				else:
					pass
				
			if ap_scan is True:
				config.plugins.wlan.hiddenessid.value = essid
				config.plugins.wlan.essid.value = 'hidden...'
			else:
				config.plugins.wlan.hiddenessid.value = essid
				config.plugins.wlan.essid.value = essid
				
			wsconfig = {
					'hiddenessid': config.plugins.wlan.hiddenessid.value,
					'ssid': config.plugins.wlan.essid.value,
					'ssidscan': config.plugins.wlan.essidscan.value,
					'encryption': config.plugins.wlan.encryption.enabled.value,
					'encryption_type': config.plugins.wlan.encryption.type.value,
					'encryption_wepkeytype': config.plugins.wlan.encryption.wepkeytype.value,
					'key': config.plugins.wlan.encryption.psk.value,
				}
		
			for (key, item) in wsconfig.items():
				if item is "None" or item is "":
					if key == 'hiddenessid':
						wsconfig['hiddenessid'] = "--NONE--"
					if key == 'ssid':
						wsconfig['ssid'] = "--NONE--"
					if key == 'ssidscan':
						wsconfig['ssidscan'] = False
					if key == 'encryption':
						wsconfig['encryption'] = False
					if key == 'encryption':
						wsconfig['encryption_type'] = "WEP 64"
					if key == 'encryption':
						wsconfig['encryption_wepkeytype'] = "ASCII"
					if key == 'encryption':
						wsconfig['key'] = "passd"
		except:
			print "[Wlan.py] Error parsing /etc/wpa_supplicant.conf"
			wsconfig = {
					'hiddenessid': "--NONE--",
					'ssid': "--NONE--",
					'ssidscan': False,
					'encryption': False,
					'encryption_type': "WEP 64",
					'encryption_wepkeytype': "ASCII",
					'key': "passd"
					}
		print "[Wlan.py] WS-CONFIG-->",wsconfig
		return wsconfig

	
	def restart(self, iface):
		if QBOXHD:
			return
		system("start-stop-daemon -K -x /usr/sbin/wpa_supplicant")
		system("start-stop-daemon -S -x /usr/sbin/wpa_supplicant -- -B -i"+iface+" -c/etc/wpa_supplicant.conf")


class iNIC_MII_WiFi_Module:
	def __init__(self):
		pass
	
	def writedefaultConfig(self, fp):
		
		fp.write('#iNIC_sta.dat Configuration by enigma2\n')
		fp.write('Default\n')
		fp.write('CountryRegion=1\n')
		fp.write('CountryRegionABand=1\n')
		fp.write('CountryCode=IT\n')
		fp.write('NetworkType=Infra\n')
		fp.write('WirelessMode=9\n')
		fp.write('Channel=0\n')
		fp.write('BasicRate=15\n')
		fp.write('BeaconPeriod=100\n')
		fp.write('TxPower=100\n')
		fp.write('BGProtection=0\n')
		fp.write('TxPreamble=0\n')
		fp.write('RTSThreshold=2347\n')
		fp.write('FragThreshold=2346\n')
		fp.write('TxBurst=1\n')
		fp.write('PktAggregate=0\n')
		fp.write('WmmCapable=0\n')
		fp.write('AckPolicy=0;0;0;0\n')
		fp.write('PSMode=CAM\n')
		fp.write('FastRoaming=0\n')
		fp.write('RoamThreshold=70\n')
		fp.write('APSDCapable=0\n')
		fp.write('APSDAC=0;0;0;0\n')
		fp.write('HT_RDG=1\n')
		fp.write('HT_EXTCHA=0\n')
		fp.write('HT_OpMode=1\n')
		fp.write('HT_MpduDensity=5\n')
		fp.write('HT_BW=1\n')
		fp.write('HT_AutoBA=1\n')
		fp.write('HT_BADecline=0\n')
		fp.write('HT_AMSDU=0\n')
		fp.write('HT_BAWinSize=64\n')
		fp.write('HT_GI=1\n')
		fp.write('HT_MCS=33\n')
		fp.write('IEEE80211H=0\n')
		fp.write('TGnWifiTest=0\n')
		fp.write('WirelessEvent=0\n')
	
		
	def writeConfig(self):	
			
		essid = config.plugins.wlan.essid.value
		essidscan = config.plugins.wlan.essidscan.value
		hiddenessid = config.plugins.wlan.hiddenessid.value
		encrypted = config.plugins.wlan.encryption.enabled.value
		encryption = config.plugins.wlan.encryption.type.value
		wepkeytype = config.plugins.wlan.encryption.wepkeytype.value
		psk = config.plugins.wlan.encryption.psk.value
		
		fp = file('/etc/iNIC_sta.dat', 'w')
		
		# write default setting to file
		self.writedefaultConfig(fp);
		
		fp.write('SSID='+essid+'\n')
		
		if encrypted:
			
			if encryption == 'WPA':
				fp.write('AuthMode=WPAPSK\n')
				fp.write('EncrypType=TKIP\n')
				fp.write('WPAPSK='+psk+'\n')
			
			elif encryption == 'WPA2':
				fp.write('AuthMode=WPA2PSK\n')
				fp.write('EncrypType=TKIP\n')
				fp.write('WPAPSK='+psk+'\n')
					
			elif encryption == 'WEP 64' or encryption == 'WEP 128':
				fp.write('AuthMode=SHARED\n')
				fp.write('EncrypType=WEP\n')
				fp.write('DefaultKeyID=1\n')
				if wepkeytype == 'ASCII':
					fp.write('Key1Type=1\n')
				else:
					fp.write('Key1Type=0\n')
				fp.write('Key1='+psk+'\n')
				fp.write('Key1Str='+psk+'\n')
		else:
			fp.write('AuthMode=OPEN\n')
			fp.write('EncrypType=NONE\n')
		
		fp.close()
		system("cat /etc/iNIC_sta.dat")
			
			
	def getIdentifierParameter(self, configStr, identifier):
		for s in configStr:
			split = s.strip().split('=',1)
			if split[0] == identifier:
				print "[Wlan.py] Got " + identifier + " :" + split[1]#[0:-1]
				return split[1]#[0:-1]
				
		print "[Wlan.py] No " + identifier + " present."
		return None
		
		
	def loadConfig(self):
		try:
			#parse the iNIC_sta.dat
			fp = file('/etc/iNIC_sta.dat', 'r')
			WiFi_Module_str = fp.readlines()
			fp.close()
			
			mode = None
			
			Essid = self.getIdentifierParameter( WiFi_Module_str, 'SSID')
			
			#identifier if encryption is activated
			AuthModeStr = self.getIdentifierParameter( WiFi_Module_str, 'AuthMode')
			EncrypTypeStr = self.getIdentifierParameter( WiFi_Module_str, 'EncrypType')
			
			if AuthModeStr == 'WPAPSK' and  EncrypTypeStr == 'TKIP':
				#validate WPA PSK TKIP Encryption	
				
				WPAPSKStr = self.getIdentifierParameter( WiFi_Module_str, 'WPAPSK')
				if WPAPSKStr != None:
					config.plugins.wlan.encryption.psk.value = WPAPSKStr
					mode = 'WPA'
			
			elif AuthModeStr == 'WPA2PSK' and  EncrypTypeStr == 'TKIP':
				#validate WPA2 PSK TKIP Encryption
				
				WPAPSKStr = self.getIdentifierParameter( WiFi_Module_str, 'WPAPSK')
				if WPAPSKStr != None:
					config.plugins.wlan.encryption.psk.value = WPAPSKStr
					mode = 'WPA2'
				
			elif AuthModeStr == 'SHARED' and EncrypTypeStr == 'WEP':
				#validate WEP SHARED Encryption
				
				Key1TypeStr = self.getIdentifierParameter( WiFi_Module_str, 'Key1Type')
				Key1Str = self.getIdentifierParameter( WiFi_Module_str, 'Key1')
				Key1StrStr = self.getIdentifierParameter( WiFi_Module_str, 'Key1Str')
				
				if (Key1TypeStr == '1' and Key1StrStr != None ):
					#validate ASCII key
					if len(Key1StrStr) == 5:
						mode = 'WEP 64'
					elif len(Key1StrStr) == 13:
						mode = 'WEP 128'
						
					if mode != None:
						config.plugins.wlan.encryption.wepkeytype.value = 'ASCII'
						config.plugins.wlan.encryption.psk.value = Key1StrStr
					
				if (Key1TypeStr == '0') and ( Key1Str != None ):
					#validate HEX key
					if len(Key1Str) == 10:
						mode = 'WEP 64'
					elif len(Key1Str) == 26:
						mode = 'WEP 128'
					
					if mode != None:
						config.plugins.wlan.encryption.wepkeytype.value = 'HEX'
						config.plugins.wlan.encryption.psk.value = Key1Str
						
			if mode == 'WEP 64' or mode == 'WEP 128':
				if mode == 'WEP 64':
					print "[Wlan.py] Got Encryption: WEP 64 - keytype is: "+config.plugins.wlan.encryption.wepkeytype.value
					print "[Wlan.py] Got Encryption: WEP 64 - key0 is: "+config.plugins.wlan.encryption.psk.value
				else:	
					print "[Wlan.py] Got Encryption: WEP 128 - keytype is: "+config.plugins.wlan.encryption.wepkeytype.value
					print "[Wlan.py] Got Encryption: WEP 128 - key0 is: "+config.plugins.wlan.encryption.psk.value
				
				config.plugins.wlan.encryption.type.value = mode
				config.plugins.wlan.encryption.enabled.value = True
				
			elif mode == 'WPA':
				print "[Wlan.py] Got Encryption: WPA-PSK KEY is: "+ config.plugins.wlan.encryption.psk.value
				config.plugins.wlan.encryption.type.value = mode
				config.plugins.wlan.encryption.enabled.value = True
				
			elif mode == 'WPA2':
				print "[Wlan.py] Got Encryption: WPA2-PSK KEY is: "+ config.plugins.wlan.encryption.psk.value
				config.plugins.wlan.encryption.type.value = mode
				config.plugins.wlan.encryption.enabled.value = True
				
			else:
				config.plugins.wlan.encryption.enabled.value = False
				
			config.plugins.wlan.hiddenessid.value = Essid
			config.plugins.wlan.essid.value = Essid
				
			iNICconfig = {
					'hiddenessid': config.plugins.wlan.hiddenessid.value,
					'ssid': config.plugins.wlan.essid.value,
					'ssidscan': config.plugins.wlan.essidscan.value,
					'encryption': config.plugins.wlan.encryption.enabled.value,
					'encryption_type': config.plugins.wlan.encryption.type.value,
					'encryption_wepkeytype': config.plugins.wlan.encryption.wepkeytype.value,
					'key': config.plugins.wlan.encryption.psk.value,
				}
		
			for (key, item) in iNICconfig.items():
				if item is "None" or item is "":
					if key == 'hiddenessid':
						iNICconfig['hiddenessid'] = "--NONE--"
					if key == 'ssid':
						iNICconfig['ssid'] = "--NONE--"
					if key == 'ssidscan':
						iNICconfig['ssidscan'] = False
					if key == 'encryption':
						iNICconfig['encryption'] = False
					if key == 'encryption':
						iNICconfig['encryption_type'] = "WEP 64"
					if key == 'encryption':
						iNICconfig['encryption_wepkeytype'] = "ASCII"
					if key == 'encryption':
						iNICconfig['key'] = "passd"
	
		except:
			print "[Wlan.py] Error parsing /etc/iNIC_sta.dat"
			iNICconfig = {
					'hiddenessid': "--NONE--",
					'ssid': "--NONE--",
					'ssidscan': False,
					'encryption': False,
					'encryption_type': "WEP 64",
					'encryption_wepkeytype': "ASCII",
					'key': "passd",
				}
				
		print "[Wlan.py] iNIC-CONFIG-->",iNICconfig
		return iNICconfig

	
	def restart(self, iface):
		pass
		#system("start-stop-daemon -K -x /usr/sbin/wpa_supplicant")
		#system("start-stop-daemon -S -x /usr/sbin/wpa_supplicant -- -B -i"+iface+" -c/etc/wpa_supplicant.conf")




class USB_WiFi_Module:
	def __init__(self):
		pass
		
	def writeConfig(self):	
			
		essid = config.plugins.wlan.essid.value
		essidscan = config.plugins.wlan.essidscan.value
		hiddenessid = config.plugins.wlan.hiddenessid.value
		encrypted = config.plugins.wlan.encryption.enabled.value
		encryption = config.plugins.wlan.encryption.type.value
		wepkeytype = config.plugins.wlan.encryption.wepkeytype.value
		psk = config.plugins.wlan.encryption.psk.value
		
		fp = file('/etc/wifi_usb.dat', 'w')
		
		fp.write('SSID='+essid+'\n')
		
		if encrypted:
			
			if encryption == 'WPA':
				fp.write('AuthMode=WPAPSK\n')
				fp.write('EncrypType=TKIP\n')
				fp.write('WPAPSK='+psk+'\n')
			
			elif encryption == 'WPA2':
				fp.write('AuthMode=WPA2PSK\n')
				fp.write('EncrypType=TKIP\n')
				fp.write('WPAPSK='+psk+'\n')
					
			elif encryption == 'WEP 64' or encryption == 'WEP 128':
				fp.write('AuthMode=SHARED\n')
				fp.write('EncrypType=WEP\n')
				fp.write('DefaultKeyID=1\n')
				if wepkeytype == 'ASCII':
					fp.write('Key1Type=1\n')
				else:
					fp.write('Key1Type=0\n')
				fp.write('Key1='+psk+'\n')
				fp.write('Key1Str='+psk+'\n')
		else:
			fp.write('AuthMode=OPEN\n')
			fp.write('EncrypType=NONE\n')
		
		fp.close()
		system("cat /etc/wifi_usb.dat")
			
			
	def getIdentifierParameter(self, configStr, identifier):
		for s in configStr:
			split = s.strip().split('=',1)
			if split[0] == identifier:
				print "[Wlan.py] Got " + identifier + " :" + split[1]#[0:-1]
				return split[1]#[0:-1]
				
		print "[Wlan.py] No " + identifier + " present."
		return None
		
		
	def loadConfig(self):
		try:
			#parse the iNIC_sta.dat
			fp = file('/etc/wifi_usb.dat', 'r')
			WiFi_Module_str = fp.readlines()
			fp.close()
			
			mode = None
			
			Essid = self.getIdentifierParameter( WiFi_Module_str, 'SSID')
			
			#identifier if encryption is activated
			AuthModeStr = self.getIdentifierParameter( WiFi_Module_str, 'AuthMode')
			EncrypTypeStr = self.getIdentifierParameter( WiFi_Module_str, 'EncrypType')
			
			if AuthModeStr == 'WPAPSK' and  EncrypTypeStr == 'TKIP':
				#validate WPA PSK TKIP Encryption	
				
				WPAPSKStr = self.getIdentifierParameter( WiFi_Module_str, 'WPAPSK')
				if WPAPSKStr != None:
					config.plugins.wlan.encryption.psk.value = WPAPSKStr
					mode = 'WPA'
			
			elif AuthModeStr == 'WPA2PSK' and  EncrypTypeStr == 'TKIP':
				#validate WPA2 PSK TKIP Encryption
				
				WPAPSKStr = self.getIdentifierParameter( WiFi_Module_str, 'WPAPSK')
				if WPAPSKStr != None:
					config.plugins.wlan.encryption.psk.value = WPAPSKStr
					mode = 'WPA2'
				
			elif AuthModeStr == 'SHARED' and EncrypTypeStr == 'WEP':
				#validate WEP SHARED Encryption
				
				Key1TypeStr = self.getIdentifierParameter( WiFi_Module_str, 'Key1Type')
				Key1Str = self.getIdentifierParameter( WiFi_Module_str, 'Key1')
				Key1StrStr = self.getIdentifierParameter( WiFi_Module_str, 'Key1Str')
				
				if (Key1TypeStr == '1' and Key1StrStr != None ):
					#validate ASCII key
					if len(Key1StrStr) == 5:
						mode = 'WEP 64'
					elif len(Key1StrStr) == 13:
						mode = 'WEP 128'
						
					if mode != None:
						config.plugins.wlan.encryption.wepkeytype.value = 'ASCII'
						config.plugins.wlan.encryption.psk.value = Key1StrStr
					
				if (Key1TypeStr == '0') and ( Key1Str != None ):
					#validate HEX key
					if len(Key1Str) == 10:
						mode = 'WEP 64'
					elif len(Key1Str) == 26:
						mode = 'WEP 128'
					
					if mode != None:
						config.plugins.wlan.encryption.wepkeytype.value = 'HEX'
						config.plugins.wlan.encryption.psk.value = Key1Str
						
			if mode == 'WEP 64' or mode == 'WEP 128':
				if mode == 'WEP 64':
					print "[Wlan.py] Got Encryption: WEP 64 - keytype is: "+config.plugins.wlan.encryption.wepkeytype.value
					print "[Wlan.py] Got Encryption: WEP 64 - key0 is: "+config.plugins.wlan.encryption.psk.value
				else:	
					print "[Wlan.py] Got Encryption: WEP 128 - keytype is: "+config.plugins.wlan.encryption.wepkeytype.value
					print "[Wlan.py] Got Encryption: WEP 128 - key0 is: "+config.plugins.wlan.encryption.psk.value
				
				config.plugins.wlan.encryption.type.value = mode
				config.plugins.wlan.encryption.enabled.value = True
				
			elif mode == 'WPA':
				print "[Wlan.py] Got Encryption: WPA-PSK KEY is: "+ config.plugins.wlan.encryption.psk.value
				config.plugins.wlan.encryption.type.value = mode
				config.plugins.wlan.encryption.enabled.value = True
				
			elif mode == 'WPA2':
				print "[Wlan.py] Got Encryption: WPA2-PSK KEY is: "+ config.plugins.wlan.encryption.psk.value
				config.plugins.wlan.encryption.type.value = mode
				config.plugins.wlan.encryption.enabled.value = True
				
			else:
				config.plugins.wlan.encryption.enabled.value = False
				
			config.plugins.wlan.hiddenessid.value = Essid
			config.plugins.wlan.essid.value = Essid
				
			USB_WiFiconfig = {
					'hiddenessid': config.plugins.wlan.hiddenessid.value,
					'ssid': config.plugins.wlan.essid.value,
					'ssidscan': config.plugins.wlan.essidscan.value,
					'encryption': config.plugins.wlan.encryption.enabled.value,
					'encryption_type': config.plugins.wlan.encryption.type.value,
					'encryption_wepkeytype': config.plugins.wlan.encryption.wepkeytype.value,
					'key': config.plugins.wlan.encryption.psk.value,
				}
		
			for (key, item) in USB_WiFiconfig.items():
				if item is "None" or item is "":
					if key == 'hiddenessid':
						USB_WiFiconfig['hiddenessid'] = "--NONE--"
					if key == 'ssid':
						USB_WiFiCconfig['ssid'] = "--NONE--"
					if key == 'ssidscan':
						USB_WiFiconfig['ssidscan'] = False
					if key == 'encryption':
						USB_WiFiconfig['encryption'] = False
					if key == 'encryption':
						USB_WiFiconfig['encryption_type'] = "WEP 64"
					if key == 'encryption':
						USB_WiFiconfig['encryption_wepkeytype'] = "ASCII"
					if key == 'encryption':
						USB_WiFiconfig['key'] = "passd"
	
		except:
			print "[Wlan.py] Error parsing /etc/wifi_usb.dat"
			USB_WiFiconfig = {
					'hiddenessid': "--NONE--",
					'ssid': "--NONE--",
					'ssidscan': False,
					'encryption': False,
					'encryption_type': "WEP 64",
					'encryption_wepkeytype': "ASCII",
					'key': "passd",
				}
				
		print "[Wlan.py] USB_WiFiconfig-->",USB_WiFiconfig
		return USB_WiFiconfig

	
	def restart(self, iface):
		pass



class Status:
	def __init__(self):
		self.wlaniface = {}
		self.backupwlaniface = {}
		
		self.wlanifaceantenna = {}
		self.backupwlanifaceantenna = {}
		
		self.wlanscaniface = {}
		self.backupwlanscaniface = {}
		
		self.WlanConsole = Console()

	def stopWlanConsole(self):
		if self.WlanConsole is not None:
			print "killing self.WlanConsole"
			self.WlanConsole = None
			
	def getDataForInterface(self, iface, callback = None):
		self.WlanConsole = Console()
		cmd = "iwconfig " + iface
		self.WlanConsole.ePopen(cmd, self.iwconfigFinished, [iface, callback])

	def iwconfigFinished(self, result, retval, extra_args):
		(iface, callback) = extra_args
		data = { 'essid': False, 'frequency': False, 'acesspoint': False, 'bitrate': False, 'encryption': False, 'quality': False, 'signal': False }
	
		for line in result.splitlines():
			line = line.strip()
			
			if "ESSID" in line:
				if "off/any" in line:
					ssid = _("No Connection")
				else:
					#tmpssid=(line[line.index('ESSID')+7:len(line)-1])
					pos = line.index('ESSID')+7
					posend = line[pos:].index('\"')
					tmpssid= line[pos:pos+posend]
					if tmpssid == '':
						ssid = _("Hidden networkname")
					elif tmpssid ==' ':
						ssid = _("Hidden networkname")
					else:
					    ssid = tmpssid
				#print "SSID->",ssid
				if ssid is not None:
					data['essid'] = ssid
					
			if 'Frequency' in line:
				frequency = line[line.index('Frequency')+10 :line.index(' GHz')]
				#print "Frequency",frequency   
				if frequency is not None:
					data['frequency'] = frequency
					
			if "Access Point" in line:
				ap=line[line.index('Access Point')+14:line.index('Access Point')+14+17]
				#print "AP",ap
				if ap is not None:
					data['acesspoint'] = ap
					if ap == "Not-Associated":
						data['essid'] = _("No Connection")
						
			if "Bit Rate" in line:
				if "kb" in line:
					br = line[line.index('Bit Rate')+9 :line.index(' kb/s')]
					if br == '0':
						br = _("Unsupported")
					else:
						br += " Mb/s"
				else:
					br = line[line.index('Bit Rate')+9 :line.index(' Mb/s')] + " Mb/s"
				#print "Bitrate",br
				if br is not None:
					data['bitrate'] = br
					
			if 'Encryption key' in line:
				if ":off" in line:
					if data['acesspoint'] is not "Not-Associated":
						enc = _("Unsupported")
					else:
						enc = _("Disabled")
				else:
					enc = line[line.index('Encryption key')+15 :len(line)] #line.index('   Security')]
					if enc is not None:
						enc = _("Enabled")
				#print "Encryption key",enc 
				if enc is not None:
					data['encryption'] = enc
					
			if 'Quality' in line:
				if "/100" in line:
					qual = line[line.index('Quality')+8:line.index('/100')]
				else:
					qual = line[line.index('Quality')+8:line.index('Sig')]
				#print "Quality",qual
				if qual is not None:
					data['quality'] = qual
					
			if 'Signal level' in line:
				if "dBm" in line:
					signal = line[line.index('Signal level')+13 :line.index(' dBm')]
					signal += " dBm"
				elif "/100" in line:
					pos = line.index('Signal level')+13
					posend = line[pos:].index("/100")
					signal = line[pos:pos+posend]
					signal += "%"
				else:
					signal = line[line.index('Signal level')+13:line.index('  Noise')]
					signal += "%"
				#print "Signal level",signal		
				if signal is not None:
					data['signal'] = signal

		self.wlaniface[iface] = data
		self.backupwlaniface = self.wlaniface
		
		if self.WlanConsole is not None:
			if len(self.WlanConsole.appContainers) == 0:
				print "self.wlaniface after loading:", self.wlaniface
				if callback is not None:
					callback(True,self.wlaniface)
					
					
	def getAntennasForInterface(self, iface, callback = None):
		self.WlanConsole = Console()
		cmd = "iwpriv %s stat; iwlist %s rate" % (iface, iface)
		self.WlanConsole.ePopen(cmd, self.iwprivFinished, [iface, callback])
		
					
	def iwprivFinished(self, result, retval, extra_args):
		(iface, callback) = extra_args
		data = { 'RSSI_A': False, 'RSSI_B': False, 'RSSI_C': False, 'Bitrate': False}
	
		for line in result.splitlines():
			line = line.strip()
			
			if "Current Bit Rate" in line:
				
				enc = line[line.index('Current Bit Rate')+17 :len(line)]
				if enc is not None:
					print "Bit Rate",enc 
					data['Bitrate'] = enc
				else:
					data['Bitrate'] = _("0 Mb/s")
			
			if "RSSI-A" in line:
				
				enc = line[line.index('RSSI-A')+34 :len(line)]
				if enc is not None:
					print "RSSI-A",enc 
					data['RSSI_A'] = enc
				else:
					data['RSSI_A'] = _("0")
					
			if 'RSSI-B' in line:
				
				enc = line[line.index('RSSI-B')+34 :len(line)]
				if enc is not None:
					print "RSSI-B",enc 
					data['RSSI_B'] = enc
				else:
					data['RSSI_B'] = _("0")
					
			if 'RSSI-C' in line:
				
				enc = line[line.index('RSSI-C')+34 :len(line)]
				if enc is not None:
					print "RSSI-C",enc 
					data['RSSI_C'] = enc
				else:
					data['RSSI_C'] = _("0")

		self.wlanifaceantenna[iface] = data
		self.backupwlanifaceantenna = self.wlanifaceantenna
		
		if self.WlanConsole is not None:
			if len(self.WlanConsole.appContainers) == 0:
				print "self.wlaniface after loading:", self.wlanifaceantenna
				if callback is not None:
					callback(True,self.wlanifaceantenna)

	def getWNetwork(self, iface, callback = None):
		self.WlanConsole = Console()
		driver = iNetwork.detectWlanModule(iface)
		if driver == 'rt73' and iface == 'wlan0':
			cmd = "ifconfig "+iface+" up;iwpriv "+iface+" get_site_survey"
		else:
			cmd = "iwpriv " + iface + " get_site_survey"
		self.WlanConsole.ePopen(cmd, self.iwWNetworkFinished, [iface, callback])


	def iwWNetworkFinished(self, result, retval, extra_args):
		(iface, callback) = extra_args
		aps = {}
		
		print result
		
		counter = 0
		for line in result.splitlines():
			if counter > 1:
				data = { 'channel': False, 'essid': False, 'bssid': False, 'encryption': False, 'signal': False, 'wlanmode': False }
				
				line = line.strip()
				
				driver = iNetwork.detectWlanModule(iface)
				
				if driver == 'iNIC_Module' and iface == 'ra0':
					if len(line) >= 50:
						data['iface'] = iface
						
						channel = line[0:3].rstrip(' ')
						if channel is not None:
							data['channel'] = channel
							
						essid = line[4:36].rstrip(' ')
						if essid is not None:
							data['essid'] = essid
							
						bssid = line[37:56].rstrip(' ')
						if bssid is not None:
							data['bssid'] = bssid
							
						encryption = line[57:79].rstrip(' ')
						if encryption is not None:
							data['encryption'] = encryption

						signal = line[80:88].rstrip(' ')
						if signal is not None:
							data['signal'] = signal
						
						wlanmode = line[89:96].rstrip(' ')
						if wlanmode is not None:
							data['wlanmode'] = wlanmode
					
						aps[bssid] = {
							'active' : True,
							'iface': iface,
							'channel': channel,
							'essid': essid,
							'bssid': bssid,
							'encryption': encryption,
							#'auth': auth,
							'signal': signal,
							'wlanmode': wlanmode,
						}
						
						print aps[bssid]
						
				elif driver == 'rt73' and iface == 'wlan0':
					
					if len(line) >= 50:
						data['iface'] = iface
						
						channel = line[0:3].rstrip(' ')
						if channel is not None:
							data['channel'] = channel
							
						essid = line[16:51].rstrip(' ')
						if essid is not None:
							data['essid'] = essid
							
						bssid = line[52:71].rstrip(' ')
						if bssid is not None:
							data['bssid'] = bssid
							
						encryption = line[84:95].rstrip(' ')
						if encryption is not None:
							data['encryption'] = encryption

						signal = line[8:15].rstrip(' ')
						if signal is not None:
							data['signal'] = signal
						
						wlanmode = None
					
						aps[bssid] = {
							'active' : True,
							'iface': iface,
							'channel': channel,
							'essid': essid,
							'bssid': bssid,
							'encryption': encryption,
							#'auth': auth,
							'signal': signal,
							'wlanmode': wlanmode,
						}
						
						print aps[bssid]
					
			counter = counter + 1
			
		self.wlanscaniface[iface] = aps
		self.backupwlanscaniface = self.wlanscaniface
		
		if len(self.WlanConsole.appContainers) == 0:
			print "self.wlanscaniface after loading:", self.wlanscaniface
			if callback is not None:
				callback(True, self.wlanscaniface)

	def getAdapterAttribute(self, iface, attribute):
		print "im getAdapterAttribute"
		self.iface = iface
		if self.wlaniface.has_key(self.iface):
			print "self.wlaniface.has_key",self.iface
			if self.wlaniface[self.iface].has_key(attribute):
				return self.wlaniface[self.iface][attribute]
		return None
	
	
	def getAdapterAntennaAttribute(self, iface, attribute):
		print "im getAdapterAntennaAttribute"
		self.iface = iface
		if self.wlanifaceantenna.has_key(self.iface):
			print "self.wlanifaceantenna.has_key",self.iface
			if self.wlanifaceantenna[self.iface].has_key(attribute):
				return self.wlanifaceantenna[self.iface][attribute]
		return None
	
iStatus = Status()
