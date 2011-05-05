from qboxhd import QBOXHD
from Screens.Screen import Screen
from Screens.MessageBox import MessageBox
from Screens.Console import Console
# QBOXHD ------------------------------
from enigma import eConsoleAppContainer
#--------------------------------------
from Components.ActionMap import ActionMap, NumberActionMap
from Components.Pixmap import Pixmap
from Components.Label import Label
from Components.MenuList import MenuList
# QBOXHD ------------------------------
from Components.Harddisk import harddiskmanager
from Components.config import config, ConfigYesNo, NoSave, ConfigSelection, getConfigListEntry
from Components.ConfigList import ConfigList, ConfigListScreen
from Components.Network import iNetwork
#--------------------------------------
from Components.config import ConfigSelection, ConfigSubsection, KEY_LEFT, KEY_RIGHT, KEY_0, getConfigListEntry
from Plugins.Plugin import PluginDescriptor

from Tools.Directories import *
from os import path, makedirs, listdir
from time import localtime
from datetime import date
from time import sleep
from enigma import eTimer
from Tools.Notifications import AddPopup

plugin_path = ""

BackupIdentifier = {
		"general_configuration" : "qboxhd_general_configuration.tgz",
		"all_skins" 		: "qboxhd_all_skin.tgz",
		"network_setting" 	: "qboxhd_network_settings.tgz",
		"channel_settings" 	: "qboxhd_channel_settings.tgz",
		"var_folder"		: "qboxhd_var_folder.tgz"
	}



class MainBackupRestoreScreen(Screen):
	skin = """
		<screen position="center,center" size="250,200" title="Main menu" >
			<widget name="menu" position="0,0" size="250,200" scrollbarMode="showOnDemand" />
		</screen>"""


	def keyCancel(self):
		self.close()

	def keyOk(self):
		idx = self["menu"].getSelectedIndex()
		if idx == 0:
			self.session.open(BackupSetup)
		elif idx == 1:
			self.session.open(MainTargetDeviceRestoreScreen)

	def __init__(self, session, args = None):
		Screen.__init__(self, session)

		self["actions"] = NumberActionMap(["SetupActions"],
		{
			"ok"	: self.keyOk,
			"cancel": self.keyCancel,
		}, -1)

		self.createSetup()


	def createSetup(self):
		print "Creating MainBackupRestoreScreen"

		list = []
		list.append(_("Backup"))
		list.append(_("Restore"))

		self["menu"] = MenuList(list)






class MainTargetDeviceRestoreScreen(Screen):
	skin = """
		<screen position="center,center" size="400,200" title="Select Target device" >
			<widget name="menu" position="0,0" size="400,200" scrollbarMode="showOnDemand" />
		</screen>"""


	def keyCancel(self):
		self.close()

	def keyOk(self):
		if self.almost_one_mountpoint:
			idx = self["menu"].getSelectedIndex()
			self.session.open(RestoreMenu, self.mountdescription[idx], self.mountpoint[idx] )


	def __init__(self, session, args = None):
		Screen.__init__(self, session)

		self["actions"] = NumberActionMap(["SetupActions"],
		{
			"ok"	: self.keyOk,
			"cancel": self.keyCancel,
		}, -1)

		self.createSetup()


	def NoStorage(self):
		self.Timer.stop()
		self.session.openWithCallback(self.close, MessageBox, _("No backup storage present."),MessageBox.TYPE_INFO)


	def createSetup(self):
		print "Creating MainTargetDeviceRestoreScreen"

		self.almost_one_mountpoint=False
		self.mountdescription = []
		self.mountpoint = []
		self.listmount = []

		# Backup Type
		for partition in harddiskmanager.getMountedPartitions():
			if (partition.mountpoint != "/") and (partition.mountpoint.find("sd") < 0) :
				self.mountdescription.append(partition.description)
				self.mountpoint.append(partition.mountpoint + "/qboxhd_backup/")
				if partition.total() is not None:
					value = partition.total() / (1000 * 1000)
					cap = _("%d.%03d GB") % (value/1000, value%1000)
					if partition.device is not None:
						dev, part = harddiskmanager.splitDeviceName(partition.device)
						if part and part != 1:
							self.listmount.append(partition.description + "\t(" + cap +")")
						else:
							self.listmount.append(partition.description + "\t\t(" + cap +")")
					else:
						self.listmount.append(partition.description + "\t\t(" + cap +")")
					self.almost_one_mountpoint=True


		if not self.almost_one_mountpoint:
			self.Timer = eTimer()
			self.Timer.callback.append(self.NoStorage)
			self.Timer.start(10, True)

		self["menu"] = MenuList(self.listmount)




