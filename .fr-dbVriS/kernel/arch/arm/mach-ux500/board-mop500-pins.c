








/* NOTE: Not used! Will be removed later, but since STE delivers this one then
 * we want GIT to help us (and not complain that STE has modified the file and
 * we have deleted it).
 *
 * SEMC wants to change gpio-pdp.c or gpio-$product.c instead.
 */













/*
 * Copyright (C) ST-Ericsson SA 2010
 *
 * License terms: GNU General Public License (GPL) version 2
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/string.h>

#include <plat/pincfg.h>
#include <plat/gpio.h>

#include <mach/hardware.h>
#include <mach/suspend.h>

#include "pins-db8500.h"
#include "pins.h"

#include "board-pins-sleep-force.h"

enum custom_pin_cfg_t {
	PINS_FOR_DEFAULT,
	PINS_FOR_U9500,
};

static enum custom_pin_cfg_t pinsfor;

static pin_cfg_t mop500_pins_common[] = {
	/* MSP0 */
	GPIO12_MSP0_TXD,
	GPIO13_MSP0_TFS,
	GPIO14_MSP0_TCK,
	GPIO15_MSP0_RXD,

	/* MSP2: HDMI */
	GPIO193_MSP2_TXD,
	GPIO194_MSP2_TCK,
	GPIO195_MSP2_TFS,
	GPIO196_MSP2_RXD | PIN_OUTPUT_LOW,

	/* LCD TE0 */
	GPIO68_LCD_VSI0	| PIN_INPUT_PULLUP,

	/* Touch screen INTERFACE */
	GPIO84_GPIO	| PIN_INPUT_PULLUP, /* TOUCH_INT1 */

	/* STMPE1601/tc35893 keypad  IRQ */
	GPIO218_GPIO	| PIN_INPUT_PULLUP,

	/* UART */
	/* uart-0 pins gpio configuration should be
	 * kept intact to prevent glitch in tx line
	 * when tty dev is opened. Later these pins
	 * are configured to uart mop500_pins_uart0
	 *
	 * It will be replaced with uart configuration
	 * once the issue is solved.
	 */
	GPIO0_GPIO   | PIN_INPUT_PULLUP,
	GPIO1_GPIO   | PIN_OUTPUT_HIGH,
	GPIO2_GPIO   | PIN_INPUT_PULLUP,
	GPIO3_GPIO   | PIN_OUTPUT_LOW,

	GPIO29_U2_RXD	| PIN_INPUT_PULLUP,
	GPIO30_U2_TXD	| PIN_OUTPUT_HIGH,
	GPIO31_U2_CTSn	| PIN_INPUT_PULLUP,
	GPIO32_U2_RTSn	| PIN_OUTPUT_HIGH,

	/* HSI */
	GPIO219_HSIR_FLA0,
	GPIO220_HSIR_DAT0,
	GPIO221_HSIR_RDY0,
	GPIO222_HSIT_FLA0,
	GPIO223_HSIT_DAT0,
	GPIO224_HSIT_RDY0,
	GPIO225_GPIO	| PIN_INPUT_PULLDOWN, /* CA_WAKE0 */
};

static pin_cfg_t mop500_pins_default[] = {
	/* XENON Flashgun INTERFACE */
	GPIO6_IP_GPIO0	| PIN_INPUT_PULLUP,/* XENON_FLASH_ID */
	GPIO7_IP_GPIO1	| PIN_INPUT_PULLUP,/* XENON_READY */

	GPIO217_GPIO	| PIN_INPUT_PULLUP, /* TC35892 IRQ */

	/* sdi0 (removable MMC/SD/SDIO cards) not handled by pm_runtime */
	GPIO21_MC0_DAT31DIR	| PIN_OUTPUT_HIGH,
};

