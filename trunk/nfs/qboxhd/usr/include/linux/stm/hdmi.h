/***********************************************************************
 *
 * File: stgfb/Linux/stm/coredisplay/stmhdmi.h
 * Copyright (c) 2007 STMicroelectronics Limited.
 * 
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
\***********************************************************************/

#ifndef _STMHDMI_H
#define _STMHDMI_H

struct stmhdmiio_spd
{
  unsigned char vendor_name[8];
  unsigned char product_name[16];
  unsigned char identifier;      /* As specified in CEA861 SPD InfoFrame byte 25 */
};


struct stmhdmiio_audio
{
  unsigned char channel_count;   /* As specified in CEA861 Audio InfoFrame byte 1 (bits 0-2) */
  unsigned char speaker_mapping; /* As specified in CEA861 Audio InfoFrame byte 4            */
  unsigned char downmix_info;    /* As specified in CEA861 Audio InfoFrame byte 5            */
};


struct stmhdmiio_iframe
{
  unsigned char type;
  unsigned char version;
  unsigned char length;
  unsigned char data[28];
};

#define STMHDMIIO_AUDIO_SOURCE_PCM   (0)
#define STMHDMIIO_AUDIO_SOURCE_SPDIF (1)
#define STMHDMIIO_VIDEO_OVERSCAN     (0)
#define STMHDMIIO_VIDEO_UNDERSCAN    (1)

#define STMHDMIIO_SET_SPD_DATA            _IOW ('H', 0x1, struct stmhdmiio_spd)
#define STMHDMIIO_SET_AUDIO_DATA          _IOW ('H', 0x2, struct stmhdmiio_audio)
#define STMHDMIIO_SEND_IFRAME             _IOW ('H', 0x3, struct stmhdmiio_iframe)
#define STMHDMIIO_SET_AVMUTE              _IO  ('H', 0x5)
#define STMHDMIIO_SET_AUDIO_SOURCE        _IO  ('H', 0x6)
#define STMHDMIIO_SET_VIDEO_SCAN_TYPE     _IO  ('H', 0x7)

#endif /* _STMHDMI_H */
