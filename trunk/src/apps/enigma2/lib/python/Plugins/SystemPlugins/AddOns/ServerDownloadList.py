from Components.MenuList import MenuList
from twisted.web.client import downloadPage

from Tools.Directories import resolveFilename, SCOPE_SKIN_IMAGE, fileExists, pathExists
from Components.MultiContent import MultiContentEntryText, MultiContentEntryPixmapAlphaTest

from enigma import eListboxPythonMultiContent, gFont
from Tools.LoadPixmap import LoadPixmap
import os

class ServerDownloadData:
	def __init__(self, title="", description="", link="", icon=""):
		self.title=str(title)
		self.description=str(description)
		self.link = str(link)
		self.icon = str(icon)


def ServerDownloadEntryComponent(server):
	res = [ server ]
	
	res.append(MultiContentEntryText(pos=(120, 5), size=(320, 25), font=0, text=server.title))
	res.append(MultiContentEntryText(pos=(120, 26), size=(320, 17), font=1, text=server.description))

	if len(server.icon) == 0:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/addon_server_download.png"))
	else:
		if fileExists( server.icon ):
			png = LoadPixmap(server.icon)
		else:
			png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/addon_server_download.png"))
		
	res.append(MultiContentEntryPixmapAlphaTest(pos=(10, 5), size=(100, 40), png = png))
	
	return res


class ServerDownloadList(MenuList):
	def __init__(self, list, enableWrapAround=False):
		MenuList.__init__(self, list, enableWrapAround, eListboxPythonMultiContent)
		self.l.setFont(0, gFont("Regular", 20))
		self.l.setFont(1, gFont("Regular", 14))
		self.l.setItemHeight(50)




class SectionDownloadData:
	
	SECTION           = 0
	ADDON             = 1
	
	def __init__(self, xmlNode=None, name="", description="", icon="", serverinfo=None):
		self.xmlNode=xmlNode
		self.name=str(name)
		self.description=str(description)
		self.icon = str(icon)
		self.serverinfo = serverinfo
		self.kind = SectionDownloadData.SECTION
		
		self.thumbnail = None
		
		
	def loadThumbnail(self, callback):
		if len(self.icon) > 0:
			thumbnailFile = "/tmp/" + os.path.basename(self.icon)
			thumbnailUrl = self.serverinfo.link + "/" + self.icon
			self.thumbnail = None
			cookie = {"entry" : self, "file" : thumbnailFile, "callback" : callback}
			downloadPage(thumbnailUrl, thumbnailFile).addCallback(self._fetchFinished, cookie).addErrback(self._fetchFailed, cookie)
		else:
			self.thumbnail = None
			callback( self )

			
	
	def _fetchFailed(self, string, cookie):
		if os.path.exists(cookie["file"]):
			os.remove(cookie["file"])
		cookie["callback"](cookie["entry"])


	def _fetchFinished(self, string, cookie):
		if os.path.exists(cookie["file"]):
			print "%s Loading filename %s" % ( cookie["entry"].name, cookie["file"] )
			cookie["entry"].thumbnail = LoadPixmap(cookie["file"])
			os.remove(cookie["file"])
		cookie["callback"](cookie["entry"])



def SectionDownloadEntryComponent(section):
	res = [ section ]
	
	res.append(MultiContentEntryText(pos=(120, 5), size=(320, 25), font=0, text=section.name))
	res.append(MultiContentEntryText(pos=(120, 26), size=(320, 17), font=1, text=section.description))

	if section.thumbnail is None:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/addon_section.png"))
	else:
		png = section.thumbnail
		
	res.append(MultiContentEntryPixmapAlphaTest(pos=(10, 5), size=(100, 40), png = png))
	
	return res



class SectionDownloadList(MenuList):
	def __init__(self, list, enableWrapAround=False):
		MenuList.__init__(self, list, enableWrapAround, eListboxPythonMultiContent)
		self.l.setFont(0, gFont("Regular", 20))
		self.l.setFont(1, gFont("Regular", 14))
		self.l.setItemHeight(60)
		


