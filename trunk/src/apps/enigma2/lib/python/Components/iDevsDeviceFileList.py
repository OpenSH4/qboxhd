from qboxhd import QBOXHD
from re import compile as re_compile
from os import path as os_path, listdir
from MenuList import MenuList
from enigma import eTimer
from Components.iDevsDevice import idevsdevicemanager, getMediaTypes

from Tools.Directories import SCOPE_SKIN_IMAGE, resolveFilename

from enigma import RT_HALIGN_LEFT, RT_HALIGN_RIGHT, RT_HALIGN_CENTER, RT_VALIGN_CENTER, eListboxPythonMultiContent, \
	eServiceReference, eServiceCenter, gFont
from Tools.LoadPixmap import LoadPixmap

AUDIO_MODALITY = 0
VIDEO_MODALITY = 1

MEDIA_ICONS = {
		"AUDIO": "music_48x48",
		"VIDEO": "movie_48x48",
	}


# Entry types
IDEVS_NO_SPECIFIC_ENTRY				= 0
IDEVSWAIT_ENTRY 					= 1
IDEVSEMPTY_ENTRY					= 2
IDEVSDEVICE_ENTRY 					= 3
IDEVSDEVICE_AUDIO_MENU_ENTRY		= 4
IDEVSDEVICE_VIDEO_MENU_ENTRY		= 5
IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY	= 6
IDEVSDEVICE_AUDIO_ARTIST_ENTRY		= 7
IDEVSDEVICE_AUDIO_ALBUM_ENTRY		= 8
IDEVSDEVICE_AUDIO_GENRE_ENTRY		= 9
IDEVSDEVICE_AUDIO_TRACK_ENTRY		= 10

IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY	= 11
IDEVSDEVICE_VIDEO_TRACK_ENTRY		= 12




# Operations
OP_NO_OPERATION					= 0
OP_SHOW_AUDIO_MENU				= 1
OP_SHOW_VIDEO_MENU				= 2
OP_CHANGE_ENTRY					= 3
OP_GOTO_PARENT					= 4
OP_SELECT_TRACK_ENTRY			= 5
OP_DESELECT_TRACK_ENTRY 		= 6
OP_SELECT_ALL_TRACK_ENTRY 		= 7
OP_DESELECT_ALL_TRACK_ENTRY		= 8



def getNextBeginningWithChar(nlist, char):
	pos = -1
	for item in nlist:
		pos += 1
		if item[0][0] == IDEVSDEVICE_AUDIO_MENU_ENTRY:
			if item[0][3].find(char) == 0:
				return pos
			
		elif item[0][0] == IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY:
			if item[0][3].name.find(char) == 0:
				return pos

		elif item[0][0] == IDEVSDEVICE_AUDIO_ARTIST_ENTRY:
			if item[0][3].find(char) == 0:
				return pos
		
		elif item[0][0] == IDEVSDEVICE_AUDIO_ALBUM_ENTRY:
			if item[0][3].find(char) == 0:
				return pos
			
		elif item[0][0] == IDEVSDEVICE_AUDIO_GENRE_ENTRY:
			if item[0][3].find(char) == 0:
				return pos
			
		elif item[0][0] == IDEVSDEVICE_AUDIO_TRACK_ENTRY:
			if item[0][3].title.find(char) == 0:
				return pos
		
		elif item[0][0] == IDEVSDEVICE_VIDEO_MENU_ENTRY:
			if item[0][3].find(char) == 0:
				return pos
			
		elif item[0][0] == IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY:
			if item[0][3].name.find(char) == 0:
				return pos
			
		elif item[0][0] == IDEVSDEVICE_VIDEO_TRACK_ENTRY:
			if item[0][3].title.find(char) == 0:
				return pos
		else:
			print "UNKNOWN"
			return -1
	
	print "NOT FOUND"
	return -1


def iDevsWaitEntryComponent(idevsdevice):
	typeEntry = IDEVSWAIT_ENTRY
	parentEntry = None
	res = [ (typeEntry, idevsdevice, parentEntry) ]
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 500, 40, 0, RT_HALIGN_LEFT, _("Getting data from %s.\n Please wait ...") % idevsdevice.description))
	
	return res


def iDevsEmptyEntryComponent(parentEntry, idevsdevice, text):
	typeEntry = IDEVSEMPTY_ENTRY
	res = [ (typeEntry, idevsdevice, parentEntry) ]
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 500, 40, 0, RT_HALIGN_LEFT, _("No %s.") % text))
	
	return res


