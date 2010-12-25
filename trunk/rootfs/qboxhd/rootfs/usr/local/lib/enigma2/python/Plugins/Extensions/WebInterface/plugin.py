Version = '$Header: /var/lib/gforge/chroot/cvsroot/enigma2-plugins/enigma2-plugins/webinterface/src/plugin.py,v 1.103 2009-11-07 12:14:26 sreichholf Exp $';

from enigma import eConsoleAppContainer
from Plugins.Plugin import PluginDescriptor
from Components.config import config, ConfigBoolean, ConfigSubsection, ConfigInteger, ConfigYesNo, ConfigText
from Components.Network import iNetwork
from Screens.MessageBox import MessageBox
from WebIfConfig import WebIfConfigScreen
from WebChilds.Toplevel import getToplevel

from Tools.Directories import copyfile, resolveFilename, SCOPE_PLUGINS, SCOPE_CONFIG

from twisted.internet import reactor, ssl
from twisted.web import server, http, util, static, resource

from zope.interface import Interface, implements
from socket import gethostname as socket_gethostname
from OpenSSL import SSL

from os.path import isfile as os_isfile



from __init__ import _, __version__

#CONFIG INIT

#init the config
config.plugins.Webinterface = ConfigSubsection()
config.plugins.Webinterface.enabled = ConfigYesNo(default=False)
config.plugins.Webinterface.allowzapping = ConfigYesNo(default=True)
config.plugins.Webinterface.includemedia = ConfigYesNo(default=False)
config.plugins.Webinterface.autowritetimer = ConfigYesNo(default=False)
config.plugins.Webinterface.loadmovielength = ConfigYesNo(default=True)
config.plugins.Webinterface.version = ConfigText(__version__) # used to make the versioninfo accessible enigma2-wide, not confgurable in GUI.

config.plugins.Webinterface.http = ConfigSubsection()
config.plugins.Webinterface.http.enabled = ConfigYesNo(default=True)
config.plugins.Webinterface.http.port = ConfigInteger(default = 80, limits=(1, 65535) )
config.plugins.Webinterface.http.auth = ConfigYesNo(default=False)

config.plugins.Webinterface.https = ConfigSubsection()
config.plugins.Webinterface.https.enabled = ConfigYesNo(default=True)
config.plugins.Webinterface.https.port = ConfigInteger(default = 443, limits=(1, 65535) )
config.plugins.Webinterface.https.auth = ConfigYesNo(default=True)

config.plugins.Webinterface.streamauth = ConfigYesNo(default=False)

global running_defered, waiting_shutdown, toplevel

running_defered = []
waiting_shutdown = 0
toplevel = None
server.VERSION = "Enigma2 WebInterface Server $Revision: 1.103 $".replace("$Revi", "").replace("sion: ", "").replace("$", "")

#===============================================================================
# Helperclass to close running Instances of the Webinterface
#===============================================================================
class Closer:
	counter = 0
	def __init__(self, session, callback=None):
		self.callback = callback
		self.session = session

#===============================================================================
# Closes all running Instances of the Webinterface
#===============================================================================
	def stop(self):
		global running_defered
		for d in running_defered:
			print "[Webinterface] stopping interface on ", d.interface, " with port", d.port
			x = d.stopListening()
			try:
				x.addCallback(self.isDown)
				self.counter += 1
			except AttributeError:
				pass
		running_defered = []
		if self.counter < 1:
			if self.callback is not None:
				self.callback(self.session)

#===============================================================================
# #Is it already down?
#===============================================================================
	def isDown(self, s):
		self.counter -= 1
		if self.counter < 1:
			if self.callback is not None:
				self.callback(self.session)

def checkCertificates():
	print "[WebInterface] checking for SSL Certificates"
	srvcert = '%sserver.pem' %resolveFilename(SCOPE_CONFIG) 
	cacert = '%scacert.pem' %resolveFilename(SCOPE_CONFIG)

	# Check whether there are regular certificates, if not copy the default ones over
	if not os_isfile(srvcert) or not os_isfile(cacert):
		return False
	
	else:
		return True
		
