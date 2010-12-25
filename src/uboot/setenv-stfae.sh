#!/bin/sh
#--------

###########
## Gmake ##
###########

export MAKEFLAGS=--no-print-directory

#############
## Toolset ##
#############

# Set STFAE=1 is mandatory for STFAE full tree compilation 
# To use only the apilib/ without stapp/ and stdebug/, STFAE variable should be set to 0 
export STFAE=1
export STFAEROOT=/home/projects/stb7109/st/STFAE
export ST20ROOT=/opt/STM/ST20R2.2.1
export ST40ROOT=/opt/STM/ST40R3.1.1
export ST200ROOT=/opt/STM/ST200R4.1
# export OSPLUSROOT=/opt/STM/OS+/R2.1.1
export GUIROOT=$STFAEROOT/stgui
export MODULEROOT=$STFAEROOT/modules
if [ -z "$STFAE_PATH_DEFINED" ] ; then
 export PATH="$ST20ROOT:$ST20ROOT/bin:$ST40ROOT:$ST40ROOT/bin:$ST200ROOT:$ST200ROOT/bin:$STFAEROOT/bin:$PATH"
 export STFAE_PATH_DEFINED=1
fi
export DUOPLAYER_INSTALL=/mnt/stb7109/usr/local/duoplayer/michele

#####################
## Clear variables ##
#####################

unset DVD_MAKE
unset DVD_ROOT
unset DVD_EXPORTS
unset DVD_INCLUDE
unset DVD_CONFIG
unset DVD_TRANSPORT
unset DVD_PLATFORM
unset DVD_BACKEND
unset DVD_FRONTEND_TUNER
unset DVD_SERVICE
unset DVD_DISPLAY_SD
unset DVD_DISPLAY_HD
unset MODULES
unset GUI
unset DVR
unset OSPLUS
unset USB
unset TCPIP
unset DEBUG
unset HEAP_WITH_STFAE
unset RUN_FROM_FLASH
unset TARGET
unset ARCHITECTURE
unset STi5100ROOT
unset STi5105ROOT
unset STi5107ROOT
unset STi5301ROOT
unset STi5528ROOT
unset STB7100ROOT
unset STB7109ROOT
unset STi7710ROOT
unset DVD_OS
unset STPTI_CONFIG
unset RPC_SOURCE
unset RPC_ROOT
unset KDIR
unset KTARGET
unset LINUX_TARGETIP
unset LINUX_SERVERIP
unset LINUX_GWIP
unset LINUX_NETMASK
unset LINUX_NAME
unset LINUX_AUTOCONF
unset LINUX_SERVERDIR
unset LINUX_KERNEL
unset LINUX_PARAMETERS
unset LINUX_VERSION
unset LINUX_UNIFIED_STAPI

#######################################################################################
## Export variables coming from the .mak platform to propagate the infos to makefile ##
#######################################################################################

export DVD_STTUNER_USE_SAT=
export DVD_STTUNER_DRV_SAT_STV0299=
export DVD_STTUNER_DRV_SAT_STEM=
export DVD_STTUNER_DRV_SAT_DUAL_STEM_299=
export DVD_STTUNER_DRV_SAT_STV0299=
export DVD_STTUNER_DRV_SAT_LNBH21=
export DVD_STTUNER_USE_SHARED=
export DVD_STTUNER_DRV_SHARED_NULL=
export DVD_STTUNER_DRV_SAT_STX0288=
export DVD_STTUNER_DRV_SAT_STV0299=
export DVD_STTUNER_DRV_SAT_STV0399=
export DVD_STTUNER_DRV_SAT_STB0899=
export DVD_STTUNER_USE_TER=
export DVD_STTUNER_DRV_TER_STV0360=
export DVD_STTUNER_DRV_TER_STV0361=
export DVD_STTUNER_DRV_TER_STV0362=
export DVD_STTUNER_USE_CAB=
export DVD_STTUNER_DRV_CAB_STV0297=
export DVD_STTUNER_DRV_CAB_EXTTUNERS=
export DVD_STTUNER_DRV_BUILD_ALL_TUNER=
export RPC_ROOT=
export STPTI_CONFIG=
export SATA_SUPPORTED=YES
export STCLKRV_EXT_CLKA_MHZ=
export STCLKRV_EXT_CLKB_MHZ=
export STVOUT_HDCP_PROTECTION=


###########################################################
## STB7109 - COCOREF_GOLD/LINUX - Hardware configuration ##
###########################################################

