from qboxhd import QBOXHD
from Screen import Screen

from Components.ActionMap import ActionMap
from Components.Language import language
from Components.config import config
from Components.Sources.List import List
from Components.Label import Label
from Components.Pixmap import Pixmap
from Components.language_cache import LANG_TEXT
if QBOXHD:
	from Screens.MessageBox import MessageBox

def _cached(x):
	return LANG_TEXT.get(config.osd.language.value, {}).get(x, "")

from Screens.Rc import Rc

from Tools.Directories import resolveFilename, SCOPE_SKIN_IMAGE

from Tools.LoadPixmap import LoadPixmap

def LanguageEntryComponent(file, name, index):
	png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "countries/" + file + ".png"))
	if png == None:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "countries/missing.png"))
	res = (index, name, png)
	return res

class LanguageSelection(Screen):
	def __init__(self, session):
		Screen.__init__(self, session)

		self.oldActiveLanguage = language.getActiveLanguage()

		self.list = []
		self["languages"] = List(self.list)
		self["languages"].onSelectionChanged.append(self.changed)
		
		if QBOXHD:
			self.wizard_active = False

		self.updateList()
		self.onLayoutFinish.append(self.selectActiveLanguage)

		self["actions"] = ActionMap(["OkCancelActions"], 
		{
			"ok": self.save,
			"cancel": self.cancel,
		}, -1)

	def selectActiveLanguage(self):
		activeLanguage = language.getActiveLanguage()
		pos = 0
		for x in self.list:
			if x[0] == activeLanguage:
				self["languages"].index = pos
				break
			pos += 1
			
	def saveCB(self, result):
		self.close()
		
	def save(self):
		self.run()
		if QBOXHD:
			if not self.wizard_active:
				self.session.openWithCallback(self.saveCB,MessageBox,_("Your new selected language is %s.\nPlease restart GUI to make it effective.") % str(_cached(config.osd.language.value)), type = MessageBox.TYPE_INFO)
			else:
				self.close()
		else:
			self.close()

	def cancel(self):
		language.activateLanguage(self.oldActiveLanguage)
		self.close()

	def run(self, justlocal = False):
		print "updating language..."
		lang = self["languages"].getCurrent()[0]
		config.osd.language.value = lang
		config.osd.language.save()
		self.setTitle(_cached("T2"))
		
		if justlocal:
			return

		language.activateLanguage(lang)
		config.misc.languageselected.value = 0
		config.misc.languageselected.save()
		print "ok"

	def updateList(self):
		first_time = not self.list

		languageList = language.getLanguageList()
		if not languageList: # no language available => display only english
			list = [ LanguageEntryComponent("en", _cached("en_EN"), "en_EN") ]
		else:
			list = [ LanguageEntryComponent(file = x[1][2].lower(), name = _cached("%s_%s" % x[1][1:3]), index = x[0]) for x in languageList]
		self.list = list

		#list.sort(key=lambda x: x[1][7])

		print "updateList"
		if first_time:
			self["languages"].list = list
		else:
			self["languages"].updateList(list)
		print "done"

	def changed(self):
		self.run(justlocal = True)
		self.updateList()

class LanguageWizard(LanguageSelection, Rc):
	def __init__(self, session):
		LanguageSelection.__init__(self, session)
		Rc.__init__(self)
		
		if QBOXHD:
			self.wizard_active = True
		
		self.onLayoutFinish.append(self.selectKeys)
				
		self["wizard"] = Pixmap()
		self["text"] = Label()
		self.setText()
		
	def selectKeys(self):
		self.clearSelectedKeys()
		self.selectKey("UP")
		self.selectKey("DOWN")
		
	def changed(self):
		self.run(justlocal = True)
		self.updateList()
		self.setText()
		
	def setText(self):
		self["text"].setText(_cached("T1"))