def iDevsDeviceEntryComponent(idevsdevice):
	typeEntry = IDEVSDEVICE_ENTRY
	parentEntry = None
	res = [ (typeEntry, idevsdevice, parentEntry) ]
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 200, 20, 0, RT_HALIGN_LEFT, idevsdevice.description))
	
	value = idevsdevice.total() / (1000 * 1000)
	value1 = idevsdevice.free() / (1000 * 1000)
	tmp_str = _("Space: %d.%02d GB Free: %d.%02d GB\n") % (value/1000, value%1000, value1/1000, value1%1000,)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 20, 250, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	tmp_str = _("Model: %s\n") % idevsdevice.model
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 40, 200, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	used = idevsdevice.total() - idevsdevice.free()
	percent = ( used * 100 ) / idevsdevice.total()
	
	res.append((eListboxPythonMultiContent.TYPE_PROGRESS, 300, 15, 125, 20, percent))
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 425, 15, 75, 20, 1, RT_HALIGN_RIGHT, str(percent) + _("% used")))
	
	if idevsdevice.firmware != "unknown":
		tmp_str = _("Firmware: %s") % (idevsdevice.firmware)
		res.append((eListboxPythonMultiContent.TYPE_TEXT, 300, 40, 125, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	tmp_str = _("Items: %d") % (idevsdevice.itemsNr)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 425, 40, 75, 20, 1, RT_HALIGN_RIGHT, tmp_str))
	
	if idevsdevice.isiPod():
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/devices/ipod_48x48.png"))
	elif idevsdevice.isiPhone():
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/devices/iphone_48x48.png"))
	elif idevsdevice.isiPad():
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/devices/ipad_48x48.png"))
	else:
		print "Unknown device"
		png = None
		
	if png is not None:
		res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
	
	return res
	
	
def iDevsDeviceAudioMenuItemEntryComponent(idevsdevice, voice):
	typeEntry = IDEVSDEVICE_AUDIO_MENU_ENTRY
	parentEntry = None
	res = [ (typeEntry, idevsdevice, parentEntry, str(voice)) ]
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, voice))
	
	if voice == _("PlayLists"):
		tmp_str = _("PlayLists: %d") % (idevsdevice.getNumberOfAudioPlayLists())
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/idevs_playlist_audio.png"))
		
	elif voice == _("Artists"):
		tmp_str = _("Artists: %d") % (idevsdevice.getNumberOfAudioArtists())
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/artist_48x48.png"))
		
	elif voice == _("Albums"):
		tmp_str = _("Albums: %d") % (idevsdevice.getNumberOfAudioAlbums())
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/album_48x48.png"))
		
	elif voice == _("Titles"):
		tmp_str = _("Titles: %d") % (len(idevsdevice.getAudioTracks()))
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/music_48x48.png"))
		
	elif voice == _("Genres"):
		tmp_str = _("Genres: %d") % (idevsdevice.getNumberOfAudioGenres())
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/genre_48x48.png"))
	
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))	
	
	return res
	

def iDevsDeviceVideoMenuItemEntryComponent(idevsdevice, voice):
	typeEntry = IDEVSDEVICE_VIDEO_MENU_ENTRY
	parentEntry = None	
	res = [ (typeEntry, idevsdevice, parentEntry, str(voice)) ]
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, voice))
	
	if voice == _("PlayLists Video"):
		tmp_str = _("PlayLists: %d") % (idevsdevice.getNumberOfVideoPlayLists())
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/idevs_playlist_video.png"))
		
	elif voice == _("Videos"):
		tmp_str = _("Videos: %d") % (len(idevsdevice.getVideoTracks()))
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/movie_48x48.png"))
	
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))	
	
	return res


def iDevsDeviceAudioPlayListEntryComponent(parentEntry, idevsdevice, playlist):
	typeEntry = IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY
	res = [ (typeEntry, idevsdevice, parentEntry, playlist) ]
	if playlist.name is None:
		res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, "unknown"))
	else:
		res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, playlist.name))

	
	tmp_str = _("Items: %d") % (idevsdevice.getNumberofAudioTracksofPlayList(playlist))
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	value = idevsdevice.getAudioPlayListSize(playlist)
	tmp_str = _("Size: %d MB\n") % (value/1024/1024)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 150, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/idevs_playlist_audio.png"))
	res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
	
	return res



def iDevsDeviceVideoPlayListEntryComponent(parentEntry, idevsdevice, playlist):
	typeEntry = IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY
	res = [ (typeEntry, idevsdevice, parentEntry, playlist) ]
	if playlist.name is None:
		res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, _("unknown")))
	else:
		res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, playlist.name))
	
	tmp_str = _("Items: %d") % (idevsdevice.getNumberofVideoTracksofPlayList(playlist))
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	value = idevsdevice.getVideoPlayListSize(playlist)
	tmp_str = _("Size: %d MB\n") % (value/1024/1024)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 150, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/idevs_playlist_video.png"))
	res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
	
	return res



def iDevsDeviceAudioArtistEntryComponent(parentEntry, idevsdevice, name, nitems, size):
	typeEntry = IDEVSDEVICE_AUDIO_ARTIST_ENTRY
	res = [ (typeEntry, idevsdevice, parentEntry, str(name)) ]
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, name))
	
	tmp_str = _("Items: %d") % (nitems)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	tmp_str = _("Size: %d MB\n") % (size/1024/1024)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 150, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/artist_48x48.png"))
	res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
	
	return res



