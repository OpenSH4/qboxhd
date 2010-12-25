from qboxhd import QBOXHD
import sys
from socket import *
from re import compile as re_compile, search as re_search

import xml.dom.minidom
from xml.dom.minidom import Node

CARD_UNKNOWN = 0
CARD_REMOVED = 1
CARD_INSERTED = 2
CARD_INITIALIZING = 3
CARD_INITIALIZED = 4



class SmartCard:
	""" 
	SmartCard Information"""

	def __init__(self, iface, inserted=False, state=CARD_REMOVED, sn="", codingsystem="UNKNOWN", atr=""):
		self.iface = iface
		self.inserted = inserted
		self.state = state
		self.codingsystem = codingsystem
		self.atr = atr
		self.sn = sn


class SubscriptionStatus:
	""" 
	Subscription status for Conax Card"""

	def __init__(self, label="", start_date1="", end_date1="", entitlement1="", start_date2="", end_date2="", entitlement2="" ):
		self.label = label
		self.start_date1 = start_date1
		self.end_date1 = end_date1
		self.entitlement1 = entitlement1
		self.start_date2 = start_date2
		self.end_date2 = end_date2
		self.entitlement2 = entitlement2



class PurseStatus:
	""" 	
	Purse status for Conax Card"""

	def __init__(self, label="", balance="" ):
		self.label = label
		self.balance = balance




class SmartCardConax(SmartCard):
	""" 
	SmartCard Conax Information"""

	CODINGSYSTEM_CONAX_IDENTIFIER = _("CONAX")

	def __init__(self, iface, inserted=False, state=CARD_REMOVED, sn="", atr="", subscriptions=[], purses=[], pin="" ):
		SmartCard.__init__( self, iface, inserted, state, sn, _("CONAX"), atr )

		self.subscriptions = subscriptions
		self.purses = purses
		self.pin = pin



