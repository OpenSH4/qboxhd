/*-
 * An rtc/i2c driver for the Ricoh R2025 real-time clock module
 *
 * Copyright (c) 2006 Shigeyuki Fukushima.
 * All rights reserved.
 *
 * Written by Shigeyuki Fukushima.
 *
 * Port to Linux by
 * Jean-Christophe Plagniol-Villard <jcplagniol@wyplay.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include "rtc-r2025.h"

/* Addresses to scan: none. This chip cannot be detected. */
static unsigned short normal_i2c[] = { R2025_I2C_ADDR,I2C_CLIENT_END };

/* Insmod parameters */
I2C_CLIENT_INSMOD;

/* Prototypes */
static int r2025_probe(struct i2c_adapter *adapter, int address, int kind);


static int r2025_get_control(struct i2c_client *client, u8 *status)
{
	unsigned char addr = R2025_CTRL1;

	struct i2c_msg msgs[] = {
		{ client->addr, 0, 1, &addr },		/* setup read ptr */
		{ client->addr, I2C_M_RD, 1, status },	/* read control */
	};

	/* read control register */
	if ((i2c_transfer(client->adapter, &msgs[0], 2)) != 2) {
		dev_err(&client->dev, "%s: read error\n", __FUNCTION__);
		return -EIO;
	}

	return 0;
}

static int r2025_set_control(struct i2c_client *client, u8 status)
{
	return 0;
#if 0
	uint8_t buf[2];
	int xfer;
	buf[0]=R2025_I2C_ADDR<<1+R2025_CTRL1;
	buf[1]=status;
	xfer = i2c_master_send(client, buf, 2);
	if (xfer != 2) {
		dev_err(&client->dev, "%s: send: %d\n", __FUNCTION__, xfer);
		return -EIO;
	}

	return 0;
#endif
}


/*
 * In the routines that deal directly with the r2025 hardware, we use
 * rtc_time -- month 0-11, hour 0-23, yr = calendar year-epoch
 * Epoch is initialized as 2000. Time is set to UTC.
 */
static int r2025_get_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	unsigned char addr = R2025_REG_BASE;
	unsigned char buf[R2025_CLK_SIZE];
	u8 ctrl;
	u8 hour;
	struct i2c_msg msgs[] = {
		{ client->addr, 0, 1, &addr },		/* setup read ptr */
		{ client->addr, I2C_M_RD, R2025_CLK_SIZE, buf },	/* read date */
	};

	/* read date registers */
	if ((i2c_transfer(client->adapter, &msgs[0], 2)) != 2) {
		dev_err(&client->dev, "%s: read error\n", __FUNCTION__);
		return -EIO;
	}

	dev_dbg(&client->dev,
		"%s: raw read data - counters=%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",
		__FUNCTION__, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);

	tm->tm_sec = BCD2BIN(buf[R2025_SEC] & R2025_SEC_MASK);
	tm->tm_min = BCD2BIN(buf[R2025_MIN] & R2025_MIN_MASK);
	hour = BCD2BIN(buf[R2025_HOUR] & R2025_HOUR_MASK);
	r2025_get_control(client,&ctrl);
	if (ctrl & R2025_CTRL1_H1224)
	{
		tm->tm_hour = hour;
	}
	else
	{
		if (hour == 12)
		{
			tm->tm_hour = 0;
		}
		else if (hour == 32)
		{
			tm->tm_hour = 12;
		}
		else if (hour > 13)
		{
			tm->tm_hour = (hour - 8);
		}
		else
		{
			/* (hour < 12) */
			tm->tm_hour = hour;
		}
	}
	tm->tm_wday = BCD2BIN(buf[R2025_WDAY] & R2025_WDAY_MASK);
	tm->tm_mday = BCD2BIN(buf[R2025_MDAY] & R2025_MDAY_MASK);
	tm->tm_mon = BCD2BIN(buf[R2025_MON] & R2025_MON_MASK) -1;
	tm->tm_year = BCD2BIN(buf[R2025_YEAR] & R2025_YEAR_MASK) + ((buf[R2025_MON] & R2025_MON_Y1920) ? 100 : 0);

	dev_dbg(&client->dev, "%s: tm is secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__FUNCTION__, tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	return 0;
}

