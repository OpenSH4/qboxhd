from qboxhd import QBOXHD
from enigma import eTimer

from Plugins.Plugin import PluginDescriptor

from Screens.Screen import Screen
from Screens.MessageBox import MessageBox
from Screens.Console import Console
from Screens.ChoiceBox import ChoiceBox
from Screens.InfoBar import InfoBar

from Components.ActionMap import ActionMap
from Components.Label import Label
from Components.Button import Button
from Components.Sources.List import List
from Components.Sources.StaticText import StaticText
from Components.Pixmap import Pixmap
from Components.MenuList import MenuList
from Components.config import config

from Tools.Directories import resolveFilename, SCOPE_SKIN_IMAGE
from Tools.LoadPixmap import LoadPixmap
from Components.Harddisk import harddiskmanager

if QBOXHD:
	from Components.iDevsDevice import idevsdevicemanager

from Components.config import config, NoSave, ConfigInteger, ConfigText, ConfigIP, getConfigListEntry

from time import sleep as os_sleep, time as os_time


class HotplugPartitionMessage(Screen):
	skin = """<screen name="HotplugPartitionMessage" position="center,center" size="430,200" title="" >
			<widget name="icon" position="5,15" size="65,65" alphatest="on" />
			<widget name="info" position="80,5" size="350,190" font="Regular;18" />
		</screen>"""
	def __init__(self, session, action, partition):
		Screen.__init__(self, session)
		
		self["icon"] = Pixmap()
		self["info"] = Label()
		
		self.partition = partition
		self.action = action
		self.hdd = harddiskmanager.getHardiskFromPartition(partition)

		self["actions"] = ActionMap(["OkCancelActions"],
			{
				"ok": self.Exit,
				"cancel": self.Exit
			},-1)
		
		self.counter = 20
		self.onLayoutFinish.append(self.__LayoutFinish)

	def __Autoclose(self):
		self.counter -= 1
		if self.counter < 1:
			self.close()
		else:
			if self.action == "add":
				tmp_str = _("A new device was found") + " (" + str(self.counter) + ")"
			elif self.action == "remove":
				tmp_str = _("A device was removed") + " (" + str(self.counter) + ")"
			
			self.setTitle(tmp_str)

	def __LayoutFinish(self):
		
		if self.action == "add":
			self.setTitle(_("A new device was found"))
			self["icon"].instance.setPixmap( LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/mount_usb.png")) )
		elif self.action == "remove":
			self.setTitle(_("A device was removed"))
			self["icon"].instance.setPixmap( LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/umount_usb.png")) )
						
		#info text
		tmp_str = ""
		
		if self.partition.description != "":
			tmp_str += _("Description: ") + " " + self.partition.description + "\n"
		if self.partition.mountpoint != "":
			tmp_str += _("MountPoint: ") + self.partition.mountpoint + "\n"
		if self.partition.device is not None:	
			tmp_str += _("Device: %s\n") % self.partition.device
				
		if self.action == "add":
			if self.partition.total() is not None:
				value = self.partition.total() / (1000 * 1000)
				tmp_str += _("Capacity: %d.%03d GB\n") % (value/1000, value%1000)
			if self.partition.free() is not None:
				value = self.partition.free() / (1000 * 1000 )
				tmp_str += _("Free: %d.%03d GB\n") % (value/1000, value%1000)
				
		else:
			if self.hdd is not None:
				if len(self.hdd.has_swap_partition_active)>0:
					tmp_str += _("\nATTENTION: This device contains a \nSWAP Partition ACTIVE!!\nYour system may be instable !!!")
		
		self["info"].setText(tmp_str)
		
		self.StatusTimer = eTimer()
		self.StatusTimer.callback.append(self.__Autoclose)
		self.StatusTimer.start(1000)
		
	def Exit(self):
		print "[HotplugPartitionMessage] close"
		self.close()
		
		
class HotplugiDevsDeviceMessage(Screen):
	skin = """<screen name="HotplugiDevsDeviceMessage" position="center,center" size="430,200" title="" >
			<widget name="icon" position="5,15" size="65,65" alphatest="on" />
			<widget name="info" position="80,5" size="350,190" font="Regular;18" />
		</screen>"""
	def __init__(self, session, action, idevs_device):
		Screen.__init__(self, session)
		
		self["icon"] = Pixmap()
		self["info"] = Label()
		
		self.idevs_device = idevs_device
		self.action = action

		self["actions"] = ActionMap(["OkCancelActions"],
			{
				"ok": self.Exit,
				"cancel": self.Exit
			},-1)
		
		self.counter = 20
		self.onLayoutFinish.append(self.__LayoutFinish)

	def __Autoclose(self):
		self.counter -= 1
		if self.counter < 1:
			self.close()
		else:
			if self.action == "add":
				tmp_str = _("A new %s was found (%d)") % (self.idevs_device.media_name, self.counter)
			
			elif self.action == "remove":
				tmp_str = _("A %s was removed (%d)") % (self.idevs_device.media_name, self.counter)
			
			self.setTitle(tmp_str)

	def __LayoutFinish(self):
		
		if self.action == "add":
			
			self.setTitle(_("A new %s was found") % self.idevs_device.media_name)
			
			if self.idevs_device.isiPod():
				self["icon"].instance.setPixmap( LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/mount_ipod.png")) )
			elif self.idevs_device.isiPhone():
				self["icon"].instance.setPixmap( LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/mount_iphone.png")) )
			elif self.idevs_device.isiPad():
				self["icon"].instance.setPixmap( LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/mount_ipad.png")) )
				
		elif self.action == "remove":
			self.setTitle(_("A %s was removed") % self.idevs_device.media_name)
			if self.idevs_device.isiPod():
				self["icon"].instance.setPixmap( LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/umount_ipod.png")) )
			elif self.idevs_device.isiPhone():
				self["icon"].instance.setPixmap( LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/umount_iphone.png")) )
			elif self.idevs_device.isiPad():
				self["icon"].instance.setPixmap( LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/umount_ipad.png")) )
				
		#info text
		tmp_str = ""
		
		if self.idevs_device.description != "":
			tmp_str += _("Description: %s\n") % str(self.idevs_device.media_name)
		if self.idevs_device.device != "":
			tmp_str += _("Device: %s\n") % str(self.idevs_device.device)
				
		if self.action == "add":
			if self.idevs_device.total() is not None:
				value = self.idevs_device.total() / (1000 * 1000)
				tmp_str += _("Capacity: %d.%03d GB\n") % (value/1000, value%1000)
			if self.idevs_device.free() is not None:
				value = self.idevs_device.free() / (1000 * 1000 )
				tmp_str += _("Free: %d.%03d GB\n") % (value/1000, value%1000)
				
				
		self["info"].setText(tmp_str)
		
		self.StatusTimer = eTimer()
		self.StatusTimer.callback.append(self.__Autoclose)
		self.StatusTimer.start(1000)
		
	def Exit(self):
		print "[HotplugiDevsDeviceMessage] close"
		self.close()
		
global_session = None

def partitionListChanged(action, device):
	print "[HotplugPartitionMessage] partitionListChanged ", action
	print "[HotplugPartitionMessage] mountpoint", device.mountpoint
	print "[HotplugPartitionMessage] description", device.description
	print "[HotplugPartitionMessage] force_mounted", device.force_mounted
	global_session.open(HotplugPartitionMessage, action, device)
		

def idevsListChanged(action, idevsdevice):
	print "[HotplugiDevsDeviceMessage] ipodListChanged ", action
	print "[HotplugiDevsDeviceMessage] device", idevsdevice.device
	print "[HotplugiDevsDeviceMessage] media_type", idevsdevice.media_type
	print "[HotplugiDevsDeviceMessage] media_name", idevsdevice.media_name
	print "[HotplugiDevsDeviceMessage] description", idevsdevice.description
	global_session.open(HotplugiDevsDeviceMessage, action, idevsdevice)
	

def autostart(reason, **kwargs):
	global global_session
	if reason == 0:
		harddiskmanager.on_partition_list_change.append(partitionListChanged)
		if QBOXHD:
			idevsdevicemanager.on_idevsdevice_insert_change.append(idevsListChanged)
	elif reason == 1:
		harddiskmanager.on_partition_list_change.remove(partitionListChanged)
		if QBOXHD:
			idevsdevicemanager.on_idevsdevice_insert_change.remove(idevsListChanged)
		global_session = None

def sessionstart(reason, session):
	global global_session
	global_session = session

def Plugins(**kwargs):
	return [
		PluginDescriptor(where = PluginDescriptor.WHERE_AUTOSTART, fnc = autostart),
		PluginDescriptor(where = PluginDescriptor.WHERE_SESSIONSTART, fnc = sessionstart)
		]