def installCertificates(session, callback = None):
	print "[WebInterface] Installing SSL Certificates to %s" %resolveFilename(SCOPE_CONFIG)
	
	srvcert = '%sserver.pem' %resolveFilename(SCOPE_CONFIG) 
	cacert = '%scacert.pem' %resolveFilename(SCOPE_CONFIG)	
	scope_webif = '%sExtensions/WebInterface/' %resolveFilename(SCOPE_PLUGINS)
	
	source = '%setc/server.pem' %scope_webif
	target = srvcert
	ret = copyfile(source, target)
	
	if ret == 0:
		source = '%setc/cacert.pem' %scope_webif
		target = cacert
		ret = copyfile(source, target)
		
		if ret == 0 and callback != None:
			callback(session)
	
	if ret < 0:
		config.plugins.Webinterface.https.enabled.value = False
		config.plugins.Webinterface.https.enabled.save()
		
		# Start without https
		callback(session)
		
		#Inform the user
		session.open(MessageBox, "Couldn't install SSL-Certifactes for https access\nHttps access is now disabled!", MessageBox.TYPE_ERROR)
	
#===============================================================================
# restart the Webinterface for all configured Interfaces
#===============================================================================
def restartWebserver(session):
	try:
		del session.mediaplayer
		del session.messageboxanswer
	except NameError:
		pass
	except AttributeError:
		pass

	global running_defered
	if len(running_defered) > 0:
		Closer(session, startWebserver).stop()
	else:
		startWebserver(session)
	
#===============================================================================
# start the Webinterface for all configured Interfaces
#===============================================================================
def startWebserver(session):
	global running_defered
	global toplevel
	
	session.mediaplayer = None
	session.messageboxanswer = None
	if toplevel is None:
		toplevel = getToplevel(session)
	
	errors = ""
	
	if config.plugins.Webinterface.enabled.value is not True:
		print "[Webinterface] is disabled!"
	
	else:
		# IF SSL is enabled we need to check for the certs first
		# If they're not there we'll exit via return here 
		# and get called after Certificates are installed properly
		if config.plugins.Webinterface.https.enabled.value:
			if not checkCertificates():
				print "[Webinterface] Installing Webserver Certificates for SSL encryption"
				installCertificates(session, startWebserver)
				return
				
		for adaptername in iNetwork.ifaces:				
			ip = '.'.join("%d" % d for d in iNetwork.ifaces[adaptername]['ip'])
						
			#Only if it's up and has a "good" IP
			if ip != '0.0.0.0' and iNetwork.ifaces[adaptername]['up'] == True:
			#HTTP
				if config.plugins.Webinterface.http.enabled.value is True:
					ret = startServerInstance(session, ip, config.plugins.Webinterface.http.port.value, config.plugins.Webinterface.http.auth.value)
					if ret == False:
						errors = "%s%s:%i\n" %(errors, ip, config.plugins.Webinterface.http.port.value)
			#HTTPS		
				if config.plugins.Webinterface.https.enabled.value is True:					
					ret = startServerInstance(session, ip, config.plugins.Webinterface.https.port.value, config.plugins.Webinterface.https.auth.value, True)
					if ret == False:
						errors = "%s%s:%i\n" %(errors, ip, config.plugins.Webinterface.https.port.value)
	
	#LOCAL HTTP Connections (Streamproxy)
		ret = startServerInstance(session, '127.0.0.1', 80, config.plugins.Webinterface.streamauth.value)			
		if ret == False:
			errors = "%s%s:%i\n" %(errors, '127.0.0.1', 80)
		
		if errors != "":
			session.open(MessageBox, "Webinterface - Couldn't listen on:\n %s" % (errors), type=MessageBox.TYPE_ERROR, timeout=30)
		
#===============================================================================
# stop the Webinterface for all configured Interfaces
#===============================================================================
def stopWebserver(session):
	try:
		del session.mediaplayer
		del session.messageboxanswer
	except NameError:
		pass
	except AttributeError:
		pass

	global running_defered
	if len(running_defered) > 0:
		Closer(session).stop()

#===============================================================================
# startServerInstance
# Starts an Instance of the Webinterface
# on given ipaddress, port, w/o auth, w/o ssl
#===============================================================================
def startServerInstance(session, ipaddress, port, useauth=False, usessl=False):
	try:
		if useauth:
# HTTPAuthResource handles the authentication for every Resource you want it to			
			root = HTTPAuthResource(toplevel, "Enigma2 WebInterface")
			site = server.Site(root)			
		else:
			site = server.Site(toplevel)
	
		if usessl:
			
			ctx = ssl.DefaultOpenSSLContextFactory('/etc/enigma2/server.pem', '/etc/enigma2/cacert.pem', sslmethod=SSL.SSLv23_METHOD)
			d = reactor.listenSSL(port, site, ctx, interface=ipaddress)
		else:
			d = reactor.listenTCP(port, site, interface=ipaddress)
		running_defered.append(d)		
		print "[Webinterface] started on %s:%i" % (ipaddress, port), "auth=", useauth, "ssl=", usessl
		return True
	
	except Exception, e:
		print "[Webinterface] starting FAILED on %s:%i!" % (ipaddress, port), e		
		return False
#===============================================================================
# HTTPAuthResource
# Handles HTTP Authorization for a given Resource
#===============================================================================
class HTTPAuthResource(resource.Resource):
	def __init__(self, res, realm):
		resource.Resource.__init__(self)
		self.resource = res
		self.realm = realm
		self.authorized = False
		self.tries = 0
		self.unauthorizedResource = UnauthorizedResource(self.realm)		
	
	def unautorized(self, request):
		request.setResponseCode(http.UNAUTHORIZED)
		request.setHeader('WWW-authenticate', 'basic realm="%s"' % self.realm)

		return self.unauthorizedResource
	
	def isAuthenticated(self, request):
		# get the Session from the Request
		sessionNs = request.getSession().sessionNamespaces
		
		# if the auth-information has not yet been stored to the session
		if not sessionNs.has_key('authenticated'):
			sessionNs['authenticated'] = check_passwd(request.getUser(), request.getPassword())
		
		#if the auth-information already is in the session				
		else:
			if sessionNs['authenticated'] is False:
				sessionNs['authenticated'] = check_passwd(request.getUser(), request.getPassword() )
		
		#return the current authentication status						
		return sessionNs['authenticated']
													
#===============================================================================
# Call render of self.resource (if authenticated)													
#===============================================================================
	def render(self, request):			
		if self.isAuthenticated(request) is True:	
			return self.resource.render(request)
		
		else:
			return self.unautorized(request).render(request)

#===============================================================================
# Override to call getChildWithDefault of self.resource (if authenticated)	
#===============================================================================
	def getChildWithDefault(self, path, request):
		if self.isAuthenticated(request) is True:
			return self.resource.getChildWithDefault(path, request)
		
		else:
			return self.unautorized(request)

#===============================================================================
# UnauthorizedResource
# Returns a simple html-ified "Access Denied"
#===============================================================================
class UnauthorizedResource(resource.Resource):
	def __init__(self, realm):
		resource.Resource.__init__(self)
		self.realm = realm
		self.errorpage = static.Data('<html><body>Access Denied.</body></html>', 'text/html')
	
	def getChild(self, path, request):
		return self.errorpage
		
	def render(self, request):	
		return self.errorpage.render(request)

# Password verfication stuff

from hashlib import md5 as md5_new
from crypt import crypt

#===============================================================================
# getpwnam
# 
# Get a password database entry for the given user name
# Example from the Python Library Reference.
#===============================================================================
def getpwnam(name, pwfile=None):
	if not pwfile:
		pwfile = '/etc/passwd'

	f = open(pwfile)
	while 1:
		line = f.readline()
		if not line:
			f.close()
			raise KeyError, name
		entry = tuple(line.strip().split(':', 6))
		if entry[0] == name:
			f.close()
			return entry

#===============================================================================
# passcrypt
#
# Encrypt a password
#===============================================================================
def passcrypt(passwd, salt=None, method='des', magic='$1$'):
	"""Encrypt a string according to rules in crypt(3)."""
	if method.lower() == 'des':
		return crypt(passwd, salt)
	elif method.lower() == 'md5':
		return passcrypt_md5(passwd, salt, magic)
	elif method.lower() == 'clear':
		return passwd