if [ "$1" = "COCOREF_GOLD_7109_LINUX" ] ; then
 # echo COCOREF_GOLD_7109_LINUX Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=cocoref_gold
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 # export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.0.151
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
# export LINUX_VERSION=2.2
 export LINUX_VERSION=2.3

 # export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 # export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 export RPC_SOURCE=/opt/STM/multicom-3.1.1-Fae-Gold
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 #export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux-2.6.17.14_stm22_0040
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux-2.6.17.14_stm22_0039
 # Pointer to linux root installation for stapi
 #export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 export KTARGET=/mnt/stb7109/root/
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.0.130
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.0.50
 # IP address of your network gateway
 export LINUX_GWIP=192.168.0.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.255.0                            
 # Initial hostname for the target
 export LINUX_NAME=stb7109
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS0,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:80:E1:12:1B:A1 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR,timeo=14,retrans=10 \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=2000" 
fi


##############################################
## STi5100 - MB390 - Hardware configuration ##
##############################################

if [ "$1" = "MB390_5100" ] ; then
 echo MB390_5100 Configuration selected !
 export STi5100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5100ROOT/make
 export DVD_ROOT=$STi5100ROOT/src
 export DVD_EXPORTS=$STi5100ROOT/lib
 export DVD_INCLUDE=$STi5100ROOT/include
 export DVD_CONFIG=$STi5100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb390
 export DVD_BACKEND=5100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_299_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

######################################################
## STi5100 - MB395 version - Hardware configuration ##
######################################################

if [ "$1" = "MB395_5100" ] ; then
 echo MB395_5100 Configuration selected !
 export STi5100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5100ROOT/make
 export DVD_ROOT=$STi5100ROOT/src
 export DVD_EXPORTS=$STi5100ROOT/lib
 export DVD_INCLUDE=$STi5100ROOT/include
 export DVD_CONFIG=$STi5100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb395
 export DVD_BACKEND=5100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_360
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

##############################################
## STi5105 - MB400 - Hardware configuration ##
##############################################

if [ "$1" = "MB400_5105" ] ; then
 echo MB400_5105 Configuration selected !
 export STi5105ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5105ROOT/make
 export DVD_ROOT=$STi5105ROOT/src
 export DVD_EXPORTS=$STi5105ROOT/lib
 export DVD_INCLUDE=$STi5105ROOT/include
 export DVD_CONFIG=$STi5105ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb400
 export DVD_BACKEND=5105
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_299_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=0
 export OSPLUS=0
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

################################################
## STi5100 - DTT5100 - Hardware configuration ##
################################################

if [ "$1" = "DTT5100_5100" ] ; then
 echo DTT5100_5100 Configuration selected !
 export STi5100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5100ROOT/make
 export DVD_ROOT=$STi5100ROOT/src
 export DVD_EXPORTS=$STi5100ROOT/lib
 export DVD_INCLUDE=$STi5100ROOT/include
 export DVD_CONFIG=$STi5100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=dtt5100
 export DVD_BACKEND=5100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_360,NIM_360
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

################################################
## STi5105 - DTT5105 - Hardware configuration ##
################################################

if [ "$1" = "DTT5105_5105" ] ; then
 echo DTT5105_5105 Configuration selected !
 export STi5105ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5105ROOT/make
 export DVD_ROOT=$STi5105ROOT/src
 export DVD_EXPORTS=$STi5105ROOT/lib
 export DVD_INCLUDE=$STi5105ROOT/include
 export DVD_CONFIG=$STi5105ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=dtt5105
 export DVD_BACKEND=5105
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_360
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=0
 export OSPLUS=0
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

################################################
## STi5105 - DTT5118 - Hardware configuration ##
################################################

if [ "$1" = "DTT5118_5105" ] ; then
 echo DTT5118_5105 Configuration selected !
 export STi5105ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5105ROOT/make
 export DVD_ROOT=$STi5105ROOT/src
 export DVD_EXPORTS=$STi5105ROOT/lib
 export DVD_INCLUDE=$STi5105ROOT/include
 export DVD_CONFIG=$STi5105ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=dtt5118
 export DVD_BACKEND=5105
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_360
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=0
 export OSPLUS=0
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

#################################################
## STi5105 - SAT5107 - Hardware configuration  ##
#################################################

if [ "$1" = "SAT5107_5105" ] ; then
 echo SAT5107_5105 Configuration selected !
 export STi5105ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5105ROOT/make
 export DVD_ROOT=$STi5105ROOT/src
 export DVD_EXPORTS=$STi5105ROOT/lib
 export DVD_INCLUDE=$STi5105ROOT/include
 export DVD_CONFIG=$STi5105ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=sat5107
 export DVD_BACKEND=5105
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_288_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=0
 export OSPLUS=0
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

