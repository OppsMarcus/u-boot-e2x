/*
 * board/renesas/alex/alex.c
 *     This file is ale6/ale4 board support.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <common.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/rmobile.h>
#include <netdev.h>
#include <i2c.h>
#include "alex.h"

DECLARE_GLOBAL_DATA_PTR;

#define s_init_wait(cnt) \
		({	\
			volatile u32 i = 0x10000 * cnt;	\
			while (i > 0)	\
				i--;	\
		})

void s_init(void)
{
	struct r8a7794x_rwdt *rwdt = (struct r8a7794x_rwdt *)RWDT_BASE;
	struct r8a7794x_swdt *swdt = (struct r8a7794x_swdt *)SWDT_BASE;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

}

#define TMU1_MSTP111    (1 << 11)

#define SCIF0_MSTP721	(1 << 21)

#define ETHER_MSTP813	(1 << 13)

int board_early_init_f(void)
{
	u32 val;

	/* TMU1 */
	val = readl(MSTPSR1);
	val &= ~TMU1_MSTP111;
	writel(val, SMSTPCR1);

	/* SCIF0 */
	val = readl(MSTPSR7);
	val &= ~SCIF0_MSTP721;
	writel(val, SMSTPCR7);

	/* ETHER */
	val = readl(MSTPSR8);
	val &= ~ETHER_MSTP813;
	writel(val, SMSTPCR8);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	u32 val;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = ALEX_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7794x_pinmux_init();

	/* ETHER Enable */
	gpio_request(GPIO_FN_ETH_CRS_DV, NULL);
	gpio_request(GPIO_FN_ETH_RX_ER, NULL);
	gpio_request(GPIO_FN_ETH_RXD0, NULL);
	gpio_request(GPIO_FN_ETH_RXD1, NULL);
	gpio_request(GPIO_FN_ETH_LINK, NULL);
	gpio_request(GPIO_FN_ETH_REF_CLK, NULL);
	gpio_request(GPIO_FN_ETH_MDIO, NULL);
	gpio_request(GPIO_FN_ETH_TXD1, NULL);
	gpio_request(GPIO_FN_ETH_TX_EN, NULL);
	gpio_request(GPIO_FN_ETH_MAGIC, NULL);
	gpio_request(GPIO_FN_ETH_TXD0, NULL);
	gpio_request(GPIO_FN_ETH_MDC, NULL);

	gpio_request(GPIO_GP_5_16, NULL);
	gpio_direction_output(GPIO_GP_5_16, 0);

	sh_timer_init();

	gpio_request(GPIO_GP_5_0, NULL);	/* PHY_RST */
	gpio_direction_output(GPIO_GP_5_0, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_5_0, 1);
	udelay(1);

	/* wait 5ms */
	udelay(5000);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = -ENODEV;
	u32 val;
	unsigned char enetaddr[6];

#ifdef CONFIG_SH_ETHER
	ret = sh_eth_initialize(bis);
	if (!eth_getenv_enetaddr("ethaddr", enetaddr))
		return ret;

	/* Set Mac address */
	val = enetaddr[0] << 24 | enetaddr[1] << 16 |
	    enetaddr[2] << 8 | enetaddr[3];
	writel(val, 0xEE7003C0);

	val = enetaddr[4] << 8 | enetaddr[5];
	writel(val, 0xEE7003C8);
#endif

	return ret;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_RMOBILE_BOARD_STRING
};

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = ALEX_SDRAM_BASE;
	gd->bd->bi_dram[0].size = ALEX_SDRAM_SIZE;
}

int board_late_init(void)
{
	return 0;
}

void reset_cpu(ulong addr)
{
}

#define TSTR1   4
#define TSTR1_STR3      0x1

void arch_preboot_os()
{
	u32 val;
	int i;

	/* stop TMU1 */
	val = readb(TMU_BASE + TSTR1);
	val &= ~TSTR1_STR3;
	writeb(val, TMU_BASE + TSTR1);

	/* TMU1 */
	val = readl(MSTPSR1);
	val |= TMU1_MSTP111;
	writel(val, SMSTPCR1);
}
