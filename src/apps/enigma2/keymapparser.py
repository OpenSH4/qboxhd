from qboxhd import QBOXHD, QBOXHD_MINI
import enigma
import xml.etree.cElementTree

from keyids import KEYIDS

# these are only informational (for help)...
from Tools.KeyBindings import addKeyBinding

if QBOXHD:
	from Components.SystemInfo import SystemInfo

class KeymapError(Exception):
    def __init__(self, message):
        self.msg = message

    def __str__(self):
        return self.msg

def parseKeys(context, filename, actionmap, device, keys):
	for x in keys.findall("key"):
		get_attr = x.attrib.get
		mapto = get_attr("mapto")
		id = get_attr("id")
		flags = get_attr("flags")
		
		if QBOXHD:
			if (len(str(get_attr("requires")))>0):
				rc_id = str(get_attr("requires"))
			else:
				rc_id = ""
			
			repeat_for_event = get_attr("repeats", "0")
			key_repeat_for_event = int(repeat_for_event)

		flag_ascii_to_id = lambda x: {'m':1,'b':2,'r':4,'l':8}[x]

		flags = sum(map(flag_ascii_to_id, flags))

		assert mapto, "%s: must specify mapto in context %s, id '%s'" % (filename, context, id)
		assert id, "%s: must specify id in context %s, mapto '%s'" % (filename, context, mapto)
		assert flags, "%s: must specify at least one flag in context %s, id '%s'" % (filename, context, id)

		if len(id) == 1:
			keyid = ord(id) | 0x8000
		elif id[0] == '\\':
			if id[1] == 'x':
				keyid = int(id[2:], 0x10) | 0x8000
			elif id[1] == 'd':
				keyid = int(id[2:]) | 0x8000
			else:
				raise KeymapError("key id '" + str(id) + "' is neither hex nor dec")
		else:
			try:
				keyid = KEYIDS[id]
			except:
				raise KeymapError("key id '" + str(id) + "' is illegal")
#				print context + "::" + mapto + " -> " + device + "." + hex(keyid)
		
		if QBOXHD:
			actionmap.bindKey(filename, device, rc_id, keyid, flags, key_repeat_for_event, context, mapto)
		else:
			actionmap.bindKey(filename, device, keyid, flags, context, mapto)
			
		addKeyBinding(filename, keyid, context, mapto, flags)

def readKeymap(filename):
	p = enigma.eActionMap.getInstance()
	assert p

	source = open(filename)

	try:
		dom = xml.etree.cElementTree.parse(source)
	except:
		raise KeymapError("keymap %s not well-formed." % filename)

	keymap = dom.getroot()

	for cmap in keymap.findall("map"):
		context = cmap.attrib.get("context")
		assert context, "map must have context"

		for device in cmap.findall("device"):
			parseKeys(context, filename, p, device.attrib.get("name"), device)

		parseKeys(context, filename, p, "generic", cmap)

def removeKeymap(filename):
	p = enigma.eActionMap.getInstance()
	p.unbindKeyDomain(filename)
