// SPDX-License-Identifier: GPL-2.0-only
#include <linux/acpi.h>
#include <linux/dmi.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#define HP_BIOS_GUID "5FB7F034-2C63-45E9-BE91-3D44E2C707E4"
#define HP_SHARED_SCAN 0x81

struct hp_bios_request {
	u32 signature;
	u32 command;
	u32 query;
	u32 input_size;
	u8 data[128];
};

struct hp_bios_reply {
	u32 signature;
	u32 status;
	u32 hotkey;
};

struct hp_keyboard_handle {
	struct input_handle handle;
	bool shared_scan;
	bool shared_key_down;
};

static struct input_dev *hotkeys;

static int hp_last_hotkey(u32 *hotkey)
{
	struct hp_bios_request request = {
		.signature = 0x55434553,
		.command = 1,
		.query = 0x0a,
	};
	struct acpi_buffer input = { sizeof(request), &request };
	struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *reply;
	acpi_status status;
	struct hp_bios_reply result;
	int ret = -EIO;

	/* HP's LastHotKey query returns one 32-bit ID. */
	status = wmi_evaluate_method(HP_BIOS_GUID, 0, 2, &input, &output);
	reply = output.pointer;
	if (ACPI_SUCCESS(status) && reply && reply->type == ACPI_TYPE_BUFFER &&
	    reply->buffer.length >= sizeof(result)) {
		memcpy(&result, reply->buffer.pointer, sizeof(result));
		if (!result.status) {
			*hotkey = result.hotkey;
			ret = 0;
		}
	}
	kfree(reply);
	return ret;
}

static void hp_report_hotkey(struct work_struct *work)
{
	unsigned int key;
	u32 hotkey;

	if (hp_last_hotkey(&hotkey)) {
		pr_warn_ratelimited("hp_elite_x2_hotkeys: LastHotKey query failed\n");
		return;
	}

	switch (hotkey) {
	case 0x1b6:
	case 0x0a0001b6:
		key = KEY_BRIGHTNESSDOWN;
		break;
	case 0x1b7:
	case 0x0a0001b7:
		key = KEY_BRIGHTNESSUP;
		break;
	case 0x270:
		key = KEY_MICMUTE;
		break;
	default:
		return;
	}

	input_report_key(hotkeys, key, 1);
	input_sync(hotkeys);
	input_report_key(hotkeys, key, 0);
	input_sync(hotkeys);
}
static DECLARE_WORK(hotkey_work, hp_report_hotkey);

static bool hp_keyboard_filter(struct input_handle *handle, unsigned int type,
			       unsigned int code, int value)
{
	struct hp_keyboard_handle *keyboard =
		container_of(handle, struct hp_keyboard_handle, handle);
	bool suppress;

	if (type == EV_MSC && code == MSC_SCAN) {
		keyboard->shared_scan = value == HP_SHARED_SCAN;
	} else if (type == EV_SYN) {
		keyboard->shared_scan = false;
	}

	if (type != EV_KEY || code != KEY_MICMUTE)
		return false;

	if (!value) {
		suppress = keyboard->shared_key_down;
		keyboard->shared_key_down = false;
		return suppress;
	}

	if (value == 1) {
		keyboard->shared_key_down = keyboard->shared_scan;
		/* The filter runs under the input lock. WMI can sleep. */
		if (keyboard->shared_key_down)
			schedule_work(&hotkey_work);
	}

	return keyboard->shared_key_down;
}

static bool hp_keyboard_match(struct input_handler *handler, struct input_dev *dev)
{
	return dev->phys && !strcmp(dev->phys, "isa0060/serio0/input0");
}

static int hp_keyboard_connect(struct input_handler *handler, struct input_dev *dev,
			       const struct input_device_id *id)
{
	struct hp_keyboard_handle *keyboard;
	int ret;

	keyboard = kzalloc(sizeof(*keyboard), GFP_KERNEL);
	if (!keyboard)
		return -ENOMEM;

	keyboard->handle.dev = dev;
	keyboard->handle.handler = handler;
	keyboard->handle.name = handler->name;
	ret = input_register_handle(&keyboard->handle);
	if (ret)
		goto free_keyboard;
	ret = input_open_device(&keyboard->handle);
	if (ret)
		goto unregister_handle;
	return 0;

unregister_handle:
	input_unregister_handle(&keyboard->handle);
free_keyboard:
	kfree(keyboard);
	return ret;
}

static void hp_keyboard_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(container_of(handle, struct hp_keyboard_handle, handle));
}

static const struct input_device_id hp_keyboard_ids[] = {
	{ .flags = INPUT_DEVICE_ID_MATCH_BUS, .bustype = BUS_I8042 },
	{ }
};

static struct input_handler hp_keyboard_handler = {
	.name = "hp_elite_x2_hotkeys",
	.id_table = hp_keyboard_ids,
	.match = hp_keyboard_match,
	.connect = hp_keyboard_connect,
	.disconnect = hp_keyboard_disconnect,
	.filter = hp_keyboard_filter,
};

static int __init hp_elite_x2_hotkeys_init(void)
{
	u32 hotkey;
	int ret;

	if (!dmi_match(DMI_SYS_VENDOR, "HP") ||
	    !dmi_match(DMI_PRODUCT_NAME, "HP Elite x2 1013 G3") ||
	    !dmi_match(DMI_BOARD_NAME, "8414"))
		return -ENODEV;
	ret = hp_last_hotkey(&hotkey);
	if (ret)
		return ret;

	hotkeys = input_allocate_device();
	if (!hotkeys)
		return -ENOMEM;
	hotkeys->name = "HP Elite x2 1013 G3 extra hotkeys";
	hotkeys->id.bustype = BUS_HOST;
	input_set_capability(hotkeys, EV_KEY, KEY_BRIGHTNESSDOWN);
	input_set_capability(hotkeys, EV_KEY, KEY_BRIGHTNESSUP);
	input_set_capability(hotkeys, EV_KEY, KEY_MICMUTE);
	ret = input_register_device(hotkeys);
	if (ret) {
		input_free_device(hotkeys);
		return ret;
	}

	ret = input_register_handler(&hp_keyboard_handler);
	if (ret)
		input_unregister_device(hotkeys);
	return ret;
}

static void __exit hp_elite_x2_hotkeys_exit(void)
{
	input_unregister_handler(&hp_keyboard_handler);
	cancel_work_sync(&hotkey_work);
	input_unregister_device(hotkeys);
}

module_init(hp_elite_x2_hotkeys_init);
module_exit(hp_elite_x2_hotkeys_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HP Elite x2 1013 G3 brightness and microphone hotkeys");
