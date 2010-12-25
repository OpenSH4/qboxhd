from qboxhd import QBOXHD, QBOXHD_MINI
from fcntl import ioctl
from struct import pack, unpack

def getFPVersion():
	ret = None
	try:
		if QBOXHD_MINI:
			ret = long(open("/proc/stb/lpc/version", "r").read())
		else:
			ret = long(open("/proc/stb/fp/version", "r").read())
	except IOError:
		try:
			fp = open("/dev/dbox/fp0")
			ret = ioctl(fp.fileno(),0)
		except IOError:
			print "getFPVersion failed!"
	return ret

def setFPWakeuptime(wutime):
	try:
		if QBOXHD_MINI:
			open("/proc/stb/lpc/wakeup_time", "w").write(str(wutime))
		else:
			open("/proc/stb/fp/wakeup_time", "w").write(str(wutime))
	except IOError:
		try:
			fp = open("/dev/dbox/fp0")
			ioctl(fp.fileno(), 6, pack('L', wutime)) # set wake up
		except IOError:
			print "setFPWakeupTime failed!"

def setRTCtime(wutime):
	try:
		if QBOXHD_MINI:
			open("/proc/stb/lpc/rtc", "w").write(str(wutime))
		else:
			open("/proc/stb/fp/rtc", "w").write(str(wutime))
	except IOError:
		try:
			fp = open("/dev/dbox/fp0")
			ioctl(fp.fileno(), 0x101, pack('L', wutime)) # set wake up
		except IOError:
			print "setRTCtime failed!"

def getFPWakeuptime():
	ret = 0
	try:
		if QBOXHD_MINI:
			ret = long(open("/proc/stb/lpc/wakeup_time", "r").read())
		else:
			ret = long(open("/proc/stb/fp/wakeup_time", "r").read())
	except IOError:
		try:
			fp = open("/dev/dbox/fp0")
			ret = unpack('L', ioctl(fp.fileno(), 5, '    '))[0] # get wakeuptime
		except IOError:
			print "getFPWakeupTime failed!"
	return ret

def getFPWasTimerWakeup():
	was_wakeup = False
	try:
		if QBOXHD_MINI:
			was_wakeup = int(open("/proc/stb/lpc/was_timer_wakeup", "r").read()) and True or False
		else:
			was_wakeup = int(open("/proc/stb/fp/was_timer_wakeup", "r").read()) and True or False
	except:
		try:
			fp = open("/dev/dbox/fp0")
			was_wakeup = unpack('B', ioctl(fp.fileno(), 9, ' '))[0] and True or False
		except IOError:
			print "wasTimerWakeup failed!"
	return was_wakeup

def clearFPWasTimerWakeup():
	try:
		if QBOXHD_MINI:
			open("/proc/stb/lpc/was_timer_wakeup", "w").write('0')
		else:
			open("/proc/stb/fp/was_timer_wakeup", "w").write('0')
	except:
		try:
			fp = open("/dev/dbox/fp0")
			ioctl(fp.fileno(), 10)
		except IOError:
			print "clearFPWasTimerWakeup failed!"
