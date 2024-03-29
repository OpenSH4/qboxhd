/*
 * Copyright (C) 2008 STMicroelectronics Limited
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 *
 * Code to handle the clockgen hardware on the STx5197.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/stm/sysconf.h>
#include <linux/io.h>
#include <asm/clock.h>
#include <asm/freq.h>

/* Values for mb704 */
#define XTAL	30000000

#define SYS_SERV_BASE_ADDR	0xfdc00000

#define PLL_CONFIG0(x)		((x*8)+0x0)
#define PLL_CONFIG1(x)		((x*8)+0x4)
#define PLL_CONFIG1_POFF	(1<<13)
#define CLKDIV0_CONFIG0		0x90
#define CLKDIV1_4_CONFIG0(n)	(0x0a0 + ((n-1)*0xc))
#define CLKDIV6_10_CONFIG0(n)	(0x0d0 + ((n-6)*0xc))
#define PLL_SELECT_CFG		0x180
static void __iomem *ss_base;

/* External XTAL ----------------------------------------------------------- */

static void xtal_init(struct clk *clk)
{
	clk->rate = XTAL;
}

static struct clk_ops xtal_ops = {
	.init		= xtal_init,
};

static struct clk xtal_osc = {
	.name		= "xtal",
	.flags		= CLK_ALWAYS_ENABLED | CLK_RATE_PROPAGATES,
	.ops		= &xtal_ops,
};

/* PLLs -------------------------------------------------------------------- */

static unsigned long pll_freq(unsigned long input, int pll_num)
{
	unsigned long config0, config1;
	unsigned long freq, ndiv, pdiv, mdiv;

	config0 = readl(ss_base + PLL_CONFIG0(pll_num));
	config1 = readl(ss_base + PLL_CONFIG1(pll_num));

	if (config1 & PLL_CONFIG1_POFF)
		return 0;

	mdiv = (config0 >> 0) & 0xff;
	ndiv = (config0 >> 8) & 0xff;
	pdiv = (config1 >> 0) & 0x7;

	freq = (((2 * (input / 1000) * ndiv) / mdiv) /
		(1 << pdiv)) * 1000;

	return freq;
}

struct pllclk
{
	struct clk clk;
	unsigned long pll_num;
};

static void pll_clk_recalc(struct clk *clk)
{
	struct pllclk *pllclk = container_of(clk, struct pllclk, clk);

	clk->rate = pll_freq(clk->parent->rate, pllclk->pll_num);
}

static struct clk_ops pll_clk_ops = {
	.recalc		= pll_clk_recalc,
};

static struct pllclk pllclks[2] = {
{
	.clk = {
		.name		= "PLLA",
		.parent		= &xtal_osc,
		.flags		= CLK_ALWAYS_ENABLED | CLK_RATE_PROPAGATES,
		.ops		= &pll_clk_ops,
	},
	.pll_num = 0
}, {
	.clk = {
		.name		= "PLLB",
		.parent		= &xtal_osc,
		.flags		= CLK_ALWAYS_ENABLED | CLK_RATE_PROPAGATES,
		.ops		= &pll_clk_ops,
	},
	.pll_num = 1
} };

/* Divided PLL clocks ------------------------------------------------------ */

/*
 * The divider is implemented as a variable length shift register
 * preloaded with a bit sequence, which is clocked by the input clock,
 * plus some additional combinatorial logic. Rather than try and work
 * out what this represents, we simply use a look up table with the
 * recommended values.
 *
 * Table bits:
 * 25     half_not_odd
 * 24     even
 * 23:20  depth[3:0]
 * 19: 0 clkdiv_seq[19:0]
 */

#define FRAC(whole, half) (((whole)*2) + (half ? 1 : 0))
#define COMBINE_DIVIDER(depth, seq, hno, even) \
	((hno << 25) | (even << 24) | (depth << 20) | (seq << 0))

