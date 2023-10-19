/***************************************************************************//**
 *   @file   max22200.c
 *   @brief  Source file of MAX22200 Driver.
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "max22200.h"
#include "no_os_util.h"
#include "no_os_alloc.h"
#include "no_os_delay.h"

int max22200_reg_read(struct max22200_desc *desc, uint32_t reg, uint32_t *val,
		      bool one_byte)
{
	struct no_os_spi_msg xfer = {
		.tx_buff = desc->buff,
		.rx_buff = desc->buff,
		.bytes_number = 1,
		.cs_change = 1,
		.cs_delay_first = 4,
		.cs_delay_last = 4
	};
	int ret;

	desc->buff[0] = no_os_field_prep(MAX22200_RW_MASK, 0) |
			no_os_field_prep(MAX22200_RES_MASK, 0) |
			no_os_field_prep(MAX22200_ADDR_MASK, reg) |
			no_os_field_prep(MAX22200_ONE_BYTE_MASK, one_byte);

	ret = no_os_gpio_set_value(desc->cmd_desc, NO_OS_GPIO_HIGH);
	if (ret)
		return ret;

	ret = no_os_spi_transfer(desc->comm_desc, &xfer, 1);
	if (ret)
		return ret;

	if (!one_byte)
		xfer.bytes_number = MAX22200_FRAME_SIZE;


	ret = no_os_gpio_set_value(desc->cmd_desc, NO_OS_GPIO_LOW);
	if (ret)
		return ret;
	ret = no_os_spi_transfer(desc->comm_desc, &xfer, 1);
	if (ret)
		return ret;

	if (one_byte) {
		*val = desc->buff[0];
		return 0;
	}

	*val = no_os_get_unaligned_be32(desc->buff);

	return 0;
}

int max22200_reg_write(struct max22200_desc *desc, uint32_t reg, uint32_t val,
		       bool one_byte)
{
	struct no_os_spi_msg xfer = {
		.tx_buff = desc->buff,
		.bytes_number = 1,
		.cs_change = 1,
	};
	int ret;
	uint8_t cmd_val;

	desc->buff[0] = no_os_field_prep(MAX22200_RW_MASK, 1) |
			no_os_field_prep(MAX22200_RES_MASK, 0) |
			no_os_field_prep(MAX22200_ADDR_MASK, reg) |
			no_os_field_prep(MAX22200_ONE_BYTE_MASK, one_byte);

	ret = no_os_gpio_set_value(desc->cmd_desc, NO_OS_GPIO_HIGH);
	if (ret)
		return ret;

	ret = no_os_spi_transfer(desc->comm_desc, &xfer, 1);
	if (ret)
		return ret;

	if (!one_byte) {
		xfer.bytes_number = MAX22200_FRAME_SIZE;
		no_os_put_unaligned_be32(val, desc->buff);
	} else
		desc->buff[0] = val;

	ret = no_os_gpio_set_value(desc->cmd_desc, NO_OS_GPIO_LOW);
	if (ret)
		return ret;

	return no_os_spi_transfer(desc->comm_desc, &xfer, 1);
}

int max22200_reg_update(struct max22200_desc *desc, uint32_t reg, uint32_t mask,
			uint32_t val, bool one_byte)
{
	int ret;
	uint32_t reg_val = 0;

	ret = max22200_reg_read(desc, reg, &reg_val, one_byte);
	if (ret)
		return ret;

	reg_val &= ~mask;
	reg_val |= mask & val;
	return max22200_reg_write(desc, reg, reg_val, one_byte);
}

int max22200_set_ch_cfg(struct max22200_desc *desc, uint32_t ch,
			enum max22200_ch_op_mode ch_config,
			enum max22200_freq_cfg freq_cfg,
			uint32_t hfs, bool voltage_drive, bool high_side)
{
	uint32_t reg_val = 0;
	int ret;

	/* Setting the fullscale bit */
	reg_val |= no_os_field_prep(MAX22200_HFS_MASK, hfs);

	/* Setting HOLD to to respect high-side/low-side config. */
	reg_val |= no_os_field_prep(MAX22200_HOLD_MASK, 127);

	/* CH_ controlled by ONCH_SPI bit */
	reg_val |= no_os_field_prep(MAX22200_TRGNSP_IO_MASK, 0);

	/* HIT current to respect high-side/low-side config. */
	reg_val |= no_os_field_prep(MAX22200_HIT_MASK, 127);

	/* No HIT time. */
	reg_val |= no_os_field_prep(MAX22200_HIT_T_MASK, 0);

	/* Selecting current or voltage drive mode. */
	reg_val |= no_os_field_prep(MAX22200_VDRNCDR_MASK, voltage_drive);

	/* Selecting high-side/low-side config. */
	reg_val |= no_os_field_prep(MAX22200_HSNLS_MASK, high_side);

	/* Selecting chopping frequency. */
	reg_val |= no_os_field_prep(MAX22200_FREQ_CFG_MASK, freq_cfg);

	/* Setting OUT transitions to be slew-rate controlled if low-side
	   config. */
	reg_val |= no_os_field_prep(MAX22200_SRC_MASK, 0);

	/* Enabling open-load detection. */
	reg_val |= no_os_field_prep(MAX22200_OL_EN, 1);

	/* Enabling detection of plunger movement. */
	reg_val |= no_os_field_prep(MAX22200_DPM_EN, 1);

	/* Enabling HIT current check. */
	reg_val |= no_os_field_prep(MAX22200_HHF_EN, 1);

	return max22200_reg_write(desc, MAX22200_CFG_CH(ch), reg_val, false);
}

