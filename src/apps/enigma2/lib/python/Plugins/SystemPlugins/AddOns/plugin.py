from qboxhd import QBOXHD
from Screens.Screen import Screen

from enigma import eConsoleAppContainer

from Components.ActionMap import ActionMap
from Components.Label import Label
from Screens.MessageBox import MessageBox
from Screens.Console import Console
from Tools.Directories import resolveFilename, fileExists, pathExists, SCOPE_PLUGINS, SCOPE_SKIN_IMAGE
from Tools.LoadPixmap import LoadPixmap

from ServerDownloadList import *
import xml.dom.minidom
from xml.dom.minidom import Node
from xml.dom.minidom import getDOMImplementation
from twisted.web.client import downloadPage2
from urllib2 import urlopen, Request, URLError, HTTPError, quote
from httplib import HTTPConnection,CannotSendRequest,BadStatusLine
from Components.Sources.Progress import Progress
import socket
import mechanize
from enigma import eTimer
from Tools.XMLTools import findChildrenByTagName, searchChildrenfromAttribute
from os import mkdir, path, remove, system

RESTART_ENIGMA_AFTER_OPERATION_FLAG = False
TURN_OFF_ON_QBOXHD_AFTER_INSTALL_FLAG = False
REBOOT_AFTER_INSTALL_FLAG = False
ADDON_INSTALLER_VERSION = "1.0.5"

class AddOnSectionBrowser(Screen):

	SERVERS_DOWNLOAD_LIST = resolveFilename(SCOPE_SKIN_IMAGE, "addon_servers_download.xml")

	def __init__(self, session):
		Screen.__init__(self, session)

		self["green"] = Label(_("Download AddOns"))

		self["text"] = Label(_(" "))

		self.onShown.append(self.refreshInfo)

		self.list = []
		self["list"] = SectionInstalledList( self.list )

		self.serverlist = self.refreshServersList()

		self["actions"] = ActionMap(["WizardActions", "ColorActions"],
		{
			"ok": self.enter,
			"back": self.close,
			"green": self.download
		})



	def refreshInfo(self):
		self.manager = AddOnInstalledManager()
		self.sectionInstalled = self.manager.get_Installed_Section()

		self.list = []
		for section in self.sectionInstalled:
			self.list.append(SectionInstalledEntryComponent(section))

		self["list"].l.setList(self.list)

		if (len(self.sectionInstalled) == 0 ):
			self["list"].instance.hide()
			self["text"].setText("No AddOns installed.")
		else:
			self["list"].instance.show()



	def enter(self):
		sel = self["list"].l.getCurrentSelection()

		if sel is None:
			return

		self.session.open( AddOnBrowser, sel[0] )



	def download(self):
		if len(self.serverlist) > 1:
			self.session.open( ChooseServerDownload, self.serverlist )
		elif len(self.serverlist) == 1:
			self.session.open( ChooseSectionDownloadable, self.serverlist[0])


	def refreshServersList(self):
		xmlserverdoc = None
		try:
			xmlserverfile = file(self.SERVERS_DOWNLOAD_LIST, 'r')
			xmlserverdoc = xml.dom.minidom.parseString(xmlserverfile.read())
			xmlserverfile.close()
		except:
			print "[AddOnBrowser] %s malformed. discard it." % self.SERVERS_DOWNLOAD_LIST
			return []

		if ( xmlserverdoc is None ):
			return []

		download_servers_node = xmlserverdoc.childNodes[0]

		servers = findChildrenByTagName( download_servers_node, "server" )

		if ( len(servers) < 1 ):
			return []

		serverlist = []
		for server in servers:
			serverlist.append( ServerDownloadData( server.getAttribute("title"), server.getAttribute("description"), server.getAttribute("link"), server.getAttribute("icon") ) )

		return serverlist



class AddOnBrowser(Screen):

	def __init__(self, session, section):
		Screen.__init__(self, session)

		self["red"] = Label(_("Back"))
		self["green"] = Label(_("Remove AddOn"))

		self["text"] = Label(_(" "))

		self.section = section

		self.onShown.append(self.refreshInfo)

		self.list = []
		self["list"] = SectionInstalledList(self.list)

		self["actions"] = ActionMap(["WizardActions", "ColorActions"],
		{
			"ok": self.info,
			"back": self.close,
			"red": self.close,
			"green": self.delete
		})


	def info(self):
		pass


	def refreshInfo(self):
		self.manager = AddOnInstalledManager()
		self.AddOnInstalled = self.manager.get_Installed_AddOn(self.section)
		self.list = []
		for addOn in self.AddOnInstalled:
			self.list.append(AddOnInstalledEntryComponent(addOn))

		self["list"].l.setList(self.list)

		if ( len(self.AddOnInstalled) == 0 ):
			self["list"].instance.hide()
			self["text"].setText("No AddOns installed.")
		else:
			self["list"].instance.show()


	def runDelete(self, answer):
		if answer:
			sel = self["list"].l.getCurrentSelection()

			if sel is None:
				return

			self.session.open( AddOnRemoveInProgress, self.section, sel[0] )


	def delete(self):
		sel = self["list"].l.getCurrentSelection()

		if sel is None:
			return

		self.session.openWithCallback(self.runDelete, MessageBox, _("Do you really want to remove\nthe addOn \"%s\"?") % sel[0].name)