#################################################
## STi5107 - SAT5107 - Hardware configuration  ##
#################################################

if [ "$1" = "SAT5107_5107" ] ; then
 echo SAT5107_5107 Configuration selected !
 export STi5107ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5107ROOT/make
 export DVD_ROOT=$STi5107ROOT/src
 export DVD_EXPORTS=$STi5107ROOT/lib
 export DVD_INCLUDE=$STi5107ROOT/include
 export DVD_CONFIG=$STi5107ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=sat5107
 export DVD_BACKEND=5107
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_288_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=0
 export OSPLUS=0
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

##############################################
## STi5301 - MB390 - Hardware configuration ##
##############################################

if [ "$1" = "MB390_5301" ] ; then
 echo MB390_5301 Configuration selected !
 export STi5301ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5301ROOT/make
 export DVD_ROOT=$STi5301ROOT/src
 export DVD_EXPORTS=$STi5301ROOT/lib
 export DVD_INCLUDE=$STi5301ROOT/include
 export DVD_CONFIG=$STi5301ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb390
 export DVD_BACKEND=5301
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_299_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST200
fi

######################################################
## STi5528/ST20 - Espresso - Hardware configuration ##
######################################################
# For ST20 side : Please check that Jumper J11 is ON  [1-2]
#                 Please check that Jumper J12 is OFF [2-3]

if [ "$1" = "ESPRESSO_5528_ST20" ] ; then
 echo ESPRESSO_5528_ST20 Configuration selected !
 export STi5528ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5528ROOT/make
 export DVD_ROOT=$STi5528ROOT/src
 export DVD_EXPORTS=$STi5528ROOT/lib
 export DVD_INCLUDE=$STi5528ROOT/include
 export DVD_CONFIG=$STi5528ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=espresso
 export DVD_BACKEND=5528
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_399
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

######################################################
## STi5528/ST40 - Espresso - Hardware configuration ##
######################################################
# For ST40 side : Please check that Jumper J11 is OFF [2-3]
#                 Please check that Jumper J12 is ON  [1-2]

if [ "$1" = "ESPRESSO_5528_ST40" ] ; then
 echo ESPRESSO_5528_ST40 Configuration selected !
 export STi5528ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5528ROOT/make
 export DVD_ROOT=$STi5528ROOT/src
 export DVD_EXPORTS=$STi5528ROOT/lib
 export DVD_INCLUDE=$STi5528ROOT/include
 export DVD_CONFIG=$STi5528ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=espresso
 export DVD_BACKEND=5528
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_399
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

##############################################
## STB7100 - MB411 - Hardware configuration ##
##############################################

if [ "$1" = "MB411_7100" ] ; then
 echo MB411_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb411
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

####################################################
## STB7100 - MB411/LINUX - Hardware configuration ##
####################################################

if [ "$1" = "MB411_7100_LINUX" ] ; then
 echo MB411_7100_LINUX Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb411
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                            
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                            
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS0,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=1500" 
fi

##############################################
## STB7109 - MB411 - Hardware configuration ##
##############################################

if [ "$1" = "MB411_7109" ] ; then
 echo MB411_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb411
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

####################################################
## STB7109 - MB411/LINUX - Hardware configuration ##
####################################################

if [ "$1" = "MB411_7109_LINUX" ] ; then
 echo MB411_7109_LINUX Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb411
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101 
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                              
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                              
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS0,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=1500" 
fi

##############################################
## STB7200 - MB519 - Hardware configuration ##
##############################################

if [ "$1" = "MB519_7200" ] ; then
 echo MB519_7200 Configuration selected !
 export STB7200ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7200ROOT/make
 export DVD_ROOT=$STB7200ROOT/src
 export DVD_EXPORTS=$STB7200ROOT/lib
 export DVD_INCLUDE=$STB7200ROOT/include
 export DVD_CONFIG=$STB7200ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb519
 export DVD_BACKEND=7200
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_NTSC
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I60HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

##############################################
## STB7109 - MB602 - Hardware configuration ##
##############################################

if [ "$1" = "MB602_7109" ] ; then
 echo MB602_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb602
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi
 
########################################################
## STB7109 - FUTARQUE_HYBRID - Hardware configuration ##
########################################################

if [ "$1" = "FUTARQUE_HYBRID_7109" ] ; then
 echo FUTARQUE_HYBRID_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=futarque_hybrid
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

##############################################################
## STB7109 - FUTARQUE_HYBRID/LINUX - Hardware configuration ##
##############################################################