class SmartCardConfig:
	"""
	Manage SmartCard Information from Socket"""

	__SmartCardxmlfile = "/var/etc/smartcardinfo.xml"
	__scinfo_request = 'scinfo_request'
	
	serverHost = 'localhost'
	serverPort = 50007

	def __init__( self ):
		pass

	def findChildrenByTagName(self, parent, tagname):
		"""Return a list of 'tagname' children of 'parent'."""
		L = []
		for child in parent.childNodes:
			if (child.nodeType == Node.ELEMENT_NODE and child.tagName == tagname):
				L.append(child)
		return L
	
	
	
	def loadConaxpursesConfig(self, scnode, smartcard):
		"""
		Load SmartCard Conax purses from File"""

		smartcard.purses = []

		# Parse all purses
		pursesnodes = self.findChildrenByTagName( scnode, "purse" )

		for pursenode in pursesnodes:
			ps_label = pursenode.getAttribute("label")
			ps_balance = pursenode.getAttribute("balance")

			smartcard.purses.append( PurseStatus( ps_label, ps_balance ) )


	
	def loadConaxsubscriptionsConfig(self, scnode, smartcard):
		"""
		Load SmartCard Conax subscriptions from File"""
				
		smartcard.subscriptions = []
		
		# Parse all subscriptions
		subscriptionnodes = self.findChildrenByTagName( scnode, "subscription" );

		for subscriptionnode in subscriptionnodes:
			ss_label = subscriptionnode.getAttribute("label")
			ss_start_date1 = subscriptionnode.getAttribute("start_date1")
			ss_end_date1 = subscriptionnode.getAttribute("end_date1")
			ss_entitlement1 = subscriptionnode.getAttribute("entitlement1")
			ss_start_date2 = subscriptionnode.getAttribute("start_date2")
			ss_end_date2 = subscriptionnode.getAttribute("end_date2")
			ss_entitlement2 = subscriptionnode.getAttribute("entitlement2")

			smartcard.subscriptions.append( SubscriptionStatus( ss_label, ss_start_date1, ss_end_date1, ss_entitlement1, ss_start_date2, ss_end_date2, ss_entitlement2))


	def parse_data_from_xml( self, idx, smartcard ):
		scinfonode = self.findChildrenByTagName( self.xmldoc, "scinfo")

		if ( len(scinfonode) != 1 ):
			return False;

		scxnodes = self.findChildrenByTagName( scinfonode[0], "sci%s" % str(idx))

		if ( len(scxnodes) != 1 ):
			return False;

		smartcard.subscription = []
		smartcard.purses = []
		smartcard.atr = ""
		smartcard.codingsystem = _("UNKNOWN")
		
		if (len(scxnodes[0].getAttribute("state") ) == 0):
			return False
			
		if (scxnodes[0].getAttribute("state") == "CARD_REMOVED"):
			smartcard.state = CARD_REMOVED
			return True
		
		elif (scxnodes[0].getAttribute("state") == "CARD_UNKNOWN"):
			smartcard.state = CARD_UNKNOWN
			return True
		
		elif (scxnodes[0].getAttribute("state") == "CARD_INITIALIZING"):
			smartcard.state = CARD_INITIALIZING
			return True
		
		elif (scxnodes[0].getAttribute("state") == "CARD_INITIALIZED"):
			smartcard.state = CARD_INITIALIZED
		else:
			return False
		
		smartcard.codingsystem = scxnodes[0].getAttribute("codingsystem")
		smartcard.atr = scxnodes[0].getAttribute("atr")
		smartcard.sn = scxnodes[0].getAttribute("sn")
			
		# Parse SC Conax Info
		if ( smartcard.codingsystem == SmartCardConax.CODINGSYSTEM_CONAX_IDENTIFIER ):
			self.loadConaxsubscriptionsConfig( scxnodes[0], smartcard )
			self.loadConaxpursesConfig( scxnodes[0], smartcard )

		return True
			
			
	def getIdentifierParameter(self, configStr, identifier):
		for s in configStr:
			split = s.strip().split(': ',1)
			if split[0] == identifier:
				print "[SmartCard.py] Got " + identifier + " :" + split[1]#[0:-1]
				return split[1]#[0:-1]
				
		print "[SmartCard.py] No " + identifier + " present."
		return None
			
			
	def checkSmartCardInserted(self, idx, smartcard):
		"""
		Check if smartcard present"""
		
		print "[SMARTCARDINFO] Get Slot status"
		
		fp = file('/proc/sc', 'r')
		result = fp.readlines()
		fp.close()
		
		status = self.getIdentifierParameter( result, 'sc%d' % idx )
	
		if status is None:
			return False
		
		if (status.find("no card") >= 0) and (smartcard.inserted):
			print "[SMARTCARDINFO] Slot %d status changed prev (inserted) actual (removed)" % idx
			smartcard.inserted = False
			return False
		
		if (status.find("card detected") >= 0) and (not smartcard.inserted):
			print "[SMARTCARDINFO] Slot %d status changed prev (removed) actual (present)" % idx
			smartcard.inserted = True
			return True
			
		return False	
		
	
	def loadSmartCardConfigfromSock(self, idx, smartcard):
		"""
		Load SmartCard Information from File"""

		print "[SMARTCARDINFO] Getting smartcard info ..."
		
		sockobj = socket( AF_INET, SOCK_STREAM )
		sockobj.settimeout(1)
		try:
			sockobj.connect((SmartCardConfig.serverHost, SmartCardConfig.serverPort))
		except:
			# Connection refused
			print "[SMARTCARDINFO] SmartcardServer not active."
			return False
		
		try:
			sockobj.send(SmartCardConfig.__scinfo_request)
		except:
			# Send request error
			return False

		try:
			data=sockobj.recv( 1024 )
		except:
			# Receive error
			return False
		
		if (data is None):
			#No data received
			return False
		
		data=data.strip()
		
		try:
			self.xmldoc = xml.dom.minidom.parseString(data)
		except:
			print '[SMARTCARDINFO] malformed xml. discard it.\n'
			return False				

		if ( self.xmldoc is None ):
			return False		
		
		return self.parse_data_from_xml( idx, smartcard )