#===============================================================================
# check_passwd
#
# Checks username and Password against a given Unix Password file 
# The default path is '/etc/passwd'
#===============================================================================
def check_passwd(name, passwd, pwfile='/etc/passwd'):
	"""Validate given user, passwd pair against password database."""

	if not pwfile or type(pwfile) == type(''):
		getuser = lambda x, pwfile = pwfile: getpwnam(x, pwfile)[1]
	else:
		getuser = pwfile.get_passwd

	try:
		enc_passwd = getuser(name)
	except (KeyError, IOError):
		print "!!! EXCEPT"
		return False
	if not enc_passwd:
		"!!! NOT ENC_PASSWD"
		return False
	elif len(enc_passwd) >= 3 and enc_passwd[:3] == '$1$':
		salt = enc_passwd[3:enc_passwd.find('$', 3)]
		return enc_passwd == passcrypt(passwd, salt, 'md5')
	else:
		return enc_passwd == passcrypt(passwd, enc_passwd[:2])

def _to64(v, n):
	DES_SALT = list('./0123456789' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' 'abcdefghijklmnopqrstuvwxyz')
	r = ''
	while (n - 1 >= 0):
		r = r + DES_SALT[v & 0x3F]
		v = v >> 6
		n = n - 1
	return r

#===============================================================================
# passcrypt_md5
# Encrypt a password via md5
#===============================================================================
def passcrypt_md5(passwd, salt=None, magic='$1$'):
	if not salt:
		pass
	elif salt[:len(magic)] == magic:
		# remove magic from salt if present
		salt = salt[len(magic):]

	# salt only goes up to first '$'
	salt = salt.split('$')[0]
	# limit length of salt to 8
	salt = salt[:8]

	ctx = md5_new(passwd)
	ctx.update(magic)
	ctx.update(salt)

	ctx1 = md5_new(passwd)
	ctx1.update(salt)
	ctx1.update(passwd)

	final = ctx1.digest()

	for i in range(len(passwd), 0 , -16):
		if i > 16:
			ctx.update(final)
		else:
			ctx.update(final[:i])

	i = len(passwd)
	while i:
		if i & 1:
			ctx.update('\0')
		else:
			ctx.update(passwd[:1])
		i = i >> 1
	final = ctx.digest()

	for i in range(1000):
		ctx1 = md5_new()
		if i & 1:
			ctx1.update(passwd)
		else:
			ctx1.update(final)
		if i % 3: ctx1.update(salt)
		if i % 7: ctx1.update(passwd)
		if i & 1:
			ctx1.update(final)
		else:
			ctx1.update(passwd)
		final = ctx1.digest()

	rv = magic + salt + '$'
	final = map(ord, final)
	l = (final[0] << 16) + (final[6] << 8) + final[12]
	rv = rv + _to64(l, 4)
	l = (final[1] << 16) + (final[7] << 8) + final[13]
	rv = rv + _to64(l, 4)
	l = (final[2] << 16) + (final[8] << 8) + final[14]
	rv = rv + _to64(l, 4)
	l = (final[3] << 16) + (final[9] << 8) + final[15]
	rv = rv + _to64(l, 4)
	l = (final[4] << 16) + (final[10] << 8) + final[5]
	rv = rv + _to64(l, 4)
	l = final[11]
	rv = rv + _to64(l, 2)

	return rv

global_session = None

#===============================================================================
# sessionstart
# Actions to take place on Session start 
#===============================================================================
def sessionstart(reason, session):
	global global_session
	global_session = session

#===============================================================================
# networkstart
# Actions to take place after Network is up (startup the Webserver)
#===============================================================================
def networkstart(reason, **kwargs):
	if reason is True:
		startWebserver(global_session)

	elif reason is False:
		stopWebserver(global_session)

#def openconfig(session, **kwargs):
#	session.openWithCallback(configCB, WebIfConfigScreen)
	
def main(session, **kwargs):
	session.openWithCallback(configCB, WebIfConfigScreen)
	
def menu(menuid, **kwargs):
	if menuid == "miscellaneous":
		return [(_("WebInterface"), main, "webinterfacedefine", None)]
	return []	

def configCB(result, session):
	if result is True:
		print "[WebIf] config changed"
		restartWebserver(session)
	else:
		print "[WebIf] config not changed"

def Plugins(**kwargs):
	return [PluginDescriptor(where=[PluginDescriptor.WHERE_SESSIONSTART], fnc=sessionstart),
			PluginDescriptor(where=[PluginDescriptor.WHERE_NETWORKCONFIG_READ], fnc=networkstart),
			PluginDescriptor(where=[PluginDescriptor.WHERE_MENU], name="WebInterface", description="Configuration for the Webinterface", fnc=menu)]