static pin_cfg_t mop500_pins_hrefv60[] = {
	/* WLAN */
	GPIO85_GPIO		| PIN_OUTPUT_LOW,/* WLAN_ENA */

	/* XENON Flashgun INTERFACE */
	GPIO6_IP_GPIO0	| PIN_INPUT_PULLUP,/* XENON_FLASH_ID */
	GPIO7_IP_GPIO1	| PIN_INPUT_PULLUP,/* XENON_READY */

	/* Assistant LED INTERFACE */
	GPIO21_GPIO | PIN_OUTPUT_LOW,  /* XENON_EN1 */
	GPIO64_IP_GPIO4 | PIN_OUTPUT_LOW,  /* XENON_EN2 */

	/* Magnetometer */
	GPIO31_GPIO | PIN_INPUT_PULLUP,  /* magnetometer_INT */
	GPIO32_GPIO | PIN_INPUT_PULLDOWN, /* Magnetometer DRDY */

	/* Display Interface */
	GPIO65_GPIO		| PIN_OUTPUT_HIGH, /* DISP1 NO RST */
	GPIO66_GPIO		| PIN_OUTPUT_LOW, /* DISP2 RST */

	/* Touch screen INTERFACE */
	GPIO143_GPIO	| PIN_OUTPUT_LOW,/*TOUCH_RST1 */

	/* Touch screen INTERFACE 2 */
	GPIO67_GPIO	| PIN_INPUT_PULLUP, /* TOUCH_INT2 */
	GPIO146_GPIO	| PIN_OUTPUT_LOW,/*TOUCH_RST2 */

	/* ETM_PTM_TRACE INTERFACE */
	GPIO70_GPIO	| PIN_OUTPUT_LOW,/* ETM_PTM_DATA23 */
	GPIO71_GPIO	| PIN_OUTPUT_LOW,/* ETM_PTM_DATA22 */
	GPIO72_GPIO	| PIN_OUTPUT_LOW,/* ETM_PTM_DATA21 */
	GPIO73_GPIO	| PIN_OUTPUT_LOW,/* ETM_PTM_DATA20 */
	GPIO74_GPIO	| PIN_OUTPUT_LOW,/* ETM_PTM_DATA19 */

	/* NAHJ INTERFACE */
	GPIO76_GPIO	| PIN_OUTPUT_LOW,/* NAHJ_CTRL */
	GPIO216_GPIO	| PIN_OUTPUT_HIGH,/* NAHJ_CTRL_INV */

	/* NFC INTERFACE */
	GPIO77_GPIO	| PIN_OUTPUT_LOW, /* NFC_ENA */
	GPIO144_GPIO	| PIN_INPUT_PULLDOWN, /* NFC_IRQ */
	GPIO142_GPIO	| PIN_OUTPUT_LOW, /* NFC_RESET */

	/* Keyboard MATRIX INTERFACE */
	GPIO90_MC5_CMD	| PIN_OUTPUT_LOW, /* KP_O_1 */
	GPIO87_MC5_DAT1	| PIN_OUTPUT_LOW, /* KP_O_2 */
	GPIO86_MC5_DAT0	| PIN_OUTPUT_LOW, /* KP_O_3 */
	GPIO96_KP_O6	| PIN_OUTPUT_LOW, /* KP_O_6 */
	GPIO94_KP_O7	| PIN_OUTPUT_LOW, /* KP_O_7 */
	GPIO93_MC5_DAT4	| PIN_INPUT_PULLUP, /* KP_I_0 */
	GPIO89_MC5_DAT3	| PIN_INPUT_PULLUP, /* KP_I_2 */
	GPIO88_MC5_DAT2	| PIN_INPUT_PULLUP, /* KP_I_3 */
	GPIO91_GPIO	| PIN_INPUT_PULLUP, /* FORCE_SENSING_INT */
	GPIO92_GPIO	| PIN_OUTPUT_LOW, /* FORCE_SENSING_RST */
	GPIO97_GPIO	| PIN_OUTPUT_LOW, /* FORCE_SENSING_WU */

	/* DiPro Sensor Interface */
	GPIO139_GPIO	| PIN_INPUT_PULLUP, /* DIPRO_INT */

	/* HAL SWITCH INTERFACE */
	GPIO145_GPIO	| PIN_INPUT_PULLDOWN,/* HAL_SW */

	/* Audio Amplifier Interface */
	GPIO149_GPIO	| PIN_OUTPUT_HIGH, /* VAUDIO_HF_EN, enable MAX8968 */

	/* GBF INTERFACE */
	GPIO171_GPIO	| PIN_OUTPUT_LOW, /* GBF_ENA_RESET */

	/* MSP : HDTV INTERFACE */
	GPIO192_GPIO	| PIN_INPUT_PULLDOWN,

	/* ACCELEROMETER_INTERFACE */
	GPIO82_GPIO		| PIN_INPUT_PULLUP, /* ACC_INT1 */
	GPIO83_GPIO		| PIN_INPUT_PULLUP, /* ACC_INT2 */

	/* Proximity Sensor */
	GPIO217_GPIO		| PIN_INPUT_PULLUP,

	/* SD card detect */
	GPIO95_GPIO	| PIN_INPUT_PULLUP,
};

