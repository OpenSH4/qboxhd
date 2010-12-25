from qboxhd import QBOXHD
from os import system, listdir, statvfs, popen, makedirs, readlink, stat, major, minor, access
from Tools.Directories import SCOPE_HDD, resolveFilename
from Tools.CList import CList
from SystemInfo import SystemInfo
import time
from Components.Console import Console

import os, os.path
import gpod
import tempfile
import re
import sys
from optparse import OptionParser
import eyeD3
import imghdr
import shutil
import md5
import sha
import struct


IPOD_DEVICE 	= 0
IPHONE_DEVICE 	= 1
IPAD_DEVICE 	= 2

# Enable / Disable log of tracks
ENABLE_CACHING_DATA = 1
DEBUG_TRACK		= 0
DEBUG_PLAYLISTS	= 0
DEBUG_ARTISTS	= 0
DEBUG_ALBUMS	= 0
DEBUG_GENRES	= 0


EXTENSIONS = {
		"mp2": "AUDIO",
		"mp3": "AUDIO",
		"wav": "AUDIO",
		"ogg": "AUDIO",
		"flac": "AUDIO",
		"ts": "VIDEO",
		"avi": "VIDEO",
		"mpg": "VIDEO",
		"mpeg": "VIDEO",
		"mkv": "VIDEO",
		"mp4": "VIDEO",
	}


def getMediaTypes(track):
	extension = track.ipod_path.split('.')
	extension = extension[-1].lower()
	if EXTENSIONS.has_key(extension):
		return EXTENSIONS[extension]
	else:
		return ""

def sortby(nlist, n):
    nlist.sort(lambda x,y:cmp(x[n], y[n]))
	
	
def indexby(nlist, n, value):
	pos = -1
	for x in nlist:
		pos += 1
		if x[n] == value:
			return pos
	return -1


class iPodModel:
	ITDB_IPOD_MODEL_INVALID 		= "Invalid model"
	ITDB_IPOD_MODEL_UNKNOWN			= "Unknown model"
	ITDB_IPOD_MODEL_COLOR			= "Color iPod"
	ITDB_IPOD_MODEL_COLOR_U2		= "Color iPod (U2)"
	ITDB_IPOD_MODEL_REGULAR			= "Regular iPod"
	ITDB_IPOD_MODEL_REGULAR_U2		= "Regular iPod (U2)"
	ITDB_IPOD_MODEL_MINI			= "iPod Mini"
	ITDB_IPOD_MODEL_MINI_BLUE		= "iPod Mini (Blue)"
	ITDB_IPOD_MODEL_MINI_PINK		= "iPod Mini (Pink)"
	ITDB_IPOD_MODEL_MINI_GREEN		= "iPod Mini (Green)"
	ITDB_IPOD_MODEL_MINI_GOLD		= "iPod Mini (Gold)"
	ITDB_IPOD_MODEL_SHUFFLE			= "iPod Shuffle"
	ITDB_IPOD_MODEL_NANO_WHITE		= "iPod Nano (White)"
	ITDB_IPOD_MODEL_NANO_BLACK		= "iPod Nano (Black)"
	ITDB_IPOD_MODEL_VIDEO_WHITE		= "iPod Video (White)"
	ITDB_IPOD_MODEL_VIDEO_BLACK		= "iPod Video (Black)"
	ITDB_IPOD_MODEL_MOBILE_1		= "Mobile iPod"
	ITDB_IPOD_MODEL_VIDEO_U2		= "iPod Video (U2)"
	ITDB_IPOD_MODEL_NANO_SILVER		= "iPod Nano (Silver)"
	ITDB_IPOD_MODEL_NANO_BLUE		= "iPod Nano (Blue)"
	ITDB_IPOD_MODEL_NANO_GREEN		= "iPod Nano (Green)"
	ITDB_IPOD_MODEL_NANO_PINK		= "iPod Nano (Pink)"
	ITDB_IPOD_MODEL_NANO_RED		= "iPod Nano (Red)"
	ITDB_IPOD_MODEL_NANO_YELLOW 	= "iPod Nano (Yellow)"
	ITDB_IPOD_MODEL_NANO_PURPLE 	= "iPod Nano (Purple)"
	ITDB_IPOD_MODEL_NANO_ORANGE 	= "iPod Nano (Orange)"
	ITDB_IPOD_MODEL_IPHONE_1		= "iPhone"
	ITDB_IPOD_MODEL_SHUFFLE_SILVER 	= "iPod Shuffle (Silver)"
	ITDB_IPOD_MODEL_SHUFFLE_BLACK	= "iPod Shuffle (Black)"
	ITDB_IPOD_MODEL_SHUFFLE_PINK	= "iPod Shuffle (Pink)"
	ITDB_IPOD_MODEL_SHUFFLE_BLUE	= "iPod Shuffle (Blue)"
	ITDB_IPOD_MODEL_SHUFFLE_GREEN	= "iPod Shuffle (Green)"
	ITDB_IPOD_MODEL_SHUFFLE_ORANGE	= "iPod Shuffle (Orange)"
	ITDB_IPOD_MODEL_SHUFFLE_PURPLE	= "iPod Shuffle (Purple)"
	ITDB_IPOD_MODEL_SHUFFLE_RED		= "iPod Shuffle (Red)"
	ITDB_IPOD_MODEL_SHUFFLE_GOLD	= "iPod Shuffle (Gold)"
	ITDB_IPOD_MODEL_CLASSIC_SILVER	= "iPod Classic (Silver)"
	ITDB_IPOD_MODEL_CLASSIC_BLACK	= "iPod Classic (Black)"
	ITDB_IPOD_MODEL_TOUCH_SILVER	= "iPod Touch (Silver)"
	ITDB_IPOD_MODEL_IPHONE_WHITE	= "iPhone (White)"
	ITDB_IPOD_MODEL_IPHONE_BLACK	= "iPhone (Black)"
	ITDB_IPOD_MODEL_IPAD			= "iPad"
	
	