if [ "$1" = "FUTARQUE_HYBRID_7109_LINUX" ] ; then
 echo FUTARQUE_HYBRID_7109_LINUX Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=futarque_hybrid
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101 
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                              
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                              
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS0,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=1500" 
fi

################################################
## STB7100 - COCOREF - Hardware configuration ##
################################################

if [ "$1" = "COCOREF_7100" ] ; then
 echo COCOREF_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=cocoref
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_360
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

###################################################
## STB7100 - COCOREF V2 - Hardware configuration ##
###################################################

if [ "$1" = "COCOREF_V2_7100" ] ; then
 echo COCOREF_V2_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=cocoref_v2
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_360
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#########################################################
## STB7100 - COCOREF V2/LINUX - Hardware configuration ##
#########################################################

if [ "$1" = "COCOREF_V2_7100_LINUX" ] ; then
 echo COCOREF_V2_7100_LINUX Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=cocoref_v2
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_299_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                            
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                            
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS0,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=1500" 
fi

###################################################
## STB7109 - COCOREF V2 - Hardware configuration ##
###################################################

if [ "$1" = "COCOREF_V2_7109" ] ; then
 echo COCOREF_V2_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=cocoref_v2
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#########################################################
## STB7109 - COCOREF_V2/LINUX - Hardware configuration ##
#########################################################

if [ "$1" = "COCOREF_V2_7109_LINUX" ] ; then
 echo COCOREF_V2_7109_LINUX Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=cocoref_v2
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                            
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                            
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS0,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=1500" 
fi

#####################################################
## STB7100 - COCOREF GOLD - Hardware configuration ##
#####################################################

if [ "$1" = "COCOREF_GOLD_7100" ] ; then
 echo COCOREF_GOLD_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=cocoref_gold
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_360
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

###########################################################
## STB7100 - COCOREF GOLD/LINUX - Hardware configuration ##
###########################################################

if [ "$1" = "COCOREF_GOLD_7100_LINUX" ] ; then
 echo COCOREF_GOLD_7100_LINUX Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=cocoref_gold
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_299_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                            
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                            
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS0,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=2000" 
fi

#####################################################
## STB7109 - COCOREF GOLD - Hardware configuration ##
#####################################################

if [ "$1" = "COCOREF_GOLD_7109" ] ; then
 echo COCOREF_GOLD_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=cocoref_gold
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi


############################################
## STB7100 - HMP - Hardware configuration ##
############################################

if [ "$1" = "HMP_7100" ] ; then
 echo HMP_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=hmp
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

############################################
## STB7109 - HMP - Hardware configuration ##
############################################

if [ "$1" = "HMP_7109" ] ; then
 echo HMP_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=hmp
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

################################################
## STB7109 - DSG2500 - Hardware configuration ##
################################################

if [ "$1" = "DSG2500_7109" ] ; then
 echo DSG2500_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=dsg2500
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_TS
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

######################################################
## STB7109 - DSG2500/LINUX - Hardware configuration ##
######################################################

if [ "$1" = "DSG2500_7109_LINUX" ] ; then
 echo DSG2500_7109_LINUX Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=dsg2500
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_TS
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101 
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                              
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                              
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS0,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=1500" 
fi

##############################################
## STi7710 - MB391 - Hardware configuration ##
##############################################

if [ "$1" = "MB391_7710" ] ; then
 echo MB391_7710 Configuration selected !
 export STi7710ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi7710ROOT/make
 export DVD_ROOT=$STi7710ROOT/src
 export DVD_EXPORTS=$STi7710ROOT/lib
 export DVD_INCLUDE=$STi7710ROOT/include
 export DVD_CONFIG=$STi7710ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=mb391
 export DVD_BACKEND=7710
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_299_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=0
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

################################################
## STi7710 - DTR7710 - Hardware configuration ##
################################################

if [ "$1" = "DTR7710_7710" ] ; then
 echo DTR7710_7710 Configuration selected !
 export STi7710ROOT=$STFAEROOT/apilib 
 export DVD_MAKE=$STi7710ROOT/make
 export DVD_ROOT=$STi7710ROOT/src
 export DVD_EXPORTS=$STi7710ROOT/lib
 export DVD_INCLUDE=$STi7710ROOT/include
 export DVD_CONFIG=$STi7710ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=dtr7710
 export DVD_BACKEND=7710
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_362,NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

################################################
## STi7710 - SDK7710 - Hardware configuration ##
################################################