def iDevsDeviceAudioAlbumEntryComponent(parentEntry, idevsdevice, name, nitems, size):
	typeEntry = IDEVSDEVICE_AUDIO_ALBUM_ENTRY
	res = [ (typeEntry, idevsdevice, parentEntry, name) ]
	if name is None:
		res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, _("unknown")))
	else:
		res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, name))
	
	tmp_str = _("Items: %d") % (nitems)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	tmp_str = _("Size: %d MB\n") % (size/1024/1024)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 150, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/album_48x48.png"))
	res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
	
	return res



def iDevsDeviceAudioGenreEntryComponent(parentEntry, idevsdevice, name, nitems, size):
	typeEntry = IDEVSDEVICE_AUDIO_GENRE_ENTRY
	res = [ (typeEntry, idevsdevice, parentEntry, name) ]
	if name is None:
		res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, _("unknown")))
	else:
		res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 470, 20, 0, RT_HALIGN_LEFT, name))
	
	tmp_str = _("Items: %d") % (nitems)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	tmp_str = _("Size: %d MB\n") % (size/1024/1024)
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 150, 25, 100, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/genre_48x48.png"))
	res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
	
	return res


def iDevsDeviceAudioTrackEntryComponent(parentEntry, idevsdevice, track, selected):
	typeEntry = IDEVSDEVICE_AUDIO_TRACK_ENTRY
	res = [ (typeEntry, idevsdevice, parentEntry, track, selected) ]
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 317, 20, 0, RT_HALIGN_LEFT, track.title))
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 380, 1, 120, 20, 0, RT_HALIGN_RIGHT, idevsdevice.getLengthofTrack(track)))
	
	if track.artist is not None:
		tmp_str = _("Artist: %s") % (track.artist)
	else:
		tmp_str = _("Artist: unknown")
	
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 25, 290, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	if track.album is not None:
		tmp_str = _("Album: %s") % (track.album)
	else:
		tmp_str = _("Album: unknown")
		
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 45, 290, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	if track.genre is not None:	
		tmp_str = _("Genre: %s") % (track.genre)
	else:
		tmp_str = _("Genre: unknown")
	
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 300, 45, 200, 20, 1, RT_HALIGN_RIGHT, tmp_str))
	
	if len(getMediaTypes(track)) > 0:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/" + MEDIA_ICONS[getMediaTypes(track)] + ".png"))
	else:
		png = None
	
	if png is not None:
		res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
		
	if selected:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/lock_on.png"))
		res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
	
	return res



def iDevsDeviceVideoTrackEntryComponent(parentEntry, idevsdevice, track, selected):
	typeEntry = IDEVSDEVICE_VIDEO_TRACK_ENTRY
	res = [ (typeEntry, idevsdevice, parentEntry, track, selected) ]
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 1, 317, 20, 0, RT_HALIGN_LEFT, track.title))
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 380, 1, 120, 20, 0, RT_HALIGN_RIGHT, idevsdevice.getLengthofTrack(track)))
	
	if track.artist is not None:
		tmp_str = _("Artist: %s") % (track.artist)
	else:
		tmp_str = _("Artist: unknown")
	
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 25, 290, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	if track.album is not None:
		tmp_str = _("Album: %s") % (track.album)
	else:
		tmp_str = _("Album: unknown")
		
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 63, 45, 290, 20, 1, RT_HALIGN_LEFT, tmp_str))
	
	if track.genre is not None:	
		tmp_str = _("Genre: %s") % (track.genre)
	else:
		tmp_str = _("Genre: unknown")
	
	res.append((eListboxPythonMultiContent.TYPE_TEXT, 300, 45, 200, 20, 1, RT_HALIGN_RIGHT, tmp_str))
	
	if len(getMediaTypes(track)) > 0:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "extensions/idevs/" + MEDIA_ICONS[getMediaTypes(track)] + ".png"))
	else:
		png = None
		
	if png is not None:
		res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
		
	if selected:
		png = LoadPixmap(resolveFilename(SCOPE_SKIN_IMAGE, "skin_default/icons/lock_on.png"))
		res.append((eListboxPythonMultiContent.TYPE_PIXMAP_ALPHATEST, 10, 2, 48, 48, png))
	
	return res



