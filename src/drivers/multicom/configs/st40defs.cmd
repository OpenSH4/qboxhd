#*******************************************************************************
# st40defs.cmd
#
# This file is used to define the procedures that will allow us to connect to
# the following boards (each is treated as the same mb411 board):
# - mb411
# - mb442
# - STb7100Ref
#
#*******************************************************************************
# Copyright (c) 2007 STMicroelectronics R&D Ltd, All Rights Reserved
#*******************************************************************************


#*******************************************************************************
# Uninhibit STb7100 audio ST231 co-processor
#*******************************************************************************
define stb7100_st231_audio_boot
  set *$SYSCONF_SYS_CFG09 |= 0x10000000
  set *$SYSCONF_SYS_CFG26 = ((*$SYSCONF_SYS_CFG27 & 0x00001ffe) << 19) | 0x00004001
  set *$SYSCONF_SYS_CFG27 |= 0x00000001
  set *$SYSCONF_SYS_CFG27 &= ~0x00000001
end
   
#*******************************************************************************
# Uninhibit STb7100 video ST231 co-processor
#*******************************************************************************
define stb7100_st231_video_boot
  set *$SYSCONF_SYS_CFG09 |= 0x10000000
  set *$SYSCONF_SYS_CFG28 = ((*$SYSCONF_SYS_CFG29 & 0x00001ffe) << 19) | 0x00004001
  set *$SYSCONF_SYS_CFG29 |= 0x00000001
  set *$SYSCONF_SYS_CFG29 &= ~0x00000001
end


#*******************************************************************************
# Connect to STb7100 mb442
#*******************************************************************************
define mb411
    stb7100refstmmx $arg0
    echo Uninhibiting both audio and video co-processor\n
    stb7100_st231_audio_boot
    stb7100_st231_video_boot
end

define mb442
    mb442stb7109stmmx $arg0
    echo Uninhibiting both audio and video co-processor\n
    stb7100_st231_audio_boot
    stb7100_st231_video_boot
end

define mb442se
    mb442stb7109sestmmx $arg0
    echo SE Mode Uninhibiting both audio and video co-processor\n
    stb7100_st231_audio_boot
    stb7100_st231_video_boot
end
