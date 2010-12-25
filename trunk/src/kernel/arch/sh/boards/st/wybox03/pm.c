/*
 * STb710x Power Manager (Suspend-To-RAM) support
 *
 * (C) Copyright 2006-2008 WyPlay SAS.
 * Pierrick Hascoet <linux-kernel@wyplay.com>
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/apm-emulation.h>
#include <linux/interrupt.h>

/*
 * Needed by the IRB driver...
 */
static int fake_resume;

void stb710x_fake_resume(int fake)
{
	fake_resume = fake;
}
EXPORT_SYMBOL(stb710x_fake_resume);

/*
 * Central control for sleep/resume process
 */
static int stb710x_pm_enter(suspend_state_t state)
{
	pr_debug("%s: entering with state=%d\n", __func__, state);

	/*
	 * All on chip memories are held for all power-down modes
	 */
	enable_wakeup_irqs();
repeat:
	fake_resume = 0;
	local_irq_enable();
	cpu_sleep();	/* cpu_sleep() has a barrier */
	local_irq_disable();
	if (fake_resume)
		goto repeat;

	disable_wakeup_irqs();
	return 0;
}

/*
 * Set to PM_DISK_FIRMWARE so we can quickly veto suspend-to-disk.
 */
static int stb710x_pm_valid(suspend_state_t state)
{
	switch (state) {
	case PM_SUSPEND_ON:
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		return 1;
	default:
		return 0;
	}
}


static struct pm_ops stb710x_pm_ops = {
	.valid		= stb710x_pm_valid,
	.enter		= stb710x_pm_enter,
};

/*
 * Attach the power management functions.
 */
static int __init stb710x_pm_init(void)
{
	pm_set_ops(&stb710x_pm_ops);
	return 0;
}

late_initcall(stb710x_pm_init);
