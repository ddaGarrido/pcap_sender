#include "home_page.h"
#include "../app/app_state.h"
#include "../app/app_view_ids.h"
#include "../ui/ui_strings.h"
#include "../services/pcap_store.h"
#include "../pages/pcaps_page.h"
#include "../pages/send_page.h"
#include "../pages/config_page.h"

typedef enum {
    HomeItemPcapFiles = 0,
    HomeItemSendFiles,
    HomeItemConfig,
    HomeItemExit,
} HomeItemId;

static void home_menu_callback(void* context, uint32_t index) {
    App* app = context;

    if(index == HomeItemPcapFiles) {
        pcap_store_load_preserve_selection(app->storage, &app->pcaps, PCAP_DIR);

        uint32_t keep = app->pcap_cursor_item_id;
        if(keep == 0 || keep == 1) keep = 10;

        pcaps_page_build_menu(app, keep);
        view_dispatcher_switch_to_view(app->view_dispatcher, ViewPcapList);
        return;
    }

    if(index == HomeItemSendFiles) {
        send_page_open(app);
        return;
    }

    if(index == HomeItemConfig) {
        config_page_build_menu(app);
        view_dispatcher_switch_to_view(app->view_dispatcher, ViewConfig);
        return;
    }

    if(index == HomeItemExit) {
        view_dispatcher_stop(app->view_dispatcher);
        return;
    }
}

void home_page_init(Submenu* menu, App* app) {
    submenu_add_item(menu, "PCAP files", HomeItemPcapFiles, home_menu_callback, app);
    submenu_add_item(menu, "Send files", HomeItemSendFiles, home_menu_callback, app);
    submenu_add_item(menu, "Config", HomeItemConfig, home_menu_callback, app);
    submenu_add_item(menu, "Exit", HomeItemExit, home_menu_callback, app);
    submenu_set_header(menu, "PCAP Sender");
}