class AddOnInstalledManager:

	ADDON_DIRECTORY = "/var/addons"
	ADDON_INSTALLED_XML = "addons_installed.xml"
	ADDON_XML_PATH = "%s/%s" % (ADDON_DIRECTORY, ADDON_INSTALLED_XML)


	def __init__(self):
		pass


	def _create_xml(self):
		impl = getDOMImplementation()
		# create document
		return impl.createDocument(None, "addon_installed", None)


	def _get_document(self):

		xml_addOninstalledDoc = None

		try:
			if fileExists( self.ADDON_XML_PATH ):
				xmlfile = file(self.ADDON_XML_PATH, 'r')
				xml_addOninstalledDoc = xml.dom.minidom.parseString(xmlfile.read())
				xmlfile.close()
			else:
				return None
		except:
			xml_addOninstalledDoc = None
			print "[AddOnInstalledManager] malformed xml. discard it."

		return xml_addOninstalledDoc


	def _create_section(self, xml_addOninstalledDoc, section):
		section_element = xml_addOninstalledDoc.createElement("category")

		xml_addOninstalledDoc.firstChild.appendChild(section_element)

		return section_element


	def _modify_section(self, xmlsectionNode, section):
		xmlsectionNode.setAttribute("name", section.name)
		xmlsectionNode.setAttribute("icon", section.icon)
		xmlsectionNode.setAttribute("description", section.description)


	def _get_section(self, xml_addOninstalledDoc, section):
		searchsection = []
		searchsection = searchChildrenfromAttribute( xml_addOninstalledDoc.firstChild, "category", "name", section.name )

		print "section n.:", len(searchsection)

		if len(searchsection) == 0:
			return None
		else:
			return searchsection[0]


	def _create_addOn(self, xml_addOninstalledDoc, xml_sectionNode, addOn ):
		addOn_element = xml_addOninstalledDoc.createElement("addon")

		xml_sectionNode.appendChild(addOn_element)

		return addOn_element



	def _modify_addOn( self, xmladdOnNode, addOn ):
		xmladdOnNode.setAttribute("name", addOn.name)
		xmladdOnNode.setAttribute("description", addOn.description)
		xmladdOnNode.setAttribute("type", addOn.type)
		xmladdOnNode.setAttribute("version", addOn.version)
		xmladdOnNode.setAttribute("uninstall_script", addOn.uninstall_script)
		xmladdOnNode.setAttribute("icon", addOn.icon)



	def _get_addOn(self, xml_addOninstalledDoc, xml_sectionNode, section, addOn):
		addOnsearch = []
		addOnsearch = searchChildrenfromAttribute( xml_sectionNode, "addon", "name", addOn.name )

		print "addOn n.:", len(addOnsearch)

		if len(addOnsearch) == 0:
			return None
		else:
			return addOnsearch[0]


	def _save_xml( self, xml_document):
		try:
			if not pathExists( self.ADDON_DIRECTORY ):
				mkdir( self.ADDON_DIRECTORY )

			save_file = open(self.ADDON_XML_PATH, 'w')
			#write the xml document to disc
			xml_document.documentElement.writexml(save_file)
			save_file.close()
		except IOError, (errno, strerror):
			print "[AddOnInstalledManager] Error saving post(%s): %s" % (errno, strerror)



	def get_Installed_Section(self):
		sectioninstalled = []

		# open xml addOn list
		xml_addOninstalledDoc = self._get_document()
		if ( xml_addOninstalledDoc is None ):
			return []

		sectionsnodes = findChildrenByTagName( xml_addOninstalledDoc.firstChild, "category")

		print "sections n.:", len(sectionsnodes)

		for section in sectionsnodes:
			addOnnodes = findChildrenByTagName( section, "addon" )

			print section.getAttribute("name"), " addOns n.:", len(addOnnodes)

			addOns = []
			for addOn in addOnnodes:
				addOns.append( AddOnInstalledData( addOn.getAttribute("name"), addOn.getAttribute("description"), addOn.getAttribute("type"), addOn.getAttribute("version"), addOn.getAttribute("icon"), addOn.getAttribute("uninstall_script" ) ) )

			sectioninstalled.append( SectionInstalledData( section.getAttribute("name"), section.getAttribute("description"), section.getAttribute("icon"), addOns ) )

		return sectioninstalled



	# Get Installed AddOn List from section installed
	def get_Installed_AddOn(self, section):
		addOninstalled = []

		# open xml addOn list
		xml_addOninstalledDoc = self._get_document()
		if ( xml_addOninstalledDoc is None ):
			return []

		sectionnode = self._get_section(xml_addOninstalledDoc, section)
		if sectionnode is None:
			return []

		addOnnodes = findChildrenByTagName( sectionnode, "addon")

		print sectionnode.getAttribute("name"), " addOns n.:", len(addOnnodes)

		for addOn in addOnnodes:
			addOninstalled.append( AddOnInstalledData( addOn.getAttribute("name"), addOn.getAttribute("description"), addOn.getAttribute("type"), addOn.getAttribute("version"), addOn.getAttribute("icon"), addOn.getAttribute("uninstall_script" ) ) )

		return addOninstalled




	# Add new addOn to install list
	def add_AddOn(self, addOn, section, serverinfo):

		# open xml addOn list
		xml_addOninstalledDoc = self._get_document()
		if ( xml_addOninstalledDoc is None ):
			print "[AddOnInstalledManager] create document"
			xml_addOninstalledDoc = self._create_xml()

		# find same section
		sectionnode = self._get_section( xml_addOninstalledDoc, section )
		if sectionnode is None:
			print "[AddOnInstalledManager] no section found. Create section"
			sectionnode = self._create_section( xml_addOninstalledDoc, section )

		self._modify_section( sectionnode, section )

		# find if exist addOn
		addOnode = self._get_addOn( xml_addOninstalledDoc, sectionnode, section, addOn )
		if addOnode is None:
			print "[AddOnInstalledManager] no addOn found. Create addOn"
			addOnode = self._create_addOn( xml_addOninstalledDoc, sectionnode, addOn )

		self._modify_addOn( addOnode, addOn )

		# save xml
		self._save_xml( xml_addOninstalledDoc )



	#Remove exist addOn from install list
	def remove_AddOn(self, addOn, section):

		# open xml addOn list
		xml_addOninstalledDoc = self._get_document()
		if ( xml_addOninstalledDoc is None ):
			return

		# find same section
		sectionnode = self._get_section( xml_addOninstalledDoc, section )
		if sectionnode is None:
			return

		# find if exist addOn
		addOnode = self._get_addOn( xml_addOninstalledDoc, sectionnode, section, addOn )
		if addOnode is None:
			return

		# remove xml node
		sectionnode.removeChild( addOnode )

		#if section has not other addon node, delete it.
		addOnnodes = findChildrenByTagName( sectionnode, "addon")
		print sectionnode.getAttribute("name"), " addOns n.:", len(addOnnodes)
		if ( len( addOnnodes ) == 0 ):
			xml_addOninstalledDoc.firstChild.removeChild( sectionnode )

		# save xml
		self._save_xml( xml_addOninstalledDoc )



