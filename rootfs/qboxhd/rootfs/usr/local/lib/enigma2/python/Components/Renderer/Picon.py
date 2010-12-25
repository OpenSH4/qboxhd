##
## Picon renderer by Gruffy .. some speedups by Ghost
##
from qboxhd import QBOXHD, QBOXHD_MINI
from Renderer import Renderer
from enigma import ePixmap
from Tools.Directories import fileExists, SCOPE_SKIN_IMAGE, SCOPE_CURRENT_SKIN, resolveFilename
if QBOXHD:
	from Components.SystemInfo import SystemInfo

class Picon(Renderer):
	searchPaths = ['/usr/share/enigma2/%s/',
				'/media/cf/%s/',
				'/media/usb/%s/']

	def __init__(self):
		Renderer.__init__(self)
		self.path = "picon"
		self.nameCache = { }
		self.pngname = ""

	def applySkin(self, desktop, parent):
		attribs = [ ]
		for (attrib, value) in self.skinAttributes:
			if attrib == "path":
				self.path = value
			else:
				attribs.append((attrib,value))
		self.skinAttributes = attribs
		return Renderer.applySkin(self, desktop, parent)

	GUI_WIDGET = ePixmap

	def changed(self, what):
		if self.instance:
			pngname = ""
			if what[0] != self.CHANGED_CLEAR:
				sname = self.source.text
				# strip all after last :
				pos = sname.rfind(':')
				if pos != -1:
					sname = sname[:pos].rstrip(':').replace(':','_')
				pngname = self.nameCache.get(sname, "")
				if pngname == "":
					pngname = self.findPicon(sname)
					if pngname != "":
						self.nameCache[sname] = pngname
			if QBOXHD:
				if self.path == "piconlcd":
					if pngname == "": # no picon for service found
						pngname = self.nameCache.get("lcddefault", "")
						if pngname == "": # no default yet in cache..
							pngname = self.findPicon("piconlcd_default")
							if pngname == "":
								tmp = resolveFilename(SCOPE_CURRENT_SKIN, "piconlcd_default.png")
								if fileExists(tmp):
									pngname = tmp
								else:
									pngname = resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/piconlcd_default.png")
							self.nameCache["lcddefault"] = pngname
				else:
					if pngname == "": # no picon for service found
						pngname = self.nameCache.get("default", "")
						if pngname == "": # no default yet in cache..
							pngname = self.findPicon("picon_default")
							if pngname == "":
								if QBOXHD:
									if QBOXHD_MINI:
										tmp = resolveFilename(SCOPE_CURRENT_SKIN, "picon_qboxhd_mini_default.png")
									else:
										tmp = resolveFilename(SCOPE_CURRENT_SKIN, "picon_qboxhd_default.png")
								else:
									tmp = resolveFilename(SCOPE_CURRENT_SKIN, "picon_default.png")
								if fileExists(tmp):
									pngname = tmp
								else:
									pngname = resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/picon_default.png")
							self.nameCache["default"] = pngname
			else: # ! QBOXHD
				if pngname == "": # no picon for service found
					pngname = self.nameCache.get("default", "")
					if pngname == "": # no default yet in cache..
						pngname = self.findPicon("picon_default")
						if pngname == "":
							tmp = resolveFilename(SCOPE_CURRENT_SKIN, "picon_default.png")
							if fileExists(tmp):
								pngname = tmp
							else:
								pngname = resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/picon_default.png")
						self.nameCache["default"] = pngname

			if self.pngname != pngname:
				self.instance.setPixmapFromFile(pngname)
				self.pngname = pngname

	def findPicon(self, serviceName):
		for path in self.searchPaths:
			pngname = (path % self.path) + serviceName + ".png"
			if fileExists(pngname):
				return pngname
		return ""
