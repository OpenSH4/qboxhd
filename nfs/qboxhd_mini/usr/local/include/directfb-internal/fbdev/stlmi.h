/*
   (c) Copyright 2007/2008  STMicroelectronics Ltd.

   based on work by:
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __STLMI_H__
#define __STLMI_H__

#include <linux/stmfb.h>


typedef struct {
     struct {
          unsigned int  part_size;   /* length   */
          unsigned long part_base;   /* physical */
     } info_all[STMFBGP_GFX_LAST - STMFBGP_GFX_FIRST + 1];

     unsigned long total_auxmem;     /* total size */

     /* compat for fbdev to compile without any changes, only first part */
     unsigned int  agp_mem;          /* length   */
     struct {
            unsigned long aper_base; /* physical */
     } info;
} AGPShared;

typedef struct {
     void *base_all[STMFBGP_GFX_LAST - STMFBGP_GFX_FIRST + 1];
     void *base;                     /* virtual of first part, for fbdev to
                                        compile without any changes */
} AGPDevice;


DFBResult dfb_agp_initialize( void );
DFBResult dfb_agp_shutdown( void );

DFBResult dfb_agp_join( void );
DFBResult dfb_agp_leave( void );

#if 0
STMFB_GFXMEMORY_PARTITION stlmi_memory_partition( unsigned long  offset,
                                                  unsigned long *offset_in_part );
#endif


#endif /* __STLMI_H__ */