static int r2025_set_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	u8 ctrl;
	uint8_t _buf[R2025_CLK_SIZE+1];
	uint8_t *buf=_buf+1;
	int xfer;
	_buf[0]=R2025_REG_BASE;
	dev_dbg(&client->dev,
		"%s: secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__FUNCTION__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);
	/* Y3K problem */
	if (tm->tm_year >= 1100)
	{
		printk("r2025 settime: "
		"RTC does not support year 3000 or over.\n");
		return -EIO;
	}
	if(r2025_get_control(client,&ctrl)!=0)
	{
		return -EIO;
	}
	ctrl |= R2025_CTRL1_H1224;

	/* setup registers 0x00-0x06 (7 byte) */
	buf[R2025_SEC] = BIN2BCD(tm->tm_sec) & R2025_SEC_MASK;
	buf[R2025_MIN] = BIN2BCD(tm->tm_min) & R2025_MIN_MASK;
	buf[R2025_HOUR] = BIN2BCD(tm->tm_hour) & R2025_HOUR_MASK;
	buf[R2025_WDAY] = BIN2BCD(tm->tm_wday) & R2025_WDAY_MASK;
	buf[R2025_MDAY] = BIN2BCD(tm->tm_mday) & R2025_MDAY_MASK;
	buf[R2025_MON] = (BIN2BCD(tm->tm_mon-1) & R2025_MON_MASK)
	| ((tm->tm_year >= 100) ? R2025_MON_Y1920 : 0);
	buf[R2025_YEAR] = BIN2BCD(tm->tm_year % 100) & R2025_YEAR_MASK;
	r2025_set_control(client,ctrl);
	xfer = i2c_master_send(client, _buf, R2025_CLK_SIZE+1);
	if (xfer != R2025_CLK_SIZE+1) {
		dev_err(&client->dev, "%s: send: %d\n", __FUNCTION__, xfer);
		return -EIO;
	}
	return 0;
}

static int r2025_set_mmss(struct i2c_client *client, unsigned long secs)
{
	struct rtc_time tm;
	rtc_time_to_tm(secs,&tm);
	return r2025_set_datetime(client,&tm);
}

static int r2025_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	return r2025_get_datetime(to_i2c_client(dev), tm);
}

static int r2025_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	return r2025_set_datetime(to_i2c_client(dev), tm);
}

static int r2025_rtc_set_mmss(struct device *dev, unsigned long secs)
{
	return r2025_set_mmss(to_i2c_client(dev), secs);
}

/* following are the sysfs callback functions */
static ssize_t show_control(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 control;
	int err;

	err = r2025_get_control(client, &control);
	if (err)
		return err;

	return sprintf(buf, "%x\n", (control ));
}
static DEVICE_ATTR(control, S_IRUGO, show_control, NULL);

static struct rtc_class_ops r2025_rtc_ops = {
	.read_time	= r2025_rtc_read_time,
	.set_time	= r2025_rtc_set_time,
	.set_mmss	= r2025_rtc_set_mmss,
};

static int r2025_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, r2025_probe);
}

static int r2025_detach(struct i2c_client *client)
{
	int err;
	struct rtc_device *rtc = i2c_get_clientdata(client);

 	if (rtc)
		rtc_device_unregister(rtc);

	if ((err = i2c_detach_client(client)))
		return err;

	kfree(client);

	return 0;
}

static struct i2c_driver r2025_driver = {
	.driver		= {
		.name	= "r2025",
	},
	.id		= I2C_DRIVERID_R2025,
	.attach_adapter = &r2025_attach,
	.detach_client	= &r2025_detach,
};

static int r2025_probe(struct i2c_adapter *adapter, int address, int kind)
{
	int err = 0;
	struct i2c_client *client;
	struct rtc_device *rtc;

	dev_dbg(&adapter->dev, "%s\n", __FUNCTION__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit;
	}

	if (!(client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	/* I2C client */
	client->addr = address;
	client->driver = &r2025_driver;
	client->adapter	= adapter;

	strlcpy(client->name, r2025_driver.driver.name, I2C_NAME_SIZE);

	/* Inform the i2c layer */
	if ((err = i2c_attach_client(client)))
		goto exit_kfree;

	dev_info(&client->dev, "chip found, driver version 0.1\n");

	rtc = rtc_device_register(r2025_driver.driver.name, &client->dev,
				&r2025_rtc_ops, THIS_MODULE);

	if (IS_ERR(rtc)) {
		err = PTR_ERR(rtc);
		goto exit_detach;
	}

	i2c_set_clientdata(client, rtc);

	/* Register sysfs hooks */
	return device_create_file(&client->dev, &dev_attr_control);

exit_detach:
	printk("RTC R2025 Not attached\n");
	i2c_detach_client(client);

exit_kfree:
	kfree(client);

exit:
	return err;
}

static int __init r2025_init(void)
{
	return i2c_add_driver(&r2025_driver);
}

static void __exit r2025_exit(void)
{
	i2c_del_driver(&r2025_driver);
}

MODULE_AUTHOR("Jean-Christophe PLAGNIOL-VILLARD <jcplagniol@wyplay.com>");
MODULE_DESCRIPTION("R2025 timekeeper driver");
MODULE_LICENSE("BSD");
MODULE_VERSION("0.1");

module_init(r2025_init);
module_exit(r2025_exit);