class ChooseServerDownload(Screen):
	def __init__(self, session, serverlist):
		Screen.__init__(self, session)

		self["red"] = Label(_("Back"))
		self["green"] = Label(_("Select"))

		self.serverlist = serverlist

		self.list = []
		self["list"] = ServerDownloadList(self.list)
		self["list"].l.setList(self.list)

		self.refreshServersList(self.serverlist)

		self["actions"] = ActionMap(["WizardActions"],
		{
			"ok": self.go,
			"back": self.close,
			"red": self.close,
			"green": self.go
		})


	def refreshServersList(self, serverlist):
		for server in serverlist:
			self.list.append( ServerDownloadEntryComponent(server) )

		self["list"].l.setList(self.list)


	def go(self):
		sel = self["list"].getSelectedIndex()
		self.session.open( ChooseSectionDownloadable, self.serverlist[sel])



class ServerDownloadQuery:

	URL_ERROR              = -3
	NOT_VALID_SERVER_XML   = -2
	NO_DATA_RECEIVED       = -1
	SERVER_DOWN            =  0
	QUERY_OK               =  1

	def __init__(self, browser, serverinfo):
		self.serverinfo = serverinfo
		self.browser = browser

	def query(self):
		data = None
		xmldoc = None

		try:
			data = self.browser.open(mechanize.Request(self.serverinfo.link + "/server_addons.xml"))

		except (CannotSendRequest, socket.gaierror, socket.error, mechanize.HTTPError, ValueError, BadStatusLine):
			print "[ServerDownloadQuery] Can not send request for addOnlist"
			return None, self.SERVER_DOWN
		except (URLError):
			return None, self.URL_ERROR

		if data is None:
			return None, self.NO_DATA_RECEIVED

		try:
			xmldoc = xml.dom.minidom.parseString(data.get_data())
		except:
			print "[ServerDownloadQuery] Not valid xml."
			return None, self.NOT_VALID_SERVER_XML

		return xmldoc, self.QUERY_OK