class iDevsDeviceFileList(MenuList):
	def __init__(self, entryEventCB = None, enableWrapAround = False ):
		MenuList.__init__(self, list, enableWrapAround, eListboxPythonMultiContent)
		
		if idevsdevicemanager.getiDevsDeviceCount() > 0:
			self.__showiDevsDeviceList()
		else:
			self.list = []
			self.l.setList(self.list)
		
		self.isTop = True
		self.showModality = AUDIO_MODALITY
		self.serviceHandler = eServiceCenter.getInstance()
		
		self.TimerOperation = eTimer()
		self.TimerOperation.callback.append(self.__operationCB)
		self.operation = None
		
		self.selectedtrack = []
		
		self.curr_selected = None
		self.entryEventCB = entryEventCB


	def moveToChar(self, char):
		# TODO fill with life
		print "Next char: ", char
		index = getNextBeginningWithChar(self.list, char)
		indexup = getNextBeginningWithChar(self.list, char.upper())
		if indexup != -1:
			if (index > indexup or index == 0):
				index = indexup
				
		self.moveToIndex(index)
		print "Moving to character %s at pos %d" % ( str(char), index )


	def __showiDevsDeviceList(self):
		self.l.setFont(0, gFont("Regular", 18))
		self.l.setFont(1, gFont("Regular", 14))
		self.l.setItemHeight(64)
		self.list = [ ]
		for idevsdevice in idevsdevicemanager.iDevsDeviceList():
			self.list.append(iDevsDeviceEntryComponent(idevsdevice))
			
		self.l.setList(self.list)
		
		
	def __showiDevsAudioMenu(self, idevsdevice):
		self.l.setFont(0, gFont("Regular", 18))
		self.l.setFont(1, gFont("Regular", 14))
		self.l.setItemHeight(55)
		self.list = []
		self.list.append(iDevsDeviceAudioMenuItemEntryComponent(idevsdevice, _("PlayLists")))
		self.list.append(iDevsDeviceAudioMenuItemEntryComponent(idevsdevice, _("Artists")))
		self.list.append(iDevsDeviceAudioMenuItemEntryComponent(idevsdevice, _("Albums")))
		self.list.append(iDevsDeviceAudioMenuItemEntryComponent(idevsdevice, _("Titles")))
		self.list.append(iDevsDeviceAudioMenuItemEntryComponent(idevsdevice, _("Genres")))
		self.l.setList(self.list)
		
		self.entryEventCB(IDEVSDEVICE_AUDIO_MENU_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsVideoMenu(self, idevsdevice):
		self.l.setFont(0, gFont("Regular", 18))
		self.l.setFont(1, gFont("Regular", 14))
		self.l.setItemHeight(55)
		self.list = []
		self.list.append(iDevsDeviceVideoMenuItemEntryComponent(idevsdevice, _("PlayLists Video")))
		self.list.append(iDevsDeviceVideoMenuItemEntryComponent(idevsdevice, _("Videos")))
		self.l.setList(self.list)
		
		self.entryEventCB(IDEVSDEVICE_VIDEO_MENU_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsAudioPlayLists(self, idevsdevice):
		playlists = idevsdevice.getAudioPlayLists()
		if playlists is None or len(playlists) == 0:
			self.__showiDevsEmpty(idevsdevice, "playlist")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(55)
			self.list = []
			for playlist in playlists:
				self.list.append(iDevsDeviceAudioPlayListEntryComponent(self.curr_selected, idevsdevice, playlist))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsAudioArtists(self, idevsdevice):
		artists = idevsdevice.getAudioArtists()
		if artists is None or len(artists) == 0:
			self.__showiDevsEmpty(idevsdevice, "artist")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(55)
			self.list = []
			for artist in artists:
				self.list.append(iDevsDeviceAudioArtistEntryComponent(self.curr_selected, idevsdevice, artist[0], artist[1], artist[2] ))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_AUDIO_ARTIST_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsAudioAlbums(self, idevsdevice):
		albums = idevsdevice.getAudioAlbums()
		if albums is None or len(albums) == 0:
			self.__showiDevsEmpty(idevsdevice, "album")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(55)
			self.list = []
			for album in albums:
				self.list.append(iDevsDeviceAudioAlbumEntryComponent(self.curr_selected, idevsdevice, album[0], album[1], album[2] ))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_AUDIO_ALBUM_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsAudioGenres(self, idevsdevice):
		genres = idevsdevice.getAudioGenres()
		if genres is None or len(genres) == 0:
			self.__showiDevsEmpty(idevsdevice, "genre")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(55)
			self.list = []
			for genre in genres:
				self.list.append(iDevsDeviceAudioGenreEntryComponent(self.curr_selected, idevsdevice, genre[0], genre[1], genre[2]))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_AUDIO_GENRE_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsAudioAllTracks(self, idevsdevice):
		tracks = idevsdevice.getAudioTracks()
		if tracks is None or len(tracks) == 0:
			self.__showiDevsEmpty(idevsdevice, "audio track")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(64)
			self.list = []
			for track in tracks:
				selected = ( self.__isSelectedTrack(idevsdevice, track) >= 0 )
				self.list.append(iDevsDeviceAudioTrackEntryComponent(self.curr_selected, idevsdevice, track, selected))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_AUDIO_TRACK_ENTRY, OP_NO_OPERATION)
		
	
	def __showiDevsVideoPlayLists(self, idevsdevice):
		playlists = idevsdevice.getVideoPlayLists()
		if playlists is None or len(playlists) == 0:
			self.__showiDevsEmpty(idevsdevice, "playlists")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(55)
			self.list = []
			for playlist in playlists:
				self.list.append(iDevsDeviceVideoPlayListEntryComponent(self.curr_selected, idevsdevice, playlist))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY, OP_NO_OPERATION)
		
	
	def __showiDevsVideoAllTracks(self, idevsdevice):
		tracks = idevsdevice.getVideoTracks()
		if tracks is None or len(tracks) == 0:
			self.__showiDevsEmpty(idevsdevice, "video track")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(64)
			self.list = []
			for track in tracks:
				selected = ( self.__isSelectedTrack(idevsdevice, track) >= 0 )
				self.list.append(iDevsDeviceVideoTrackEntryComponent(self.curr_selected, idevsdevice, track, selected))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_VIDEO_TRACK_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsAudioTracksofPlayList(self, idevsdevice, playlist):
		tracks = idevsdevice.getAudioTracksFromPlayList(playlist)
		if tracks is None or len(tracks) == 0:
			self.__showiDevsEmpty(idevsdevice, "audio")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(64)
			self.list = []
			for track in tracks:
				selected = ( self.__isSelectedTrack(idevsdevice, track) >= 0 )
				self.list.append(iDevsDeviceAudioTrackEntryComponent(self.curr_selected, idevsdevice, track, selected))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_AUDIO_TRACK_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsVideoTracksofPlayList(self, idevsdevice, playlist):
		tracks = idevsdevice.getVideoTracksFromPlayList(playlist)
		if tracks is None or len(tracks) == 0:
			self.__showiDevsEmpty(idevsdevice, "playlist")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(64)
			self.list = []
			for track in tracks:
				selected = ( self.__isSelectedTrack(idevsdevice, track) >= 0 )
				self.list.append(iDevsDeviceVideoTrackEntryComponent(self.curr_selected, idevsdevice, track, selected))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_VIDEO_TRACK_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsAudioTracksofArtist(self, idevsdevice, artist):
		tracks = idevsdevice.getAudioTracksFromArtist(artist)
		if tracks is None or len(tracks) == 0:
			self.__showiDevsEmpty(idevsdevice, "artist")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(64)
			self.list = []
			for track in tracks:
				selected = ( self.__isSelectedTrack(idevsdevice, track) >= 0 )
				self.list.append(iDevsDeviceAudioTrackEntryComponent(self.curr_selected, idevsdevice, track, selected))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_AUDIO_TRACK_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsAudioTracksofAlbum(self, idevsdevice, album):
		tracks = idevsdevice.getAudioTracksFromAlbum(album)
		if tracks is None or len(tracks) == 0:
			self.__showiDevsEmpty(idevsdevice, "album")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(64)
			self.list = []
			for track in tracks:
				selected = ( self.__isSelectedTrack(idevsdevice, track) >= 0 )
				self.list.append(iDevsDeviceAudioTrackEntryComponent(self.curr_selected, idevsdevice, track, selected))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_AUDIO_TRACK_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsAudioTracksofGenre(self, idevsdevice, genre):
		tracks = idevsdevice.getAudioTracksFromGenre(genre)
		if tracks is None or len(tracks) == 0:
			self.__showiDevsEmpty(idevsdevice, "genre")
		else:
			self.l.setFont(0, gFont("Regular", 18))
			self.l.setFont(1, gFont("Regular", 14))
			self.l.setItemHeight(64)
			self.list = []
			for track in tracks:
				selected = ( self.__isSelectedTrack(idevsdevice, track) >= 0 )
				self.list.append(iDevsDeviceAudioTrackEntryComponent(self.curr_selected, idevsdevice, track, selected))
			self.l.setList(self.list)
			
			self.entryEventCB(IDEVSDEVICE_AUDIO_TRACK_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsWait(self, idevsdevice):
		self.list = []
		self.l.setFont(0, gFont("Regular", 18))
		self.l.setFont(1, gFont("Regular", 14))
		self.l.setItemHeight(64)
		self.list.append(iDevsWaitEntryComponent(idevsdevice))
		self.l.setList(self.list)
		
		self.entryEventCB(IDEVSWAIT_ENTRY, OP_NO_OPERATION)
		
		
	def __showiDevsEmpty(self, idevsdevice, text):
		self.list = []
		self.l.setFont(0, gFont("Regular", 18))
		self.l.setFont(1, gFont("Regular", 14))
		self.l.setItemHeight(64)
		self.list.append(iDevsEmptyEntryComponent(self.curr_selected, idevsdevice, text))
		self.l.setList(self.list)
		
		self.entryEventCB(IDEVSEMPTY_ENTRY, OP_NO_OPERATION)
		
		
	def __operationCB(self):
		self.TimerOperation.stop()
		
		if self.curr_selected is None:
			return
		
		currentType = self.curr_selected[0]
		currentiDev = self.curr_selected[1]
		
		if self.operation == OP_SHOW_AUDIO_MENU:
			self.__showiDevsAudioMenu( currentiDev )
			
		elif self.operation == OP_SHOW_VIDEO_MENU:
			self.__showiDevsVideoMenu( currentiDev )
			
		elif self.operation == OP_CHANGE_ENTRY:
			
			if currentType == IDEVSWAIT_ENTRY or currentType == IDEVSEMPTY_ENTRY:
				return
				
			elif currentType == IDEVSDEVICE_ENTRY:
				
				self.isTop = False
				if self.showModality == AUDIO_MODALITY:
					self.__showiDevsAudioMenu( currentiDev )
				else:
					self.__showiDevsVideoMenu( currentiDev )
				
			elif currentType == IDEVSDEVICE_AUDIO_MENU_ENTRY:
				
				voice = self.curr_selected[3]
				
				if voice == _("PlayLists"):
					self.__showiDevsAudioPlayLists( currentiDev )
				elif voice == _("Artists"):
					self.__showiDevsAudioArtists( currentiDev )
				elif voice == _("Albums"):
					self.__showiDevsAudioAlbums( currentiDev )
				elif voice == _("Titles"):
					self.__showiDevsAudioAllTracks( currentiDev )
				elif voice == _("Genres"):
					self.__showiDevsAudioGenres( currentiDev )
			
			elif currentType == IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY:
				self.__showiDevsAudioTracksofPlayList( currentiDev, self.curr_selected[3] )
				
			elif currentType == IDEVSDEVICE_AUDIO_ARTIST_ENTRY:
				self.__showiDevsAudioTracksofArtist( currentiDev, self.curr_selected[3] )
			
			elif currentType == IDEVSDEVICE_AUDIO_ALBUM_ENTRY:
				self.__showiDevsAudioTracksofAlbum( currentiDev, self.curr_selected[3] )
			
			elif currentType == IDEVSDEVICE_AUDIO_GENRE_ENTRY:
				self.__showiDevsAudioTracksofGenre( currentiDev, self.curr_selected[3] )
			
			elif currentType == IDEVSDEVICE_VIDEO_MENU_ENTRY:
				
				voice = self.curr_selected[3]
				
				if voice == _("PlayLists Video"):
					self.__showiDevsVideoPlayLists( currentiDev )
				elif voice == _("Videos"):
					self.__showiDevsVideoAllTracks( currentiDev )
			
			elif currentType == IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY:
				self.__showiDevsVideoTracksofPlayList( currentiDev, self.curr_selected[3] )
			
		elif self.operation == OP_GOTO_PARENT:
			
			if currentType == IDEVSWAIT_ENTRY:
				return
			
			elif currentType == IDEVSDEVICE_AUDIO_MENU_ENTRY or currentType == IDEVSDEVICE_VIDEO_MENU_ENTRY:
				self.isTop = True
				self.__showiDevsDeviceList()
				
			elif currentType == IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY or currentType == IDEVSDEVICE_AUDIO_ARTIST_ENTRY or currentType == IDEVSDEVICE_AUDIO_ALBUM_ENTRY or currentType == IDEVSDEVICE_AUDIO_GENRE_ENTRY:
				self.__showiDevsAudioMenu( currentiDev )
				
			elif currentType == IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY:
				self.__showiDevsVideoMenu( currentiDev )
			
			else:
				
				parentEntry = self.curr_selected[2]
				
				if currentType == IDEVSDEVICE_AUDIO_TRACK_ENTRY:
					# Titles 
					if parentEntry[0] == IDEVSDEVICE_AUDIO_MENU_ENTRY:
						self.__showiDevsAudioMenu( currentiDev )
						
					elif parentEntry[0] == IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY:
						self.__showiDevsAudioPlayLists( currentiDev )
						
					elif parentEntry[0] == IDEVSDEVICE_AUDIO_ARTIST_ENTRY:
						self.__showiDevsAudioArtists( currentiDev )
						
					elif parentEntry[0] == IDEVSDEVICE_AUDIO_ALBUM_ENTRY:
						self.__showiDevsAudioAlbums( currentiDev )
				
					elif parentEntry[0] == IDEVSDEVICE_AUDIO_GENRE_ENTRY:
						self.__showiDevsAudioGenres( currentiDev )
				
				elif currentType == IDEVSDEVICE_VIDEO_TRACK_ENTRY:
					# Videos
					if parentEntry[0] == IDEVSDEVICE_VIDEO_MENU_ENTRY:
						self.__showiDevsVideoMenu( currentiDev )
					
					elif parentEntry[0] == IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY:
						self.__showiDevsVideoPlayLists( currentiDev )
						
				elif currentType == IDEVSEMPTY_ENTRY:
					
					# Audio 
					if parentEntry[0] == IDEVSDEVICE_AUDIO_MENU_ENTRY:
						self.__showiDevsAudioMenu( currentiDev )
						
					elif parentEntry[0] == IDEVSDEVICE_AUDIO_PLAYLIST_ENTRY:
						self.__showiDevsAudioPlayLists( currentiDev )
						
					elif parentEntry[0] == IDEVSDEVICE_AUDIO_ARTIST_ENTRY:
						self.__showiDevsAudioArtists( currentiDev )
						
					elif parentEntry[0] == IDEVSDEVICE_AUDIO_ALBUM_ENTRY:
						self.__showiDevsAudioAlbums( currentiDev )
				
					elif parentEntry[0] == IDEVSDEVICE_AUDIO_GENRE_ENTRY:
						self.__showiDevsAudioGenres( currentiDev )
						
					# Videos
					elif parentEntry[0] == IDEVSDEVICE_VIDEO_MENU_ENTRY:
						self.__showiDevsVideoMenu( currentiDev )
					
					elif parentEntry[0] == IDEVSDEVICE_VIDEO_PLAYLIST_ENTRY:
						self.__showiDevsVideoPlayLists( currentiDev )
						
	
	def showAudioMenu(self):
		if self.showModality == AUDIO_MODALITY:
			return
		
		if self.curr_selected is None:
			return
		
		self.showModality = AUDIO_MODALITY
		self.operation = OP_SHOW_AUDIO_MENU
		self.__showiDevsWait(self.curr_selected[1])
		self.TimerOperation.start(100, True)
		
		
	def showVideoMenu(self):
		if self.showModality == VIDEO_MODALITY:
			return
		
		if self.curr_selected is None:
			return
		
		self.showModality = VIDEO_MODALITY
		self.operation = OP_SHOW_VIDEO_MENU
		self.__showiDevsWait(self.curr_selected[1])
		self.TimerOperation.start(100, True)
		

	def getSelection(self):
		if self.l.getCurrentSelection() is None:
			return None
		return self.l.getCurrentSelection()[0]


	def getCurrentEvent(self):
		l = self.l.getCurrentSelection()
		if not l or l[0][0] != IDEVSDEVICE_TRACK_ENTRY:
			return None
		else:
			filename = self.getRealiDevsDevicePath( l[0][1], l[0][3] )
			return self.serviceHandler.info(filename).getEvent(filename)


	def gotoParent(self):
		l = self.l.getCurrentSelection()
		if not l:
			return 
		
		self.curr_selected = l[0]
		
		self.operation = OP_GOTO_PARENT
		self.__showiDevsWait(self.curr_selected[1])
		self.TimerOperation.start(100, True)
		
	
	def changeEntry(self):
		l = self.l.getCurrentSelection()
		if not l:
			return 
		
		if l[0][0] == IDEVSEMPTY_ENTRY:
			return
		
		self.curr_selected = l[0]
		
		self.operation = OP_CHANGE_ENTRY
		self.__showiDevsWait(self.curr_selected[1])
		self.TimerOperation.start(100, True)
			
			
		
	def close(self):
		if idevsdevicemanager.getiDevsDeviceCount() > 0:
			for idev in idevsdevicemanager.iDevsDeviceList():
				idev.close()
			
			
	def isTrack(self):
		curr_entry = self.getSelection()
		if curr_entry is None:
			return False
		
		return (curr_entry[0] == IDEVSDEVICE_AUDIO_TRACK_ENTRY) or (curr_entry[0] == IDEVSDEVICE_VIDEO_TRACK_ENTRY)
			
			
	def getRealiDevsDevicePath(self, idevsdevice, track):
		ipod_path = track.ipod_path
		path_lines = ipod_path.split(":")
		
		if len(path_lines) < 2:
			return None
		
		ipod_path = "%s/%s" % (path_lines[1], path_lines[2])
		for i in range(3,len(path_lines)):
			ipod_path = "%s/%s" % (ipod_path, path_lines[i].lower() )
		
		filename = ("%s/%s") % ( str(idevsdevice.mountpoint), ipod_path )
		
		print "filename: %s" % filename
		
		return filename
		
			
	def getTrackFilename(self):
		if not self.isTrack():
			return None
		
		x =  self.getSelection()
		return self.getRealiDevsDevicePath(x[1], x[3])
	
	
	def getcurrTrack(self):
		if not self.isTrack():
			return None
		
		curr_entry = self.getSelection()
		
		return curr_entry[3]
	
	
	def getTrackMediaTypes(self):
		return getMediaTypes(self.getcurrTrack())
	
	
	def __isSelectedTrack(self, idevsdevice, track):
		pos = -1
		for item in self.selectedtrack:
			pos += 1
			if item[1].mountpoint == idevsdevice.mountpoint and item[3].ipod_path == track.ipod_path:
				return pos
			
		return -1
	
	
	def isTrackSelected(self):
		if not self.isTrack():
			return False
		curr_entry =  self.getSelection()
		
		return (self.__isSelectedTrack(curr_entry[1], curr_entry[3]) >= 0)
	
	
	def __deselectAllTrack(self):
		pos = 0
		for item in self.list:
			if item[0][0] == IDEVSDEVICE_AUDIO_TRACK_ENTRY and item[0][4] == True:
				self.list.pop(pos)
				self.list.insert(pos, iDevsDeviceAudioTrackEntryComponent(item[0][2], item[0][1], item[0][3], False))
			elif item[0][0] == IDEVSDEVICE_VIDEO_TRACK_ENTRY and item[0][4] == True:
				self.list.pop(pos)
				self.list.insert(pos, iDevsDeviceVideoTrackEntryComponent(item[0][2], item[0][1], item[0][3], False))
			pos += 1
			
		self.l.setList(self.list)
		
	
	def deselectTrack(self):
		if not self.isTrack():
			return False
		curr_entry =  self.getSelection()
		
		pos = self.__isSelectedTrack(curr_entry[1], curr_entry[3])
		if pos < 0:
			return False
			
		self.selectedtrack.pop(pos)
		pos = self.getSelectedIndex()
		
		self.list.pop(pos)
		
		if curr_entry[0] == IDEVSDEVICE_AUDIO_TRACK_ENTRY:
			self.list.insert(pos, iDevsDeviceAudioTrackEntryComponent(curr_entry[2], curr_entry[1], curr_entry[3], False))
		elif curr_entry[0] == IDEVSDEVICE_VIDEO_TRACK_ENTRY:
			self.list.insert(pos, iDevsDeviceVideoTrackEntryComponent(curr_entry[2], curr_entry[1], curr_entry[3], False))
			
		self.l.setList(self.list)
		
		self.entryEventCB(curr_entry[0], OP_DESELECT_TRACK_ENTRY)
		
		return True
		
	
	def selectTrack(self):
		if not self.isTrack():
			return False
		curr_entry =  self.getSelection()
		
		pos = self.__isSelectedTrack(curr_entry[1], curr_entry[3])
		if pos >= 0:
			return False
		
		pos = self.getSelectedIndex()
		
		self.list.pop(pos)
		
		if curr_entry[0] == IDEVSDEVICE_AUDIO_TRACK_ENTRY:
			item = iDevsDeviceAudioTrackEntryComponent(curr_entry[2], curr_entry[1], curr_entry[3], True)
		elif curr_entry[0] == IDEVSDEVICE_VIDEO_TRACK_ENTRY:
			item = iDevsDeviceVideoTrackEntryComponent(curr_entry[2], curr_entry[1], curr_entry[3], True)
		
		self.list.insert(pos, item)
		self.selectedtrack.append(item[0])
			
		self.l.setList(self.list)
		
		self.entryEventCB(curr_entry[0], OP_SELECT_TRACK_ENTRY)
		
		return True
		
		
	def clearSelectedTrack(self):
		self.selectedtrack = []
		self.__deselectAllTrack()
		self.entryEventCB(IDEVS_NO_SPECIFIC_ENTRY, OP_DESELECT_ALL_TRACK_ENTRY)
			
	
	def getSelectedTrackNumber(self):
		return len(self.selectedtrack)
			
			
	def gotoFirstSelectedTracks(self):
		self.selectedindex = 0
			
			
	def getNextSelectedTracks(self):
		if self.selectedindex <= (self.getSelectedTrackNumber() - 1):
			self.selectedindex += 1
			return True
		else:
			return False
		
			
	def getPreviousSelectedTracks(self):
		if self.selectedindex > 0 and self.getSelectedTrackNumber() > 0:
			self.selectedindex -= 1
			return True
		else:
			return False
			
			
	def getSelectedServiceRef(self):
		if self.selectedindex <= (self.getSelectedTrackNumber() - 1):
			item = self.selectedtrack[self.selectedindex]
			myreference = eServiceReference("4097:0:0:0:0:0:0:0:0:0:%s" % self.getRealiDevsDevicePath(item[1], item[3]))
			myreference.setName(item[3].title)
			return myreference
		else:
			return None
		
		
	def getcurrSelectedTrack(self):
		if self.selectedindex <= (self.getSelectedTrackNumber() - 1):
			item = self.selectedtrack[self.selectedindex]
			return item[3]
		else:
			return None
		
	def getSelectedTrackMediaTypes(self):
		return getMediaTypes(self.getcurrSelectedTrack())
		
	def getSelectedTrackFilename(self):
		if self.selectedindex <= (self.getSelectedTrackNumber() - 1):
			item = self.selectedtrack[self.selectedindex]
			return self.getRealiDevsDevicePath(item[1], item[3])
		else:
			return None
		
			
			
	def canDescent(self):
		return not self.isTrack()


	def descent(self):
		if self.getSelection() is None:
			return
		self.changeEntry()
		
		
	def getServiceRef(self):
		if self.getSelection() is None:
			return None
		
		track = self.getcurrTrack()
		
		if self.isTrack():
			myreference =  eServiceReference("4097:0:0:0:0:0:0:0:0:0:%s" % self.getTrackFilename())
			myreference.setName(track.title)
			return myreference
		else:
			return None