static pin_cfg_t u9500_pins[] = {
	GPIO4_U1_RXD    | PIN_INPUT_PULLUP,
	GPIO5_U1_TXD    | PIN_OUTPUT_HIGH,
	GPIO144_GPIO	| PIN_INPUT_PULLUP,/* WLAN_IRQ */
	GPIO226_GPIO	| PIN_OUTPUT_HIGH, /* HSI AC_WAKE0 */
};

static pin_cfg_t u8500_pins[] = {
	GPIO226_GPIO	| PIN_OUTPUT_LOW, /* WLAN_PMU_EN */
	GPIO4_GPIO		| PIN_INPUT_PULLUP,/* WLAN_IRQ */
};


static UX500_PINS(mop500_pins_uart0,
	GPIO0_U0_CTSn	| PIN_INPUT_PULLUP,
	GPIO1_U0_RTSn	| PIN_OUTPUT_HIGH,
	GPIO2_U0_RXD	| PIN_INPUT_PULLUP,
	GPIO3_U0_TXD	| PIN_OUTPUT_HIGH,
);

/*
 * I2C
 */

static UX500_PINS(mop500_pins_i2c0,
	GPIO147_I2C0_SCL,
	GPIO148_I2C0_SDA,
);

static UX500_PINS(mop500_pins_i2c1,
	GPIO16_I2C1_SCL,
	GPIO17_I2C1_SDA,
);

static UX500_PINS(mop500_pins_i2c2,
	GPIO10_I2C2_SDA,
	GPIO11_I2C2_SCL,
);

static UX500_PINS(mop500_pins_i2c3,
	GPIO229_I2C3_SDA,
	GPIO230_I2C3_SCL,
);

static UX500_PINS(mop500_pins_mcde_dpi,
	GPIO64_LCDB_DE,
	GPIO65_LCDB_HSO,
	GPIO66_LCDB_VSO,
	GPIO67_LCDB_CLK,
	GPIO70_LCD_D0,
	GPIO71_LCD_D1,
	GPIO72_LCD_D2,
	GPIO73_LCD_D3,
	GPIO74_LCD_D4,
	GPIO75_LCD_D5,
	GPIO76_LCD_D6,
	GPIO77_LCD_D7,
	GPIO153_LCD_D24,
	GPIO154_LCD_D25,
	GPIO155_LCD_D26,
	GPIO156_LCD_D27,
	GPIO157_LCD_D28,
	GPIO158_LCD_D29,
	GPIO159_LCD_D30,
	GPIO160_LCD_D31,
	GPIO161_LCD_D32,
	GPIO162_LCD_D33,
	GPIO163_LCD_D34,
	GPIO164_LCD_D35,
	GPIO165_LCD_D36,
	GPIO166_LCD_D37,
	GPIO167_LCD_D38,
	GPIO168_LCD_D39,
);

static UX500_PINS(mop500_pins_mcde_tvout,
	GPIO78_LCD_D8,
	GPIO79_LCD_D9,
	GPIO80_LCD_D10,
	GPIO81_LCD_D11,
	GPIO150_LCDA_CLK,
);