int max22200_init(struct max22200_desc **desc,
		  struct max22200_init_param *init_param)
{
	struct max22200_desc *descriptor;
	uint32_t reg_val;
	uint32_t status_reg = 0;
	int ret, i;

	descriptor = no_os_calloc(1, sizeof(*descriptor));
	if (!descriptor)
		return -ENOMEM;

	ret = no_os_spi_init(&descriptor->comm_desc, init_param->comm_param);
	if (ret)
		goto err;

	ret = no_os_gpio_get(&descriptor->enable_desc,
			     init_param->enable_param);
	if (ret)
		goto spi_err;

	ret = no_os_gpio_direction_output(descriptor->enable_desc,
					  NO_OS_GPIO_HIGH);
	if (ret)
		goto enable_err;

	/* Time between enable pin set and device power-up. */
	no_os_udelay(500);

	ret = no_os_gpio_get(&descriptor->cmd_desc,
			     init_param->cmd_param);
	if (ret)
		goto enable_err;

	ret = no_os_gpio_direction_output(descriptor->cmd_desc,
					  NO_OS_GPIO_HIGH);
	if (ret)
		goto cmd_err;

	ret = no_os_gpio_get_optional(&descriptor->trig_desc,
				      init_param->trig_param);
	if (ret)
		goto cmd_err;

	/* External trigger if used, is set logic LOW at initialization.
	   Can be set logic HIGH in case of choosing to use the external
	   trigger for Full-Bridge Mode. */
	if (descriptor->trig_desc) {
		ret = no_os_gpio_direction_output(descriptor->trig_desc,
						  NO_OS_GPIO_LOW);
		if (ret)
			goto trig_err;
	}

	ret = no_os_gpio_get_optional(&descriptor->fault_desc,
				      init_param->fault_param);
	if (ret)
		goto trig_err;

	if (descriptor->fault_desc) {
		ret = no_os_gpio_direction_input(descriptor->fault_desc);
		if (ret)
			goto fault_err;
	}

	ret = max22200_reg_read(descriptor, MAX22200_STATUS_REG, &reg_val,
				true);
	if (ret)
		goto fault_err;

	/* Getting channel mode info from init_param */
	for (i = 0; i < MAX22200_CHANNELS; i += 2) {
		*(descriptor->ch_config + i) = *(init_param->ch_config + i);
		*(descriptor->ch_config + i + 1) = *(init_param->ch_config + i);
		status_reg |= no_os_field_prep(MAX22200_CH_MODE_MASK(i),
					       *(init_param->ch_config + i));
	}
	/* Writing to the status register the mode of the channels and
	   setting active bit to 1. */
	status_reg |= no_os_field_prep(MAX22200_ACTIVE_MASK, 1);
	ret = max22200_reg_update(descriptor, MAX22200_STATUS_REG,
				  MAX22200_ONCH_MASK | MAX22200_ACTIVE_MASK |
				  MAX22200_STATUS_MODE_MASK,
				  status_reg, false);
	if (ret)
		goto fault_err;

	no_os_udelay(2500);

	/* Configuring channels at startup. */
	for (i = 0; i < MAX22200_CHANNELS; i++) {
		ret = max22200_set_ch_cfg(descriptor, i,
					  MAX22200_INDEPENDENT_MODE,
					  MAX22200_FREQMAIN_DIV_4, 0, true,
					  true);
		if (ret)
			goto fault_err;
	}

	/* Reading from the status register again. */
	ret = max22200_reg_read(descriptor, MAX22200_STATUS_REG, &reg_val,
				false);
	if (ret)
		goto fault_err;

	*desc = descriptor;

	return 0;

fault_err:
	no_os_gpio_remove(descriptor->fault_desc);
trig_err:
	no_os_gpio_remove(descriptor->trig_desc);
cmd_err:
	no_os_gpio_remove(descriptor->cmd_desc);
enable_err:
	no_os_gpio_remove(descriptor->enable_desc);
spi_err:
	no_os_spi_remove(descriptor->comm_desc);
err:
	no_os_free(descriptor);
	return ret;
}

int max22200_remove(struct max22200_desc *desc)
{
	int ret, i;

	if (!desc)
		return -ENODEV;

	no_os_spi_remove(desc->comm_desc);

	no_os_gpio_remove(desc->enable_desc);
	no_os_gpio_remove(desc->cmd_desc);
	no_os_gpio_remove(desc->trig_desc);
	no_os_gpio_remove(desc->fault_desc);

	no_os_free(desc);

	return 0;
}