class ServerDownloadBrowser:
		# Operation permitted
		DOWNLOAD = 0
		REMOVE = 1

		# Info Status
		INSTALLER_EXPIRED = -2
		ERROR   = -1
		OFFLINE = 0
		OK 	    = 1

		def __init__(self, serverinfo):
			self.browser = mechanize.Browser()
			self.browser.addheaders = []
			self.serverinfo = serverinfo
			self.xmldoc = None
			self.sections = []
			self.addons = []
			self.rootNode = None
			self.currentNode = None
			self.currentSection = None
			self.maincurrentSection = None


		def getMainSectionName(self):
			if ( self.maincurrentSection != None ):
				return self.maincurrentSection.xmlNode.getAttribute("name")
			else:
				return ""


		def getInfoFromPreviousNode(self):
			if not self.isMainSection():
				self.getInfoFromSelectedNode(self.currentNode.parentNode)



		def getInfoFromSelectedNode(self, selNode):
			self.sections = []
			self.addons = []

			sectionsnodes = findChildrenByTagName( selNode, "section")
			addOnnodes = findChildrenByTagName( selNode, "addon")

			print "sections n.:", len(sectionsnodes)
			print "addons n.:", len(addOnnodes)

			self.currentNode = selNode

			for section in sectionsnodes:
				self.sections.append( SectionDownloadData( section, section.getAttribute("name"), section.getAttribute("description"), section.getAttribute("icon"), self.serverinfo ) )

			# In main section there isn't any addons
			if self.rootNode != self.currentNode:
				for addOn in addOnnodes:
					self.addons.append( AddOnDownloadData( addOn, addOn.getAttribute("name"), addOn.getAttribute("type"), addOn.getAttribute("description"), addOn.getAttribute("version"), addOn.getAttribute("icon"), addOn.getAttribute("file"), addOn.getAttribute("uninstall_script" ), self.serverinfo ) )

			if not self.isMainSection():
				self.currentSection = SectionDownloadData( selNode, selNode.getAttribute("name"), selNode.getAttribute("description"), selNode.getAttribute("icon"), self.serverinfo )

				if self.maincurrentSection is None:
					self.maincurrentSection = self.currentSection
			else:
				self.currentSection = None
				self.maincurrentSection = None


		def verify_installer_version( self, productxmlNode ):
			# check for installer version ...

			installerversion = productxmlNode.getAttribute("version")

			if ( len(installerversion) == 0 ):
				return False

			installer_version = installerversion.strip().split(".", 3)
			curr_installer_version = ADDON_INSTALLER_VERSION.strip().split(".", 3)

			for i in range(0, 3):
				if ( int(curr_installer_version[i]) > int(installer_version[i]) ):
					return True
				elif ( int(curr_installer_version[i]) == int(installer_version[i]) ):
					continue
				else:
					return False

			return True


		def _getIdentifierParameter(self, configStr, identifier):
			for s in configStr:
				split = s.strip().split(': ',1)
				if split[0] == identifier:
					return split[1]#[0:-1]

			return None


		# Read STB info
		def getSTBInfo(self):

			if not fileExists( "/proc/stb/info/model" ):
				return ""

			fp = file('/proc/stb/info/model', 'r')
			status = fp.read()
			fp.close()

			if status.find("qboxhd-mini") >= 0:
				print "STB Type: QBoxHD mini board"
				return "QBoxHD_mini"
			elif status.find("qboxhd") >= 0:
				print "STB Type: QBoxHD board"
				return "QBoxHD_r1"
			else:
				print "STB Type: <UNKNOWN>"
				return ""


		# Read sections and addOns from server
		def getInfo(self):
			self.server = ServerDownloadQuery( self.browser, self.serverinfo )

			# Get sections form server
			self.xmldoc, result = self.server.query()

			if result != ServerDownloadQuery.QUERY_OK:
				return ServerDownloadBrowser.ERROR, "", result

			servers_node = findChildrenByTagName(self.xmldoc, "server")

			print "server n.:",  len(servers_node)

			if len(servers_node) != 1:
				return ServerDownloadBrowser.ERROR, "", ServerDownloadQuery.QUERY_OK

			productInfo = self.getSTBInfo()

			if len(productInfo) == 0:
				 return ServerDownloadBrowser.ERROR, "", ServerDownloadQuery.QUERY_OK

			product = searchChildrenfromAttribute( servers_node[0], "product", "name", productInfo )

			print "product n.:", len(product)

			if len(product) != 1:
				return ServerDownloadBrowser.ERROR, "", ServerDownloadQuery.QUERY_OK

			attr_status = servers_node[0].getAttribute("status")
			attr_msg = servers_node[0].getAttribute("message")

			if attr_status == "OFFLINE":
				return ServerDownloadBrowser.OFFLINE, attr_msg, ServerDownloadQuery.QUERY_OK

			elif attr_status != "OK" :
				return ServerDownloadBrowser.ERROR, "", ServerDownloadQuery.QUERY_OK

			# Get information about Addon installer
			addon_installer_node = findChildrenByTagName(product[0], "addon_installer")

			if len(addon_installer_node) != 1:
				return ServerDownloadBrowser.ERROR, "", ServerDownloadQuery.QUERY_OK

			self.rootNode = product[0]
			self.currentSection = None
			self.maincurrentSection = None

			# verify installer version
			if not self.verify_installer_version(addon_installer_node[0]):
				print "[verify_installer_version FAILED]"
				self.addons.append( AddOnDownloadData( addon_installer_node[0], addon_installer_node[0].getAttribute("name"), addon_installer_node[0].getAttribute("type"), addon_installer_node[0].getAttribute("description"), addon_installer_node[0].getAttribute("version"), addon_installer_node[0].getAttribute("icon"), addon_installer_node[0].getAttribute("file"), addon_installer_node[0].getAttribute("uninstall_script" ), self.serverinfo ) )

				self.currentNode = self.rootNode

				return ServerDownloadBrowser.INSTALLER_EXPIRED, attr_msg, ServerDownloadQuery.QUERY_OK

			else:
				self.getInfoFromSelectedNode(self.rootNode)

			print "[ServerDownloadBrowser OK]"
			status = ServerDownloadBrowser.OK

			return status, attr_msg, ServerDownloadQuery.QUERY_OK


		# Read all sections
		def getSections(self):
			return self.sections


		# Read all addons
		def getAddOn(self):
			return self.addons


		# Return True is User is in Main Section
		def isMainSection(self):
			return ( self.rootNode == self.currentNode )



