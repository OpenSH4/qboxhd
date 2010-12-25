/* 
 * Copyright (C) 2004 STMicroelectronics Limited
 * Author: Stuart Menefy <stuart.menefy@st.com>
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 *
 * Definitions applicable to the STMicroelectronics ST220 Eval Board.
 */

#define EPLD_BASE        0xa2000000

#define EPLD_EPLD_VER    (EPLD_BASE+0x00)
#define EPLD_PCB_VER     (EPLD_BASE+0x04)
#define EPLD_LED         (EPLD_BASE+0x0c)
#define EPLD_FLASH_WE    (EPLD_BASE+0x1c)	/* Note not VPP enable */

#define EPLD_LED_ON     0
#define EPLD_LED_OFF    1
