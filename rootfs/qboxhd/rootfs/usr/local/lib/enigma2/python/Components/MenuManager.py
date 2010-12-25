from qboxhd import QBOXHD
from Tools.CList import CList

class MenuManager:
	def __init__(self):
		self.on_menu_change = CList()

menumanager = MenuManager()