/*
 * Wyplay GaiaBox
 * PCB revision GAIABA_MBRD_V0100
 * PCB revision GAIABA_MBRD_V0200
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

#define STM_PWM_BASE            P4SEGADDR(0x18010000)
#define STM_PWM0_VAL            (STM_PWM_BASE + 0x00)
#define STM_PWM_CTRL            (STM_PWM_BASE + 0x50)

void wygaiabox_pio_init(void)
{
	struct stpio_pin *pio_ptr = NULL;
	u64 sysconf;
	struct sysconf_field *sc;

	/** configure SYS_CFG7 register, "PIOs alternate" **/
	sc = sysconf_claim(SYS_CFG, 7, 0, 20, __func__);
	sysconf = sysconf_read(sc);
	/* PIO4_6 (LED PWM) = PWM_OUT0, and not SCI_SCK */
	sysconf &= ~(1 << 0);
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

	/** MMI **/
	/* PIO1_3 = LED1: red
	 * PIO1_4 = LED2: blue
	 * PIO1_5 = not connected, provision for LED3
	 * PIO4_6 = common pulse-width modulation for LED1 to LED3 */
	/* Turn LED1 off, LED2 on, LED3 off */
	pio_ptr = stpio_request_set_pin(1, 3, "LED1", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_set_pin(1, 4, "LED2", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_set_pin(1, 5, "LED3", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_pin(4, 6, "LED_PWM_CONTROL", STPIO_ALT_OUT);
	stpio_free_pin(pio_ptr);
	/* set PWM signal to lowest frequency, 412Hz, and duty cycle to 60% */
	/* PWM_CTRL.PWM_EN = 0 */
	sysconf = ctrl_inl(STM_PWM_CTRL);
	sysconf &= ~(1 << 9);
	ctrl_outl(sysconf, STM_PWM_CTRL);
	/* PWM0_VAL = 153 */
	sysconf = ctrl_inl(STM_PWM0_VAL);
	sysconf &= ~0xFF;
	sysconf |= 153;
	ctrl_outl(sysconf, STM_PWM0_VAL);
	/* PWM_CLK_VAL = 0xFF */
	sysconf = ctrl_inl(STM_PWM_CTRL);
	sysconf |= (0xF << 11);
	sysconf |= (0xF << 0);
	sysconf &= ~(1 << 10);
	/* PWM_CTRL.PWM_EN = 1 */
	sysconf |= (1 << 9);
	ctrl_outl(sysconf, STM_PWM_CTRL);

	/** Memories **/
	/* PIO1_6 = SST55VD020:nWPnPD (NAND Flash Write Protect)
	 * PIO1_7 = S29GL064N:RYnBY (NOR Flash Ready/Busy output)
	 * PIO2_6 = AT24C08AN:WP (EEPROM Write Protect)
	 * PIO3_6 = S29GL064N:nWP (NOR Flash Write Protect)
	 * PIO5_3 = SST55VD020:nRESET (ATA NAND Flash Controller Reset) */
	/* NAND Flash initially read/write (high) */
	pio_ptr = stpio_request_set_pin(1, 6, "NAND_FLASH_PROTECT", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);
	/* NOR Flash Ready/Busy */
	pio_ptr = stpio_request_pin(1, 7, "NOR_FLASH_BUSY", STPIO_IN);
	stpio_free_pin(pio_ptr);
	/* EEPROM initially read-only (high) */
	pio_ptr = stpio_request_set_pin(2, 6, "EEPROM_PROTECT", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);
	/* NOR Flash initially read-only (low) */
	pio_ptr = stpio_request_set_pin(3, 6, "NOR_FLASH_PROTECT", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	/* ATA NAND Flash Controller Reset:
	 * let's not reset it now, U-Boot did it already
	 * not sure whether the NAND Flash may be under access already */
	pio_ptr = stpio_request_set_pin(5, 3, "NAND_FLASH_RESET", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);

	/** RS-232 **/
	/* PIO1_0 = ASC1_TXD
	 * PIO1_1 = ASC1_RXD */
	/* note: SYS_CFG7:PIO1_SCCLK_NOT_CLK_DSS has been cleared upon reset */
	/* The STM Asynchronous Serial Controller driver configures
	 * TXD and RXD PIO's itself, see asc_set_termios_cflag() */

	/** I2C busses **/
	/* PIO2_0 = I2C0 SCL
	 * PIO2_1 = I2C0 SDA
	 * PIO4_0 = HDMI SCL (I2C1)
	 * PIO4_1 = HDMI SDA (I2C1) */
	/* The STM Synchronous Serial Controller driver configures
	 * SCL and SDA PIO's itself, see st_i2c.c */

	/** SPI bus **/
	/* PIO3_0 = SPI CLK
	 * PIO3_1 = SPI DOUT
	 * PIO3_2 = SPI DIN
	 * PIO3_4 = M25P16:nS (SPI Flash for MPEG Video and Audio Encoder chip select): active low, no external pull-up */
	/* The STM Synchronous Serial Controller driver configures
	 * CLK, DOUT and DIN PIO's itself.
	 * Chip Select PIO's are allocated by the SSC driver too, upon application
	 * request, for now leave them as initialized by U-Boot */

	/** USB **/
	/* PIO2_7 = USB2514:nRESET
	 * PIO4_4 = NET2272:nRESET (USB Peripheral Controller) */
	/* Reset the PLX USB Peripheral Controller
	 * assert nRESET (low) for at least 2ms, or 100ns if not in suspended state */
	pio_ptr = stpio_request_set_pin(4, 4, "USB_UDC_RESET", STPIO_OUT, 0);
	udelay(2000);
	stpio_set_pin(pio_ptr, 1);
	stpio_free_pin(pio_ptr);
	/* Reset the USB hub
	 * assert nRESET (low) for at least 1usec */
	pio_ptr = stpio_request_set_pin(2, 7, "USB_HUB_RESET", STPIO_OUT, 0);
	udelay(2);
	stpio_set_pin(pio_ptr, 1);
	stpio_free_pin(pio_ptr);

	/** Ethernet **/
	/* PIO2_3 = 88E6031:nRESET
	 * PIO3_7 = 88E6031:XTAL_IN (MII PHY clock)
	 *          PIO2_3 is also accessed by phy_reset() in setup.c */
	/* Reset the Ethernet switch
	 * assert nRESET (low) for at least 10ms
	 * not fully asynchronous, the running PHY clock
	 * shall be fed to XTAL_IN before releasing nRESET */
	pio_ptr = stpio_request_pin(3, 7, "MII_PHYCLK", STPIO_ALT_OUT);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_set_pin(2, 3, "ETH_SWITCH_RESET", STPIO_OUT, 0);
	udelay(10000);
	stpio_set_pin(pio_ptr, 1);
	stpio_free_pin(pio_ptr);

	/** Wireless LAN **/
	/* PIO5_5 = Mini-PCI:16 (WiFi_module nRESET) */
	/* Reset the wireless module
	 * assert nRESET (low) for at least... TODO check Mini-PCI & Ralink reqs */
	pio_ptr = stpio_request_set_pin(5, 5, "WLAN_RESET", STPIO_OUT, 0);
	udelay(100);
	stpio_set_pin(pio_ptr, 1);
	stpio_free_pin(pio_ptr);

	/** InfraRed **/
	/* PIO3_3 = InfraRed IN
	 * the LIRC driver configures IN and OUT PIOs itself */

	/** HDMI **/
	/* PIO4_5 = HDMI Hot Plug Detect: input, on-board pull-down (1K pull-up in typical HDMI receiver)
	 * PIO5_7 = HDMI Consumer Electronics Control: weak on-board pull-up */
	/* HPD PIO configured by stgfb itself (>= 2.0_stm22_0007-alpha),
	 * see Linux/stm/coredisplay/stx7109c3.c */
	pio_ptr = stpio_request_set_pin(5, 7, "HDMI_CEC", STPIO_BIDIR, 1);
	stpio_free_pin(pio_ptr);

	/** Analog Audio/Video Outputs **/
	/* PIO0_6 = TSH345:Fs0 (Video Output Filter)
	 * PIO0_7 = TSH345:Fs1
	 * the possible, expected output video mode is unknown here,
	 * for now let the analog video output buffer in power-down mode */
	pio_ptr = stpio_request_set_pin(0, 6, "DAC_FILTER0", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);
	pio_ptr = stpio_request_set_pin(0, 7, "DAC_FILTER1", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);

	/** DVB-T receivers **/
	/* PIO2_4 = STV0362:nRESET (DVB-T receiver) */
	/* the STM Programmable Transport Interface driver configures
	 * tuner RESET PIO's itself, see st-tuner.c */

	/** Analog Audio/Video Capture Subsystem **/
	/* PIO4_3 = SPI Flash (for the MPEG Video and Audio Encoder) multiplexer: low for XC2105, high for STi7109
	 * PIO5_0 = PCM1851A:nRST and SAA7117AE:CE
	 * PIO5_1 = XC2105:nRST
	 * PIO5_2 = M3750GBE-27.000MHz:ON
	 * PIO5_4 = XC2105 DDR-SDRAM power control: low for normal operation, high to void voltage reference */
	/* Have the SPI Flash driven by the XC2105 */
	pio_ptr = stpio_request_set_pin(4, 3, "SPI_FLASH_MUX", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	/* Pull PCM1851A nRST and SAA7117AE CE down to low level
	 * forces a reset and put the chip in power-down mode.
	 * If there is a pull-down, weak pull-up is not enough
	 * and we need to output a real 1 level before reset.
	 * note: the driver, if any, will need to wake the chip
	 * up before probing for it */
	pio_ptr = stpio_request_set_pin(5, 0, "ADC_ENABLE", STPIO_OUT, 1);
	udelay(10);
	stpio_set_pin(pio_ptr, 0);
	stpio_free_pin(pio_ptr);
	/* XC2105 initially reset (low) */
	pio_ptr = stpio_request_set_pin(5, 1, "VIDEO_ENCODER_RESET", STPIO_OUT, 0);
	stpio_free_pin(pio_ptr);
	/* OSC27MHZ for analog capture initially off (low) */
	pio_ptr = stpio_request_set_pin(5, 2, "CAPTURE_OSC_ENABLE", STPIO_OUT, 1);
	udelay(10);
	stpio_set_pin(pio_ptr, 0);
	stpio_free_pin(pio_ptr);
	/* XC2105 DDR-SDRAM initially in power-save mode (high) */
	pio_ptr = stpio_request_set_pin(5, 4, "VIDEO_ENCODER_RAM_PWR", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);

	/** HDD **/
	/* PIO1_2 = FDC6330L:ON/OFF (HDD power switch): powered high */
	/* HDD power switch initially ON
	 * note: due to the pull-up on this line, the HDD is powered already */
	pio_ptr = stpio_request_set_pin(1, 2, "HDD_PWR", STPIO_OUT, 1);
	stpio_free_pin(pio_ptr);

	/** Fan **/
	/* PIO4_7 = PWM_FAN_CTRL */
	pio_ptr = stpio_request_pin(4, 7, "FAN_CONTROL", STPIO_ALT_OUT);
	stpio_free_pin(pio_ptr);

	/** Reset **/
	/* PIO3_5 = Factory Reset Button (active low, not debounced) */
	pio_ptr = stpio_request_pin(3, 5, "RESET_BUTTON", STPIO_IN);
	stpio_free_pin(pio_ptr);
}
