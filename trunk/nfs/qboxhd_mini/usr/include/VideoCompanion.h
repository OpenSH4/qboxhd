/*
 * Copyright (C) STMicroelectronics Limited 2007. All rights reserved.
 *
 * file name:   VideoCompanion.h
 *
 * AUTOMATICALLY GENERATED - DO NOT EDIT
 */

#ifndef VIDEO_COMPANION_H
#define VIDEO_COMPANION_H

typedef int ST_ErrorCode_t ;

#if defined (CONFIG_CPU_SUBTYPE_STB7100)

  #define VIDEO_COMPANION_ONLY 
  #define TRANSFORMER_H264DEC 
  #define DELTA_BASE_ADDRESS	0x19540000 
  #define mb411_video 
  #define HOST_DELTA_COM_MULTICOM 
  #define MAILBOX_BASE_ADDRESS	0x19211000 
  #define H264_MME_VERSION	42 
  #define DELTA_VERSION_DELTAMU_GREEN 
  #define SHARED_MEMORY_ADDRESS	0 
  #define SHARED_MEMORY_SIZE	0 
  #define PLATFORM_RTL 
  #define SOC_NAME_STi7109 
  #define SOC_CUT	30 
  #define TRANSFORMER_MPEG2DEC 
  #define TRANSFORMER_VC1DEC 
  #define MULTICOM 
  #define SUPPORT_DPART 
  #define VC9_MME_VERSION	16

#elif defined(CONFIG_CPU_SUBTYPE_STX7200)

  #define VIDEO_COMPANION_ONLY 
  #define TRANSFORMER_H264DEC 
  #define DELTA_BASE_ADDRESS	0xfd900000 
  #define mb519_video1 
  #define HOST_DELTA_COM_MULTICOM 
  #define MAILBOX_BASE_ADDRESS	0xfd801000 
  #define H264_MME_VERSION	42 
  #define DELTA_VERSION_DELTAMU_GREEN 
  #define SHARED_MEMORY_ADDRESS	0 
  #define SHARED_MEMORY_SIZE	0 
  #define PLATFORM_RTL 
  #define SOC_NAME_STi7200 
  #define SOC_CUT	10 
  #define TRANSFORMER_MPEG2DEC 
  #define TRANSFORMER_VC1DEC 
  #define MULTICOM 
  #define SUPPORT_DPART 
  #define VC9_MME_VERSION	16

#else
  #error Please define either CONFIG_CPU_SUBTYPE_STB7100 or CONFIG_CPU_SUBTYPE_STX7200
#endif

#include <MPEG2_VideoTransformerTypes.h>
#include <H264_VideoTransformerTypes.h>
#include <VC9_VideoTransformerTypes.h>
#include <DivX_decoderTypes.h> 

#endif /* VIDEO_COMPANION_H */
