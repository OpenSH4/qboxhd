/*
 * Copyright (C) 2007 STMicroelectronics Limited
 * Author: Stuart Menefy <stuart.menefy@st.com>
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/stm/soc.h>
#include <linux/stm/sysconf.h>

#include <asm/io.h>

#define DRIVER_NAME "sysconf"

struct sysconf_field {
	u16 offset;
	u8 lsb, msb;
	char* dev;
	struct list_head list;
};

static void __iomem *sysconf_base;
static int sysconf_offsets[3];
static DEFINE_SPINLOCK(sysconf_lock);
static LIST_HEAD(sysconf_fields);

/* We need a small stash of allocations before kmalloc becomes available */
#define NUM_EARLY_FIELDS 50
static struct sysconf_field early_fields[NUM_EARLY_FIELDS];
static int next_early_field = 0;

static struct sysconf_field* field_alloc(void)
{
	if (next_early_field < NUM_EARLY_FIELDS)
		return &early_fields[next_early_field++];

	return kzalloc(sizeof(struct sysconf_field), GFP_KERNEL);
}

struct sysconf_field* sysconf_claim(int regtype, int regnum, int lsb, int msb,
				    const char *dev)
{
	struct sysconf_field *field = NULL;
	int offset = sysconf_offsets[regtype] + (regnum * 4);


#if 0
	struct sysconf_field *new_field = NULL;

	spin_lock(&sysconf_lock);

	list_for_each(field, sysconf_fields) {
		if (field->offset < offset)
			continue;
		if (field->offset > offset)
			break;
		if (field->lsb > msb)
			continue;
		if (field->msb < lsb)
			break;

	}

	/* Insert before field */
	list_add_tail(new_field, field);
#endif


	field = field_alloc();
	if (!field)
		return NULL;

	field->offset = offset;
	field->lsb = lsb;
	field->msb = msb;

	return field;
}
EXPORT_SYMBOL(sysconf_claim);

void sysconf_release(struct sysconf_field *field)
{

}
EXPORT_SYMBOL(sysconf_release);

void sysconf_write(struct sysconf_field *field, u64 value)
{
	void __iomem *reg;
	int field_bits;	/* Number of bits */

	reg = sysconf_base + field->offset;
	field_bits = field->msb - field->lsb + 1;

	if (field_bits == 32) {
		/* Operating on the whole register, nice and easy */
		writel(value, reg);
	} else {
		u32 reg_mask;
		u32 tmp;

		reg_mask = ~(((1 << field_bits) -1) << field->lsb);
		spin_lock(&sysconf_lock);
		tmp = readl(reg);
		tmp &= reg_mask;
		tmp |= value << field->lsb;
		writel(tmp, reg);
		spin_unlock(&sysconf_lock);
	}
}
EXPORT_SYMBOL(sysconf_write);

u64 sysconf_read(struct sysconf_field *field)
{
	void __iomem *reg;
	int field_bits;	/* Number of bits -1 */
	u32 tmp;

	reg = sysconf_base + field->offset;
	tmp = readl(reg);
	field_bits = field->msb - field->lsb + 1;

	if (field_bits != 32) {
		tmp >>= field->lsb;
		tmp &= (1 << field_bits) -1;
	}

	return (u64)tmp;
}
EXPORT_SYMBOL(sysconf_read);

/* This is called early to allow board start up code to use sysconf
 * registers (in particular console devices). */
void __init sysconf_early_init(struct platform_device* pdev)
{
	int size = pdev->resource[0].end - pdev->resource[0].start + 1;
	struct plat_sysconf_data *data = pdev->dev.platform_data;

#if 1
	sysconf_base = ioremap(pdev->resource[0].start, size);

	/* I don't like panicing here, but it we failed to ioremap, we
	 * probably don't have any way to report things have gone
	 * wrong. So a panic here at least gives some hope of being able to
	 * debug the problem.
	 */
	if (!sysconf_base)
		panic("Unable to ioremap sysconf registers");
#else
	set_fixmap_nocache(FIX_SYSCONF, pdev->resource[0].start);
	sysconf_base = fix_to_virt(FIX_SYSCONF);
#endif

	sysconf_offsets[SYS_DEV] = data->sys_device_offset;
	sysconf_offsets[SYS_STA] = data->sys_sta_offset;
	sysconf_offsets[SYS_CFG] = data->sys_cfg_offset;
}

static int __init sysconf_probe(struct platform_device *pdev)
{
	int size = pdev->resource[0].end - pdev->resource[0].start + 1;

	if (!request_mem_region(pdev->resource[0].start, size, pdev->name))
		return -EBUSY;

	/* Have we already been set up through sysconf_init? */
	if (sysconf_base)
		return 0;

#if 1
	sysconf_early_init(pdev);
#else
	sysconf_base = ioremap(pdev->resource[0].start, size);
	if (!sysconf_base)
		return -ENOMEM;
#endif

	return 0;
}

static struct platform_driver sysconf_driver = {
	.probe		= sysconf_probe,
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init sysconf_init(void)
{
	return platform_driver_register(&sysconf_driver);
}

arch_initcall(sysconf_init);