if [ "$1" = "SDK7710_7710" ] ; then
 echo SDK7710_7710 Configuration selected !
 export STi7710ROOT=$STFAEROOT/apilib 
 export DVD_MAKE=$STi7710ROOT/make
 export DVD_ROOT=$STi7710ROOT/src
 export DVD_EXPORTS=$STi7710ROOT/lib
 export DVD_INCLUDE=$STi7710ROOT/include
 export DVD_CONFIG=$STi7710ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=sdk7710
 export DVD_BACKEND=7710
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_299_6000,NIM_TS
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

#####################################################
## STB7100 - CUSTOM001001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001001_7100" ] ; then
 echo CUSTOM001001_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001001
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STi5100 - CUSTOM001002 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001002_5100" ] ; then
 echo CUSTOM001002_5100 Configuration selected !
 export STi5100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5100ROOT/make
 export DVD_ROOT=$STi5100ROOT/src
 export DVD_EXPORTS=$STi5100ROOT/lib
 export DVD_INCLUDE=$STi5100ROOT/include
 export DVD_CONFIG=$STi5100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001002
 export DVD_BACKEND=5100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

#####################################################
## STi5301 - CUSTOM001002 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001002_5301" ] ; then
 echo CUSTOM001002_5301 Configuration selected !
 export STi5301ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5301ROOT/make
 export DVD_ROOT=$STi5301ROOT/src
 export DVD_EXPORTS=$STi5301ROOT/lib
 export DVD_INCLUDE=$STi5301ROOT/include
 export DVD_CONFIG=$STi5301ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001002
 export DVD_BACKEND=5301
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST200
fi

#####################################################
## STB7100 - CUSTOM001003 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001003_7100" ] ; then
 echo CUSTOM001003_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001003
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001003 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001003_7109" ] ; then
 echo CUSTOM001003_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001003
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM001004 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001004_7100" ] ; then
 echo CUSTOM001004_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001004
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

##########################################################
## STi5528/ST20 - CUSTOM001005 - Hardware configuration ##
##########################################################
# For ST20 side : Please check that Jumper J11 is ON  [1-2]
#                 Please check that Jumper J12 is OFF [2-3]

if [ "$1" = "CUSTOM001005_5528_ST20" ] ; then
 echo CUSTOM001005_5528_ST20 Configuration selected !
 export STi5528ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5528ROOT/make
 export DVD_ROOT=$STi5528ROOT/src
 export DVD_EXPORTS=$STi5528ROOT/lib
 export DVD_INCLUDE=$STi5528ROOT/include
 export DVD_CONFIG=$STi5528ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001005
 export DVD_BACKEND=5528
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

##########################################################
## STi5528/ST40 - CUSTOM001005 - Hardware configuration ##
##########################################################
# For ST40 side : Please check that Jumper J11 is OFF [2-3]
#                 Please check that Jumper J12 is ON  [1-2]

if [ "$1" = "CUSTOM001005_5528_ST40" ] ; then
 echo CUSTOM001005_5528_ST40 Configuration selected !
 export STi5528ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5528ROOT/make
 export DVD_ROOT=$STi5528ROOT/src
 export DVD_EXPORTS=$STi5528ROOT/lib
 export DVD_INCLUDE=$STi5528ROOT/include
 export DVD_CONFIG=$STi5528ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001005
 export DVD_BACKEND=5528
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=0
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM001006 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001006_7100" ] ; then
 echo CUSTOM001006_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001006
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM001006 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001006_7100_LINUX" ] ; then
 echo CUSTOM001006_7100_LINUX Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001006
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                            
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                            
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS0,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=1500" 
fi

#####################################################
## STi5100 - CUSTOM001007 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001007_5100" ] ; then
 echo CUSTOM001007_5100 Configuration selected !
 export STi5100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5100ROOT/make
 export DVD_ROOT=$STi5100ROOT/src
 export DVD_EXPORTS=$STi5100ROOT/lib
 export DVD_INCLUDE=$STi5100ROOT/include
 export DVD_CONFIG=$STi5100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001007
 export DVD_BACKEND=5100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM,NIM_361,NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

#####################################################
## STi5100 - CUSTOM001008 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001008_5100" ] ; then
 echo CUSTOM001008_5100 Configuration selected !
 export STi5100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5100ROOT/make
 export DVD_ROOT=$STi5100ROOT/src
 export DVD_EXPORTS=$STi5100ROOT/lib
 export DVD_INCLUDE=$STi5100ROOT/include
 export DVD_CONFIG=$STi5100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001008
 export DVD_BACKEND=5100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_297
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

