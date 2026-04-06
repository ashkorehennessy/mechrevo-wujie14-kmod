#ifndef WUJIE14_KM_H
#define WUJIE14_KM_H

#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>

enum wujie14_variant {
	WUJIE14_ORIGINAL,
	WUJIE14_PRO,
};

static const struct dmi_system_id wujie14_allowlist[] = {
	{
		.ident = "WUJIE 14",
		.driver_data = (void *)WUJIE14_ORIGINAL,
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "MECHREVO"),
			DMI_MATCH(DMI_PRODUCT_NAME, "WUJIE 14"),
			DMI_MATCH(DMI_PRODUCT_SKU, "WUJIE 14")
		}
	},
	{
		.ident = "WUJIE14 PRO",
		.driver_data = (void *)WUJIE14_PRO,
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "MECHREVO"),
			DMI_MATCH(DMI_PRODUCT_NAME, "WUJIE14 PRO"),
			DMI_MATCH(DMI_PRODUCT_SKU, "WUJIE14")
		}
	},
	{} /* terminator */
};

struct wujie14_private {
	struct platform_device *pdev;
	struct device *ppdev;
	enum wujie14_variant variant;
};

/* WMAA protocol for PRO variant */
#define WUJIE14_PRO_WMI_METHOD_GUID "B60BFB48-3E5B-49E4-A0E9-8CFFE1B3434B"
#define WUJIE14_PRO_WMAA_METHODID   1

#define WMAA_CMD_GET		0xFA00
#define WMAA_CMD_SET		0xFB00

#define WMAA_FEAT_POWERMODE	0x0800
#define WMAA_FEAT_KBBACKLIGHT	0x1200

int wujie14_wmaa_query(struct wujie14_private *priv, u16 cmd, u16 feature,
		       u8 val, u16 *result);

int wujie_plat_probe(struct platform_device *pdev);
void wujie_plat_remove(struct platform_device *pdev);

#endif