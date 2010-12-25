Dear Users,

Duolabs proudly presents "Stone Distribution Filesystem version 1.0.0 for
QBoxHD".

Duolabs is thankful to all those who belived in us and patiently waited for
this big software release. We have been working hard in the last 6 months to
completely rewrite almost all the software for this box. The implementation of
this new software has been started from the QBox Mini that will be on the
market from next week.

We dedicated most of our time in solving big problems with Player 2 (the main
engine for playing) and finally we obtained a stable version working for
QboxHD.

We apologize to all those who have been disappointed for the leak of features
and stability that were present on QBox Hd for the last year and that's why we
completely addandoned the support of Fire Distibution to develope this
completely new distribution. We preferred not to release any beta version up
to this moment just not to disattend any expectation until we have had the
final approval from the Qbox Mini and then ported all to QboxHD. 

This version is dedicated to Duolabs' fans who stuck with us through this bad
times and to all those who opposed.

Let's talk about the new features now:

PLEASE READ CAREFULLY THE NEW FEATURES AND HOW TO USE IT!!!

Features:

1. Stability Problem. Most of the stability problems have been successfully
solved by using native hardware functions of the STB. In the previous version
most of this functions were handled in software. USB Filesystem handling is
now much more stable thanks to some fixes on USB drivers. Other low lever
drivers have been completely rewritten for more stability. Larger memory for
applications (8MB of RAM) now available.

2. Channel Speed. The Channel speed now is less than 3 seconds per each
channel while it was from 3 to 6 on previous version.

3. HD Channels and SD bug fixed. HD & SD Channels on some transponders were
looping the machine ending a crash if freezes occoured. This problem have been
solved in the most critical HD channels. This is not related to scanning on
some Thor channels (this needs to use the AVLink tuner and not Sharp).
Unfortunately the Sharp tuner has some problems with some transponders and
seems like Sharp cannot fix it releasing a firmware for it. 
AVLink Tuner has no problem unders this point of view.

4. 3x Tuner finally supported. Finally the "Magic Box" inside is dynamically
mixing the Tuner input and output. Now user can finally install any tuner
configuration (2xDVB-S2 + 1x DVB-T or DVB-C etc etc) zapping without any
problem from Tuner 1 to Tuner 2 to Tuner 3 with no problem.
HOW TO INSTALL NEW TUNER:
Tuner 1 port: DVB-T or DVB-C or DVB-S2 Tuner (Sharp or Avlink)
Tuner 2 port: DVB-T or DVB-C or DVB-S2 Tuner (Sharp or Avlink)
Tuner 3 port: DVB-T or DVB-C NOT DVB-S2 Tuner
Once the tuner/s are installed please do a "Factory Reset" - IMPORTANT!!!

5. PVR is now fully supported and stable. It is possible to record from Tuner
x and watch/timeshft on Tuner y. Up to now it is possible to record from one
tuner and watch on another tuner. We will enlarge this possibility next. 
Example. Watch on Tuner 1 (timeshift also) and record from Tuner 2 or Tuner 3. 

6. NAS Server fully working. Now NAS Server is fully supported so it will be
possible to use Network HDD or PC.

7. Internal CSA descrambling. Now the box is using the native descrambler
inside and not the FPGA descrambler. This is much faster and can support third
parties software much easily.

8. Multimedia Center. Multimedia Center completely changed with the following
new features:

 8.1 MKV, AVI, XVID formats now fully supported.
 8.2 MP3 formats now fully supported.
 8.3 Picture Player fully supported - NOT LARGER THAN 2 MB.
 8.4 MyTube (YouTube) fully supported no need for PC. Possibility of recording
YouTube movies directly on the Harddrive
 8.5 IDEVS. This is the support for IPOD and coming next IPhone. This is an
exclusive function that none (AFAIK) is supporting. By connecting your IPOD
via USB you can browse the AUDIO and VIDEO contents on your IPOD and play it
directly on your TV.
 8.6 Internet Browser. Linx internet broser is supported using Mouse and
Keyboard. This is a small and light Internet browser that will be developed
more in the future. Up to now it is supporting simple web pages protocols.
 8.7 Other minor appz.

9. Receiver information screen added. Now you can get all the receiver
information such as connected tuners, IP address, peripherias etc etc from a
dedicated page. All the important informations you have to know about your box
are now shown.
This page is inside Infromation->Receiver Information menu. It is very
important to have a look at this to have a shortcut to all the settings of the
receiver like Tuner Status, IP Address, Keyboard, IPOD connection etc etc.

10. Menu structure is completely changed to make it more user friendly.

11. HD Skin by default with automatic screen resize.

12. HDMI and other video Output problems solved. 

13. Minor bugs are solved and general stability is improved uncomparably with
the previous version especailly if freezes are coming from the streaming.
WebInterface fully working, Teletext fully working etc etc

Known bugs:

1. Minor bugs related to menus (like OSD languages not translated in full) and
other small menu leaks.

2. Motor scan on 4W is stopping. Work in progress.

3. DVB-C scanning is now working just with pre-loaded settings.


Requirements
=============================

- The bootloader update QHU from 2009.09.25
    This update is done through the serial port connected to the PC.
    Further information can be read in the "Boot Update" section of Tutorial 4
    that can be downloaded from http://www.qboxsvn.com/blog/?page_id=81


Instructions
=============================

1.  Copy the compressed file qboxhd_update_ddmmyy_xxx.tbz to a USB flash drive
2.  Optional but strongly recommended: If you want a faster update procedure, 
	uncompress the file inside the USB flash drive. 
	This will create a qboxhd_update directory
3.  Turn off the QBox HD
4.  Insert the USB flash drive that contains the update file
5.  Turn on the QBox HD
6.  The update procedure starts. The LCD will display the update status.
	If you decompressed the file before, this operation takes approximately
	<5 min. If you didn't decompress it, it can take up to 15 min.
7.  When the update procedure finishes, the QBox HD will continue with
	the normal start-up
8.  Remove the USB flash drive


Update contents
=============================

.
|-- ChangeLog.txt		Changes since last version
|-- README.txt			This file
|-- nor					Directory with files for the NOR flash
|   |-- bs				
|   |-- reset_st40
|   |-- uboot
|   |-- uimage
|   `-- uinitramfs
|-- rootfs.cpio			Filesystem image
`-- update.sh			Update script

