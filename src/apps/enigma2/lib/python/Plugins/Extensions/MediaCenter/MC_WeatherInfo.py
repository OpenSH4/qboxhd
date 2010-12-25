from enigma import eTimer, eWidget, eRect, eServiceReference, iServiceInformation
from Screens.Screen import Screen
from Screens.ServiceInfo import ServiceInfoList, ServiceInfoListEntry
from Components.ActionMap import ActionMap, NumberActionMap
from Components.Pixmap import Pixmap, MovingPixmap
from Components.Label import Label
from Components.Button import Button

from Screens.MessageBox import MessageBox

from Components.ConfigList import ConfigList, ConfigListScreen
from Components.config import *

from Tools.Directories import resolveFilename, fileExists, pathExists, createDir, SCOPE_MEDIA, SCOPE_SKIN_IMAGE
from Components.FileList import FileList
from Components.AVSwitch import AVSwitch
from Screens.InfoBar import MoviePlayer
from Plugins.Plugin import PluginDescriptor

try:
	from twisted.web.client import getPage
except Exception, e:
	print "Media Center Weather Info: Import twisted.web.client failed"
	
import os

config.plugins.mc_wi = ConfigSubsection()
config.plugins.mc_wi.city = ConfigSelection(default="GMXX0012", choices = [("GMXX0012", _("DE: Berlin")),("GMXX0049", _("DE: Hamburg")),("GMXX0072", _("DE: Leipzig")),("GMXX0051", _("DE: Hannover")),("GMXX0018", _("DE: Koeln")),("GMXX0040", _("DE: Frankfurt/Main")),("GMXX0128", _("DE: Stuttgart")),("GMXX0087", _("DE: Muenchen")),("AUXX0025", _("AT: Wien")),("AUXX0008", _("AT: Graz")),("AUXX0016", _("AT: Linz")),("AUXX0018", _("AT: Salzburg")),("AUXX0037", _("AT: Klagenfurt")),("AUXX0010", _("AT: Innsbruck")),("AUXX0032", _("AT: Bregenz"))])
config.plugins.mc_wi.metric = ConfigSelection(default="C", choices = [("C", _("Metric")),("F", _("English"))])

#------------------------------------------------------------------------------------------

class MC_WeatherInfo(Screen):
	def __init__(self, session):
		Screen.__init__(self, session)

		# Show Background MVI
		if not QBOXHD:
			os.system("/usr/bin/showiframe /usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/background.mvi &")

		self["key_blue"] = Button("Settings")
		
		self["actions"] = ActionMap(["OkCancelActions", "DirectionActions", "ColorActions", "MovieSelectionActions", "MenuActions", "MoviePlayerActions", "ChannelSelectBaseActions"],
		{
			"cancel": self.close,
			"blue": self.WeatherSetup
		}, -1)
		
		
		self["CatTemp"] = Label("Temp.:")
		self["CatPrecip"] = Label("Precip.:")
		self["CatUVIndex"] = Label("UV-Index:")
		self["CatWind"] = Label("Wind:")
		self["CatHumidity"] = Label("Humidity:")
		
		self["TodayPicture"] = Pixmap()
		self["TmrwPicture"] = Pixmap()
		self["TdatPicture"] = Pixmap()

		self["CurrentCity"] = Label("Updating Weather Info ...")

		self["TodayTemp"] = Label("")
		self["TodayPrecip"] = Label("")
		self["TodayWind"] = Label("")
		self["TodayUVIndex"] = Label("")
		self["TodayHumidity"] = Label("")
		self["TmrwTemp"] = Label("")
		self["TmrwPrecip"] = Label("")
		self["TmrwWind"] = Label("")
		self["TmrwUVIndex"] = Label("")
		self["TmrwHumidity"] = Label("")
		self["TdatTemp"] = Label("")
		self["TdatPrecip"] = Label("")
		self["TdatWind"] = Label("")
		self["TdatUVIndex"] = Label("")
		self["TdatHumidity"] = Label("")
		
		self.onFirstExecBegin.append(self.GetWeatherInfo)
		
	def error(self, error):
		self.session.open(MessageBox,("UNEXPECTED ERROR:\n%s") % (error),  MessageBox.TYPE_INFO)

	def WeatherSetup(self):
		self.session.openWithCallback(self.GetWeatherInfo, WeatherSetup)
		
	def GetWeatherInfo(self):
		#Get Weather Info from my webserver
		self.url = "http://www.homeys-bunker.de/dm800/feeds/weatherinfo2.php?citycode=%s&metric=%s" % (config.plugins.mc_wi.city.value, config.plugins.mc_wi.metric.value)
		try:
			getPage(self.url).addCallback(self.UpdateWeatherInfo).addErrback(self.error)
			self["CurrentCity"].setText(_("Updating Weather Info ..."))
		except Exception, e:
			self["CurrentCity"].setText(_("Updating Weather Info failed (Twisted Web not installed)"))
	
	def UpdateWeatherInfo(self, html):
		wi_infolines = html.splitlines()
		
		#self.session.open(MessageBox,("RECEIVED FEED:\n%s") % (wi_infolines[1]),  MessageBox.TYPE_INFO)
		
		# Weather Info Today
		self["TodayTemp"].setText(wi_infolines[1])
		self["TodayPrecip"].setText(wi_infolines[2])
		self["TodayWind"].setText(wi_infolines[3])
		self["TodayUVIndex"].setText(wi_infolines[4])
		self["TodayHumidity"].setText(wi_infolines[5])
		self["TodayPicture"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/" + wi_infolines[6])

		# Weather Info Tomorrow
		self["TmrwTemp"].setText(wi_infolines[8])
		self["TmrwPrecip"].setText(wi_infolines[9])
		self["TmrwWind"].setText(wi_infolines[10])
		self["TmrwUVIndex"].setText(wi_infolines[11])
		self["TmrwHumidity"].setText(wi_infolines[12])
		self["TmrwPicture"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/" + wi_infolines[13])

		# Weather Info The day after tomorrow
		self["TdatTemp"].setText(wi_infolines[15])
		self["TdatPrecip"].setText(wi_infolines[16])
		self["TdatWind"].setText(wi_infolines[17])
		self["TdatUVIndex"].setText(wi_infolines[18])
		self["TdatHumidity"].setText(wi_infolines[19])
		self["TdatPicture"].instance.setPixmapFromFile("/usr/lib/enigma2/python/Plugins/Extensions/MediaCenter/icons/" + wi_infolines[20])

		# Get Current City Name
		self["CurrentCity"].setText(config.plugins.mc_wi.city.getText())
		
#------------------------------------------------------------------------------------------

class WeatherSetup(Screen, ConfigListScreen):
	skin = """
		<screen name="FavEdit" position="80,140" size="560,330" title="Edit Weather Info Settings">
			<widget name="config" position="10,10" size="540,250" scrollbarMode="showOnDemand" />
		</screen>"""
		
	def __init__(self, session):
		Screen.__init__(self, session)

		self["actions"] = NumberActionMap(["SetupActions","OkCancelActions"],
		{
			"ok": self.keyOK,
			"cancel": self.close
		}, -1)
		
		self.list = []
		self.list.append(getConfigListEntry(_("City"), config.plugins.mc_wi.city))
		self.list.append(getConfigListEntry(_("Metric"), config.plugins.mc_wi.metric))

		ConfigListScreen.__init__(self, self.list, session)
		
	def keyOK(self):
		config.plugins.mc_wi.save()
		self.close()		
