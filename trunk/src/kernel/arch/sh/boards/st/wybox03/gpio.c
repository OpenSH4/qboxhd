/*
 * Wyplay WyBox, PCB revision WYBOXA_MBRD_V0200
 *
 * (C) Copyright 2006-2008 WyPlay SAS.
 * Aubin Constans <aconstans@wyplay.com>
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/stm/pio.h>
#include <linux/stm/soc.h>
#include <linux/stm/sysconf.h>

#include <asm/io.h>

void wybox03_pio_init(void)
{
	struct stpio_pin *pio_ptr = NULL;
	u64 sysconf;
	struct sysconf_field *sc;

	/** configure SYS_CFG7 register, "PIOs alternate" **/
	sc = sysconf_claim(SYS_CFG, 7, 0, 20, __func__);
	sysconf = sysconf_read(sc);
	/* PIO2_1 (I2C0 SDA) = SSC0_MTSR, and not SSC0_MRST */
	sysconf &= ~(1 << 1);
	/* PIO3_1 (SPI DOUT) = SSC1_MTSR, and not SSC1_MRST */
	sysconf &= ~(1 << 2);
	/* PIO4_1 (I2C1 SDA) = SSC2_MTSR, and not SSC2_MRST */
	sysconf &= ~(1 << 3);
	/* have ASC0 (SmartCard interface) clock derive from CLK_DSS,
	 * not from SC0_CLK_OUT */
	sysconf &= ~(1 << 4);
	/* ensure ASC1 (RS-232 UART) clock derives from CLK_DSS */
	sysconf &= ~(1 << 5);
	/* configure SC0_nSETVCC: derived from SC0_DETECT input */
	sysconf &= ~(1 << 7);
	/* set polarity of SC0_nSETVCC: SC0_nSETVCC = NOT(SC0_DETECT) */
	sysconf |= (1 << 8);
	/* use SSC0 and SSC1, and not the video horizontal and vertical synchro
	 * VTG_X outputs, nor the DVO_BLANK digital video blanking output */
	sysconf &= ~(1 << 10);
	/* enable Synopsys Ethernet MAC Subsystem, and Media Independent Interface */
	/* select MII interface as function of the shared pad, and not DVO output */
	sysconf |= (1 << 16);
	/* enable MII pad */
	sysconf |= (1 << 17);
	/* select MII interface, and not Reduced MII */
	sysconf &= ~(1 << 18);
	/* PHY clock is provided by STx7109 */
	sysconf &= ~(1 << 19);
	/* for now have the MAC run at 10Mbps speed */
	sysconf &= ~(1 << 20);
	sysconf_write(sc, sysconf);

	/** SmartCard **/
	/* PIO0_0 = SC0_DATAOUT	 = TDA8024TT:AUX1UC	= ISO C4
	 * PIO0_1 = SC0_DATAIN	 = TDA8024TT:I/OUC	= ISO C7
	 * PIO0_2 = SC0_EXTCLKIN = TDA8024TT:AUX2UC	= ISO C8
	 * PIO0_3 = SC0CG_CLK	 = TDA8024TT:XTAL1
	 * PIO0_4		 = TDA8024TT:RESET
	 * PIO0_5 = SC0_nSETVCC	 = TDA8024TT:nCMDVCC
	 * PIO0_7 = SC0_DETECT	 = TDA8024TT:nOFF
	 * PIO1_6		 = TDA8024TT:5Vn3V */

	/* configure initial smartcard power supply level: VCC = 3V */
	pio_ptr = stpio_request_set_pin(1, 6, "SC_VCC_LEVEL", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_pin(0, 0, "SC_DOUT", STPIO_ALT_OUT);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_pin(0, 1, "SC_DIN", STPIO_IN);
	stpio_free_pin(pio_ptr);
	/* TODO: PIO0_2 */
	pio_ptr = stpio_request_pin(0, 3, "SC_CLKOUT", STPIO_ALT_OUT);
	stpio_free_pin(pio_ptr);
	/* for now assert RESET input */
	pio_ptr = stpio_request_set_pin(0, 4, "SC_RESET", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	/* have SC0_DETECT input ready before turning SC0_nSETVCC output on */
	pio_ptr = stpio_request_pin(0, 7, "SC_DETECT", STPIO_IN);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_pin(0, 5, "SC_nSETVCC", STPIO_ALT_OUT);
	stpio_free_pin(pio_ptr);

	/** General Power Supply **/
	/* PIO0_6 = SWITCHOFF
	 * PIO4_2 = POWER BUTTON = LED1702 */

	/* allow general power supply */
	pio_ptr = stpio_request_set_pin(0, 6, "POWER_TURN_OFF", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	/* PIO4_2 is configured and read by the wyntiic/i2c-irq-handler driver */

	/** RS-232 **/
	/* PIO1_0 = ASC1_TXD
	 * PIO1_1 = ASC1_RXD
	 * The STM Asynchronous Serial Controller driver configures
	 * TXD and RXD PIO's itself, see asc_set_termios_cflag() */

	/** IDE - ATA **/
	/* PIO1_2 = SST55LD019A:nWP (ATA NAND Flash Controller Write Protect)
	 * PIO5_3 = SST55LD019A:nRESET and ATAPI:nRESET */
	/* ATA NAND Flash: begin RD/WR (high) */
	pio_ptr = stpio_request_set_pin(1, 2, "NAND_PROTECT", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);
	/* let's not reset ATA now, U-Boot did it already
	 * not sure whether the NAND Flash may be under access already */
	pio_ptr = stpio_request_set_pin(5, 3, "ATA_RESET", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);

	/** CPLD - TS Multiplexer **/
	/* PIO1_3 = CPLD S0
	 * PIO1_4 = CPLD S1 */
	pio_ptr = stpio_request_set_pin(1, 3, "TS0", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_set_pin(1, 4, "TS1", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);

	/** Daughter Board GPIOs and Debug LEDs **/
	/* PIO1_5 = GPIO1 = LED1700
	 * PIO1_7 = GPIO2 = LED1701
	 * TODO: adjust, this depends on plugged daughter board, for now
	 * configure as inputs to prevent HW line contention */
	pio_ptr = stpio_request_pin(1, 5, "DBRD_GPIO1", STPIO_IN);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_pin(1, 7, "DBRD_GPIO2", STPIO_IN);
	stpio_free_pin(pio_ptr);

	/** I2C busses **/
	/* PIO2_0 = I2C0 SCL
	 * PIO2_1 = I2C0 SDA
	 * PIO4_0 = I2C1 SCL
	 * PIO4_1 = I2C1 SDA
	 * The STM Synchronous Serial Controller driver configures
	 * SCL and SDA PIO's itself, see st_i2c.c */

	/** SPI bus **/
	/* PIO3_0 = SPI CLK
	 * PIO3_1 = SPI DOUT
	 * PIO3_2 = SPI DIN
	 * PIO3_4 = LCD SPI nCS
	 * PIO4_4 = SPI Flash nCS
	 * The STM Synchronous Serial Controller driver configures
	 * CLK, DOUT and DIN PIO's itself.
	 * Chip Select PIO's are allocated by the SSC driver too, upon application
	 * request, for now leave them as initialized by U-Boot */

	/** Interrupt Multiplexer **/
	/* PIO2_2 = Interrupt Multiplexer Latch Enable
	 * configured and driven by the wyntiic/i2c-irq-handler driver */

	/** Tuners **/
	/* PIO2_3 = Tuner1n2 Tuner Select
	 * PIO2_4 = Tuner1 nRESET
	 * PIO2_5 = Tuner2 nRESET
	 * PIO2_6 = Tuner1 LoopThrough
	 * PIO2_7 = Tuner2 LoopThrough
	 * PIO5_4 = DiSEqC RX
	 * PIO5_5 = DiSEqC TX */
	/* for now select Tuner1 */
	pio_ptr = stpio_request_set_pin(2, 3, "TUNER_SEL", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);
	/* the STM Programmable Transport Interface driver configures
	 * tuner RESET PIO's itself, see st-tuner.c
	 * sounds like DiSEqC RX and TX PIO's are covered as well */
	pio_ptr = stpio_request_set_pin(2, 6, "TUNER1_LOOP", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_set_pin(2, 7, "TUNER2_LOOP", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);

	/** InfraRed **/
	/* PIO3_3 = InfraRed IN
	 * PIO3_5 = InfraRed OUT
	 * the LIRC driver configures IN and OUT PIOs itself */

	/** Secure NOR Flash **/
	/* PIO3_6 = VPP */
	/* set NOR flash initially read-only */
	pio_ptr = stpio_request_set_pin(3, 6, "SEC_FLASH_VPP", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);

	/** Ethernet **/
	/* PIO3_7 = LAN8700:CLKIN (PHY clock)
	 * PIO4_6 = LAN8700:nRST, or LAN9117:FIFO_SEL (MAC FIFO Select)
	 *	    PIO4_6 is also accessed by phy_reset() in setup.c */
	pio_ptr=stpio_request_pin(3, 7, "MII_PHYCLK", STPIO_ALT_OUT);
	stpio_free_pin(pio_ptr);

	/* Reset SMSC LAN8700 PHY
	 * assert nRST (low) for at least 100usec,
	 * with MII clock fed in at the same time */
	pio_ptr = stpio_request_set_pin(4, 6, "ETH_PHY_RESET", STPIO_OUT, 1);
	/* PHY clock has just been enabled, allow the PHY to settle */
	udelay(2);
	stpio_set_pin(pio_ptr, 0);
	udelay(110);
	stpio_set_pin(pio_ptr, 1);
	udelay(1);
	stpio_free_pin(pio_ptr);

	/** SPI Flash **/
	/* PIO4_3 = SPI Flash Multiplexer */
	/* have the SPI Flash driven by the XC2105 */
	pio_ptr = stpio_request_set_pin(4, 3, "SPI_FLASH_MUX", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);

	/** HDMI **/
	/* PIO4_5 = HDMI Hot Plug Detect (input)
	 * PIO5_7 = HDMI CEC
	 * HPD PIO configured by stgfb itself (>= 2.0_stm22_0007-alpha) */
	pio_ptr = stpio_request_set_pin(5, 7, "HDMI_CEC", STPIO_BIDIR, 1);
	stpio_free_pin(pio_ptr);

	/** Fan **/
	/* PIO4_7 = PWM_FAN_CTRL */
	pio_ptr = stpio_request_pin(4, 7, "PWM", STPIO_ALT_OUT);
	stpio_free_pin(pio_ptr);

	/** Analog Audio/Video Capture Subsystem **/
	/* PIO5_0 = PCM1851A:nRST and SAA7117AE:CE and OSC27MHZ:ON
	 * PIO5_1 = XC2105:nRST */
	pio_ptr = stpio_request_set_pin(5, 0, "ADC_ENABLE", STPIO_OUT, 1);
	udelay(200);
	/* pull SAA7117AE:CE and its friends down to low level
	 * forces a reset and put the chip in power-down mode
	 * note: the driver, if any, will need to wake the chip
	 * up before probing for it */
	stpio_set_pin(pio_ptr, 0);
	stpio_free_pin(pio_ptr);
	/* reset the XC2105
	 * the SPI Flash Multiplexer shall have been initialized already */
	pio_ptr = stpio_request_set_pin(5, 1, "VIDEO_ENCODER_RESET", STPIO_OUT, 1);
	udelay(10);
	stpio_set_pin(pio_ptr, 0);
	udelay(50);
	stpio_set_pin(pio_ptr, 1);
	stpio_free_pin(pio_ptr);

	/** DVB - Common Interface **/
	/* PIO5_2 = StarCI2Win RESET */
	/* 11us minimum pulse for STARCI2WIN RESET, then 329ms minimum */
	pio_ptr = stpio_request_set_pin(5, 2, "STARCI2WIN_RESET", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);
	/* PIO5_6 = unused ? */
}
