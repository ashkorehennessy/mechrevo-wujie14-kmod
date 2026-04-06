#include <linux/acpi.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/wmi.h>

#include "wujie14-km.h"
#include "wujie14-perfmode.h"
#include "wujie14-wmi-event.h"
#include "wujie14-kb.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Xu");
MODULE_DESCRIPTION("mechrevo wujie 14 driver km");

struct wujie14_private priv;

int wujie14_wmaa_query(struct wujie14_private *priv, u16 cmd, u16 feature,
		       u8 val, u16 *result)
{
	acpi_status status;
	u8 input[7] = {0};
	struct acpi_buffer in_buf = {sizeof(input), input};
	struct acpi_buffer out_buf = {ACPI_ALLOCATE_BUFFER, NULL};
	union acpi_object *obj;
	int ret = 0;

	/* Build WMAA input buffer (little-endian on x86) */
	*((u16 *)&input[0]) = cmd;
	*((u16 *)&input[2]) = feature;
	input[4] = val;

	status = wmi_evaluate_method(WUJIE14_PRO_WMI_METHOD_GUID,
				     0, WUJIE14_PRO_WMAA_METHODID,
				     &in_buf, &out_buf);
	if (ACPI_FAILURE(status)) {
		dev_err(&priv->pdev->dev, "WMAA call failed\n");
		return -EIO;
	}

	obj = out_buf.pointer;
	if (obj && obj->type == ACPI_TYPE_BUFFER && obj->buffer.length >= 6) {
		u16 sger = *((u16 *)(obj->buffer.pointer));
		if (sger != 0x8000) {
			dev_err(&priv->pdev->dev,
				"WMAA error status 0x%04x\n", sger);
			ret = -EIO;
		} else if (result) {
			*result = *((u16 *)(obj->buffer.pointer + 4));
		}
	} else if (result) {
		ret = -EIO;
	}

	ACPI_FREE(out_buf.pointer);
	return ret;
}

int wujie_plat_probe(struct platform_device *pdev)
{
	const struct dmi_system_id *dmi_match;
	struct acpi_device *adev;
	unsigned long long ec_sta_result;
	int err;
	priv.pdev = pdev;
	// do dmi match first
	dmi_match = dmi_first_match(wujie14_allowlist);
	if (dmi_match == NULL) {
		dev_err(&pdev->dev, "Unsupported laptop model.\n");
		return -EINVAL;
	}
	priv.variant = (enum wujie14_variant)(unsigned long)dmi_match->driver_data;
	dev_info(&pdev->dev, "Detected variant: %s\n",
		 priv.variant == WUJIE14_PRO ? "WUJIE14 PRO" : "WUJIE 14");
	// EC acpi _STA test
	adev = ACPI_COMPANION(&pdev->dev);
	err = acpi_evaluate_integer(adev->handle, "_STA", NULL, &ec_sta_result);
	if (err) {
		dev_err(&pdev->dev, "ACPI _STA evaluation failed.\n");
		return err;
	}
	dev_dbg(&pdev->dev, "got _STA %lld\n", ec_sta_result);
	err = wujie14_platform_profile_init(&priv);
	if (err) {
		dev_err(&pdev->dev, "Platform profile initialization failed.\n");
		return err;
	}
	err = wujie14_powermode_sysfs_init(&priv);
	if (err) {
		dev_err(&pdev->dev, "Powermode sysfs initialization failed.\n");
		goto err_profile;
	}
	err = wujie14_wmi_event_init(&priv);
	if (err) {
		goto err_sysfs;
	}
	err = wujie14_kbd_sysfs_init(&priv);
	if (err) {
		goto err_wmi;
	}

	return 0;

err_wmi:
	wujie14_wmi_event_exit(&priv);
err_sysfs:
	wujie14_powermode_sysfs_exit(&priv);
err_profile:
	wujie14_platform_profile_exit(&priv);
	return err;
}

void wujie_plat_remove(struct platform_device *pdev)
{
	wujie14_platform_profile_exit(&priv);
	wujie14_powermode_sysfs_exit(&priv);
	wujie14_wmi_event_exit(&priv);
	wujie14_kbd_sysfs_exit(&priv);
}


static const struct acpi_device_id wujie_device_ids[] = {
	{ "PNP0C09", 0 },
	{ "", 0 },
};
MODULE_DEVICE_TABLE(acpi, wujie_device_ids);

static struct platform_driver wujie14_platdriver = {
	.probe = wujie_plat_probe,
	.remove = wujie_plat_remove,
	.driver = {
		.name   = "wujie14",
		.acpi_match_table = ACPI_PTR(wujie_device_ids),
	},
};

static int __init wujie14_init(void)
{
	int err;
	err = platform_driver_register(&wujie14_platdriver);
	return err;

}

static void __exit wujie14_exit(void)
{
	platform_driver_unregister(&wujie14_platdriver);
}

module_init(wujie14_init);
module_exit(wujie14_exit);