#####################################################
## STi5301 - CUSTOM001008 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001008_5301" ] ; then
 echo CUSTOM001008_5301 Configuration selected !
 export STi5301ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5301ROOT/make
 export DVD_ROOT=$STi5301ROOT/src
 export DVD_EXPORTS=$STi5301ROOT/lib
 export DVD_INCLUDE=$STi5301ROOT/include
 export DVD_CONFIG=$STi5301ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001008
 export DVD_BACKEND=5301
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_297
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST200
fi

#####################################################
## STB7109 - CUSTOM001009 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001009_7109" ] ; then
 echo CUSTOM001009_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001009
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_288_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM001010 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001010_7100" ] ; then
 echo CUSTOM001010_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001010
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM001011 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001011_7100" ] ; then
 echo CUSTOM001011_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001011
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

###########################################################
## STB7100 - CUSTOM001011/LINUX - Hardware configuration ##
###########################################################

if [ "$1" = "CUSTOM001011_7100_LINUX" ] ; then
 echo CUSTOM001011_7100_LINUX Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001011
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                            
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                            
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS1,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=2000" 
fi

#####################################################
## STB7109 - CUSTOM001011 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001011_7109" ] ; then
 echo CUSTOM001011_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001011
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001012 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001012_7109" ] ; then
 echo CUSTOM001012_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001012
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM,CUSTOM,NIM_362,NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

################################################################
## STB7109 - CUSTOM001012_7109/LINUX - Hardware configuration ##
################################################################

if [ "$1" = "CUSTOM001012_7109_LINUX" ] ; then
 echo CUSTOM001012_7109_LINUX Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001012
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM,CUSTOM,NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                            
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                            
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS1,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=2000" 
fi

#####################################################
## STB7109 - CUSTOM001013 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001013_7109" ] ; then
 echo CUSTOM001013_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001013
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM,CUSTOM
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001014 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001014_7109" ] ; then
 echo CUSTOM001014_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001014
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM,CUSTOM,NIM_362,NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001015 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001015_7109" ] ; then
 echo CUSTOM001015_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001015
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001016 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001016_7109" ] ; then
 echo CUSTOM001016_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001016
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_288_6000,NIM_288_6000,NIM_288_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM001017 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001017_7100" ] ; then
 echo CUSTOM001017_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001017
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NONE
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001017 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001017_7109" ] ; then
 echo CUSTOM001017_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001017
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NONE
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001018 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001018_7109" ] ; then
 echo CUSTOM001018_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001018
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_362,NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

################################################################
## STB7109 - CUSTOM001018_7109/LINUX - Hardware configuration ##
################################################################

if [ "$1" = "CUSTOM001018_7109_LINUX" ] ; then
 echo CUSTOM001018_7109_LINUX Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001018
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_362,NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                            
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                            
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS1,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=2000" 
fi

#####################################################
## STB7109 - CUSTOM001019 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001019_7109" ] ; then
 echo CUSTOM001019_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001019
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001020 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001020_7109" ] ; then
 echo CUSTOM001020_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001020
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_288_6000,NIM_288_6000,NIM_288_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM001021 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001021_7100" ] ; then
 echo CUSTOM001021_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001021
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi
 
###########################################################
## STB7100 - CUSTOM001021/LINUX - Hardware configuration ##
###########################################################

if [ "$1" = "CUSTOM001021_7100_LINUX" ] ; then
 echo CUSTOM001021_7100_LINUX Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001021
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_361
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=0
 export USB=0
 export TCPIP=1
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=192.168.1.101
 export ARCHITECTURE=ST40
 export DVD_OS=LINUX
 export LINUX_VERSION=2.2
 export LINUX_UNIFIED_STAPI=1
 # Add path to multicom source in order to force recompil
 export RPC_SOURCE=/opt/STM/STLinux-$LINUX_VERSION/multicom-3.1.1
 # Add cross compiler to path
 export PATH=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/bin:$PATH
 # Add man doc to man path
 export MANPATH=/opt/STM/STLinux-$LINUX_VERSION/host/man:$MANPATH
 # Pointer to linux kernel root
 export KDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sources/kernel/linux
 # Pointer to linux root installation for stapi
 export KTARGET=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target/root
 # IP address to be given to the target
 export LINUX_TARGETIP=192.168.1.50                            
 # IP address of your NFS server
 export LINUX_SERVERIP=192.168.1.51                            
 # IP address of your network gateway
 export LINUX_GWIP=192.168.1.254                               
 # Local network subnet mask
 export LINUX_NETMASK=255.255.0.0                            
 # Initial hostname for the target
 export LINUX_NAME=marco                                       
 # Try to determine addresses automatically? 
 export LINUX_AUTOCONF=off                                     
 # Root of target's file system 
 export LINUX_SERVERDIR=/opt/STM/STLinux-$LINUX_VERSION/devkit/sh4/target 
 # Kernel image
 export LINUX_KERNEL=$KDIR/vmlinux                             
 # Kernel parameters
 export LINUX_PARAMETERS="console=ttyAS1,115200 \
                          root=/dev/nfs \
                          nwhwconf=device:eth0,hwaddr:00:FA:E0:FA:E0:00 \
                          nfsroot=$LINUX_SERVERIP:$LINUX_SERVERDIR \
                          ip=$LINUX_TARGETIP::$LINUX_GWIP:$LINUX_NETMASK:$LINUX_NAME::$LINUX_AUTOCONF \
                          bigphysarea=2000" 