class iPodGeneration:
	ITDB_IPOD_GENERATION_UNKNOWN	= "Unknown iPod"
	ITDB_IPOD_GENERATION_FIRST		= "First Generation iPod"
	ITDB_IPOD_GENERATION_SECOND		= "Second Generation iPod"
	ITDB_IPOD_GENERATION_THIRD		= "Third Generation iPod"
	ITDB_IPOD_GENERATION_FOURTH		= "Fourth Generation iPod"
	ITDB_IPOD_GENERATION_PHOTO		= "Photo iPod"
	ITDB_IPOD_GENERATION_MOBILE		= "Mobile iPod"
	ITDB_IPOD_GENERATION_MINI_1		= "First Generation iPod Mini"
	ITDB_IPOD_GENERATION_MINI_2		= "Second Generation iPod Mini"
	ITDB_IPOD_GENERATION_SHUFFLE_1	= "First Generation iPod Shuffle"
	ITDB_IPOD_GENERATION_SHUFFLE_2	= "Second Generation iPod Shuffle"
	ITDB_IPOD_GENERATION_SHUFFLE_3	= "Third Generation iPod Shuffle"
	ITDB_IPOD_GENERATION_SHUFFLE_4	= "Third Generation iPod Shuffle"
	ITDB_IPOD_GENERATION_NANO_1		= "First Generation iPod Nano"
	ITDB_IPOD_GENERATION_NANO_2		= "Second Generation iPod Nano"
	ITDB_IPOD_GENERATION_NANO_3		= "Third Generation iPod Nano"
	ITDB_IPOD_GENERATION_NANO_4		= "Fourth Generation iPod Nano"
	ITDB_IPOD_GENERATION_NANO_5		= "Fifth Generation iPod Nano (with camera)"
	ITDB_IPOD_GENERATION_VIDEO_1	= "First Generation iPod Video (aka 5g)"
	ITDB_IPOD_GENERATION_VIDEO_2	= "Second Generation iPod Video (aka 5.5g)"
	ITDB_IPOD_GENERATION_CLASSIC_1	= "First Generation iPod Classic"
	ITDB_IPOD_GENERATION_CLASSIC_2	= "Second Generation iPod Classic"
	ITDB_IPOD_GENERATION_CLASSIC_3	= "Third Generation iPod Classic"
	ITDB_IPOD_GENERATION_TOUCH_1	= "First Generation iPod Touch"
	ITDB_IPOD_GENERATION_TOUCH_2	= "Second Generation iPod Touch"
	ITDB_IPOD_GENERATION_TOUCH_3	= "Third Generation iPod Touch"
	ITDB_IPOD_GENERATION_IPHONE_1	= "First Generation iPhone"
	ITDB_IPOD_GENERATION_IPHONE_2	= "Second Generation iPhone (aka iPhone 3G)"
	ITDB_IPOD_GENERATION_IPHONE_3	= "Third Generation iPhone (aka iPhone 3GS)"
	ITDB_IPOD_GENERATION_IPAD_1		= "First Generation iPad"
	
	