class ChooseSectionDownloadable(Screen):

	def __init__(self, session, serverinfo):
		Screen.__init__(self, session)

		self.onLayoutFinish.append(self.firstRun)

		self.serverinfo = serverinfo

		self.list = []
		self["list"] = SectionDownloadList(self.list)

		self["text"] = Label(_("Downloading server information. Please wait..."))

		self["actions"] = ActionMap(["WizardActions"],
		{
			"ok": self.go,
			"back": self.close_download,
		})

		global RESTART_ENIGMA_AFTER_OPERATION_FLAG
		RESTART_ENIGMA_AFTER_OPERATION_FLAG = False

		global TURN_OFF_ON_QBOXHD_AFTER_INSTALL_FLAG
		TURN_OFF_ON_QBOXHD_AFTER_INSTALL_FLAG = False

		global REBOOT_AFTER_INSTALL_FLAG
		REBOOT_AFTER_INSTALL_FLAG = False

		self.Timer = eTimer()
		self.Timer.callback.append(self.startRun)
		self.Timer.start(1000, True)



	def chooseRestart(self, answer):
		if answer:
			from Screens.Standby import TryQuitMainloop
			self.session.open(TryQuitMainloop, 3) #Restart Enigma2
			self.close()
		else:
			self.close()


	def chooseReboot(self, answer):
		if answer:
			from Screens.Standby import TryQuitMainloop
			self.session.open(TryQuitMainloop, 2) #Reboot
			self.close()
		else:
			self.close()


	def close_download(self):

		if self.serverbrowser == None:
			self.close()

		if self.serverbrowser.isMainSection():
			global RESTART_ENIGMA_AFTER_OPERATION_FLAG
			global TURN_OFF_ON_QBOXHD_AFTER_INSTALL_FLAG

			if TURN_OFF_ON_QBOXHD_AFTER_INSTALL_FLAG:
				self.session.openWithCallback(None, MessageBox, "For view new features installed, you have to turn OFF and turn ON your QBoxHD.", MessageBox.TYPE_INFO)
				self.close()

			elif REBOOT_AFTER_INSTALL_FLAG:
				self.session.openWithCallback(self.chooseReboot, MessageBox, "For view new features installed, you have to Reboot your QBoxHD.Reboot it now?")

			elif RESTART_ENIGMA_AFTER_OPERATION_FLAG:
				self.session.openWithCallback(self.chooseRestart, MessageBox, "For view new features installed, you have to restart Enigma2.\nRestart it now?")



			else:
				self.close()
		else:
			self.serverbrowser.getInfoFromPreviousNode()
			self.refreshItemList(0)


	def firstRun(self):
		self["list"].instance.hide()
		self.setTitle(_("Downloadable addOns"))


	def insertEntry(self, item):
		if ( item.kind == SectionDownloadData.SECTION ):
			self.list.append(SectionDownloadEntryComponent(item))

		elif ( item.kind == SectionDownloadData.ADDON ):
			self.list.append(AddOnDownloadEntryComponent(item))

		self["list"].setList(self.list)
		self["list"].instance.show()


	def refreshItemList(self, answer):
		self.list = []
		if ( ( len(self.serverbrowser.sections) == 0 ) and ( len(self.serverbrowser.addons) == 0 ) ):
			self["list"].instance.hide()
			txt = _( "No new %s found." ) % self.serverbrowser.getMainSectionName()
			self["text"].setText(str(txt))
			return

		for item in self.serverbrowser.sections:
			item.loadThumbnail(self.insertEntry)

		for item in self.serverbrowser.addons:
			item.loadThumbnail(self.insertEntry)



	def startRun(self):
		self.Timer.stop()
		self.serverbrowser = ServerDownloadBrowser(self.serverinfo)

		result, msg, errcode = self.serverbrowser.getInfo()

		if ( result == ServerDownloadBrowser.OFFLINE ):
			if (len(msg) > 0):
				self["text"].setText( str(msg) )
			return

		elif ( result == ServerDownloadBrowser.INSTALLER_EXPIRED ):
			self.session.openWithCallback(self.refreshItemList, MessageBox, _("Your installer is Expired.\nPlease download new AddOn installer."), MessageBox.TYPE_INFO)
			return

		elif ( result != ServerDownloadBrowser.OK ):
			if errcode == ServerDownloadQuery.URL_ERROR:
				self["text"].setText( "Could not establish a connection to the server.\nPlease check if the wire is connected or if your internet connection is working properly." )
			elif errcode == ServerDownloadQuery.NO_DATA_RECEIVED:
				self["text"].setText( "No data received from Server." )
			elif errcode == ServerDownloadQuery.SERVER_DOWN or errcode == ServerDownloadQuery.NOT_VALID_SERVER_XML:
				self["text"].setText( "Server not Ready." )
			else:
				self["text"].setText( "Server Error." )

			return

		if (len(msg) > 0):
			self.session.openWithCallback(self.refreshItemList, MessageBox, str(msg), MessageBox.TYPE_INFO)
		else:
			self.refreshItemList(0)


	def getFileSize(self, uri):
		try:
			import urllib

			file = urllib.urlopen(uri)
			size = file.headers.get("content-length")
			file.close()

			value = int(size)

		except:
			return 0

		return int(size)


	def rundownloadInstaller(self, answer):

		if answer:
			# check if it is a valid link
			if ( len(item.file) == 0 ):
				self.session.open(MessageBox,  _("Link not present."), MessageBox.TYPE_ERROR)
				self.close()
				return

			remotefile = self.serverinfo.link + "/" + item.file

			if ( self.getFileSize(remotefile) == 0 ):
				self.session.open(MessageBox,  _("%s not exist") % remotefile, MessageBox.TYPE_ERROR)
				self.close()
				return

			self.session.open(AddOnDownloadAndInstallInProgress, sel[0], self.serverbrowser.maincurrentSection, self.serverinfo)
		else:
			self.close()


	def runInstall(self, answer):
		if answer:
			sel = self["list"].l.getCurrentSelection()

			if sel is None:
				return

			self.session.open(AddOnDownloadAndInstallInProgress, sel[0], self.serverbrowser.maincurrentSection, self.serverinfo)


	def downloadAddOn(self, item):
		# check if it is a valid link
		if ( len(item.file) == 0 ):
			self.session.open(MessageBox,  _("Link not present."), MessageBox.TYPE_ERROR)
			return

		remotefile = self.serverinfo.link + "/" + item.file

		if ( self.getFileSize(remotefile) == 0 ):
			self.session.open(MessageBox,  _("%s not exist") % remotefile, MessageBox.TYPE_ERROR)
			return

		self.session.openWithCallback(self.runInstall, MessageBox, _("Do you really want to download\nthe addOn \"%s\"?") % item.name)



	def go(self):

		sel = self["list"].l.getCurrentSelection()
		if sel is None:
			return

		# addon selected go to download it
		if sel[0].kind == SectionDownloadData.ADDON:
			self.downloadAddOn(sel[0])

		#enter to subsection
		elif sel[0].kind == SectionDownloadData.SECTION:
			self.serverbrowser.getInfoFromSelectedNode(sel[0].xmlNode)
			self.refreshItemList(0)