static UX500_PINS(mop500_pins_mcde_hdmi,
	GPIO69_LCD_VSI1	| PIN_INPUT_PULLUP,
);

static UX500_PINS(mop500_pins_ske,
	GPIO153_KP_I7 | PIN_INPUT_PULLDOWN | PIN_SLPM_INPUT_PULLUP,
	GPIO154_KP_I6 | PIN_INPUT_PULLDOWN | PIN_SLPM_INPUT_PULLUP,
	GPIO155_KP_I5 | PIN_INPUT_PULLDOWN | PIN_SLPM_INPUT_PULLUP,
	GPIO156_KP_I4 | PIN_INPUT_PULLDOWN | PIN_SLPM_INPUT_PULLUP,
	GPIO161_KP_I3 | PIN_INPUT_PULLDOWN | PIN_SLPM_INPUT_PULLUP,
	GPIO162_KP_I2 | PIN_INPUT_PULLDOWN | PIN_SLPM_INPUT_PULLUP,
	GPIO163_KP_I1 | PIN_INPUT_PULLDOWN | PIN_SLPM_INPUT_PULLUP,
	GPIO164_KP_I0 | PIN_INPUT_PULLDOWN | PIN_SLPM_INPUT_PULLUP,
	GPIO157_KP_O7 | PIN_INPUT_PULLUP | PIN_SLPM_OUTPUT_LOW,
	GPIO158_KP_O6 | PIN_INPUT_PULLUP | PIN_SLPM_OUTPUT_LOW,
	GPIO159_KP_O5 | PIN_INPUT_PULLUP | PIN_SLPM_OUTPUT_LOW,
	GPIO160_KP_O4 | PIN_INPUT_PULLUP | PIN_SLPM_OUTPUT_LOW,
	GPIO165_KP_O3 | PIN_INPUT_PULLUP | PIN_SLPM_OUTPUT_LOW,
	GPIO166_KP_O2 | PIN_INPUT_PULLUP | PIN_SLPM_OUTPUT_LOW,
	GPIO167_KP_O1 | PIN_INPUT_PULLUP | PIN_SLPM_OUTPUT_LOW,
	GPIO168_KP_O0 | PIN_INPUT_PULLUP | PIN_SLPM_OUTPUT_LOW,
);

/* sdi0 (removable MMC/SD/SDIO cards) */
static UX500_PINS(mop500_pins_sdi0,
	GPIO18_MC0_CMDDIR	| PIN_OUTPUT_HIGH,
	GPIO19_MC0_DAT0DIR	| PIN_OUTPUT_HIGH,
	GPIO20_MC0_DAT2DIR	| PIN_OUTPUT_HIGH,

	GPIO22_MC0_FBCLK	| PIN_INPUT_NOPULL,
	GPIO23_MC0_CLK		| PIN_OUTPUT_LOW,
	GPIO24_MC0_CMD		| PIN_INPUT_PULLUP,
	GPIO25_MC0_DAT0		| PIN_INPUT_PULLUP,
	GPIO26_MC0_DAT1		| PIN_INPUT_PULLUP,
	GPIO27_MC0_DAT2		| PIN_INPUT_PULLUP,
	GPIO28_MC0_DAT3		| PIN_INPUT_PULLUP,
);

/* sdi1 (WLAN CW1200) */
static UX500_PINS(mop500_pins_sdi1,
	GPIO208_MC1_CLK		| PIN_OUTPUT_LOW,
	GPIO209_MC1_FBCLK	| PIN_INPUT_NOPULL,
	GPIO210_MC1_CMD		| PIN_INPUT_PULLUP,
	GPIO211_MC1_DAT0	| PIN_INPUT_PULLUP,
	GPIO212_MC1_DAT1	| PIN_INPUT_PULLUP,
	GPIO213_MC1_DAT2	| PIN_INPUT_PULLUP,
	GPIO214_MC1_DAT3	| PIN_INPUT_PULLUP,
);