class iDevsDevice:
	
	__models = {
		"Invalid":( 0,  iPodModel.ITDB_IPOD_MODEL_INVALID, iPodGeneration.ITDB_IPOD_GENERATION_UNKNOWN),
		"Unknown":( 0,  iPodModel.ITDB_IPOD_MODEL_UNKNOWN, iPodGeneration.ITDB_IPOD_GENERATION_UNKNOWN),
		# First Generation 
		# Mechanical buttons arranged around rotating "scroll wheel".
		# 8513, 8541 and 8709 are Mac types, 8697 is PC
		"8513": (5, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_FIRST),
		"8541": (5, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_FIRST),
		"8697": (5, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_FIRST),
		"8709": (10, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_FIRST),
		# Second Generation
		# Same buttons as First Generation but around touch-sensitive
		#   "touch wheel". 8737 and 8738 are Mac types, 8740 and 8741 * are
		#   PC 
		"8737": (10, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_SECOND),
		"8740": (10, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_SECOND),
		"8738": (20, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_SECOND),
		"8741": (20, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_SECOND),
		
		# Third Generation 
		# Touch sensitive buttons and arranged in a line above "touch
		#   wheel". Docking connector was introduced here, same models for
		#   Mac and PC from now on.
		"8976": (10, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_THIRD),
		"8946": (15, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_THIRD),
		"9460": (15, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_THIRD),
		"9244": (20, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_THIRD),
		"8948": (30, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_THIRD),
		"9245": (40, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_THIRD),
		
		# Fourth Generation
		# Buttons are now integrated into the "touch wheel".
		"9282": (20, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_FOURTH),
		"9787": (25, iPodModel.ITDB_IPOD_MODEL_REGULAR_U2,  iPodGeneration.ITDB_IPOD_GENERATION_FOURTH),
		"9268": (40, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_FOURTH),
		
		# First Generation Mini
		"9160": (4, iPodModel.ITDB_IPOD_MODEL_MINI,        iPodGeneration.ITDB_IPOD_GENERATION_MINI_1),
		"9436": (4, iPodModel.ITDB_IPOD_MODEL_MINI_BLUE,   iPodGeneration.ITDB_IPOD_GENERATION_MINI_1),
		"9435": (4, iPodModel.ITDB_IPOD_MODEL_MINI_PINK,   iPodGeneration.ITDB_IPOD_GENERATION_MINI_1),
		"9434": (4, iPodModel.ITDB_IPOD_MODEL_MINI_GREEN,  iPodGeneration.ITDB_IPOD_GENERATION_MINI_1),
		"9437": (4, iPodModel.ITDB_IPOD_MODEL_MINI_GOLD,   iPodGeneration.ITDB_IPOD_GENERATION_MINI_1),
		# Second Generation Mini
		"9800": (4, iPodModel.ITDB_IPOD_MODEL_MINI,        iPodGeneration.ITDB_IPOD_GENERATION_MINI_2),
		"9802": (4, iPodModel.ITDB_IPOD_MODEL_MINI_BLUE,   iPodGeneration.ITDB_IPOD_GENERATION_MINI_2),
		"9804": (4, iPodModel.ITDB_IPOD_MODEL_MINI_PINK,   iPodGeneration.ITDB_IPOD_GENERATION_MINI_2),
		"9806": (4, iPodModel.ITDB_IPOD_MODEL_MINI_GREEN,  iPodGeneration.ITDB_IPOD_GENERATION_MINI_2),
		"9801": (6, iPodModel.ITDB_IPOD_MODEL_MINI,        iPodGeneration.ITDB_IPOD_GENERATION_MINI_2),
		"9803": (6, iPodModel.ITDB_IPOD_MODEL_MINI_BLUE,   iPodGeneration.ITDB_IPOD_GENERATION_MINI_2),
		"9805": (6, iPodModel.ITDB_IPOD_MODEL_MINI_PINK,   iPodGeneration.ITDB_IPOD_GENERATION_MINI_2),
		"9807": (6, iPodModel.ITDB_IPOD_MODEL_MINI_GREEN,  iPodGeneration.ITDB_IPOD_GENERATION_MINI_2),
		
		# Photo / Fourth Generation
		# Buttons are integrated into the "touch wheel".
		"A079": (20, iPodModel.ITDB_IPOD_MODEL_COLOR,       iPodGeneration.ITDB_IPOD_GENERATION_PHOTO),
		"A127": (20, iPodModel.ITDB_IPOD_MODEL_COLOR_U2,    iPodGeneration.ITDB_IPOD_GENERATION_PHOTO),
		"9829": (30, iPodModel.ITDB_IPOD_MODEL_COLOR,       iPodGeneration.ITDB_IPOD_GENERATION_PHOTO),
		"9585": (40, iPodModel.ITDB_IPOD_MODEL_COLOR,       iPodGeneration.ITDB_IPOD_GENERATION_PHOTO),
		"9830": (60, iPodModel.ITDB_IPOD_MODEL_COLOR,       iPodGeneration.ITDB_IPOD_GENERATION_PHOTO),
		"9586": (60, iPodModel.ITDB_IPOD_MODEL_COLOR,       iPodGeneration.ITDB_IPOD_GENERATION_PHOTO),
		
		# Shuffle / Fourth Generation 
		"9724": (0.5,iPodModel.ITDB_IPOD_MODEL_SHUFFLE,     iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_1),
		"9725": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE,     iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_1),
		
		# Shuffle / Sixth Generation
		# Square, connected to computer via cable */
		"A546": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_2),
		"A947": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_PINK,   iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_2),
		"A949": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_BLUE,   iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_2),
		"A951": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_GREEN,  iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_2),
		"A953": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_ORANGE, iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_2),
		"C167": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_GOLD,   iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_2),
		
		# Shuffle / Seventh Generation
		# Square, connected to computer via cable -- look identicaly to
		# Sixth Generation
		"B225": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B233": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_PURPLE, iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B231": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_RED,    iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B227": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_BLUE,   iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B228": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_BLUE,   iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B229": (1,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_GREEN,  iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B518": (2,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B520": (2,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_BLUE,   iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B522": (2,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_GREEN,  iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B524": (2,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_RED,    iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		"B526": (2,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_PURPLE, iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_3),
		
		# Shuffle / Eigth Generation
		# Bar, button-less, speaking
		"B867": (4,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_4),
		"C164": (4,  iPodModel.ITDB_IPOD_MODEL_SHUFFLE_BLACK,  iPodGeneration.ITDB_IPOD_GENERATION_SHUFFLE_4),
		
		# Nano / Fifth Generation (first nano generation)
		# Buttons are integrated into the "touch wheel".
		"A350": (1, iPodModel.ITDB_IPOD_MODEL_NANO_WHITE,  iPodGeneration.ITDB_IPOD_GENERATION_NANO_1),
		"A352": (1, iPodModel.ITDB_IPOD_MODEL_NANO_BLACK,  iPodGeneration.ITDB_IPOD_GENERATION_NANO_1),
		"A004": (2, iPodModel.ITDB_IPOD_MODEL_NANO_WHITE,  iPodGeneration.ITDB_IPOD_GENERATION_NANO_1),
		"A099": (2, iPodModel.ITDB_IPOD_MODEL_NANO_BLACK,  iPodGeneration.ITDB_IPOD_GENERATION_NANO_1),
		"A005": (4, iPodModel.ITDB_IPOD_MODEL_NANO_WHITE,  iPodGeneration.ITDB_IPOD_GENERATION_NANO_1),
		"A107": (4, iPodModel.ITDB_IPOD_MODEL_NANO_BLACK,  iPodGeneration.ITDB_IPOD_GENERATION_NANO_1),
		
		# Video / Fifth Generation
		# Buttons are integrated into the "touch wheel".
		"A002": (30, iPodModel.ITDB_IPOD_MODEL_VIDEO_WHITE, iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_1),
		"A146": (30, iPodModel.ITDB_IPOD_MODEL_VIDEO_BLACK, iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_1),
		"A003": (60, iPodModel.ITDB_IPOD_MODEL_VIDEO_WHITE, iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_1),
		"A147": (60, iPodModel.ITDB_IPOD_MODEL_VIDEO_BLACK, iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_1),
		"A452": (30, iPodModel.ITDB_IPOD_MODEL_VIDEO_U2,    iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_1),
		
		# Video / Sixth Generation
		# Pretty much identical to fifth generation with better display,
		# * extended battery operation time and gap-free playback
		"A444": (30, iPodModel.ITDB_IPOD_MODEL_VIDEO_WHITE, iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_2),
		"A446": (30, iPodModel.ITDB_IPOD_MODEL_VIDEO_BLACK, iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_2),
		"A664": (30, iPodModel.ITDB_IPOD_MODEL_VIDEO_U2,    iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_2),
		"A448": (80, iPodModel.ITDB_IPOD_MODEL_VIDEO_WHITE, iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_2),
		"A450": (80, iPodModel.ITDB_IPOD_MODEL_VIDEO_BLACK, iPodGeneration.ITDB_IPOD_GENERATION_VIDEO_2),
		
		# Nano / Sixth Generation (second nano generation)
		# Pretty much identical to fifth generation with better display,
		# * extended battery operation time and gap-free playback
		"A477": (2, iPodModel.ITDB_IPOD_MODEL_NANO_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_NANO_2),
		"A426": (4, iPodModel.ITDB_IPOD_MODEL_NANO_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_NANO_2),
		"A428": (4, iPodModel.ITDB_IPOD_MODEL_NANO_BLUE,   iPodGeneration.ITDB_IPOD_GENERATION_NANO_2),
		"A487": (4, iPodModel.ITDB_IPOD_MODEL_NANO_GREEN,  iPodGeneration.ITDB_IPOD_GENERATION_NANO_2),
		"A489": (4, iPodModel.ITDB_IPOD_MODEL_NANO_PINK,   iPodGeneration.ITDB_IPOD_GENERATION_NANO_2),
		"A725": (4, iPodModel.ITDB_IPOD_MODEL_NANO_RED,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_2),
		"A726": (8, iPodModel.ITDB_IPOD_MODEL_NANO_RED,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_2),
		"A497": (8, iPodModel.ITDB_IPOD_MODEL_NANO_BLACK,  iPodGeneration.ITDB_IPOD_GENERATION_NANO_2),
		
		# HP iPods, need contributions for this table
		# Buttons are integrated into the "touch wheel".
		"E436": (40, iPodModel.ITDB_IPOD_MODEL_REGULAR,     iPodGeneration.ITDB_IPOD_GENERATION_FOURTH),
		"S492": (30, iPodModel.ITDB_IPOD_MODEL_COLOR,       iPodGeneration.ITDB_IPOD_GENERATION_PHOTO),
		
		# iPod Classic G1
		# First generation with "cover flow"
		"B029": (80, iPodModel.ITDB_IPOD_MODEL_CLASSIC_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_CLASSIC_1),
		"B147": (80, iPodModel.ITDB_IPOD_MODEL_CLASSIC_BLACK,  iPodGeneration.ITDB_IPOD_GENERATION_CLASSIC_1),
		"B145": (160, iPodModel.ITDB_IPOD_MODEL_CLASSIC_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_CLASSIC_1),
		"B150": (160, iPodModel.ITDB_IPOD_MODEL_CLASSIC_BLACK,  iPodGeneration.ITDB_IPOD_GENERATION_CLASSIC_1),
		
		# iPod Classic G2
		"B562": (120, iPodModel.ITDB_IPOD_MODEL_CLASSIC_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_CLASSIC_2),
		"B565": (120, iPodModel.ITDB_IPOD_MODEL_CLASSIC_BLACK,  iPodGeneration.ITDB_IPOD_GENERATION_CLASSIC_2),
		
		# iPod Classic G3 
		"C293": (160, iPodModel.ITDB_IPOD_MODEL_CLASSIC_SILVER, iPodGeneration.ITDB_IPOD_GENERATION_CLASSIC_3),
		"C297": (160, iPodModel.ITDB_IPOD_MODEL_CLASSIC_BLACK,  iPodGeneration.ITDB_IPOD_GENERATION_CLASSIC_3),
		
		# iPod nano video G1 (Third Nano Generation)
		# First generation of video support for nano
		"A978": (4, iPodModel.ITDB_IPOD_MODEL_NANO_SILVER,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_3),
		"A980": (8, iPodModel.ITDB_IPOD_MODEL_NANO_SILVER,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_3),
		"B261": (8, iPodModel.ITDB_IPOD_MODEL_NANO_BLACK,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_3),
		"B249": (8, iPodModel.ITDB_IPOD_MODEL_NANO_BLUE,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_3),
		"B253": (8, iPodModel.ITDB_IPOD_MODEL_NANO_GREEN,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_3),
		"B257": (8, iPodModel.ITDB_IPOD_MODEL_NANO_RED,       iPodGeneration.ITDB_IPOD_GENERATION_NANO_3),
		
		# iPod nano video G2 (Fourth Nano Generation)
		"B480": (4, iPodModel.ITDB_IPOD_MODEL_NANO_SILVER,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B651": (4, iPodModel.ITDB_IPOD_MODEL_NANO_BLUE,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B654": (4, iPodModel.ITDB_IPOD_MODEL_NANO_PINK,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B657": (4, iPodModel.ITDB_IPOD_MODEL_NANO_PURPLE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B660": (4, iPodModel.ITDB_IPOD_MODEL_NANO_ORANGE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B663": (4, iPodModel.ITDB_IPOD_MODEL_NANO_GREEN,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B666": (4, iPodModel.ITDB_IPOD_MODEL_NANO_YELLOW,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		
		"B598": (8, iPodModel.ITDB_IPOD_MODEL_NANO_SILVER,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B732": (8, iPodModel.ITDB_IPOD_MODEL_NANO_BLUE,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B735": (8, iPodModel.ITDB_IPOD_MODEL_NANO_PINK,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B739": (8, iPodModel.ITDB_IPOD_MODEL_NANO_PURPLE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B742": (8, iPodModel.ITDB_IPOD_MODEL_NANO_ORANGE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B745": (8, iPodModel.ITDB_IPOD_MODEL_NANO_GREEN,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B748": (8, iPodModel.ITDB_IPOD_MODEL_NANO_YELLOW,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B751": (8, iPodModel.ITDB_IPOD_MODEL_NANO_RED,       iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B754": (8, iPodModel.ITDB_IPOD_MODEL_NANO_BLACK,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
	
		"B903": (16, iPodModel.ITDB_IPOD_MODEL_NANO_SILVER,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B905": (16, iPodModel.ITDB_IPOD_MODEL_NANO_BLUE,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B907": (16, iPodModel.ITDB_IPOD_MODEL_NANO_PINK,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B909": (16, iPodModel.ITDB_IPOD_MODEL_NANO_PURPLE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B911": (16, iPodModel.ITDB_IPOD_MODEL_NANO_ORANGE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B913": (16, iPodModel.ITDB_IPOD_MODEL_NANO_GREEN,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B915": (16, iPodModel.ITDB_IPOD_MODEL_NANO_YELLOW,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B917": (16, iPodModel.ITDB_IPOD_MODEL_NANO_RED,       iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		"B918": (16, iPodModel.ITDB_IPOD_MODEL_NANO_BLACK,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_4),
		
		# iPod nano with camera (Fifth Nano Generation)
		"C027": (8, iPodModel.ITDB_IPOD_MODEL_NANO_SILVER,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C031": (8, iPodModel.ITDB_IPOD_MODEL_NANO_BLACK,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C034": (8, iPodModel.ITDB_IPOD_MODEL_NANO_PURPLE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C037": (8, iPodModel.ITDB_IPOD_MODEL_NANO_BLUE,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C040": (8, iPodModel.ITDB_IPOD_MODEL_NANO_GREEN,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C043": (8, iPodModel.ITDB_IPOD_MODEL_NANO_YELLOW,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C046": (8, iPodModel.ITDB_IPOD_MODEL_NANO_ORANGE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C049": (8, iPodModel.ITDB_IPOD_MODEL_NANO_RED,       iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C050": (8, iPodModel.ITDB_IPOD_MODEL_NANO_PINK,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		
		"C060": (16, iPodModel.ITDB_IPOD_MODEL_NANO_SILVER,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C062": (16, iPodModel.ITDB_IPOD_MODEL_NANO_BLACK,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C064": (16, iPodModel.ITDB_IPOD_MODEL_NANO_PURPLE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C066": (16, iPodModel.ITDB_IPOD_MODEL_NANO_BLUE,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C068": (16, iPodModel.ITDB_IPOD_MODEL_NANO_GREEN,     iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C070": (16, iPodModel.ITDB_IPOD_MODEL_NANO_YELLOW,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C072": (16, iPodModel.ITDB_IPOD_MODEL_NANO_ORANGE,    iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C074": (16, iPodModel.ITDB_IPOD_MODEL_NANO_RED,       iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		"C075": (16, iPodModel.ITDB_IPOD_MODEL_NANO_PINK,      iPodGeneration.ITDB_IPOD_GENERATION_NANO_5),
		
		# iPod Touch 1st gen
		"A623": (8, iPodModel.ITDB_IPOD_MODEL_TOUCH_SILVER,   iPodGeneration.ITDB_IPOD_GENERATION_TOUCH_1),
		"A627": (16, iPodModel.ITDB_IPOD_MODEL_TOUCH_SILVER,   iPodGeneration.ITDB_IPOD_GENERATION_TOUCH_1),
		"B376": (32, iPodModel.ITDB_IPOD_MODEL_TOUCH_SILVER,   iPodGeneration.ITDB_IPOD_GENERATION_TOUCH_1),
		
		# iPod Touch 2nd gen
		"B528": (8, iPodModel.ITDB_IPOD_MODEL_TOUCH_SILVER,   iPodGeneration.ITDB_IPOD_GENERATION_TOUCH_2),
		"B531": (16, iPodModel.ITDB_IPOD_MODEL_TOUCH_SILVER,   iPodGeneration.ITDB_IPOD_GENERATION_TOUCH_2),
		"B533": (32, iPodModel.ITDB_IPOD_MODEL_TOUCH_SILVER,   iPodGeneration.ITDB_IPOD_GENERATION_TOUCH_2),
		
		# iPod Touch 3rd gen
		# The 8GB model is marked as 2nd gen because it's actually what the 
		# hardware is even if Apple markets it the same as the 2 bigger models
		"C086": (8, iPodModel.ITDB_IPOD_MODEL_TOUCH_SILVER,   iPodGeneration.ITDB_IPOD_GENERATION_TOUCH_2),
		"C008": (32, iPodModel.ITDB_IPOD_MODEL_TOUCH_SILVER,   iPodGeneration.ITDB_IPOD_GENERATION_TOUCH_3),
		"C011": (64, iPodModel.ITDB_IPOD_MODEL_TOUCH_SILVER,   iPodGeneration.ITDB_IPOD_GENERATION_TOUCH_3),
		
		# iPhone
		"A501": (4, iPodModel.ITDB_IPOD_MODEL_IPHONE_1,       iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_1),
		"A712": (8, iPodModel.ITDB_IPOD_MODEL_IPHONE_1,       iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_1),
		"B384": (16, iPodModel.ITDB_IPOD_MODEL_IPHONE_1,       iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_1),
		
		# iPhone G2 aka iPhone 3G (yeah, confusing ;)
		"B046": (8, iPodModel.ITDB_IPOD_MODEL_IPHONE_BLACK,   iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_2),
		"B500": (16, iPodModel.ITDB_IPOD_MODEL_IPHONE_WHITE,   iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_2),
		"B048": (16, iPodModel.ITDB_IPOD_MODEL_IPHONE_BLACK,   iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_2),
		"B496": (16, iPodModel.ITDB_IPOD_MODEL_IPHONE_BLACK,   iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_2),
		
		# iPhone 3GS
		"C131": (16, iPodModel.ITDB_IPOD_MODEL_IPHONE_BLACK,   iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_3),
		"C133": (32, iPodModel.ITDB_IPOD_MODEL_IPHONE_BLACK,   iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_3),
		"C134": (32, iPodModel.ITDB_IPOD_MODEL_IPHONE_WHITE,   iPodGeneration.ITDB_IPOD_GENERATION_IPHONE_3),
		
		# iPad
		"B292": (16, iPodModel.ITDB_IPOD_MODEL_IPAD,           iPodGeneration.ITDB_IPOD_GENERATION_IPAD_1),
		"B293": (32, iPodModel.ITDB_IPOD_MODEL_IPAD,           iPodGeneration.ITDB_IPOD_GENERATION_IPAD_1),
		"B294": (64, iPodModel.ITDB_IPOD_MODEL_IPAD,           iPodGeneration.ITDB_IPOD_GENERATION_IPAD_1),
		
		# iPad with 3G
		"C349": (16, iPodModel.ITDB_IPOD_MODEL_IPAD,           iPodGeneration.ITDB_IPOD_GENERATION_IPAD_1),
		"C496": (32, iPodModel.ITDB_IPOD_MODEL_IPAD,           iPodGeneration.ITDB_IPOD_GENERATION_IPAD_1),
		"C497": (64, iPodModel.ITDB_IPOD_MODEL_IPAD,           iPodGeneration.ITDB_IPOD_GENERATION_IPAD_1),
		
		# No known model number -- create a Device/SysInfo file with
		# one entry, e.g.:
		#   ModelNumStr: Mmobile1
		"mobile1": (-1, iPodModel.ITDB_IPOD_MODEL_MOBILE_1, iPodGeneration.ITDB_IPOD_GENERATION_MOBILE),
	}	
	
	
	def __init__(self, device, media_type, media_name):
		self.device = device
		self.mountpoint = self.__getMountpointFromDevice(device)
		self.media_type = media_type
		self.media_name = media_name
		self.itdb = None
		self.description, self.itemsNr, self.model, self.capacity, self.firmware = self.__getMainInfo()
		self.audioAlbums = None
		self.audioGenres = None
		self.audioArtists = None
		self.__initcaching()
		
		
	def __initcaching(self):
		if ENABLE_CACHING_DATA:
			self.getAudioAlbums()
			self.getAudioGenres()
			self.getAudioArtists()


	def __del__(self):
		if 	ENABLE_CACHING_DATA:
			self.audioAlbums = None
			self.audioGenres = None
			self.audioArtists = None
		
		self.__closeItdb()

	
	def __stat(self):
		return statvfs(self.mountpoint)
	
	
	def __get_details(self):
		d = {}
		sysinfo = os.path.join(self.mountpoint, 'iPod_Control', 'Device', 'SysInfo')
		
		if os.path.isfile(sysinfo):
			file = open(sysinfo)
			while True:
				line = file.readline()
				if not line: break
				parts = line.split()
				if len(parts) < 2: continue
				
				parts[0] = parts[0].rstrip(":")
				if parts[0] == "ModelNumStr" and parts[1][1:] in self.__models:
					d['space'], d['model'], d['generation'] = self.__models[parts[1][1:]]
				elif parts[0] == "visibleBuildID":
					d['firmware'] = parts[2].strip("()")
			file.close()
		else:
			# Assume an iPod shuffle
			info = statvfs(self.mountpoint)
			space = info.f_bsize * info.f_blocks
			if space > 512 * 1024 * 1024:
				model = 'M9725'
			else:
				model = 'M9724'
			if model in self.__models:
				d['space'], d['model'], d['generation'] = self.__models[model]
				
		return d
	
	
	
	def __getMainInfo(self):
		model = "unknown"
		capacity = "unknown"
		firmware = "unknown"
		
		if self.itdb is None:
			self.itdb = self.__openItdb()
			if self.itdb is None:
				return None, 0, model, capacity, firmware
			
		details = self.__get_details()
		if len(details) > 0:
			if 'model' in details:
				model = details['model']
			if 'space' in details:
				capacity = details['space']
			if 'firmware' in details:
				firmware = details['firmware']
			
		playlists = gpod.sw_get_playlists(self.itdb)
		if len(playlists)>0:
			return playlists[0].name, len(gpod.sw_get_playlist_tracks(playlists[0])), model, capacity, firmware
		else:
			return "unknown", 0, model, capacity, firmware
		
		self.__closeItdb()

	
	def free(self):
		try:
			s = self.__stat()
			return s.f_bavail * s.f_bsize
		except OSError:
			return None

	def total(self):
		try:
			s = self.__stat()
			return s.f_blocks * s.f_bsize
		except OSError:
			return None


	def __getMountpointFromDevice(self, device):
		#check if it is mounted
		lines = open("/proc/mounts", "r").readlines()
		for line in lines:
			res = line.find(device)
			if res > -1:
				voices = line.split(" ")
				return voices[1]
		
		raise "[iDevsDevice] No mountpoint found for device %s" % device

	# Opens the ITDB
	# Exits if fail, or returns nothing
	def __openItdb(self):
		print "DEBUG: Opening itdb .."
		i_itdb = gpod.itdb_parse(self.mountpoint, None)
		if not os.path.isdir(self.mountpoint):
			print "ERROR: Can't find ipod mountpoint %s" % self.mountpoint
			return None
	
		if not i_itdb:
			print "ERROR: Failed to read iPod itdb!"
			return None
		
		return i_itdb
	
	def __closeItdb(self):
		if self.itdb is not None:
			gpod.itdb_free(self.itdb)
			self.itdb = None
			
	def close(self):
		self.__closeItdb()
	
	# Return track rating as string eg '***'
	def __stars(self, t):
		if not t.rating:
			return '.'
		rate = ''
		for i in range(t.rating/20):
			rate += '*'
		return rate	
		
		
	# Convert millisecs to "h:m:s"
	def __prettyTime(self, time):
		s = time/1000
		m,s=divmod(s,60)
		h,m=divmod(m,60)
		d,h=divmod(h,24)
		ret = "%2.2d:%2.2d" % (m,s)
		if h:
			ret = "%d:%s" % (h, ret)
		return ret
		
	
	def getLengthofTrack(self, track):
		return self.__prettyTime(track.tracklen)
	
	
	def __sortTrackbyName(self, nlist):
		nlist.sort(lambda x,y:cmp(x.title, y.title))
	
				
	def __getTracks(self, kind):
		if self.itdb is None:
			self.itdb = self.__openItdb()
			if self.itdb is None:
				return None
		
		tracks=[]

		for track in gpod.sw_get_tracks(self.itdb):
			extension = track.ipod_path.split('.')
			extension = extension[-1].lower()
			if EXTENSIONS.has_key(extension):
				if EXTENSIONS[extension] == kind:
					tracks.append(track)
		
		self.__sortTrackbyName(tracks)
		
		return tracks


	def getAudioTracks(self):
		return 	self.__getTracks("AUDIO")
	
	
	def getVideoTracks(self):
		return 	self.__getTracks("VIDEO")
	
	


	
#######################################
# PLAYLIST FUNCTIONS
#######################################	

	def __getTracksFromPlayList(self, playlist, kind):
		tracks=[]
		for track in gpod.sw_get_playlist_tracks(playlist):
			extension = track.ipod_path.split('.')
			extension = extension[-1].lower()
			if EXTENSIONS.has_key(extension):
				if EXTENSIONS[extension] == kind:
					tracks.append(track)
		
		return tracks
				
				
	def getAudioTracksFromPlayList(self, playlist):
		return self.__getTracksFromPlayList(playlist, "AUDIO")
	
	
	def getVideoTracksFromPlayList(self, playlist):
		return self.__getTracksFromPlayList(playlist, "VIDEO")
				

	def __getPlayLists(self, kind):
		if self.itdb is None:
			self.itdb = self.__openItdb()
			if self.itdb is None:
				return None
			
		allplaylists = gpod.sw_get_playlists(self.itdb)
		
		# Remove first occurence of playlists
		if len(allplaylists)>0:
			allplaylists.pop(0)
		
		playlists=[]
		
		for playlist in allplaylists:
			
			# check if there is in this playlist kind many kind of file of "kind"
			tracks = self.__getTracksFromPlayList(playlist, kind)
			if tracks is None:
				return None
			
			if len(tracks)>0:
				playlists.append(playlist)
				
		if DEBUG_PLAYLISTS:
			print "| Name              | Items | Size   | Smart? |"
			print "+-------------------+-------+--------+--------+"
			for playlist in playlists:
				size = 0
				for track in self.__getTracksFromPlayList(playlist, kind):
					size += track.size
				if playlist.is_spl:
					isspl = "Yes"
				else:
					isspl = "No"
				print " %-19.19s  %4d   %5dMb    %3s " % (playlist.name, len(self.__getTracksFromPlayList(playlist, kind)), size/1024/1024, isspl)
		
		return playlists


	def getAudioPlayLists(self):
		return self.__getPlayLists("AUDIO")
	
	
	def getVideoPlayLists(self):
		return self.__getPlayLists("VIDEO")
		
	
	def __getNumberofTracksofPlayList(self, playlist, kind):
		return len(self.__getTracksFromPlayList(playlist, kind))
		
		
	def getNumberofAudioTracksofPlayList(self, playlist):
		return self.__getNumberofTracksofPlayList(playlist, "AUDIO")
		
		
	def getNumberofVideoTracksofPlayList(self, playlist):
		return self.__getNumberofTracksofPlayList(playlist, "VIDEO")
		
		
	def __getNumberOfPlayLists(self, kind):
		playlists = self.__getPlayLists(kind)
		if playlists is not None:
			return len(playlists)
		else:
			return 0
	
	
	def getNumberOfAudioPlayLists(self):
		return self.__getNumberOfPlayLists("AUDIO")
	
	
	def getNumberOfVideoPlayLists(self):
		return self.__getNumberOfPlayLists("VIDEO")
	
	
	def __getPlayListSize(self, playlist, kind):
		size = 0
		for track in self.__getTracksFromPlayList(playlist, kind):
			size += track.size
		return size
		
		
	def getAudioPlayListSize(self, playlist):
		return self.__getPlayListSize(playlist, "AUDIO")
		
	
	def getVideoPlayListSize(self, playlist):
		return self.__getPlayListSize(playlist, "VIDEO")
		
		
		
#######################################
# ARTISTs FUNCTIONS
#######################################	
	def __getArtists(self, kind):
		
		if self.itdb is None:
			self.itdb = self.__openItdb()
			if self.itdb is None:
				return None
		
		tracks =  self.__getTracks(kind)
		if tracks is None:
			return None
		
		artists = []
		for track in tracks:
			pos = indexby(artists, 0, track.artist )
			if pos >= 0:
				artists[pos][1] += 1
				artists[pos][2] += track.size
			else:
				artists.append([track.artist, 1, track.size])
		
		sortby(artists, 0)
		
		if DEBUG_ARTISTS:
			print "| Artists                       | Items | Size |"
			print "+-------------------------------+-------+------+"
			for artist in artists:
				print " %-19.19s  %4d   %5dMb" % (artist[0], artist[1], artist[2]/1024/1024)
		
		return artists
			
			
	def getAudioArtists(self):
		if ENABLE_CACHING_DATA:
			if self.audioArtists is None:
				self.audioArtists = self.__getArtists("AUDIO")
			return self.audioArtists
		else:
			return self.__getArtists("AUDIO")
	
	
	def getVideoArtists(self):
		return self.__getArtists("VIDEO")
	
	
	def __getNumberOfArtists(self, kind):
		artists = self.__getArtists(kind)
		if artists is not None:
			return len(artists)
		else:
			return 0
		
		
	def getNumberOfAudioArtists(self):
		if ENABLE_CACHING_DATA:
			if self.audioArtists is None:
				self.audioArtists = self.getAudioArtists()
				
			if self.audioArtists is not None:
				return len(self.audioArtists)
			else:
				return 0
				
		else:		
			return self.__getNumberOfArtists("AUDIO")
	
	
	def getNumberOfVideoArtists(self):
		return self.__getNumberOfArtists("VIDEO")
	
	
	def __getTracksFromArtist(self, kind, artist):
		if DEBUG_ARTISTS:
			print "%s Tracks for artist '%s':" % (kind, artist)
			print "| Title                       | Rating | Length |Plays|  Path |"
			print "+-----------------------------+--------+--------+-----+-------"
			
		tracks = self.__getTracks(kind)
		if tracks is None:
			return []
		
		artist_tracks = []
		for t in tracks:
			if t.artist == artist:
				if DEBUG_ARTISTS:
					print " %-30.30s %-5.5s %8s   %3d %s" % (t.title, self.__stars(t), self.__prettyTime(t.tracklen),t.playcount, t.ipod_path)
				artist_tracks.append( t )
		
		return artist_tracks
	
	
	def getAudioTracksFromArtist(self, artist):
		return self.__getTracksFromArtist( "AUDIO", artist )
	
	
	def getVideoTracksFromArtist(self, artist):
		return self.__getTracksFromArtist( "VIDEO", artist )
	
	
	def __getNumberofTracksofArtist(self, kind, artist):
		return len( self.__getTracksFromArtist( kind, artist ))

	
	def getNumberofAudioTracksofArtist(self, artist):
		return self.__getNumberofTracksofArtist( "AUDIO", artist )
	
	
	def getNumberofVideoTracksofArtist(self, artist):
		return self.__getNumberofTracksofArtist( "VIDEO", artist )
	
	
	def __getArtistSize(self, kind, artist):
		tracks = self.__getTracksFromArtist(kind, artist)
		if tracks is None:
			return 0
		
		size = 0
		for t in tracks:
			size += t.size
		return size
	
	
	def getAudioArtistSize(self, artist):
		return self.__getArtistSize("AUDIO", artist)
	
	
	def getVideoArtistSize(self, artist):
		return self.__getArtistSize("VIDEO", artist)
			
	
#######################################
# ALBUMs FUNCTIONS
#######################################	
	def __getAlbums(self, kind):
		if self.itdb is None:
			self.itdb = self.__openItdb()
			if self.itdb is None:
				return None
		
		tracks = self.__getTracks(kind)
		if tracks is None:
			return []
		
		albums = []
		for track in tracks:
			pos = indexby(albums, 0, track.album )
			if pos >= 0:
				albums[pos][1] += 1
				albums[pos][2] += track.size
			else:
				albums.append([track.album, 1, track.size])
		
		sortby(albums, 0)
		
		if DEBUG_ALBUMS:
			print "| Albums                       | Items | Size |"
			print "+-------------------------------+-------+------+"
			for album in albums:
				print " %-19.19s  %4d   %5dMb" % (album[0], album[1], album[2]/1024/1024)
		
		return albums
			
			
	def getAudioAlbums(self):
		if ENABLE_CACHING_DATA:
			if self.audioAlbums is None:
				self.audioAlbums = self.__getAlbums("AUDIO")
			return self.audioAlbums
		else:
			return self.__getAlbums("AUDIO")
	
	
	def getVideoAlbums(self):
		return self.__getAlbums("VIDEO")
	
			
	def __getNumberOfAlbums(self, kind):
		albums = self.__getAlbums(kind)
		if albums is not None:
			return len(albums)
		else:
			return 0
		
		
	def getNumberOfAudioAlbums(self):
		if ENABLE_CACHING_DATA:
			if self.audioAlbums is None:
				self.audioAlbums = self.getAudioAlbums()
				
			if self.audioAlbums is not None:
				return len(self.audioAlbums)
			else:
				return 0
		else:
			return self.__getNumberOfAlbums("AUDIO")
		

	def getNumberOfVideoAlbums(self):
		return self.__getNumberOfAlbums("VIDEO")
	
	
	def __getTracksFromAlbum(self, kind, album):
		if DEBUG_ALBUMS:
			print "%s Tracks for Album '%s':" % ( kind, album )
			print "| Title                       | Rating | Length |Plays|  Path |"
			print "+-----------------------------+--------+--------+-----+-------"
			
		tracks = self.__getTracks(kind)
		if tracks is None:
			return []
		
		album_tracks = []
		for t in tracks:
			if t.album == album:
				if DEBUG_ALBUMS:
					print " %-30.30s %-5.5s %8s   %3d %s" % (t.title, self.__stars(t), self.__prettyTime(t.tracklen),t.playcount, t.ipod_path)
				album_tracks.append( t )
		
		return album_tracks
				
				
	def getAudioTracksFromAlbum(self, album):
		return self.__getTracksFromAlbum("AUDIO", album)
	
	
	def getVideoTracksFromAlbum(self, album):
		return self.__getTracksFromAlbum("VIDEO", album)
	
	
	def __getNumberofTracksofAlbum(self, kind, album):
		return len(self.__getTracksFromAlbum(kind, album))

	
	def getNumberofAudioTracksofAlbum(self, album):
		return self.__getNumberofTracksofAlbum("AUDIO", album)
	
	
	def getNumberofVideoTracksofAlbum(self, album):
		return self.__getNumberofTracksofAlbum("VIDEO", album)
	
	
	def __getAlbumSize(self, kind, album):
		tracks = self.__getTracksFromAlbum(kind, album)
		if tracks is None:
			return 0
		
		size = 0
		for t in tracks:
			size += t.size
		return size
	
	
	def getAudioAlbumSize(self, album):
		return self.__getAlbumSize("AUDIO", album)
	
	
	def getVideoAlbumSize(self, album):
		return self.__getAlbumSize("VIDEO", album)
				
				
#######################################
# GENREs FUNCTIONS
#######################################	
	def __getGenres(self, kind):
		if self.itdb is None:
			self.itdb = self.__openItdb()
			if self.itdb is None:
				return None
		
		tracks = self.__getTracks(kind)
		if tracks is None:
			return []
		
		genres = []
		for track in tracks:
			pos = indexby(genres, 0, track.genre )
			if pos >= 0:
				genres[pos][1] += 1
				genres[pos][2] += track.size
			else:
				genres.append([track.genre, 1, track.size])
		
		sortby(genres, 0)
		
		if DEBUG_GENRES:
			print "| Genres                        | Items | Size |"
			print "+-------------------------------+-------+------+"
			for genre in genres:
				print " %-19.19s  %4d   %5dMb" % (genre[0], genre[1], genre[2]/1024/1024)
		
		return genres
			
			
	def getAudioGenres(self):
		if ENABLE_CACHING_DATA:
			if self.audioGenres is None:
				self.audioGenres = self.__getGenres("AUDIO")
			return self.audioGenres
		else:
			return self.__getGenres("AUDIO")
	
	
	def getVideoGenres(self):
		return self.__getGenres("VIDEO")
	
			
	def __getNumberOfGenres(self, kind):
		genres = self.__getGenres(kind)
		if genres is not None:
			return len(genres)
		else:
			return 0
		
		
	def getNumberOfAudioGenres(self):
		if ENABLE_CACHING_DATA:
			if self.audioGenres is None:
				self.audioGenres = self.getAudioGenres()
				
			if self.audioGenres is not None:
				return len(self.audioGenres)
			else:
				return 0
		else:
			return self.__getNumberOfGenres("AUDIO")
		

	def getNumberOfVideoGenres(self):
		return self.__getNumberOfGenres("VIDEO")
	
	
	def __getTracksFromGenre(self, kind, genre):
		if DEBUG_GENRES:
			print "%s Tracks for Genre '%s':" % ( kind, genre )
			print "| Title                       | Rating | Length |Plays|  Path |"
			print "+-----------------------------+--------+--------+-----+-------"
			
		tracks = self.__getTracks(kind)
		if tracks is None:
			return []
		
		genre_tracks = []
		for t in tracks:
			if t.genre == genre:
				if DEBUG_GENRES:
					print " %-30.30s %-5.5s %8s   %3d %s" % (t.title, self.__stars(t), self.__prettyTime(t.tracklen),t.playcount, t.ipod_path)
				genre_tracks.append( t )
		
		return genre_tracks
				
				
	def getAudioTracksFromGenre(self, genre):
		return self.__getTracksFromGenre("AUDIO", genre)
	
	
	def getVideoTracksFromGenre(self, genre):
		return self.__getTracksFromGenre("VIDEO", genre)
	
	
	def __getNumberofTracksofGenre(self, kind, genre):
		return len(self.__getTracksFromGenre(kind, genre))

	
	def getNumberofAudioTracksofGenre(self, genre):
		return self.__getNumberofTracksofGenre("AUDIO", genre)
	
	
	def getNumberofVideoTracksofGenre(self, genre):
		return self.__getNumberofTracksofGenre("VIDEO", genre)
	
	
	def __getGenreSize(self, kind, genre):
		tracks = self.__getTracksFromGenre(kind, genre)
		if tracks is None:
			return 0
		
		size = 0
		for t in tracks:
			size += t.size
		return size
	
	
	def getAudioGenreSize(self, genre):
		return self.__getGenreSize("AUDIO", genre)
	
	
	def getVideoGenreSize(self, album):
		return self.__getAlbumSize("VIDEO", genre)
	
	
	def isiPod(self):
		return self.media_type == IPOD_DEVICE
	
	
	def isiPad(self):
		return self.media_type == IPAD_DEVICE
		
		
	def isiPhone(self):
		return self.media_type == IPHONE_DEVICE


class iPod(iDevsDevice):
	def __init__(self, device):
		iDevsDevice.__init__(self, device = device, media_type = IPOD_DEVICE, media_name ="Apple iPod")
	

class iPhone(iDevsDevice):
	def __init__(self, device):
		iDevsDevice.__init__(self, device = device, media_type = IPHONE_DEVICE, media_name ="Apple iPhone")


class iPad(iDevsDevice):
	def __init__(self, device):
		iDevsDevice.__init__(self, device = device, media_type = IPAD_DEVICE, media_name ="Apple iPad" )


class iDevsDeviceManager:
	def __init__(self):
		self.idevsdevices = [ ]
		
		self.on_idevsdevice_insert_change = CList()
		
		if not self.check():
			return
		
		self.__enumerateiDevsDevices()
		
		SystemInfo["iPod"] = self.getiPodCount() > 0
		SystemInfo["NoiPod"] = self.getiPodCount() == 0
		
		SystemInfo["iPhone"] = self.getiPhoneCount() > 0
		SystemInfo["NoiPhone"] = self.getiPhoneCount() == 0
		
		SystemInfo["iPad"] = self.getiPadCount() > 0
		SystemInfo["NoiPad"] = self.getiPadCount() == 0
			
			
	def check(self):
		fp = file('/proc/stb/info/model', 'r')
		status = fp.read()
		fp.close()

		if status.find("qboxhd") >= 0:
			return True
		else:
			return False
		
	def iDevsDeviceList(self):
		return self.idevsdevices
			
			
	def iPodList(self):
		ipodslist = []
		for idevsdevice in self.idevsdevices:
			if idevsdevice.media_type == IPOD_DEVICE:
				ipodslist.append(idevsdevice)
		return ipodslist
			
			
	def iPhoneList(self):
		iphoneslist = []
		for idevsdevice in self.idevsdevices:
			if idevsdevice.media_type == IPHONE_DEVICE:
				iphoneslist.append(idevsdevice)
		return iphoneslist
	
	
	def iPadList(self):
		ipadslist = []
		for idevsdevice in self.idevsdevices:
			if idevsdevice.media_type == IPAD_DEVICE:
				ipadslist.append(idevsdevice)
		return ipadslist
		
		
	def getiDevsDeviceCount(self):
		return len(self.idevsdevices)
	
	
	def getiPodCount(self):
		counter = 0
		if len(self.idevsdevices) > 0:
			for idevsdevice in self.idevsdevices:
				if idevsdevice.media_type == IPOD_DEVICE:
					counter += 1
		return counter
	
	
	def getiPhoneCount(self):
		counter = 0
		if len(self.idevsdevices) > 0:
			for idevsdevice in self.idevsdevices:
				if idevsdevice.media_type == IPHONE_DEVICE:
					counter += 1
		return counter
	
	
	def getiPadCount(self):
		counter = 0
		if len(self.idevsdevices) > 0:
			for idevsdevice in self.idevsdevices:
				if idevsdevice.media_type == IPAD_DEVICE:
					counter += 1
		return counter
		
		
	def __getiDevsDeviceInfo(self, blockdev):
		devpath = "/sys/block/" + blockdev
		removable = False
		error = False
		media_type = ""
		vendor = ""
		device = ""
		mountpoint = ""
		idevsdevice = False
		
		try:
			removable = bool(int(open(devpath + "/removable").read()))
			vendor = str(open(devpath + "/device/vendor").read())
			if vendor.find("Apple") < 0:
				idevsdevice = False
				return error, idevsdevice, media_type, device, mountpoint
			
			media_type = str(open(devpath + "/device/model").read())
			
			if media_type.find("iPod") > -1:
				media_type = "iPod"
			elif media_type.find("iPhone") > -1:
				media_type = "iPhone"
			elif media_type.find("iPad") > -1:
				media_type = "iPad"
			else:
				idevsdevice = False
				return error, idevsdevice, media_type, device, mountpoint
				
			#check if it is mounted
			lines = open("/proc/mounts", "r").readlines()
			for line in lines:
				res = line.find("/media/%s" % media_type)
				res1 = line.find("/dev/%s" % blockdev)
				
				if (res > -1) and (res1 > -1):
					voices = line.split(" ")
					device = voices[0]
					mountpoint = voices[1]
					idevsdevice = True
					return error, idevsdevice, media_type, device, mountpoint
			
		except IOError:
			error = True

		return error, idevsdevice, media_type, device, mountpoint
		
		
	def __enumerateiDevsDevices(self):
		print "enumerating iDevsDevices..."
		for blockdev in listdir("/sys/block"):
			error, idevsdevice, media_type, device, mountpoint = self.__getiDevsDeviceInfo(blockdev)
			if not error and idevsdevice:
				print "ok, device=%s, partition=%s, mountpoint=%s, media_type=%s" % (blockdev, device, mountpoint, media_type)
				self.addiDevsDevice(device, media_type)


	def addiDevsDevice(self, device, media_type):
		
		if not self.check():
			return
		
		if media_type.find("iPod") >= 0:
			p = iPod(device)
			self.idevsdevices.append(p)
			
			self.on_idevsdevice_insert_change("add", p)
			
			SystemInfo["iPod"] = self.getiPodCount() > 0
			SystemInfo["NoiPod"] = self.getiPodCount() == 0
			
		elif media_type.find("iPhone") >= 0:
			p = iPhone(device)
			self.idevsdevices.append(p)
			
			self.on_idevsdevice_insert_change("add", p)
			
			SystemInfo["iPhone"] = self.getiPhoneCount() > 0
			SystemInfo["NoiPhone"] = self.getiPhoneCount() == 0
			
		elif media_type.find("iPad") >= 0:
			p = iPad(device)
			self.idevsdevices.append(p)
			
			self.on_idevsdevice_insert_change("add", p)
			
			SystemInfo["iPad"] = self.getiPadCount() > 0
			SystemInfo["NoiPad"] = self.getiPadCount() == 0
			
	
	
	def removeiDevsDevice(self, device, media_type):
		
		if not self.check():
			return
		
		if media_type.find("iPod") >= 0:
			for x in self.idevsdevices:
				if x.device == device and x.media_type == IPOD_DEVICE:
					self.idevsdevices.remove(x)
					self.on_idevsdevice_insert_change("remove", x)
					
			SystemInfo["iPod"] = self.getiPodCount() > 0
			SystemInfo["NoiPod"] = self.getiPodCount() == 0
			
		elif media_type.find("iPhone") >= 0:
			for x in self.idevsdevices:
				if x.device == device and x.media_type == IPHONE_DEVICE:
					self.idevsdevices.remove(x)
					self.on_idevsdevice_insert_change("remove", x)
					
			SystemInfo["iPhone"] = self.getiPhoneCount() > 0
			SystemInfo["NoiPhone"] = self.getiPhoneCount() == 0
		
		elif media_type.find("iPad") >= 0:
			for x in self.idevsdevices:
				if x.device == device and x.media_type == IPAD_DEVICE:
					self.idevsdevices.remove(x)
					self.on_idevsdevice_insert_change("remove", x)
		
			SystemInfo["iPad"] = self.getiPadCount() > 0
			SystemInfo["NoiPad"] = self.getiPadCount() == 0
			

idevsdevicemanager = iDevsDeviceManager()