class AddOnRemoveInProgress(Screen):

	PROGRESS_TEXT_REMOVE = "/.remove_progress"

	# Step Operation
	ERROR		  = 0
	REMOVING	  = 1
	REMOVED		  = 2

	def __init__(self, session, section, addOn ):
		Screen.__init__(self, session)

		self.onLayoutFinish.append(self.firstRun)

		self.section = section
		self.addOn = addOn

		self.container = eConsoleAppContainer()
		self.container.appClosed.append(self.removeFinished)

		self.state = self.REMOVING
		self.timerMonitor = eTimer()

		self["text"] = Label(_("Removing %s ..." % self.addOn.name))
		self["progress"] = Progress()
		self["green"] = Label(_("OK"))
		self["text_progress"] = Label(_("Waiting to removing ..."))

		self["actions"] = ActionMap(["WizardActions", "ColorActions"],
		{
			"green": self.terminate,
			"ok": self.terminate,
		})

		global RESTART_ENIGMA_AFTER_OPERATION_FLAG
		RESTART_ENIGMA_AFTER_OPERATION_FLAG = False



	def chooseRestart(self, answer):
		if answer:
			from Screens.Standby import TryQuitMainloop
			self.session.open(TryQuitMainloop, 3) #Restart Enigma2
			self.close()
		else:
			self.close()


	def close_download(self):
		global RESTART_ENIGMA_AFTER_OPERATION_FLAG
		if RESTART_ENIGMA_AFTER_OPERATION_FLAG:
			self.session.openWithCallback(self.chooseRestart, MessageBox, "For remove features uninstalled, you have to restart Enigma2.\nRestart it now?")
		else:
			self.close()


	def terminate(self):
		if ( self.state == self.ERROR or self.state == self.REMOVED ):
			self.timerMonitor.stop()
			self.close_download()


	def removeFinished(self, retval):
		if retval == 0:
			self.state = self.REMOVED
		else:
			self.state = self.ERROR


	def removeAddOn(self):
		if len(self.addOn.uninstall_script) == 0:
			self.state = self.REMOVED
			return

		if not fileExists( self.addOn.uninstall_script ):
			self.state = self.ERROR
			return

		self.container.execute(self.addOn.uninstall_script)



	def TimerMonitorStateHandler(self):
		self.timerMonitor.stop()
		active_timer = self.MonitorState()

		if ( active_timer ):
			self.timerMonitor.start(500)


	def removeTemporanyFile(self):
		# Remove progress text
		if fileExists(self.PROGRESS_TEXT_REMOVE):
			remove(self.PROGRESS_TEXT_REMOVE)


	def removeUninstallScript(self):
		if not fileExists( self.addOn.uninstall_script ):
			return

		remove( self.addOn.uninstall_script )


	def manage_type(self):
		if ( self.addOn.type.find("RESTART_ENIGMA_AFTER_INSTALL") >= 0 ):
			global RESTART_ENIGMA_AFTER_OPERATION_FLAG
			RESTART_ENIGMA_AFTER_OPERATION_FLAG = True


	def MonitorState(self):
		if self.state == self.ERROR:
			self.setTitle(_("AddOn error"))
			self["text"].text= _("Error to remove %s." % self.addOn.name)
			self["text_progress"].text = (_(" "))
			self["progress"].value = 100

			self["green"].instance.show()

			self.removeTemporanyFile()


		elif self.state == self.REMOVING:
			self.setTitle(_("Removing ..."))
			self["green"].instance.hide()
			self["text"].text= _(("Removing %s. Please wait ...") % self.addOn.name)

			# Set progress text if present
			if fileExists(self.PROGRESS_TEXT_REMOVE):
				fileprogresstext = file(self.PROGRESS_TEXT_REMOVE, 'r')
				self["text_progress"].text = fileprogresstext.read()
				fileprogresstext.close()
			else:
				self["text_progress"].text = (_(" "))

			return True


		elif self.state == self.REMOVED:
			self["progress"].value = 100
			self.setTitle(_("AddOn Removed"))
			print "%s Removed successffully" % self.addOn.name
			self["text"].text= _(("%s removed successffully.") % self.addOn.name)
			self["text_progress"].text = (_(" "))

			self["green"].instance.show()

			self.manager = AddOnInstalledManager()
			self.manager.remove_AddOn( self.addOn, self.section )

			self.removeUninstallScript()
			self.removeTemporanyFile()

			self.manage_type()

		return False


	def firstRun(self):
		self["green"].instance.hide()

		self.timerMonitor.callback.append(self.TimerMonitorStateHandler)
		self.timerMonitor.start(500)

		self.removeAddOn()