fi

#####################################################
## STB7100 - CUSTOM001022 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001022_7100" ] ; then
 echo CUSTOM001022_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001022
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001022 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001022_7109" ] ; then
 echo CUSTOM001022_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001022
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM,NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM001023 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001023_7109" ] ; then
 echo CUSTOM001023_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001023
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NONE
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STi5100 - CUSTOM001024 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM001024_5100" ] ; then
 echo CUSTOM001024_5100 Configuration selected !
 export STi5100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5100ROOT/make
 export DVD_ROOT=$STi5100ROOT/src
 export DVD_EXPORTS=$STi5100ROOT/lib
 export DVD_INCLUDE=$STi5100ROOT/include
 export DVD_CONFIG=$STi5100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom001024
 export DVD_BACKEND=5100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=CUSTOM
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

#####################################################
## STB7100 - CUSTOM002001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM002001_7100" ] ; then
 echo CUSTOM002001_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom002001
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM002002 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM002002_7109" ] ; then
 echo CUSTOM002002_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom002002
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM002003 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM002003_7109" ] ; then
 echo CUSTOM002003_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom002003
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STi5100 - CUSTOM003001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM003001_5100" ] ; then
 echo CUSTOM003001_5100 Configuration selected !
 export STi5100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STi5100ROOT/make
 export DVD_ROOT=$STi5100ROOT/src
 export DVD_EXPORTS=$STi5100ROOT/lib
 export DVD_INCLUDE=$STi5100ROOT/include
 export DVD_CONFIG=$STi5100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom003001
 export DVD_BACKEND=5100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NONE
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=1
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST20
fi

#####################################################
## STB7100 - CUSTOM004001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM004001_7100" ] ; then
 echo CUSTOM004001_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom004001
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]  
 export DVD_FRONTEND_TUNER=NIM_299_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_NTSC
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I60HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM004001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM004001_7109" ] ; then
 echo CUSTOM004001_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom004001
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]  
 export DVD_FRONTEND_TUNER=NIM_299_6000
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_NTSC
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I60HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM004002 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM004002_7109" ] ; then
 echo CUSTOM004002_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom004002
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]  
 export DVD_FRONTEND_TUNER=NIM_297_MT2060
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I60HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=0
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM005001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM005001_7109" ] ; then
 echo CUSTOM005001_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom005001
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=1
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM006001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM006001_7100" ] ; then
 echo CUSTOM006001_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom006001
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=NONE
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM007001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM007001_7100" ] ; then
 echo CUSTOM007001_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom007001
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_899_6100
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7100 - CUSTOM009001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM009001_7100" ] ; then
 echo CUSTOM009001_7100 Configuration selected !
 export STB7100ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7100ROOT/make
 export DVD_ROOT=$STB7100ROOT/src
 export DVD_EXPORTS=$STB7100ROOT/lib
 export DVD_INCLUDE=$STB7100ROOT/include
 export DVD_CONFIG=$STB7100ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom009001
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NONE
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

#####################################################
## STB7109 - CUSTOM009001 - Hardware configuration ##
#####################################################

if [ "$1" = "CUSTOM009001_7109" ] ; then
 echo CUSTOM009001_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom009001
 export DVD_BACKEND=7100
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NONE
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

##########################################################
## STB7109 - CUSTOM010001_7109 - Hardware configuration ##
##########################################################

