from qboxhd import QBOXHD
from Screen import Screen
from Components.ServiceScan import ServiceScan as CScan
from Components.ProgressBar import ProgressBar
from Components.Label import Label
from Components.ActionMap import ActionMap
from Components.FIFOList import FIFOList
from Components.Sources.FrontendInfo import FrontendInfo
if QBOXHD:
	from Screens.MessageBox import MessageBox

class ServiceScanSummary(Screen):
	if QBOXHD:
	#	skin = """
	#	<screen position="0,0" size="132,64">
	#		<widget name="Title" position="6,4" size="120,42" font="Regular;16" transparent="1" />
	#		<widget name="scan_progress" position="6,50" zPosition="1" borderWidth="1" size="56,12" backgroundColor="dark" />
	#		<widget name="Service" position="6,22" size="120,26" font="Regular;12" transparent="1" />
	#	</screen>"""


	# 320/132=2.42 240/64=3.75 ->  6,4 -> (6*2.42),(4*3.75) -> 14.52,15 -> 14,15
	# 3 lines for the service name
	#	skin = """
	#	<screen position="0,0" size="320,240">
	#		<widget name="Title" position="20,15" size="290,157" font="Regular;48" transparent="1" />
	#		<widget name="scan_progress" position="16,200" zPosition="1" borderWidth="1" size="283,25" backgroundColor="dark" />
	#		<widget name="Service" position="25,70" size="200,110" font="Regular;36" transparent="1" />
	#	</screen>"""


	# 2 lines for the service name
		skin = """
		<screen position="0,0" size="320,240">
			<widget name="Title" position="20,15" size="290,157" font="Regular;48" transparent="1" />
			<widget name="scan_progress" position="16,192" zPosition="1" borderWidth="1" size="283,25" backgroundColor="dark" />
			<widget name="Service" position="23,85" size="275,73" font="Regular;36" transparent="1" />
		</screen>"""
	else:
		skin = """
		<screen position="0,0" size="132,64">
			<widget name="Title" position="6,4" size="120,42" font="Regular;16" transparent="1" />
			<widget name="scan_progress" position="6,50" zPosition="1" borderWidth="1" size="56,12" backgroundColor="dark" />
			<widget name="Service" position="6,22" size="120,26" font="Regular;12" transparent="1" />
		</screen>"""

	def __init__(self, session, parent, showStepSlider = True):
		Screen.__init__(self, session, parent)

		self["Title"] = Label(parent.title or "ServiceScan")
		self["Service"] = Label("No Service")
		self["scan_progress"] = ProgressBar()

	def updateProgress(self, value):
		self["scan_progress"].setValue(value)

	def updateService(self, name):
		self["Service"].setText(name)

class ServiceScan(Screen):

	def __init__(self, session, scanList):
		Screen.__init__(self, session)

		self.scanList = scanList

		self.session.nav.stopService()

		self["scan_progress"] = ProgressBar()
		self["scan_state"] = Label(_("scan state"))
		self["network"] = Label()
		self["transponder"] = Label()

		self["pass"] = Label("")
		self["servicelist"] = FIFOList(len=10)
		self["FrontendInfo"] = FrontendInfo()

		self["actions"] = ActionMap(["OkCancelActions"],
			{
				"ok": self.ok,
				"cancel": self.cancel
			})

		self.onFirstExecBegin.append(self.doServiceScan)
		
	def okCB(self, result):
		self.close()

	def ok(self):
		print "ok"
		if self["scan"].isDone():
			if QBOXHD:
				self.session.openWithCallback(self.okCB, MessageBox, _("The new Channel List has been saved."), type = MessageBox.TYPE_INFO)
			else:
				self.close()

	def cancel(self):
		self.close()

	def doServiceScan(self):
		self["scan"] = CScan(self["scan_progress"], self["scan_state"], self["servicelist"], self["pass"], self.scanList, self["network"], self["transponder"], self["FrontendInfo"], self.session.summary)

	def createSummary(self):
		print "ServiceScanCreateSummary"
		return ServiceScanSummary
