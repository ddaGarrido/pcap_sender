#pragma once

#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/text_input.h>
#include <storage/storage.h>

#include "app_fwd.h"
#include "../services/config_store.h"
#include "../services/pcap_store.h"
#include "../pages/send_page.h"

/**
 * IMPORTANT:
 * App must be defined as `struct App { ... }` (not an anonymous typedef),
 * so all `typedef struct App App;` forward decls match the same type.
 */
struct App {
    ViewDispatcher* view_dispatcher;

    Submenu* home_menu;
    Submenu* pcap_menu;
    Submenu* config_menu;

    DialogEx* info_dialog;

    TextInput* text_input;
    char input_buf[CONFIG_SERVER_MAX + 1];

    Storage* storage;

    ConfigStore config;
    PcapStore pcaps;

    SendPage send_page;

    // PCAP cursor persistence (submenu item id: 0/1/10+)
    uint32_t pcap_cursor_item_id;
};