class AddOnDownloadAndInstallInProgress(Screen):

	TEMPORANY_PATH = "/qboxhd_update"
	INSTALL_SCRIPT = "/post_install.sh"
	PROGRESS_TEXT_INSTALL = "/.install_progress"

	# Step Operation
	ERROR		  = 0
	STOPPED		  = 1
	DOWNLOADING   = 2
	DOWNLOADED    = 3
	INSTALLING    = 4
	INSTALLED	  = 5

	def __init__(self, session, addOn, section, serverinfo):
		Screen.__init__(self, session)

		self.onLayoutFinish.append(self.firstRun)

		self.addOn = addOn
		self.serverinfo = serverinfo
		self.section = section
		self.clientConn = None
		self.remotefile = self.serverinfo.link + "/" + self.addOn.file
		self.localfile = self.TEMPORANY_PATH + "/" + path.basename( self.addOn.file )
		self.size = self.getFileSize(self.remotefile)

		self.tarcontainer = eConsoleAppContainer()
		self.tarcontainer.appClosed.append(self.tarFinished)

		self.container = eConsoleAppContainer()
		self.container.appClosed.append(self.installFinished)


		self.state = self.DOWNLOADING
		self.timerMonitor = eTimer()

		print "remotefile: ", self.remotefile
		print "remote file size: ", self.size
		print "localfile: ", self.localfile

		# Clean all previous install
		self.removeTemporanyFile()

		# Create temporany path
		if not pathExists(self.TEMPORANY_PATH):
			mkdir( self.TEMPORANY_PATH )

		self["text"] = Label(_("Downloading %s ..." % self.addOn.name))
		self["progress"] = Progress()
		self["red"] = Label(_("Stop Download"))
		self["green"] = Label(_("OK"))
		self["text_progress"] = Label(_("Waiting to start download ..."))

		self["actions"] = ActionMap(["WizardActions", "ColorActions"],
		{
			"red": self.stopOperation,
			"back": self.stopOperation,
			"green": self.terminate,
			"ok": self.terminate,
		})


	def terminate(self):
		if ( self.state == self.ERROR or self.state == self.INSTALLED or  self.state == self.STOPPED ):
			self.timerMonitor.stop()
			self.close()


	def getFileSize(self, uri):
		import urllib

		file = urllib.urlopen(uri)
		size = file.headers.get("content-length")
		file.close()
		return int(size)


	def downloadAddOnFinished(self, string):
		if self.state != self.STOPPED:
			print "finish OK download"
			self.state = self.DOWNLOADED
		else:
			self.MonitorState()


	def downloadAddOnFailed(self, string):
		if self.state != self.STOPPED:
			print "finish ERROR download"
			self.state = self.ERROR
		else:
			self.MonitorState()


	def stopOperation(self):
		if self.state == self.DOWNLOADING:
			self.state = self.STOPPED


	def removeTemporanyFile(self):
		#FIXME: Remove return for delete temporany file
