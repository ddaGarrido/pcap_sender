#include "pcaps_page.h"
#include "../app/app_state.h"
#include "../app/app_view_ids.h"
#include "../ui/ui_strings.h"
#include "../ui/ui_helpers.h"
#include "../services/pcap_store.h"

static void pcap_menu_callback(void* context, uint32_t index) {
    App* app = context;

    // remember cursor
    app->pcap_cursor_item_id = index;

    if(index == 0) {
        view_dispatcher_switch_to_view(app->view_dispatcher, ViewHome);
        return;
    }

    if(index == 1) {
        if(!ui_dir_exists(app->storage, PCAP_DIR)) {
            ui_show_info(app->info_dialog, app->view_dispatcher, ViewInfo, "Folder missing", PCAP_DIR);
        } else {
            ui_show_info(app->info_dialog, app->view_dispatcher, ViewInfo, "No files", PCAP_DIR);
        }
        return;
    }

    if(index < 10) return;

    uint32_t file_idx = index - 10;
    if(file_idx >= app->pcaps.file_count) return;

    app->pcaps.selected[file_idx] = !app->pcaps.selected[file_idx];

    // rebuild keep cursor
    pcaps_page_build_menu(app, app->pcap_cursor_item_id);
}

void pcaps_page_build_menu(App* app, uint32_t keep_item_id) {
    submenu_reset(app->pcap_menu);

    submenu_add_item(app->pcap_menu, "< Back", 0, pcap_menu_callback, app);

    if(!ui_dir_exists(app->storage, PCAP_DIR)) {
        if(keep_item_id < 10) keep_item_id = 10;
        submenu_add_item(app->pcap_menu, "(folder missing)", 1, pcap_menu_callback, app);
        submenu_set_selected_item(app->pcap_menu, keep_item_id);
        return;
    }

    if(app->pcaps.file_count == 0) {
        if(keep_item_id < 10) keep_item_id = 10;
        submenu_add_item(app->pcap_menu, "(no pcaps found)", 1, pcap_menu_callback, app);
        submenu_set_selected_item(app->pcap_menu, keep_item_id);
        return;
    }

    pcap_store_build_labels(&app->pcaps);

    for(uint32_t i = 0; i < app->pcaps.file_count; i++) {
        submenu_add_item(app->pcap_menu, app->pcaps.labels[i], i + 10, pcap_menu_callback, app);
    }

    if(keep_item_id < 10) keep_item_id = 10;
    submenu_set_selected_item(app->pcap_menu, keep_item_id);
}

void pcaps_page_init(Submenu* menu, App* app) {
    (void)menu;
    (void)app;
    // menu is built dynamically each time (same as monolith)
}
