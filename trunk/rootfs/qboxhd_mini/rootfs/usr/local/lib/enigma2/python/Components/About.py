from qboxhd import QBOXHD
from Tools.Directories import resolveFilename, SCOPE_SYSETC
from enigma import getEnigmaVersionString
if QBOXHD:
	from os import popen
	from re import compile as re_compile, search as re_search

class About:
	def __init__(self):
		pass

	def getVersionString(self):
		return self.getImageVersionString()

	def getImageVersionString(self):
		try:
			file = open(resolveFilename(SCOPE_SYSETC, 'image-version'), 'r')
			lines = file.readlines()
			for x in lines:
				splitted = x.split('=')
				if QBOXHD:
					if splitted[0] == "version":
						#VVVVMMM DD YYYY hh:mm
						#0003Jun 24 2009 17:16
						version = splitted[1]
						image = '-'.join(["QBoxHD", version[4:22]])
						return image
				else:
					if splitted[0] == "version":
						#     YYYY MM DD hh mm
						#0120 2005 11 29 01 16
						#0123 4567 89 01 23 45
						version = splitted[1]
						year = version[4:8]
						month = version[8:10]
						day = version[10:12]

						return '-'.join(("dev", year, month, day))
			file.close()
		except IOError:
			pass

		return "unavailable"

	def getEnigmaVersionString(self):
		return getEnigmaVersionString()
	
	def regExpMatch(self, pattern, string):
		if string is None:
			return None
		try:
			return pattern.search(string).group()
		except AttributeError:
			None

	def getKernelInfo(self):
		#Kernel Info
		try:
			result = popen("uname -r").read()
			kernel_date = popen("uname -v").read()
		except IOError:
			return "unknown"
		
		KernelRegexp = '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}'
		KernelLinePattern = re_compile(KernelRegexp + '_')
		KernelPattern = re_compile(KernelRegexp)
		
		kernel_name = self.regExpMatch(KernelPattern, self.regExpMatch(KernelLinePattern, result))
		if kernel_name is not None:
			return kernel_name + " " + kernel_date
			
		return "unknown"
	
	def getRamfsInfo(self):
		# Ramfs Info
		try:
			result = open("/etc/initramfs-version", "r").read()
		except:
			return "unknown"
		
		return result
	
	def getFilesystemInfo(self):
		# Ramfs Info
		try:
			result = open("/etc/fs-version", "r").read()
		except:
			return "unknown"
		
		return result	
	
	def getBitstreamInfo(self):
		# BitStream Info
		try:
			bsinfo = popen("testreg 02").read()[15:19]
		except IOError:
			bsinfo = "unknown"
			
		return bsinfo
	
	def getEth0IpAddress(self):
		data = { 'up': False, 'dhcp': False, 'preup' : False, 'postdown' : False, 'upup' : False, 'wifi_active' : False }
		try:
			result = popen("ip -o addr").read()
		except IOError:
			return ""
		
		globalIPpattern = re_compile("scope global")
		ipRegexp = '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}'
		ipLinePattern = re_compile('inet ' + ipRegexp + '/')
		ipPattern = re_compile(ipRegexp)
		upPattern = re_compile('UP')

		for line in result.splitlines():
			split = line.strip().split(' ',2)
			if (split[1][:-1] == "eth0"):
				up = self.regExpMatch(upPattern, split[2])
				if up is None:
					return ""
				
			if (split[1] == "eth0"):
				if re_search(globalIPpattern, split[2]):
					ip = self.regExpMatch(ipPattern, self.regExpMatch(ipLinePattern, split[2]))
					if ip is not None:
						return ip
		return ""
	
	def getKeyBoardInfo(self):
		keyboard_name = "None"
		# KeyBoard Info
		try:
			result = open("/proc/bus/input/devices", "r").read()
		except:
			return "None"
		
		for line in result.splitlines():
			if line[0:9] == ("N: Name=\""):
				keyboard_name = line[9:-1]
				
			if keyboard_name != "None":
				if line[0:-1] == ("H: Handlers=kbd"):
					return keyboard_name
		
		return "None"
		
	def getMouseInfo(self):
		mouse_name = "None"
		# Mouse Info
		try:
			result = open("/proc/bus/input/devices", "r").read()
		except:
			return "None"
		
		for line in result.splitlines():
			if line[0:9] == ("N: Name=\""	):
				mouse_name = line[9:-1]
				
			if mouse_name != "None":
				if line[0:-1] == ("H: Handlers=mouse0"):
					return mouse_name
		
		return "None"
	

about = About()
