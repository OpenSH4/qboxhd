/*
 * HW dependent actions: STb7100 platform
 *
 * (C) Copyright 2005 STMicroelectronics Limited.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define N_COPROC        2

#define COPR_RAM_START	 	0x4000000	/* ST231 LMI RAM base address */

#define SYSCON_REGS_BASE        P2SEGADDR(0x19001000)
#define SYSCFG_09		(SYSCON_REGS_BASE + 0x124)
#define SYSCFG_BOOT_REG(x)	(SYSCON_REGS_BASE + (x ? 0x168 : 0x170))
#define SYSCFG_RESET_REG(x)	(SYSCON_REGS_BASE + (x ? 0x16c : 0x174))