class BackupSetup(Screen, ConfigListScreen):
	skin = """
		<screen position="center,center" size="560,400" title="Backup">
			<widget name="config" position="5,5" size="550,360" scrollbarMode="showOnDemand" zPosition="1"/>
			<widget name="key_red" position="280,360" size="140,40" valign="center" halign="center" zPosition="5" transparent="1" foregroundColor="white" font="Regular;18"/>
			<widget name="key_green" position="420,360" size="140,40" valign="center" halign="center" zPosition="5" transparent="1" foregroundColor="white" font="Regular;18"/>
			<ePixmap name="red" pixmap="skin_default/buttons/red.png" position="280,360" size="140,40" zPosition="4" transparent="1" alphatest="on"/>
			<ePixmap name="green" pixmap="skin_default/buttons/green.png" position="420,360" size="140,40" zPosition="4" transparent="1" alphatest="on"/>
		</screen>"""


	def keyCancel(self):
		self.close()

	def keyLeft(self):
		ConfigListScreen.keyLeft(self)

	def keyRight(self):
		ConfigListScreen.keyRight(self)

	def __init__(self, session, args = None):
		Screen.__init__(self, session)
		self.skin_path = plugin_path

		self["key_green"] = Label(_("OK"))
		self["key_red"] = Label(_("Cancel"))
		self["green"] = Pixmap()
		self["red"] = Pixmap()

		self["actions"] = NumberActionMap(["SetupActions"],
		{
			"ok"	: self.keySave,
			"cancel": self.keyCancel,
			"left"	: self.keyLeft,
			"right"	: self.keyRight
		}, -1)

		self["shortcuts"] = ActionMap(["ShortcutActions"],
		{
			"red"	: self.keyCancel,
			"green"	: self.Backup,
		})

		self.list = []
		ConfigListScreen.__init__(self, self.list)

		self.container = eConsoleAppContainer()
		self.createSetup()


	def NoStorage(self):
		self.Timer.stop()
		self.session.openWithCallback(self.close, MessageBox, _("No backup storage present."),MessageBox.TYPE_INFO)

	def createSetup(self):
		print "Creating BackupSetup"

		self.mountdescription = []
		self.mountpoint = []
		self.listmount = []
		self.list = []

		found=False

		# Backup Type
		for partition in harddiskmanager.getMountedPartitions():
			if (partition.mountpoint != "/") and (partition.mountpoint.find("sd") < 0) :
				self.mountdescription.append(partition.description)
				self.mountpoint.append(partition.mountpoint + "/qboxhd_backup/")
				if partition.total() is not None:
					value = partition.total() / (1000 * 1000)
					cap = _("%d.%03d GB") % (value/1000, value%1000)
					self.listmount.append(partition.description + "\t(" + cap +")")
					found=True

		if found:
			self.configbackup = NoSave(ConfigYesNo(default = True))
			self.skinbackup = NoSave(ConfigYesNo(default = True))
			self.networkbackup = NoSave(ConfigYesNo(default = True))
			self.settingsbackup = NoSave(ConfigYesNo(default = True))
			self.varbackup = NoSave(ConfigYesNo(default = True))

			self.list.append((_("Save data on device"), ConfigSelection(self.listmount, self.listmount[0])))
			self.configbackupEntry = getConfigListEntry(_("Backup General Configuration"), self.configbackup)
			self.list.append(self.configbackupEntry)
			self.skinbackupEntry = getConfigListEntry(_("Backup All Skins"), self.skinbackup)
			self.list.append(self.skinbackupEntry)
			self.networkbackupEntry = getConfigListEntry(_("Backup Network Settings"), self.networkbackup)
			self.list.append(self.networkbackupEntry)
			self.settingsbackupEntry = getConfigListEntry(_("Backup Channel Settings"), self.settingsbackup)
			self.list.append(self.settingsbackupEntry)
			self.varbackupEntry = getConfigListEntry(_("Backup VAR folder"), self.varbackup)
			self.list.append(self.varbackupEntry)

		else:
			self.Timer = eTimer()
			self.Timer.callback.append(self.NoStorage)
			self.Timer.start(10, True)

		self["config"].list = self.list
		self["config"].l.setList(self.list)


	def createBackupfolders(self):
		self.path = self.mountpoint[self.list[0][1].getIndex()]
		print "Creating Backup Folder if not already there..."
		if (path.exists(self.path) == False):
			makedirs(self.path)

	def Backup(self):
		print "this will start the backup now!"
		self.session.openWithCallback(self.runBackup, MessageBox, _("Do you want to backup now?\nAfter pressing OK, please wait!"))

	def BackupFinish(self, retval):
		if QBOXHD:
			cmds="sync"                                                            
			self.container.execute(cmds)                                           
 		self.backupMsg["text"].setText(_("Backup successfully!"))
		self.container.appClosed.remove(self.BackupFinish)


	def runBackup(self, result):
		almost_one=False
		if result:
			self.createBackupfolders()
			cmd = ""
			if ( self.configbackup.value ):
				almost_one=True
				print "Backup: General Configuration inserted"
				cmd +=  "tar -czvf " + self.path + "/" + str(BackupIdentifier["general_configuration"]) + " /etc/enigma2/settings  /var/custombutton.dat;"


			if (self.skinbackup.value):
				almost_one=True
				print "Backup: All Skins inserted"
				cmd +=  "tar -czvf " + self.path + "/" + str(BackupIdentifier["all_skins"]) + " /usr/share/enigma2/;"


			if (self.networkbackup.value):
				almost_one=True
				print "Backup: Network Configuration inserted"
				cmd +=  "tar -czvf " + self.path + "/" + str(BackupIdentifier["network_setting"]) + " /etc/network/interfaces /etc/resolv.conf /etc/iNIC_sta.dat /etc/wifi_usb.dat;"


			if (self.settingsbackup.value):
				almost_one=True
				print "Backup: Channel Settings Configuration inserted"
				cmd +=  "tar -czvf " + self.path + "/" + str(BackupIdentifier["channel_settings"]) + " /etc/enigma2/;"


			if (self.varbackup.value):
				almost_one=True
				print "Backup: VAR folder inserted"
				cmd +=  "tar -czvf " + self.path + "/" + str(BackupIdentifier["var_folder"]) + " /var/;"

			if not almost_one:
				return

			self.backupMsg = self.session.open(MessageBox, _("Backup running ..."), MessageBox.TYPE_INFO)
			self.container.appClosed.append(self.BackupFinish)
			self.container.execute(cmd)