/* sdi2 (POP eMMC) */
static UX500_PINS(mop500_pins_sdi2,
	GPIO128_MC2_CLK		| PIN_OUTPUT_LOW,
	GPIO129_MC2_CMD		| PIN_INPUT_PULLUP,
	GPIO130_MC2_FBCLK	| PIN_INPUT_NOPULL,
	GPIO131_MC2_DAT0	| PIN_INPUT_PULLUP,
	GPIO132_MC2_DAT1	| PIN_INPUT_PULLUP,
	GPIO133_MC2_DAT2	| PIN_INPUT_PULLUP,
	GPIO134_MC2_DAT3	| PIN_INPUT_PULLUP,
	GPIO135_MC2_DAT4	| PIN_INPUT_PULLUP,
	GPIO136_MC2_DAT5	| PIN_INPUT_PULLUP,
	GPIO137_MC2_DAT6	| PIN_INPUT_PULLUP,
	GPIO138_MC2_DAT7	| PIN_INPUT_PULLUP,
);

/* sdi4 (PCB eMMC) */
static UX500_PINS(mop500_pins_sdi4,
	GPIO197_MC4_DAT3	| PIN_INPUT_PULLUP,
	GPIO198_MC4_DAT2	| PIN_INPUT_PULLUP,
	GPIO199_MC4_DAT1	| PIN_INPUT_PULLUP,
	GPIO200_MC4_DAT0	| PIN_INPUT_PULLUP,
	GPIO201_MC4_CMD		| PIN_INPUT_PULLUP,
	GPIO202_MC4_FBCLK	| PIN_INPUT_NOPULL,
	GPIO203_MC4_CLK		| PIN_OUTPUT_LOW,
	GPIO204_MC4_DAT7	| PIN_INPUT_PULLUP,
	GPIO205_MC4_DAT6	| PIN_INPUT_PULLUP,
	GPIO206_MC4_DAT5	| PIN_INPUT_PULLUP,
	GPIO207_MC4_DAT4	| PIN_INPUT_PULLUP,
);

/* USB */
static UX500_PINS(mop500_pins_usb,
	GPIO256_USB_NXT,
	GPIO257_USB_STP		| PIN_OUTPUT_HIGH,
	GPIO258_USB_XCLK,
	GPIO259_USB_DIR,
	GPIO260_USB_DAT7,
	GPIO261_USB_DAT6,
	GPIO262_USB_DAT5,
	GPIO263_USB_DAT4,
	GPIO264_USB_DAT3,
	GPIO265_USB_DAT2,
	GPIO266_USB_DAT1,
	GPIO267_USB_DAT0,
);

/* SPI2 */
static UX500_PINS(mop500_pins_spi2,
		GPIO216_GPIO    | PIN_OUTPUT_HIGH,
		GPIO218_SPI2_RXD        | PIN_INPUT_PULLDOWN,
		GPIO215_SPI2_TXD        | PIN_OUTPUT_LOW,
		GPIO217_SPI2_CLK        | PIN_OUTPUT_LOW,
);

static struct ux500_pin_lookup mop500_pins[] = {
	PIN_LOOKUP("mcde-dpi", &mop500_pins_mcde_dpi),
	PIN_LOOKUP("mcde-tvout", &mop500_pins_mcde_tvout),
	PIN_LOOKUP("av8100-hdmi", &mop500_pins_mcde_hdmi),
	PIN_LOOKUP("uart0", &mop500_pins_uart0),
	PIN_LOOKUP("nmk-i2c.0", &mop500_pins_i2c0),
	PIN_LOOKUP("nmk-i2c.1", &mop500_pins_i2c1),
	PIN_LOOKUP("nmk-i2c.2", &mop500_pins_i2c2),
	PIN_LOOKUP("nmk-i2c.3", &mop500_pins_i2c3),
	PIN_LOOKUP("ske", &mop500_pins_ske),
	PIN_LOOKUP("sdi0", &mop500_pins_sdi0),
	PIN_LOOKUP("sdi1", &mop500_pins_sdi1),
	PIN_LOOKUP("sdi2", &mop500_pins_sdi2),
	PIN_LOOKUP("sdi4", &mop500_pins_sdi4),
	PIN_LOOKUP("ab8500-usb.0", &mop500_pins_usb),
	PIN_LOOKUP("spi2", &mop500_pins_spi2),
};