static const struct {
	unsigned long ratio2, value;
} divide_table[] = {
	{ FRAC(2 , 0), COMBINE_DIVIDER(0x01, 0x00AAA, 0x1, 0x1) },
	{ FRAC(2 , 5), COMBINE_DIVIDER(0x04, 0x05AD6, 0x1, 0x0) },
	{ FRAC(3 , 0), COMBINE_DIVIDER(0x01, 0x00DB6, 0x0, 0x0) },
	{ FRAC(3 , 5), COMBINE_DIVIDER(0x03, 0x0366C, 0x1, 0x0) },
	{ FRAC(4 , 0), COMBINE_DIVIDER(0x05, 0x0CCCC, 0x1, 0x1) },
	{ FRAC(4 , 5), COMBINE_DIVIDER(0x07, 0x3399C, 0x1, 0x0) },
	{ FRAC(5 , 0), COMBINE_DIVIDER(0x04, 0x0739C, 0x0, 0x0) },
	{ FRAC(5 , 5), COMBINE_DIVIDER(0x00, 0x0071C, 0x1, 0x0) },
	{ FRAC(6 , 0), COMBINE_DIVIDER(0x01, 0x00E38, 0x1, 0x1) },
	{ FRAC(6 , 5), COMBINE_DIVIDER(0x02, 0x01C78, 0x1, 0x0) },
	{ FRAC(7 , 0), COMBINE_DIVIDER(0x03, 0x03C78, 0x0, 0x0) },
	{ FRAC(7 , 5), COMBINE_DIVIDER(0x04, 0x07878, 0x1, 0x0) },
	{ FRAC(8 , 0), COMBINE_DIVIDER(0x05, 0x0F0F0, 0x1, 0x1) },
	{ FRAC(8 , 5), COMBINE_DIVIDER(0x06, 0x1E1F0, 0x1, 0x0) },
	{ FRAC(9 , 0), COMBINE_DIVIDER(0x07, 0x3E1F0, 0x0, 0x0) },
	{ FRAC(9 , 5), COMBINE_DIVIDER(0x08, 0x7C1F0, 0x1, 0x0) },
	{ FRAC(10, 0), COMBINE_DIVIDER(0x09, 0xF83E0, 0x1, 0x1) },
	{ FRAC(11, 0), COMBINE_DIVIDER(0x00, 0x007E0, 0x0, 0x0) },
	{ FRAC(12, 0), COMBINE_DIVIDER(0x01, 0x00FC0, 0x1, 0x1) },
	{ FRAC(13, 0), COMBINE_DIVIDER(0x02, 0x01FC0, 0x0, 0x0) },
	{ FRAC(14, 0), COMBINE_DIVIDER(0x03, 0x03F80, 0x1, 0x1) },
	{ FRAC(15, 0), COMBINE_DIVIDER(0x04, 0x07F80, 0x0, 0x0) },
	{ FRAC(16, 0), COMBINE_DIVIDER(0x05, 0x0FF00, 0x1, 0x1) },
	{ FRAC(17, 0), COMBINE_DIVIDER(0x06, 0x1FF00, 0x0, 0x0) },
	{ FRAC(18, 0), COMBINE_DIVIDER(0x07, 0x3FE00, 0x1, 0x1) },
	{ FRAC(19, 0), COMBINE_DIVIDER(0x08, 0x7FE00, 0x0, 0x0) },
	{ FRAC(20, 0), COMBINE_DIVIDER(0x09, 0xFFC00, 0x1, 0x1) },
	/* Semi-synchronous operation */
	{ FRAC(2, 0), COMBINE_DIVIDER(0x01, 0x00555, 0x1, 0x1) },
	{ FRAC(4, 0), COMBINE_DIVIDER(0x05, 0x03333, 0x1, 0x1) },
	{ FRAC(6, 0), COMBINE_DIVIDER(0x01, 0x001C7, 0x1, 0x1) },
};

static unsigned long divider_freq(unsigned long input, int div_num)
{
	int offset;
	unsigned long config0, config1, config2;
	unsigned long seq, depth, hno, even;
	unsigned long combined;
	int i;

	switch (div_num) {
	case 0:
		offset = CLKDIV0_CONFIG0;
		break;
	case 1 ... 4:
		offset = CLKDIV1_4_CONFIG0(div_num);
		break;
	case 6 ... 10:
		offset = CLKDIV6_10_CONFIG0(div_num);
		break;
	default:
		BUG();
		return 0;
	}

	config0 = readl(ss_base + offset + 0x0);
	config1 = readl(ss_base + offset + 0x4);
	config2 = readl(ss_base + offset + 0x8);

	seq = (config0 & 0xffff) | ((config1 & 0xf) << 16);
	depth = config2 & 0xf;
	hno = (config2 & (1<<6)) ? 1 : 0;
	even = (config2 & (1<<5)) ? 1 : 0;
	combined = COMBINE_DIVIDER(depth, seq, hno, even);

	for (i = 0; i < ARRAY_SIZE(divide_table); i++) {
		if (divide_table[i].value == combined)
			return (input*2)/divide_table[i].ratio2;
	}

	printk(KERN_DEBUG "Unrecognised value in divide table %lx\n", combined);

	return 0;
}