#		return
		#-------------------------------
		# Remove temporany path
		if pathExists(self.TEMPORANY_PATH):
			system("rm -rf " + str(self.TEMPORANY_PATH))

		# Remove installation script
		if fileExists(self.INSTALL_SCRIPT):
			remove(self.INSTALL_SCRIPT)

		# Remove progress text
		if fileExists(self.PROGRESS_TEXT_INSTALL):
			remove(self.PROGRESS_TEXT_INSTALL)


	def installFinished(self, retval):
		print "installFinished: ", str(retval)
		if retval == 0:
			self.state = self.INSTALLED
		else:
			self.state = self.ERROR


	def tarFinished(self, retval):
		print "tarFinished: ", str(retval)
		if retval == 0:
			#extract all to directly in root and execute post_install.sh script
			if fileExists( str(self.INSTALL_SCRIPT) ):
				cmd = "cd / && ." + str(self.INSTALL_SCRIPT)
				self.container.execute(cmd)
			else:
				self.state = self.INSTALLED
		else:
			self.state = self.ERROR



	def installAddOn(self):
		cmd = "tar -vxf " + str(self.localfile) + " -C /"
		self.tarcontainer.execute(cmd)


	def runInstall(self, answer):
		if answer:
			self.timerMonitor.start(500)
			print "Choose Install"
			self.state = self.INSTALLING

			self.installAddOn()

		else:
			print "Choose Abort install"
			self.state = self.STOPPED

		self.MonitorState()


	def TimerMonitorStateHandler(self):
		self.timerMonitor.stop()
		active_timer = self.MonitorState()

		if ( active_timer ):
			self.timerMonitor.start(500)


	def _manage_addOn_type(self):
		if ( self.addOn.type.find("INSTALL_AND_REMOVE_ADDON") >= 0 ):
			manager = AddOnInstalledManager()
			manager.add_AddOn( self.addOn, self.section, self.serverinfo )

		if ( self.addOn.type.find("RELOAD_SETTINGS") >= 0 ):
			self["text_progress"].text = (_("Reloading Settings. Please wait ..."))
			from enigma import eDVBDB
			eDVBDB.getInstance().reloadServicelist()
			eDVBDB.getInstance().reloadBouquets()

		if ( self.addOn.type.find("RESTART_ENIGMA_AFTER_INSTALL") >= 0 ):
			global RESTART_ENIGMA_AFTER_OPERATION_FLAG
			RESTART_ENIGMA_AFTER_OPERATION_FLAG = True

		if ( self.addOn.type.find("TURN_OFF_ON_QBOXHD_AFTER_INSTALL") >= 0 ):
			global TURN_OFF_ON_QBOXHD_AFTER_INSTALL_FLAG
			TURN_OFF_ON_QBOXHD_AFTER_INSTALL_FLAG = True

		if ( self.addOn.type.find("REBOOT_AFTER_INSTALL") >= 0 ):
			global REBOOT_AFTER_INSTALL_FLAG
			REBOOT_AFTER_INSTALL_FLAG = True


	def MonitorState(self):
		if self.state == self.DOWNLOADING:
			self.setTitle(_("Downloading ..."))
			self["text"].text= _("Downloading %s ..." % self.addOn.name)

			if fileExists(self.localfile):
				currentsize = path.getsize( self.localfile )
			else:
				currentsize = 0

			print "Size: ", currentsize
			percent = 100 * currentsize / float(self.size)

			Gcurrentvalue = float(currentsize)/1024
			Gtotalvalue = float(self.size)/1024

			self["text_progress"].text =  "(%d.%03d MB / %d.%03d MB)" % ( (Gcurrentvalue/1024), (Gcurrentvalue%1024), (Gtotalvalue/1024), (Gtotalvalue%1024) )
			self["progress"].value = int(percent)

			self["red"].instance.show()
			self["green"].instance.hide()

			return True


		elif self.state == self.ERROR:
			self.setTitle(_("AddOn error"))
			self["text"].text= _("Error to install %s." % self.addOn.name)
			self["text_progress"].text = (_(" "))
			self["progress"].value = 100
			if self.clientConn is not None:
				self.clientConn.disconnect()

			self["red"].instance.hide()
			self["green"].instance.show()

			self.removeTemporanyFile()


		elif self.state == self.STOPPED:
			self.setTitle(_("Operation Stopped"))
			if self.clientConn is not None:
				self.clientConn.disconnect()

			self["text"].text= _("Operation stopped by user.")
			self["text_progress"].text = (_(" "))
			self["progress"].value = 100

			self["red"].instance.hide()
			self["green"].instance.show()

			self.removeTemporanyFile()


		elif self.state == self.DOWNLOADED:
			self.setTitle(_("Download Terminate"))
			self["text"].text= _("Download Terminate.")
			self["text_progress"].text = (_(" "))
			self["progress"].value = 100
			print "Download terminated successffully"

			self["red"].instance.hide()
			self["green"].instance.hide()

			self.session.openWithCallback(self.runInstall, MessageBox, _("Do you really want to install\nthe addOn \"%s\"?") % self.addOn.name)


		elif self.state == self.INSTALLING:
			self.setTitle(_("Installing ..."))
			self["red"].instance.hide()
			self["green"].instance.hide()
			self["text"].text= _(("Installing %s. Please wait ...") % self.addOn.name)

			# Set progress text if present
			if fileExists(self.PROGRESS_TEXT_INSTALL):
				fileprogresstext = file(self.PROGRESS_TEXT_INSTALL, 'r')
				self["text_progress"].text = fileprogresstext.read()
				fileprogresstext.close()
			else:
				self["text_progress"].text = (_(" "))

			return True


		elif self.state == self.INSTALLED:

			self._manage_addOn_type()

			self.setTitle(_("AddOn Installed"))
			if QBOXHD:
				cmds="sync"
				self.container.execute(cmds)

			print "%s Installed successffully" % self.addOn.name
			self["text"].text= _(("%s installed successffully.") % self.addOn.name)
			self["text_progress"].text = (_(" "))

			self["red"].instance.hide()
			self["green"].instance.show()

			self.removeTemporanyFile()
			if QBOXHD:
				cmds="sync"
				self.container.execute(cmds)


		return False



	def firstRun(self):

		self["red"].instance.show()
		self["green"].instance.hide()

		self.timerMonitor.callback.append(self.TimerMonitorStateHandler)
		self.timerMonitor.start(500)

		if self.size == 0:
			self.state = self.ERROR
		else:
			defered, downloader, self.clientConn  = downloadPage2(self.remotefile, self.localfile)

			defered.addErrback(self.downloadAddOnFailed)
			defered.addCallback(self.downloadAddOnFinished)


def main(session, **kwargs):
	session.open(AddOnSectionBrowser)

def menu(menuid, **kwargs):
	if menuid == "installation_menu":
		return [(_("AddOns"), main, "addons", None)]
	return []

from Plugins.Plugin import PluginDescriptor
def Plugins(**kwargs):
	return [PluginDescriptor(name = "AddOns", description = "Add new feature to your QBox-HD.", where = PluginDescriptor.WHERE_MENU, fnc = menu)]