/*
 * Sleep pin configuration for u8500 platform.
 * If another HW is used the GPIO's must be configured
 * correctly when entering sleep for optimal power
 * consumption.
 */
static pin_cfg_t mop500_pins_common_power_save_bank0[] = {
	GPIO0_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO1_GPIO | PIN_SLPM_OUTPUT_HIGH  | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO2_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO3_GPIO | PIN_SLPM_DIR_OUTPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO4_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO5_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO6_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO7_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO8_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO9_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO10_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO11_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO12_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO13_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO14_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO15_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO16_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO17_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO18_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO19_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO20_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO21_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO22_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO23_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO24_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO25_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO26_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO27_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO28_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO29_U2_RXD | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO30_U2_TXD | PIN_SLPM_DIR_OUTPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO31_U2_CTSn | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank0_href60[] = {
	GPIO0_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO1_GPIO | PIN_SLPM_OUTPUT_HIGH  | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO2_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO3_GPIO | PIN_SLPM_DIR_OUTPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO4_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO5_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO6_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO7_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO8_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO9_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO10_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO11_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO12_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO13_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO14_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO15_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO16_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO17_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO18_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO19_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO20_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO21_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO22_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO23_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO24_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO25_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO26_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO27_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO28_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO29_U2_RXD | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO30_U2_TXD | PIN_SLPM_DIR_OUTPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO31_U2_CTSn | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank1[] = {
	GPIO32_U2_RTSn | PIN_SLPM_DIR_OUTPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO33_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO34_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO35_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO36_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank2[] = {
	GPIO64_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO65_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO66_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO67_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO68_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO69_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO70_STMAPE_CLK | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO71_STMAPE_DAT3 | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE  | PIN_SLPM_PDIS_DISABLED,

	GPIO72_STMAPE_DAT2 | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO73_STMAPE_DAT1 | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO74_STMAPE_DAT0 | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO75_U2_RXD | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO76_U2_TXD | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO77_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO78_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO79_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO80_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO81_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO82_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO83_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO84_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO85_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO86_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO87_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO88_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO89_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO90_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO91_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO92_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO93_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO94_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO95_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank2_href60[] = {
	GPIO64_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO65_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO66_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO67_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO68_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO69_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO70_STMAPE_CLK | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO71_STMAPE_DAT3 | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO72_STMAPE_DAT2 | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO73_STMAPE_DAT1 | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO74_STMAPE_DAT0 | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_DISABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO75_U2_RXD | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO76_U2_TXD | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO77_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO78_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO79_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO80_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO81_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO82_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO83_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO84_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO85_GPIO,
	GPIO86_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO87_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO88_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO89_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO90_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO91_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO92_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO93_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO94_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO95_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank3[] = {
	GPIO96_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO97_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank4[] = {
	GPIO128_MC2_CLK | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO129_MC2_CMD | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO130_MC2_FBCLK | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO131_MC2_DAT0 | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO132_MC2_DAT1 | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO133_MC2_DAT2 | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO134_MC2_DAT3 | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO135_MC2_DAT4 | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO136_MC2_DAT5 | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO137_MC2_DAT6 | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO138_MC2_DAT7 | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO139_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO140_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO141_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO142_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO143_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO144_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO145_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO146_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO147_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO148_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO149_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO150_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO151_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO152_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO153_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO154_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO155_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO156_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO157_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO158_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO159_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank5[] = {
	GPIO160_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO161_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO162_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO163_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO164_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO165_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO166_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO167_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO168_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO169_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO170_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO171_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank5_href60[] = {
	GPIO160_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO161_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO162_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO163_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO164_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO165_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO166_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO167_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO168_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO169_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO170_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO171_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank6[] = {
	GPIO192_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO193_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO194_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO195_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO196_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO197_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO198_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO199_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO200_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO201_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO202_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO203_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO204_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO205_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO206_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO207_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO208_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO209_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO210_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO211_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO212_GPIO,
	GPIO213_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO214_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO215_GPIO,

	GPIO216_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO217_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO218_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO219_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO220_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO221_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO222_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO223_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank6_href60[] = {
	GPIO192_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO193_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO194_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO195_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO196_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO197_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO198_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO199_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO200_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO201_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO202_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO203_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO204_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO205_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO206_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO207_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO208_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO209_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO210_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO211_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO212_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO213_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO214_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO215_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO216_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO217_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO218_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO219_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO220_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO221_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO222_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO223_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank7[] = {
	GPIO224_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO225_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO226_GPIO | PIN_SLPM_DIR_OUTPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO227_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO228_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO229_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO230_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank7_href60[] = {
	GPIO224_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO225_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO226_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO227_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,

	GPIO228_GPIO | PIN_SLPM_OUTPUT_LOW | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO229_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO230_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
};

static pin_cfg_t mop500_pins_common_power_save_bank8[] = {
	GPIO256_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO257_GPIO | PIN_SLPM_OUTPUT_HIGH | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_DISABLED,
	GPIO258_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO259_GPIO | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,

	GPIO260_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO261_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO262_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO263_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,

	GPIO264_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO265_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO266_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
	GPIO267_GPIO | PIN_SLPM_DIR_INPUT | PIN_SLPM_WAKEUP_ENABLE | PIN_SLPM_PDIS_ENABLED,
};

void mop500_pins_suspend_force(void)
{
	if (machine_is_hrefv60())
		sleep_pins_config_pm(mop500_pins_common_power_save_bank0_href60,
			ARRAY_SIZE(mop500_pins_common_power_save_bank0_href60));
	else
		sleep_pins_config_pm(mop500_pins_common_power_save_bank0,
			ARRAY_SIZE(mop500_pins_common_power_save_bank0));

	sleep_pins_config_pm(mop500_pins_common_power_save_bank1,
		ARRAY_SIZE(mop500_pins_common_power_save_bank1));

	if (machine_is_hrefv60())
		sleep_pins_config_pm(mop500_pins_common_power_save_bank2_href60,
			ARRAY_SIZE(mop500_pins_common_power_save_bank2_href60));
	else
		sleep_pins_config_pm(mop500_pins_common_power_save_bank2,
			ARRAY_SIZE(mop500_pins_common_power_save_bank2));

	sleep_pins_config_pm(mop500_pins_common_power_save_bank3,
		ARRAY_SIZE(mop500_pins_common_power_save_bank3));

	sleep_pins_config_pm(mop500_pins_common_power_save_bank4,
		ARRAY_SIZE(mop500_pins_common_power_save_bank4));

	if (machine_is_hrefv60())
		sleep_pins_config_pm(mop500_pins_common_power_save_bank5_href60,
			ARRAY_SIZE(mop500_pins_common_power_save_bank5_href60));
	else
		sleep_pins_config_pm(mop500_pins_common_power_save_bank5,
			ARRAY_SIZE(mop500_pins_common_power_save_bank5));

	if (machine_is_hrefv60())
		sleep_pins_config_pm(mop500_pins_common_power_save_bank6_href60,
			ARRAY_SIZE(mop500_pins_common_power_save_bank6_href60));
	else
		sleep_pins_config_pm(mop500_pins_common_power_save_bank6,
			ARRAY_SIZE(mop500_pins_common_power_save_bank6));

	if (machine_is_hrefv60())
		sleep_pins_config_pm(mop500_pins_common_power_save_bank7_href60,
			ARRAY_SIZE(mop500_pins_common_power_save_bank7_href60));
	else
		sleep_pins_config_pm(mop500_pins_common_power_save_bank7,
			ARRAY_SIZE(mop500_pins_common_power_save_bank7));

	sleep_pins_config_pm(mop500_pins_common_power_save_bank8,
		ARRAY_SIZE(mop500_pins_common_power_save_bank8));
}

/*
 * This function is called to force gpio power save
 * mux settings during suspend.
 * This is a temporary solution until all drivers are
 * controlling their pin settings when in inactive mode.
 */
static void mop500_pins_suspend_force_mux(void)
{
	sleep_pins_config_pm_mux(mop500_pins_common_power_save_bank0,
		ARRAY_SIZE(mop500_pins_common_power_save_bank0));

	sleep_pins_config_pm_mux(mop500_pins_common_power_save_bank1,
		ARRAY_SIZE(mop500_pins_common_power_save_bank1));

	sleep_pins_config_pm_mux(mop500_pins_common_power_save_bank2,
		ARRAY_SIZE(mop500_pins_common_power_save_bank2));

	sleep_pins_config_pm_mux(mop500_pins_common_power_save_bank3,
		ARRAY_SIZE(mop500_pins_common_power_save_bank3));

	sleep_pins_config_pm_mux(mop500_pins_common_power_save_bank4,
		ARRAY_SIZE(mop500_pins_common_power_save_bank4));

	sleep_pins_config_pm_mux(mop500_pins_common_power_save_bank5,
		ARRAY_SIZE(mop500_pins_common_power_save_bank5));

	sleep_pins_config_pm_mux(mop500_pins_common_power_save_bank6,
		ARRAY_SIZE(mop500_pins_common_power_save_bank6));

	sleep_pins_config_pm_mux(mop500_pins_common_power_save_bank7,
		ARRAY_SIZE(mop500_pins_common_power_save_bank7));

	sleep_pins_config_pm_mux(mop500_pins_common_power_save_bank8,
		ARRAY_SIZE(mop500_pins_common_power_save_bank8));
}

/*
 * passing "pinsfor=" in kernel cmdline allows for custom
 * configuration of GPIOs on u8500 derived boards.
 */
static int __init early_pinsfor(char *p)
{
	pinsfor = PINS_FOR_DEFAULT;

	if (strcmp(p, "u9500-21") == 0)
		pinsfor = PINS_FOR_U9500;

	return 0;
}
early_param("pinsfor", early_pinsfor);

int pins_for_u9500(void)
{
	if (pinsfor == PINS_FOR_U9500)
		return 1;

	return 0;
}

static UX500_PINS(mop500_offchip_gpio_cfg,
	/*
	 * Workaround for auto shutdown of 3.2MHz oscillator during
	 * deep sleep. APESPICSn/GPIO37 must be floating on the board
	 * to use this fix.
	 */
	AB8500_PIN_GPIO37 | PIN_OUTPUT_HIGH,
);

void __init mop500_pins_init(void)
{
	nmk_config_pins(mop500_pins_common,
				ARRAY_SIZE(mop500_pins_common));


	if (machine_is_hrefv60())
		nmk_config_pins(mop500_pins_hrefv60,
				ARRAY_SIZE(mop500_pins_hrefv60));
	else
		nmk_config_pins(mop500_pins_default,
				ARRAY_SIZE(mop500_pins_default));

	ux500_pins_add(mop500_pins, ARRAY_SIZE(mop500_pins));

	switch (pinsfor) {
	case PINS_FOR_U9500:
		nmk_config_pins(u9500_pins, ARRAY_SIZE(u9500_pins));
		break;

	case PINS_FOR_DEFAULT:
		nmk_config_pins(u8500_pins, ARRAY_SIZE(u8500_pins));
	default:
		break;
	}

	suspend_set_pins_force_fn(mop500_pins_suspend_force,
				  mop500_pins_suspend_force_mux);
}

static int __init mop500_offchip_gpio_init(void)
{
	if (machine_is_hrefv60())
                ux500_offchip_gpio_init(&mop500_offchip_gpio_cfg);

	return 0;
}
/* Let gpio chip drivers initialize. */
late_initcall(mop500_offchip_gpio_init);