from qboxhd import QBOXHD
from Screens.Screen import Screen
from Screens.ChoiceBox import ChoiceBox
from Screens.InputBox import InputBox
from Screens.MessageBox import MessageBox
from Screens.HelpMenu import HelpableScreen
from Components.ActionMap import HelpableActionMap, ActionMap
from Components.Sources.List import List
from Components.Sources.StaticText import StaticText
from Components.Sources.Progress import Progress
from Components.FileList import FileList
from Components.Pixmap import Pixmap
from enigma import eListboxPythonMultiContent, gFont, RT_HALIGN_LEFT, ePicLoad
from Tools.Directories import fileExists, resolveFilename, SCOPE_PLUGINS, SCOPE_FONTS, SCOPE_HDD
from Components.config import config, getConfigListEntry, ConfigInteger, ConfigSubsection, ConfigSelection
from Components.ConfigList import ConfigListScreen
from Components.AVSwitch import AVSwitch
import DVDTitle

class TitleProperties(Screen,ConfigListScreen):
	skin = """
		<screen position="90,83" size="560,445" title="Properties of current title" >
		    <ePixmap pixmap="skin_default/buttons/red.png" position="0,0" size="140,40" alphatest="on" />
		    <ePixmap pixmap="skin_default/buttons/green.png" position="140,0" size="140,40" alphatest="on" />
		    <ePixmap pixmap="skin_default/buttons/blue.png" position="420,0" size="140,40" alphatest="on" />
		    <widget source="key_red" render="Label" position="0,0" zPosition="1" size="140,40" font="Regular;20" halign="center" valign="center" backgroundColor="#9f1313" transparent="1" />
		    <widget source="key_green" render="Label" position="140,0" zPosition="1" size="140,40" font="Regular;20" halign="center" valign="center" backgroundColor="#1f771f" transparent="1" />
		    <widget source="key_blue" render="Label" position="420,0" zPosition="1" size="140,40" font="Regular;20" halign="center" valign="center" backgroundColor="#18188b" transparent="1" />
		    <widget source="serviceinfo" render="Label" position="10,46" size="350,144" font="Regular;18" />
		    <widget name="thumbnail" position="370,46" size="180,144" alphatest="on" />
		    <widget name="config" position="10,206" size="540,228" scrollbarMode="showOnDemand" />
		</screen>"""

	def __init__(self, session, parent, project, title_idx):
		Screen.__init__(self, session)
		self.parent = parent
		self.project = project
		self.title_idx = title_idx

		self["key_red"] = StaticText(_("Cancel"))
		self["key_green"] = StaticText(_("OK"))
		self["key_blue"] = StaticText(_("Edit Title"))
		self["serviceinfo"] = StaticText()

		self["thumbnail"] = Pixmap()
		self.picload = ePicLoad()
		self.picload.PictureData.get().append(self.paintThumbPixmapCB)

		self.properties = project.titles[title_idx].properties
		ConfigListScreen.__init__(self, [])
		self.properties.crop = DVDTitle.ConfigFixedText("crop")
		self.properties.autochapter.addNotifier(self.initConfigList)
		self.properties.aspect.addNotifier(self.initConfigList)
		for audiotrack in self.properties.audiotracks:
			audiotrack.active.addNotifier(self.initConfigList)

		self["setupActions"] = ActionMap(["SetupActions", "ColorActions"],
		{
		    "green": self.exit,
		    "red": self.cancel,
		    "blue": self.editTitle,
		    "cancel": self.cancel,
		    "ok": self.ok,
		}, -2)
		
		self.onShown.append(self.update)

	def initConfigList(self, element=None):
		try:
			self.properties.position = ConfigInteger(default = self.title_idx+1, limits = (1, len(self.project.titles)))
			title = self.project.titles[self.title_idx]
			self.list = []
			self.list.append(getConfigListEntry("DVD " + _("Track"), self.properties.position))
			self.list.append(getConfigListEntry("DVD " + _("Title"), self.properties.menutitle))
			self.list.append(getConfigListEntry("DVD " + _("Description"), self.properties.menusubtitle))
			if config.usage.setup_level.index >= 2: # expert+
				for audiotrack in self.properties.audiotracks:
					DVB_aud = audiotrack.DVB_lang.getValue() or audiotrack.pid.getValue()
					self.list.append(getConfigListEntry(_("burn audio track (%s)") % DVB_aud, audiotrack.active))
					if audiotrack.active.getValue():
						self.list.append(getConfigListEntry(_("audio track (%s) format") % DVB_aud, audiotrack.format))
						self.list.append(getConfigListEntry(_("audio track (%s) language") % DVB_aud, audiotrack.language))
						
				self.list.append(getConfigListEntry("DVD " + _("Aspect Ratio"), self.properties.aspect))
				if self.properties.aspect.getValue() == "16:9":
					self.list.append(getConfigListEntry("DVD " + "widescreen", self.properties.widescreen))
				else:
					self.list.append(getConfigListEntry("DVD " + "widescreen", self.properties.crop))
			if len(title.chaptermarks) == 0:
				self.list.append(getConfigListEntry(_("Auto chapter split every ? minutes (0=never)"), self.properties.autochapter))
			infotext = "DVB " + _("Title") + ': ' + title.DVBname + "\n" + _("Description") + ': ' + title.DVBdescr + "\n" + _("Channel") + ': ' + title.DVBchannel + '\n' + _("Begin time") + title.formatDVDmenuText(": $D.$M.$Y, $T\n", self.title_idx+1)
			chaptermarks = title.getChapterMarks(template="$h:$m:$s")
			chapters_count = len(chaptermarks)
			if chapters_count >= 1:
				infotext += str(chapters_count+1) + ' ' + _("chapters") + ': '
				infotext += ' / '.join(chaptermarks)
			self["serviceinfo"].setText(infotext)
			self["config"].setList(self.list)
		except AttributeError:
			pass

	def editTitle(self):
		self.parent.editTitle()

	def update(self):
		print "[onShown]"
		self.initConfigList()
		self.loadThumb()

	def loadThumb(self):
		thumbfile = self.project.titles[self.title_idx].inputfile.rsplit('.',1)[0] + ".png"
		sc = AVSwitch().getFramebufferScale()
		self.picload.setPara((self["thumbnail"].instance.size().width(), self["thumbnail"].instance.size().height(), sc[0], sc[1], False, 1, "#00000000"))
		self.picload.startDecode(thumbfile)

	def paintThumbPixmapCB(self, picInfo=None):
		ptr = self.picload.getData()
		if ptr != None:
			self["thumbnail"].instance.setPixmap(ptr.__deref__())

	def changedConfigList(self):
		self.initConfigList()

	def exit(self):
		self.applySettings()
		self.close()

	def applySettings(self):
		for x in self["config"].list:
			x[1].save()
		current_pos = self.title_idx+1
		new_pos = self.properties.position.getValue()
		if new_pos != current_pos:
			print "title got repositioned from ", current_pos, "to", new_pos
			swaptitle = self.project.titles.pop(current_pos-1)
			self.project.titles.insert(new_pos-1, swaptitle)

	def ok(self):
		#key = self.keydict[self["config"].getCurrent()[1]]
		#if key in self.project.filekeys:
			#self.session.openWithCallback(self.FileBrowserClosed, FileBrowser, key, self.settings)
		pass

	def cancel(self):
		self.close()

