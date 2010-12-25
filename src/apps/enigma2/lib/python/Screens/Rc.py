from qboxhd import QBOXHD, QBOXHD_MINI
from Components.Pixmap import MovingPixmap, MultiPixmap
from Tools.Directories import resolveFilename, SCOPE_SKIN
#from elementtree import ElementTree
from xml.etree.ElementTree import ElementTree
from Components.config import config, ConfigInteger

if QBOXHD:
	from enigma import eActionMap

	if QBOXHD_MINI:
		config.misc.rcused = ConfigInteger(default = 1)
		config.misc.rcused.value = 1
	else:
		config.misc.rcused = ConfigInteger(default = 0)
		config.misc.rcused.value = 0
else:
	config.misc.rcused = ConfigInteger(default = 0)


class Rc:
	def __init__(self):
		self["rc"] = MultiPixmap()
		self["arrowdown"] = MovingPixmap()
		self["arrowdown2"] = MovingPixmap()
		self["arrowup"] = MovingPixmap()
		self["arrowup2"] = MovingPixmap()
		
		if QBOXHD:
			rcname = eActionMap.getInstance().getLastRCId()
			rcid = self.getIdfromRCName( str(rcname) )
			print "[RC] %s remotecontrol. Using id=%d " % (str(rcname), int(rcid) )
			
			if rcid>=0:
				config.misc.rcused = ConfigInteger(default = 0)
				config.misc.rcused.value = rcid
			else:
				if QBOXHD_MINI:
					config.misc.rcused = ConfigInteger(default = 1)
					config.misc.rcused.value = 1
				else:
					config.misc.rcused = ConfigInteger(default = 0)
					config.misc.rcused.value = 0
		
		self.rcheight = 500
		self.rcheighthalf = 250
		
		self.selectpics = []
		self.selectpics.append((self.rcheighthalf, ["arrowdown", "arrowdown2"], (-18,-70)))
		self.selectpics.append((self.rcheight, ["arrowup", "arrowup2"], (-18,0)))
		
		self.readPositions()
		self.clearSelectedKeys()
		self.onShown.append(self.initRc)
		
	def getIdfromRCName(self, name ):
		tree = ElementTree(file = resolveFilename(SCOPE_SKIN, "rcpositions.xml"))
		rcs = tree.getroot()
		self.rcs = {}
		for rc in rcs:
			if rc.attrib["name"] == name:
				id = int(rc.attrib["id"])
				return id
		return -1

	def initRc(self):
		self["rc"].setPixmapNum(config.misc.rcused.value)
				
	def readPositions(self):
		tree = ElementTree(file = resolveFilename(SCOPE_SKIN, "rcpositions.xml"))
		rcs = tree.getroot()
		self.rcs = {}
		for rc in rcs:
			id = int(rc.attrib["id"])
			self.rcs[id] = {}
			for key in rc:
				name = key.attrib["name"]
				pos = key.attrib["pos"].split(",")
				self.rcs[id][name] = (int(pos[0]), int(pos[1]))
		
	def getSelectPic(self, pos):
		for selectPic in self.selectpics:
			if pos[1] <= selectPic[0]:
				return (selectPic[1], selectPic[2])
		return None
	
	def hideRc(self):
		self["rc"].hide()
		self.hideSelectPics()
		
	def showRc(self):
		self["rc"].show()

	def selectKey(self, key):
		rc = self.rcs[config.misc.rcused.value]
		if rc.has_key(key):
			rcpos = self["rc"].getPosition()
			pos = rc[key]
			selectPics = self.getSelectPic(pos)
			selectPic = None
			for x in selectPics[0]:
				if x not in self.selectedKeys:
					selectPic = x
					break
			if selectPic is not None:
				print "selectPic:", selectPic
				self[selectPic].moveTo(rcpos[0] + pos[0] + selectPics[1][0], rcpos[1] + pos[1] + selectPics[1][1], 1)
				self[selectPic].startMoving()
				self[selectPic].show()
				self.selectedKeys.append(selectPic)
	
	def clearSelectedKeys(self):
		self.showRc()
		self.selectedKeys = []
		self.hideSelectPics()
		
	def hideSelectPics(self):
		for selectPic in self.selectpics:
			for pic in selectPic[1]:
				self[pic].hide()
