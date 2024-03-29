/* $Id: byteorder.h,v 1.1.1.1 2004/10/27 17:21:34 sturgesa Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) by Ralf Baechle
 */
#ifndef _MIPS_BYTEORDER_H
#define _MIPS_BYTEORDER_H

#include <asm/types.h>

#ifdef __GNUC__

#if !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __BYTEORDER_HAS_U64__
#  define __SWAB_64_THRU_32__
#endif

#endif /* __GNUC__ */

#if defined (__MIPSEB__)
#  include <linux/byteorder/big_endian.h>
#elif defined (__MIPSEL__)
#  include <linux/byteorder/little_endian.h>
#else
#  error "MIPS, but neither __MIPSEB__, nor __MIPSEL__???"
#endif

#endif /* _MIPS_BYTEORDER_H */
