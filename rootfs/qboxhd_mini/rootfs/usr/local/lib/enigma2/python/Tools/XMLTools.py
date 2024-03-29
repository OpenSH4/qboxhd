from qboxhd import QBOXHD
import xml.dom.minidom
from xml.dom.minidom import Node

def elementsWithTag(el, tag):

	"""filters all elements of childNode with the specified function
	example: nodes = elementsWithTag(childNodes, lambda x: x == "bla")"""

	# fiiixme! (works but isn't nice)
	if isinstance(tag, str):
		s = tag
		tag = lambda x: x == s
		
	for x in el:
		if x.nodeType != xml.dom.minidom.Element.nodeType:
			continue
		if tag(x.tagName):
			yield x

def mergeText(nodelist):
	rc = ""
	for node in nodelist:
		if node.nodeType == node.TEXT_NODE:
			rc = rc + node.data
	return rc

def stringToXML(text):
	return text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;').replace("'", '&apos;').replace('"', '&quot;')

def findChildrenByTagName(parent, tagname):
	"""return a list of 'tagname' children of 'parent'."""
	L = []
	for child in parent.childNodes:
		if (child.nodeType == Node.ELEMENT_NODE and child.tagName == tagname):
			L.append(child)
	return L

def searchChildrenfromAttribute(parent, tagname, attrname, attrvalue):
	L = []
	children = findChildrenByTagName( parent, tagname )
	
	for child in children:
		if (child.getAttribute(attrname) == attrvalue):
			L.append(child)
	return L
	