class SectionInstalledData:
	def __init__(self, name="", description="", icon="", addOnlist=[]):
		self.name=str(name)
		self.description=str(description)
		self.icon = str(icon)
		self.addOnlist = addOnlist
		


def SectionInstalledEntryComponent(section):
	res = [ section ]
	
	res.append(MultiContentEntryText(pos=(120, 5), size=(320, 25), font=0, text=section.name))
	res.append(MultiContentEntryText(pos=(120, 26), size=(320, 17), font=1, text=section.description))

	if len(section.icon) == 0:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/addon_section.png"))
	else:
		if fileExists( section.icon ):
			png = LoadPixmap(section.icon)
		else:
			png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/addon_section.png"))
		
	res.append(MultiContentEntryPixmapAlphaTest(pos=(10, 5), size=(100, 40), png = png))
	
	return res



class SectionInstalledList(MenuList):
	def __init__(self, list, enableWrapAround=False):
		MenuList.__init__(self, list, enableWrapAround, eListboxPythonMultiContent)
		self.l.setFont(0, gFont("Regular", 20))
		self.l.setFont(1, gFont("Regular", 14))
		self.l.setItemHeight(60)



class AddOnDownloadData(SectionDownloadData):
	def __init__(self, xmlNode=None, name="", type="", description="", version="", icon="", file="", uninstall_script="", serverinfo=None):
		
		SectionDownloadData.__init__(self, xmlNode, name, description, icon, serverinfo)
		
		self.kind = SectionDownloadData.ADDON
		
		self.type = str(type)
		self.version = str(version)
		self.file = str(file)
		self.uninstall_script = str(uninstall_script)

		



def AddOnDownloadEntryComponent(addOn):
	res = [ addOn ]
	
	version = "Version %s" % addOn.version
	
	res.append(MultiContentEntryText(pos=(120, 5), size=(320, 25), font=0, text=addOn.name))
	
	if len(addOn.version) > 0:
		res.append(MultiContentEntryText(pos=(120, 26), size=(320, 17), font=1, color = 0xFFA323, color_sel = 0xFFA323, text=version))
		res.append(MultiContentEntryText(pos=(120, 42), size=(320, 17), font=1, text=addOn.description))
	else:
		res.append(MultiContentEntryText(pos=(120, 26), size=(320, 17), font=1, text=addOn.description))


	if addOn.thumbnail is None:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/addon_download.png"))
	else:
		png = addOn.thumbnail
		
	res.append(MultiContentEntryPixmapAlphaTest(pos=(10, 10), size=(100, 50), png = png))
	
	return res



class AddOnInstalledData:
	def __init__(self, name="", description="", type="", version="", icon="", uninstall_script=""):
		self.name=str(name)
		self.description=str(description)
		self.icon = str(icon)
		self.type = str(type)
		self.version = str(version)
		self.uninstall_script = str(uninstall_script)
		
		

def AddOnInstalledEntryComponent(addOn):
	res = [ addOn ]
	
	version = "Version %s" % addOn.version
	
	res.append(MultiContentEntryText(pos=(120, 5), size=(320, 25), font=0, text=addOn.name))
	res.append(MultiContentEntryText(pos=(120, 26), size=(320, 17), font=1, color = 0xFFA323, color_sel = 0xFFA323, text=version))
	res.append(MultiContentEntryText(pos=(120, 42), size=(320, 17), font=1, text=addOn.description))
	
	if len(addOn.icon) == 0:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/addon_installed.png"))
	else:
		if fileExists( addOn.icon ):
			png = LoadPixmap(addOn.icon)
		else:
			png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/addon_installed.png"))

	res.append(MultiContentEntryPixmapAlphaTest(pos=(10, 10), size=(100, 50), png = png))
	
	return res