class RestoreMenu(Screen, ConfigListScreen):
	skin = """
		<screen position="center,center" size="560,400" title="Restore">
			<widget name="config" position="5,5" size="550,360" scrollbarMode="showOnDemand" zPosition="1"/>
			<widget name="key_red" position="280,360" size="140,40" valign="center" halign="center" zPosition="5" transparent="1" foregroundColor="white" font="Regular;18"/>
			<widget name="key_green" position="420,360" size="140,40" valign="center" halign="center" zPosition="5" transparent="1" foregroundColor="white" font="Regular;18"/>
			<ePixmap name="red" pixmap="skin_default/buttons/red.png" position="280,360" size="140,40" zPosition="4" transparent="1" alphatest="on"/>
			<ePixmap name="green" pixmap="skin_default/buttons/green.png" position="420,360" size="140,40" zPosition="4" transparent="1" alphatest="on"/>
		</screen>"""


	def keyCancel(self):
		self.close()

	def keyLeft(self):
		ConfigListScreen.keyLeft(self)

	def keyRight(self):
		ConfigListScreen.keyRight(self)

	def setInitialize(self):
		Screen.setTitle(self, _("Restore from \"") + self.mountdescription + "\"")

	def __init__(self, session, mountdescription, mountpoint):
		Screen.__init__(self, session)
		self.skin_path = plugin_path

		self.mountdescription = mountdescription
		self.mountpoint = mountpoint

		self["key_green"] = Label(_("OK"))
		self["key_red"] = Label(_("Cancel"))
		self["green"] = Pixmap()
		self["red"] = Pixmap()

		self["actions"] = NumberActionMap(["SetupActions"],
		{
			"cancel": self.keyCancel,
			"left"	: self.keyLeft,
			"right"	: self.keyRight
		}, -1)

		self["shortcuts"] = ActionMap(["ShortcutActions"],
		{
			"red"	: self.keyCancel,
			"green"	: self.KeyOk,
		})

		self.list = []
		ConfigListScreen.__init__(self, self.list)
		self.createSetup()
		self.container = eConsoleAppContainer()
		self.onFirstExecBegin.append(self.setInitialize)


	def createSetup(self):
		print "Creating RestoreMenu"

		self.list = []
		almost_a_file= False

		self.configbackup = NoSave(ConfigYesNo(default = False))
		self.skinbackup = NoSave(ConfigYesNo(default = False))
		self.networkbackup = NoSave(ConfigYesNo(default = False))
		self.settingsbackup = NoSave(ConfigYesNo(default = False))
		self.varbackup = NoSave(ConfigYesNo(default = False))

		self.path = self.mountpoint
		if (path.exists(self.path) == False):
			makedirs(self.path)

		for file in listdir(self.path):
			if (file.endswith(".tgz")):
				if file == BackupIdentifier["general_configuration"]:
					almost_a_file = True
					self.configbackup.value = True
					self.configbackupEntry = getConfigListEntry(_("General Configuration"), self.configbackup)
					self.list.append(self.configbackupEntry)
				elif file == BackupIdentifier["all_skins"]:
					almost_a_file = True
					self.skinbackup.value = True
					self.skinbackupEntry = getConfigListEntry(_("All Skins"), self.skinbackup)
					self.list.append(self.skinbackupEntry)
				elif file == BackupIdentifier["network_setting"]:
					almost_a_file = True
					self.networkbackup.value = True
					self.networkbackupEntry = getConfigListEntry(_("Network Settings"), self.networkbackup)
					self.list.append(self.networkbackupEntry)
				elif file == BackupIdentifier["channel_settings"]:
					almost_a_file = True
					self.settingsbackup.value = True
					self.settingsbackupEntry = getConfigListEntry(_("Channel Settings"), self.settingsbackup)
					self.list.append(self.settingsbackupEntry)
				elif file == BackupIdentifier["var_folder"]:
					almost_a_file = True
					self.varbackup.value = True
					self.varbackupEntry = getConfigListEntry(_("VAR folder"), self.varbackup)
					self.list.append(self.varbackupEntry)

		if not almost_a_file:
			self["key_green"].hide()
			self["green"].hide()
			self.disable_ok = True
		else:
			self["key_green"].show()
			self["green"].show()
			self.disable_ok = False

		self["config"].list = self.list
		self["config"].l.setList(self.list)

	def KeyOk(self):
		if self.disable_ok == False:
			if not self.configbackup.value and not self.skinbackup.value and not self.networkbackup.value and not self.settingsbackup.value and not self.varbackup.value:
				return
			
			self.disable_ok = True
			self.session.openWithCallback(self.startRestore, MessageBox, _("are you sure you want to restore\nfollowing backup:\n"))


	def keyCancel(self):
		self.close()

	def startRestore(self, ret = False):
		if (ret == True):
			cmd = ""
			if self.configbackup.value:
				print "Restore: General Configuration"
				cmd += "tar -xzvf " + self.path + "/" + BackupIdentifier["general_configuration"] + " -C /;"

			if self.skinbackup.value:
				print "Restore: All Skins"
				cmd += "tar -xzvf " + self.path + "/" + BackupIdentifier["all_skins"] + " -C /;"

			if self.networkbackup.value:
				print "Restore: Network Settings"
				cmd += "tar -xzvf " + self.path + "/" + BackupIdentifier["network_setting"] + " -C /;"

			if self.settingsbackup.value:
				print "Restore: Channel Settings"
				cmd += "tar -xzvf " + self.path + "/" + BackupIdentifier["channel_settings"] + " -C /;"

			if self.varbackup.value:
				print "Restore: VAR folder"
				cmd += "tar -xzvf " + self.path + "/" + BackupIdentifier["var_folder"] + " -C /;"

			self.restoreMsg = self.session.open(MessageBox, _("Restore running ..."), MessageBox.TYPE_INFO, enable_input = False)
			self.container.appClosed.append(self.RestoreFinish)
			self.container.execute(cmd)
		else:
			self.disable_ok = False
	
	def RestoreFinish(self, retval):
		if QBOXHD:
			cmds="sync"
			self.container.execute(cmds)
		
		self.restoreMsg["text"].setText(_("Restore successfully!"))
		self.container.appClosed.remove(self.RestoreFinish)
		self.disable_ok = False
		sleep(2)
		self.steprestartcounter=0
		self.restart()


	def restart(self, ret = False):
		self.timer = eTimer()
		self.timer.callback.append(self.doStepRestart)
		self.timer.start(100)

	def doStepRestart(self):
		self.timer.stop()

		if (self.networkbackup.value):
			if self.steprestartcounter == 0:
				self.restoreMsg["text"].setText(_("Restarting Network ..."))

			elif self.steprestartcounter == 1:
				iNetwork.deactivateNetworkConfig()
				iNetwork.activateNetworkConfig()

			elif self.steprestartcounter == 2:
				self.restoreMsg["text"].setText(_("Restarting Enigma ..."))

			elif self.steprestartcounter == 3:
				self.session.open(Console, title = _("Restarting Enigma ..."), cmdlist = ["killall -9 enigma2"])
				return

		else:

			if self.steprestartcounter == 0:
				self.restoreMsg["text"].setText(_("Restarting Enigma ..."))

			elif self.steprestartcounter == 1:
				self.session.open(Console, title = _("Restarting Enigma ..."), cmdlist = ["killall -9 enigma2"])
				return

		print "[STEP] %s" % str(self.steprestartcounter)

		self.steprestartcounter+=1
		self.timer.start(100)


def BackupMain(session, **kwargs):
	session.open(MainBackupRestoreScreen)

def menu(menuid, **kwargs):
	if menuid == "installation_menu":
		return [(_("Backup/Restore"), BackupMain, "backuprestore", None)]

	return []

def Plugins(path, **kwargs):
	global plugin_path
	plugin_path = path
	plugins = list()
	plugins.append(PluginDescriptor(name="Backup/Restore", description="Backup and Restore your Settings", where = PluginDescriptor.WHERE_MENU, fnc=menu))

	return plugins;