from Tools.ISO639 import LanguageCodes
class LanguageChoices():
	def __init__(self):
		from Components.Language import language as syslanguage
		syslang = syslanguage.getLanguage()[:2]
		self.langdict = { }
		self.choices = []
		for key, val in LanguageCodes.iteritems():
			if len(key) == 2:
				self.langdict[key] = val[0]
		for key, val in self.langdict.iteritems():
			if key not in (syslang, 'en'):
				self.langdict[key] = val
				self.choices.append((key, val))
		self.choices.sort()
		self.choices.insert(0,("nolang", ("unspecified")))
		self.choices.insert(1,(syslang, self.langdict[syslang]))
		self.choices.insert(2,("en", self.langdict["en"]))

	def getLanguage(self, DVB_lang):
		DVB_lang = DVB_lang.lower()
		for word in ("stereo", "audio", "description", "2ch", "dolby digital"):
			DVB_lang = DVB_lang.replace(word,"").strip()
		for key, val in LanguageCodes.iteritems():
			if DVB_lang.find(key.lower()) == 0:
				if len(key) == 2:
					return key
				else:
					DVB_lang = (LanguageCodes[key])[0]
			elif DVB_lang.find(val[0].lower()) > -1:
				if len(key) == 2:
					return key
				else:
					DVB_lang = (LanguageCodes[key])[0]
		for key, val in self.langdict.iteritems():
			if val == DVB_lang:
				return key
		return "nolang"

languageChoices = LanguageChoices()
