#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/acpi.h>
#include <linux/wmi.h>

#include "wujie14-km.h"
#include "wujie14-wmi-event.h"
#include "wujie14-perfmode.h"

/* Original WUJIE 14 event handler table */
static const struct wujie14_wmi_notify_handler_entry handler_tbl[] = {
    {
        .febc_offset = 1,
        .febc_value = 0x11,
        .handler = wujie14_powermode_wmi_event_handler
    },
    {
        .febc_offset = 1,
        .febc_value = 0x12,
        .handler = wujie14_powermode_wmi_event_handler
    },
    {
        .febc_offset = 1,
        .febc_value = 0x13,
        .handler = wujie14_powermode_wmi_event_handler
    },
    {}
};

static inline wujie14_wmi_notify_handler locate_handler(
    u8* response_buffer
){
    const struct wujie14_wmi_notify_handler_entry* cur;
    wujie14_wmi_notify_handler ret_handler = NULL;
    cur = handler_tbl;
    while(cur->febc_offset){
        if (response_buffer[cur->febc_offset] == cur->febc_value){
            ret_handler = cur->handler;
            break;
        }
        cur++;
    }
    return ret_handler;
}

/* Original WUJIE 14: handler for IP3 WMI events */
static void wujie14_wmi_event_handler(
    union acpi_object *data,
    void* context
)
{
    wujie14_wmi_notify_handler handler;
    struct wujie14_private* priv = context;
    if (!data || data->type != ACPI_TYPE_BUFFER) {
        dev_err(&priv->pdev->dev, "invalid wmi event data\n");
        return;
    }
    handler = locate_handler(data->buffer.pointer);
    if (handler == NULL){
        return;
    }
    handler(priv); 
}

/* WUJIE14 PRO: handler for WMAA WMI events (EVBU format) */
static void wujie14_pro_wmi_event_handler(
    union acpi_object *data,
    void *context
)
{
    struct wujie14_private *priv = context;
    u8 *evbu;

    if (!data || data->type != ACPI_TYPE_BUFFER || data->buffer.length < 3) {
        dev_err(&priv->pdev->dev, "invalid wmi event data\n");
        return;
    }

    evbu = data->buffer.pointer;
    if (evbu[0] != 1)  /* event not valid */
        return;

    switch (evbu[1]) {
    case 0x0F:  /* power mode change */
        wujie14_powermode_wmi_event_handler(priv);
        break;
    case 0x05:  /* keyboard backlight change */
        /* notification only, no action needed */
        break;
    }
}

int wujie14_wmi_event_init(struct wujie14_private* priv)
{
    acpi_status status;
    const char *guid;
    wmi_notify_handler handler;

    if (priv->variant == WUJIE14_PRO) {
        guid = WUJIE14_PRO_WMIEVENT_GUID;
        handler = wujie14_pro_wmi_event_handler;
    } else {
        guid = WUJIE14_IP3WMIEVENT_GUID;
        handler = wujie14_wmi_event_handler;
    }

    status = wmi_install_notify_handler(guid, handler, priv);
    if (ACPI_FAILURE(status)){
        dev_err(&priv->pdev->dev, "WMI event handler install failed\n");
        return -ENODEV;
    }
    return 0;
}

void wujie14_wmi_event_exit(struct wujie14_private* priv)
{
    const char *guid = (priv->variant == WUJIE14_PRO) ?
                       WUJIE14_PRO_WMIEVENT_GUID :
                       WUJIE14_IP3WMIEVENT_GUID;
    wmi_remove_notify_handler(guid);
}