/***************************************************************************//**
 *   @file   max22200.h
 *   @brief  Header file of MAX22200 Driver.
 *   @author Radu Sabau (radu.sabau@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#ifndef _MAX22200_H
#define _MAX22200_H

#include <stdint.h>
#include <stdbool.h>
#include "no_os_gpio.h"
#include "no_os_spi.h"
#include "no_os_util.h"

#define MAX22200_FRAME_SIZE		4

#define MAX22200_CHANNELS		8

#define MAX22200_STATUS_REG		0x00
#define MAX22200_CFG_CH(x)		(0x01 + (x))
#define MAX22200_FAULT_REG		0x09
#define MAX22200_CFG_DPM_REG		0x0A

#define MAX22200_RW_MASK		NO_OS_BIT(7)
#define MAX22200_RES_MASK		NO_OS_GENMASK(6, 5)
#define MAX22200_ADDR_MASK		NO_OS_GENMASK(4, 1)
#define MAX22200_ONE_BYTE_MASK		NO_OS_BIT(0)

/* Status Register Masks */
#define MAX22200_ONCH_MASK		NO_OS_GENMASK(31, 24)
#define MAX22200_STATUS_FAULT_MASK	NO_OS_GENMASK(23, 16)
#define MAX22200_STATUS_MODE_MASK	NO_OS_GENMASK(15, 8)
#define MAX22200_CH_MODE_MASK(x)	NO_OS_GENMASK(15 - (2 * (x)), 14 - (2 * (x)))
#define MAX22200_STATUS_FLAG_MASK	NO_OS_GENMASK(7, 0)
#define MAX22200_ACTIVE_MASK		NO_OS_BIT(0)

/* CFG CH Register Masks */
#define MAX22200_HFS_MASK		NO_OS_BIT(31)
#define MAX22200_HOLD_MASK		NO_OS_GENMASK(30, 24)
#define MAX22200_TRGNSP_IO_MASK		NO_OS_BIT(23)
#define MAX22200_HIT_MASK		NO_OS_GENMASK(22, 16)
#define MAX22200_HIT_T_MASK		NO_OS_GENMASK(15, 8)
#define MAX22200_VDRNCDR_MASK		NO_OS_BIT(7)
#define MAX22200_HSNLS_MASK		NO_OS_BIT(6)
#define MAX22200_FREQ_CFG_MASK		NO_OS_GENMASK(5, 4)
#define MAX22200_SRC_MASK		NO_OS_BIT(3)
#define MAX22200_OL_EN			NO_OS_BIT(2)
#define MAX22200_DPM_EN			NO_OS_BIT(1)
#define MAX22200_HHF_EN			NO_OS_BIT(0)

/* CFG DPM Register Masks */
#define MAX22200_DPM_ISTART_MASK	NO_OS_GENMASK(14, 8)
#define MAX22200_DPM_TDEB_MASK		NO_OS_GENMASK(7, 4)
#define MAX22200_DPM_IPTH_MASK		NO_OS_GENMASK(3, 0)



enum max22200_ch_op_mode {
	MAX22200_INDEPENDENT_MODE,
	MAX22200_PARALLEL_MODE,
	MAX22200_HALF_BRIDGE_MODE,
};

enum max22200_freq_cfg {
	MAX22200_FREQMAIN_DIV_4,
	MAX22200_FREQMAIN_DIV_3,
	MAX22200_FREQMAIN_DIV_2,
	MAX22200_FREQMAIN
};

struct max22200_init_param {
	struct no_os_spi_init_param *comm_param;
	struct no_os_gpio_init_param *fault_param;
	struct no_os_gpio_init_param *enable_param;
	struct no_os_gpio_init_param *cmd_param;
	struct no_os_gpio_init_param *trig_param;
	enum max22200_ch_op_mode ch_config[MAX22200_CHANNELS];
};

struct max22200_desc {
	struct no_os_spi_desc *comm_desc;
	struct no_os_gpio_desc *fault_desc;
	struct no_os_gpio_desc *enable_desc;
	struct no_os_gpio_desc *cmd_desc;
	struct no_os_gpio_desc *trig_desc;
	uint8_t buff[MAX22200_FRAME_SIZE];
	enum max22200_ch_op_mode ch_config[MAX22200_CHANNELS];
};

int max22200_reg_read(struct max22200_desc *, uint32_t, uint32_t *, bool);

int max22200_reg_write(struct max22200_desc *, uint32_t, uint32_t, bool);

int max22200_reg_update(struct max22200_desc *, uint32_t, uint32_t, uint32_t,
			bool);

/* Specific API(set_ch_cfg) */
int max22200_set_ch_cfg(struct max22200_desc *, uint32_t,
			enum max22200_ch_op_mode, enum max22200_freq_cfg,
			uint32_t, bool, bool);

int max22200_init(struct max22200_desc **, struct max22200_init_param *);

int max22200_remove(struct max22200_desc *);

#endif /* _MAX22200_H */