struct dividedpll_clk
{
	struct clk clk;
	unsigned long num;
};

static void dividedpll_clk_init(struct clk *clk)
{
	struct dividedpll_clk *dpc =
		container_of(clk, struct dividedpll_clk, clk);
	unsigned long num = dpc->num;
	unsigned long data;

	data = readl(ss_base + PLL_SELECT_CFG);
	clk->parent = &pllclks[(data & (1<<(num+1))) ? 1 : 0].clk;
}

static void dividedpll_clk_recalc(struct clk *clk)
{
	struct dividedpll_clk *dpc =
		container_of(clk, struct dividedpll_clk, clk);
	unsigned long num = dpc->num;

	clk->rate = divider_freq(clk->parent->rate, num);
}

static struct clk_ops dividedpll_clk_ops = {
	.init		= dividedpll_clk_init,
	.recalc		= dividedpll_clk_recalc,
};

#define DIVIDEDPLL_CLK(_num, _name)					\
{									\
	.clk = {							\
		 .name	= _name,					\
		.flags	= CLK_ALWAYS_ENABLED | CLK_RATE_PROPAGATES,	\
		.ops	= &dividedpll_clk_ops,				\
	},								\
	.num = _num,							\
}

static struct dividedpll_clk dividedpll_clks[] = {
	DIVIDEDPLL_CLK(0, "ddr"), /* or spare? */
	DIVIDEDPLL_CLK(1, "lmi"),
	DIVIDEDPLL_CLK(2, "blt"),
	DIVIDEDPLL_CLK(3, "sys"),
	DIVIDEDPLL_CLK(4, "fdma"), /* can also be a freq synth */
	/* 5: DDR */
	DIVIDEDPLL_CLK(6, "av"),
	/* 7: Spare */
	DIVIDEDPLL_CLK(8, "eth"),
	DIVIDEDPLL_CLK(9, "st40_ick"),
	DIVIDEDPLL_CLK(10, "st40_pck"),
};

/* SH4 generic clocks ------------------------------------------------------ */

static void generic_clk_recalc(struct clk *clk)
{
	clk->rate = clk->parent->rate;
}

static struct clk_ops generic_clk_ops = {
	.recalc		= generic_clk_recalc,
};

static struct clk generic_module_clk = {
	.name		= "module_clk",
	.parent		= &dividedpll_clks[8].clk, /* st40_pck */
	.flags		= CLK_ALWAYS_ENABLED,
	.ops		= &generic_clk_ops,
};

static struct clk generic_comms_clk = {
	.name		= "comms_clk",
	.parent		= &dividedpll_clks[3].clk, /* clk_sys */
	.flags		= CLK_ALWAYS_ENABLED,
	.ops		= &generic_clk_ops,
};

int __init clk_init(void)
{
	int i, ret;

	ss_base = ioremap(SYS_SERV_BASE_ADDR, 1024);
	if (! ss_base)
		panic("Unable to remap system services");

	ret = clk_register(&xtal_osc);
	clk_enable(&xtal_osc);

	for (i = 0; i < 2; i++) {
		ret |= clk_register(&pllclks[i].clk);
		clk_enable(&pllclks[i].clk);
	}

	for (i = 0; i < ARRAY_SIZE(dividedpll_clks); i++) {
		ret |= clk_register(&dividedpll_clks[i].clk);
		clk_enable(&dividedpll_clks[i].clk);
	}

	ret = clk_register(&generic_module_clk);
	clk_enable(&generic_module_clk);
	ret = clk_register(&generic_comms_clk);
	clk_enable(&generic_comms_clk);


	/* Propagate the clk osc value down */
	clk_set_rate(&xtal_osc, clk_get_rate(&xtal_osc));
	clk_put(&xtal_osc);

	return ret;
}
