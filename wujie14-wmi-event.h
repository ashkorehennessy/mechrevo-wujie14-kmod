#include <linux/kernel.h>
#include <linux/module.h>
#include "wujie14-km.h"

#define WUJIE14_IP3WMIEVENT_GUID "8FAFC061-22DA-46E2-91DB-1FE3D7E5FF3C"
#define WUJIE14_PRO_WMIEVENT_GUID "46C93E13-EE9B-4262-8488-563BCA757FEF"

typedef void (*wujie14_wmi_notify_handler)(struct wujie14_private*);

struct wujie14_wmi_notify_handler_entry{
    u32 febc_offset;
    u8 febc_value;
    wujie14_wmi_notify_handler handler;
};


int wujie14_wmi_event_init(struct wujie14_private* priv);
void wujie14_wmi_event_exit(struct wujie14_private* priv);