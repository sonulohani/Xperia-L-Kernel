/*
 *  Jack abstraction layer
 *
 *  Copyright 2008 Wolfson Microelectronics
 *  Copyright (C) 2012 Sony Mobile Communications AB.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/input.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <sound/jack.h>
#include <sound/core.h>
#include <linux/switch.h>  // AUD_MOD

static int jack_switch_types[] = {
	SW_HEADPHONE_INSERT,
	SW_MICROPHONE_INSERT,
	SW_LINEOUT_INSERT,
	SW_JACK_PHYSICAL_INSERT,
	SW_VIDEOOUT_INSERT,
	SW_LINEIN_INSERT,
	SW_HPHL_OVERCURRENT,
	SW_HPHR_OVERCURRENT,
	SW_UNSUPPORT_INSERT,
};

// AUD_MOD start
static ssize_t simple_remote_print_name(struct switch_dev *swdev, char *buf)
{
	//struct simple_remote_driver *jack =
	//	container_of(sdev, struct simple_remote_driver, swdev);
	printk("%s\n", __func__);

	switch (switch_get_state(swdev)) {
	case NO_DEVICE:
		printk("No Device\n");
		return sprintf(buf, "No Device\n");
	case DEVICE_HEADSET:
		printk("Headset\n");
		return sprintf(buf, "Headset\n");
	case DEVICE_HEADPHONE:
		printk("Headphone\n");
		return sprintf(buf, "Headphone\n");
	case DEVICE_UNSUPPORTED:
		printk("Unsupported\n");
		return sprintf(buf, "Unsupported\n");
	case DEVICE_UNKNOWN:
		printk("Device Unknown\n");
		return sprintf(buf, "Device Unknown\n");
	}
	return -EINVAL;
}
// AUD_MOD end

static int snd_jack_dev_free(struct snd_device *device)
{
	struct snd_jack *jack = device->device_data;

	if (jack->private_free)
		jack->private_free(jack);

	/* If the input device is registered with the input subsystem
	 * then we need to use a different deallocator. */
	if (jack->registered)
		input_unregister_device(jack->input_dev);
	else
		input_free_device(jack->input_dev);

// AUD_MOD start
	if (jack->registered)
		input_unregister_device(jack->indev_appkey);
	else
		input_free_device(jack->indev_appkey);
	switch_dev_unregister(&jack->swdev);	// SoMC Uevent Start
// end

	kfree(jack->id);
	kfree(jack);

	return 0;
}

static int snd_jack_dev_register(struct snd_device *device)
{
	struct snd_jack *jack = device->device_data;
	struct snd_card *card = device->card;
	int err, i;

// AUD_MOD start
	//if( !strcmp(jack->id, "Headset Jack"))
		
	if( !strcmp(jack->id, "Button Jack"))
		jack->indev_appkey->name = "simple_remote_appkey";
	else
		jack->indev_appkey->name = "hs_plug";
	
	/* Default to the sound card device. */
	if (!jack->indev_appkey->dev.parent)
		jack->indev_appkey->dev.parent = snd_card_get_device_link(card);

	/* Add capabilities for any keys that are enabled */
	for (i = 0; i < ARRAY_SIZE(jack->key); i++) {
		int testbit = SND_JACK_BTN_0 >> i;

		if (!(jack->type & testbit))
			continue;

		if (!jack->key[i])
			jack->key[i] = BTN_0 + i;

		input_set_capability(jack->indev_appkey, EV_KEY, jack->key[i]);
	}

	err = input_register_device(jack->indev_appkey);
	if (err != 0)
		return err;
// AUD_MOD end

	snprintf(jack->name, sizeof(jack->name), "%s %s",
		 card->shortname, jack->id);
	jack->input_dev->name = jack->name;

	/* Default to the sound card device. */
	if (!jack->input_dev->dev.parent)
		jack->input_dev->dev.parent = snd_card_get_device_link(card);

	/* Add capabilities for any keys that are enabled */
	for (i = 0; i < ARRAY_SIZE(jack->key); i++) {
		int testbit = SND_JACK_BTN_0 >> i;

		if (!(jack->type & testbit))
			continue;

		if (!jack->key[i])
			jack->key[i] = BTN_0 + i;

		input_set_capability(jack->input_dev, EV_KEY, jack->key[i]);
	}

	err = input_register_device(jack->input_dev);
	if (err == 0)
		jack->registered = 1;

	return err;
}

/**
 * snd_jack_new - Create a new jack
 * @card:  the card instance
 * @id:    an identifying string for this jack
 * @type:  a bitmask of enum snd_jack_type values that can be detected by
 *         this jack
 * @jjack: Used to provide the allocated jack object to the caller.
 *
 * Creates a new jack object.
 *
 * Returns zero if successful, or a negative error code on failure.
 * On success jjack will be initialised.
 */
int snd_jack_new(struct snd_card *card, const char *id, int type,
		 struct snd_jack **jjack)
{
	struct snd_jack *jack;
	int err;
	int i;
	static struct snd_device_ops ops = {
		.dev_free = snd_jack_dev_free,
		.dev_register = snd_jack_dev_register,
	};

	jack = kzalloc(sizeof(struct snd_jack), GFP_KERNEL);
	if (jack == NULL)
		return -ENOMEM;

