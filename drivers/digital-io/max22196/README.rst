MAX22196 no-OS driver
=====================

Supported Devices
-----------------

`MAX22196 <https://www.analog.com/MAX22196>`

Overview
--------

The MAX22196 is an industrial octal digital input that translates eight
industrial 24V or TTL level inputs to logic level outputs. 
The device has a serial interface allowing configuration and reading of 
serialized data through SPI.

The input channels are individually configurable as sinking (p-type) or 
sourcing (n-type) inputs. Current limiters on each digital input minimize 
power dissipation while ensuring compliance with the IEC 61131-2 standard. 
With a single current-setting resistor, the inputs are individually 
configurable for Type 1/3, Type 2, TTL or HTL (high-impedance 24V levels). 
The current sinks or sources can be individually disabled.

Applications
------------
* Programmable Logic Controllers
* Factory Automation
* Process Control

MAX22196 Device Configuration
-----------------------------

In order to be able to use the device, you will have to provide the support
for the communication protocol (SPI).

The first API to be called is **max22196_init**. Make sure that it returns 0,
which means that the driver was initialized correctly.

Channel configuration
---------------------

Each channel can be configured in source mode or sink mode with the 
**max22196_set_mode** API.
More than that each channel's voltage threshold state and sink/source current
can be set with **max22196_chan_cfg** API, as well as the channel counter
that can be changed for both MSB and LSB Byte's with the help of
**max22196_set_chan_cnt**, or read with **max22196_get_chan_cnt**.

Filter configuration
--------------------

Each channel also has a filter that can be enabled/disabled, but also have
a delay attached to it. The filter data can be set with the help of
**max22196_filter_set**, and read with **max22196_filter_get** API.

Global configuration
--------------------

In case of wanting to configure the global_cfg register, it can be done
using the **max22196_global_cfg** API.

Fault mask configuration
------------------------

Any fault mask can be separately configured with **max22196_fault_mask_set**
API, and also read with **max22196_fault_mask_get** API.


MAX22196 Driver Initialization Example
--------------------------------------

.. code-block:: bash

	struct max22196_desc *max22196;
	struct no_os_spi_init_param spi_ip = {
		.device_id = 0,
		.extra = &max22196_spi_extra,
		.max_speed_hz = 100000,
		.platform_ops = &max_spi_ops,
		.chip_select = 0,
	};
	struct max22196_init_param max22196_ip = {
		.chip_address = 0,
		.comm_desc = &spi_ip,
	};

	ret = max22196_init(&max22196, &max22196_ip);
	if (ret)
		goto error;

MAX22196 no-OS IIO support
--------------------------

The MAX22196 IIO driver comes on top of the MAX22196 driver and offers support
for interfacing IIO clients through IIO lib.

MAX22196 IIO Device Configuration
---------------------------------

Channel Attributes
------------------

MAX22196 has a total of 10 channel attributes :

* raw - the state of the cannel.
* offset - always 0.
* scale - always 1.
* filter_enable - 0 or 1 (depending on the state of the filter).
* filter_delay - The filter's delay value if it is enabled (otherwise it is 50us).
* filter_delay_available - List of delay available values.
* di_mode - Digital Input mode, is the mode in which each channel is configured.
* di_mode_available - List of digital input modes (Sink/Source).
* current_source - Current source to be selected for each channel.
* current_source_available - Current sources available for selection.

Device Channels
---------------

MAX22196 has a specific API, **max22196_iio_setup_channels** for configuring the
channels at the initialization, therefore the channels can be configured as  
enabled/disabled and attributes are assigned to each channel (if enabled).

MAX22196 IIO Driver Initialization Example
------------------------------------------

.. code-block:: bash

	struct max22196_iio_desc *max22196_iio_desc;
	struct max22196_iio_desc_init_param max22196_iio_ip = {
		.max22196_init_param = &max22196_ip,
		.chans_enabled = {
			true, true, true, false, false, false, false, false
		},
	};

	struct iio_app_desc *app;
	struct iio_app_init_param app_init_param = { 0 };

	ret = max22196_iio_init(&max22196_iio_desc, &max22196_iio_ip);
	if (ret)
		goto error;

	struct iio_app_device iio_devices[] = {
		{
			.name = "max22196",
			.dev = max22196_iio_desc,
			.dev_descriptor = max22196_iio_desc->iio_dev,
		},
	};

	app_init_param.devices = iio_devices;
	app_init_param.nb_devices = NO_OS_ARRAY_SIZE(iio_devices);
	app_init_param.uart_init_params = max22196_uart_ip;

	ret = iio_app_init(&app, app_init_param);
	if (ret)
		goto app_error;

	return iio_app_run(app);