if [ "$1" = "CUSTOM010001_7109" ] ; then
 echo CUSTOM010001_7109 Configuration selected !
 export STB7109ROOT=$STFAEROOT/apilib
 export DVD_MAKE=$STB7109ROOT/make
 export DVD_ROOT=$STB7109ROOT/src
 export DVD_EXPORTS=$STB7109ROOT/lib
 export DVD_INCLUDE=$STB7109ROOT/include
 export DVD_CONFIG=$STB7109ROOT/config
 export DVD_TRANSPORT=stpti4
 export DVD_PLATFORM=custom010001
 export DVD_BACKEND=7109
 # DVD_FRONTEND_TUNER should be [STEM_299,NIM_299,NIM_299_6000,NIM_288_6000,NIM_899_6100,NIM_399,NIM_360,NIM_361,NIM_362,NIM_297,NIM_TS,CUSTOM]
 export DVD_FRONTEND_TUNER=NIM_362,NIM_362
 export DVD_SERVICE=DVB
 # DVD_DISPLAY_SD should be [D_PAL,D_NTSC]
 export DVD_DISPLAY_SD=D_PAL
 # DVD_DISPLAY_HD should be [NONE,D_1080I50HZ,D_720P50HZ,D_576P50HZ,D_1080I60HZ,D_720P60HZ,D_480P60HZ]
 export DVD_DISPLAY_HD=D_1080I50HZ
 export MODULES=0
 export GUI=0
 export DVR=1
 export OSPLUS=1
 export USB=1
 export TCPIP=0
 export DEBUG=1
 export HEAP_WITH_STFAE=0
 export RUN_FROM_FLASH=0
 export TARGET=usb
 export ARCHITECTURE=ST40
fi

####################
## Menu selection ##
####################

if [ -z "$DVD_PLATFORM" ] ; then
 echo Config list available
 echo ---------------------
 echo
 echo MB390_5100
 echo MB390_5301
 echo MB391_7710
 echo MB395_5100
 echo MB400_5105
 echo MB411_7100
 echo MB411_7100_LINUX
 echo MB411_7109
 echo MB411_7109_LINUX
 echo MB519_7200
 echo MB602_7109
 echo COCOREF_7100
 echo COCOREF_V2_7100
 echo COCOREF_V2_7100_LINUX
 echo COCOREF_V2_7109
 echo COCOREF_V2_7109_LINUX
 echo COCOREF_GOLD_7100
 echo COCOREF_GOLD_7100_LINUX
 echo COCOREF_GOLD_7109
 echo COCOREF_GOLD_7109_LINUX
 echo DTT5100_5100
 echo DTT5105_5105
 echo DTT5118_5105
 echo FUTARQUE_HYBRID_7109
 echo FUTARQUE_HYBRID_7109_LINUX
 echo SAT5107_5105
 echo SAT5107_5107
 echo ESPRESSO_5528_ST20
 echo ESPRESSO_5528_ST40
 echo HMP_7100
 echo HMP_7109
 echo DSG2500_7109
 echo DSG2500_7109_LINUX
 echo DTR7710_7710
 echo SDK7710_7710
 echo CUSTOM001001_7100
 echo CUSTOM001002_5100
 echo CUSTOM001002_5301
 echo CUSTOM001003_7100
 echo CUSTOM001003_7109
 echo CUSTOM001004_7100
 echo CUSTOM001005_5528_ST20
 echo CUSTOM001005_5528_ST40
 echo CUSTOM001006_7100
 echo CUSTOM001006_7100_LINUX
 echo CUSTOM001007_5100
 echo CUSTOM001008_5100
 echo CUSTOM001008_5301
 echo CUSTOM001009_7109
 echo CUSTOM001010_7100
 echo CUSTOM001011_7100
 echo CUSTOM001011_7100_LINUX
 echo CUSTOM001011_7109
 echo CUSTOM001012_7109
 echo CUSTOM001012_7109_LINUX
 echo CUSTOM001013_7109
 echo CUSTOM001014_7109
 echo CUSTOM001015_7109
 echo CUSTOM001016_7109
 echo CUSTOM001017_7100
 echo CUSTOM001017_7109
 echo CUSTOM001018_7109
 echo CUSTOM001018_7109_LINUX
 echo CUSTOM001019_7109
 echo CUSTOM001020_7109
 echo CUSTOM001021_7100
 echo CUSTOM001021_7100_LINUX
 echo CUSTOM001022_7100
 echo CUSTOM001022_7109
 echo CUSTOM001023_7109
 echo CUSTOM001024_5100
 echo CUSTOM002001_7100
 echo CUSTOM002002_7109
 echo CUSTOM002003_7109
 echo CUSTOM003001_5100
 echo CUSTOM004001_7100
 echo CUSTOM004001_7109
 echo CUSTOM004002_7109
 echo CUSTOM005001_7109
 echo CUSTOM006001_7100
 echo CUSTOM007001_7100
 echo CUSTOM009001_7100
 echo CUSTOM009001_7109
 echo CUSTOM010001_7109
fi




  