	jack->id = kstrdup(id, GFP_KERNEL);

// AUD_MOD start
	if( !strcmp(jack->id, "Headset Jack"))
	{
		printk("Headset Jack swdev register\n");
		jack->swdev.name = "h2w";
		jack->swdev.print_name = simple_remote_print_name;
		err = switch_dev_register(&jack->swdev);
		if (err < 0) {
			printk("switch_dev_register failed:%d\n", err);
		}
	}
	else if( !strcmp(jack->id, "Button Jack"))
	{
		printk("Button Jack swdev register\n");
		jack->swdev.name = "h2w_Bt";
		jack->swdev.print_name = simple_remote_print_name;
		err = switch_dev_register(&jack->swdev);
		if (err < 0) {
			printk("switch_dev_register failed:%d\n", err);
		}
	}
	// SoMC Uevent End

	// SoMC Input Event Start
	jack->indev_appkey = input_allocate_device();
	if (jack->indev_appkey == NULL) {
		err = -ENOMEM;
		goto fail_input;
	}

	jack->indev_appkey->phys = "ALSA";

	jack->type = type;

	for (i = 0; i < ARRAY_SIZE(jack_switch_types); i++)
		if (type & (1 << i))
			input_set_capability(jack->indev_appkey, EV_SW,
						 jack_switch_types[i]);
// AUD_MOD end

	jack->input_dev = input_allocate_device();
	if (jack->input_dev == NULL) {
		err = -ENOMEM;
		goto fail_input;
	}

	jack->input_dev->phys = "ALSA";

	jack->type = type;

	for (i = 0; i < ARRAY_SIZE(jack_switch_types); i++)
		if (type & (1 << i))
			input_set_capability(jack->input_dev, EV_SW,
					     jack_switch_types[i]);

	err = snd_device_new(card, SNDRV_DEV_JACK, jack, &ops);
	if (err < 0)
		goto fail_input;

	*jjack = jack;

	return 0;

fail_input:
	input_free_device(jack->indev_appkey);  // AUD_MOD
	input_free_device(jack->input_dev);
	kfree(jack->id);
	kfree(jack);
	return err;
}
EXPORT_SYMBOL(snd_jack_new);

/**
 * snd_jack_set_parent - Set the parent device for a jack
 *
 * @jack:   The jack to configure
 * @parent: The device to set as parent for the jack.
 *
 * Set the parent for the jack input device in the device tree.  This
 * function is only valid prior to registration of the jack.  If no
 * parent is configured then the parent device will be the sound card.
 */
void snd_jack_set_parent(struct snd_jack *jack, struct device *parent)
{
	WARN_ON(jack->registered);

	jack->input_dev->dev.parent = parent;
}
EXPORT_SYMBOL(snd_jack_set_parent);

/**
 * snd_jack_set_key - Set a key mapping on a jack
 *
 * @jack:    The jack to configure
 * @type:    Jack report type for this key
 * @keytype: Input layer key type to be reported
 *
 * Map a SND_JACK_BTN_ button type to an input layer key, allowing
 * reporting of keys on accessories via the jack abstraction.  If no
 * mapping is provided but keys are enabled in the jack type then
 * BTN_n numeric buttons will be reported.
 *
 * Note that this is intended to be use by simple devices with small
 * numbers of keys that can be reported.  It is also possible to
 * access the input device directly - devices with complex input
 * capabilities on accessories should consider doing this rather than
 * using this abstraction.
 *
 * This function may only be called prior to registration of the jack.
 */
int snd_jack_set_key(struct snd_jack *jack, enum snd_jack_types type,
		     int keytype)
{
	int key = fls(SND_JACK_BTN_0) - fls(type);

	WARN_ON(jack->registered);

	if (!keytype || key >= ARRAY_SIZE(jack->key))
		return -EINVAL;

	jack->type |= type;
	jack->key[key] = keytype;

	return 0;
}
EXPORT_SYMBOL(snd_jack_set_key);

/**
 * snd_jack_report - Report the current status of a jack
 *
 * @jack:   The jack to report status for
 * @status: The current status of the jack
 */
void snd_jack_report(struct snd_jack *jack, int status)
{
	int i;

	if (!jack)
		return;

	for (i = 0; i < ARRAY_SIZE(jack->key); i++) {
		int testbit = SND_JACK_BTN_0 >> i;

		if (jack->type & testbit)
			input_report_key(jack->input_dev, jack->key[i],
					 status & testbit);
	}

	for (i = 0; i < ARRAY_SIZE(jack_switch_types); i++) {
		int testbit = 1 << i;
		if (jack->type & testbit)
			input_report_switch(jack->input_dev,
					    jack_switch_types[i],
					    status & testbit);
	}

	input_sync(jack->input_dev);

	// AUD_MOD start
	if( !strcmp(jack->id, "Headset Jack"))
	{
		if( status )
			{
			switch ( status )
				{
				case SND_JACK_HEADPHONE:
							switch_set_state(&jack->swdev, DEVICE_HEADPHONE);
							break;
				case SND_JACK_HEADSET:
							switch_set_state(&jack->swdev, DEVICE_HEADSET);
							break;
				case SND_JACK_UNSUPPORTED:
							switch_set_state(&jack->swdev, DEVICE_UNSUPPORTED);
							break;
						default:
							switch_set_state(&jack->swdev, DEVICE_UNKNOWN);
							break;
					}
				}
				else
				{
					switch_set_state(&jack->swdev, NO_DEVICE);
				}
			}
	// AUD_MOD end	
}
EXPORT_SYMBOL(snd_jack_report);

MODULE_AUTHOR("Mark Brown <broonie@opensource.wolfsonmicro.com>");
MODULE_DESCRIPTION("Jack detection support for ALSA");
MODULE_LICENSE("GPL");