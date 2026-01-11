#include "app.h"
#include "app_view_ids.h"

#include <gui/gui.h>
#include <furi.h>

#include "../pages/home_page.h"
#include "../pages/pcaps_page.h"
#include "../pages/send_page.h"
#include "../pages/config_page.h"
#include "../pages/info_page.h"

#include "../ui/ui_strings.h"
#include "../ui/ui_helpers.h"

// static bool app_custom_event_callback(void* context, uint32_t event) {
//     UNUSED(context);
//     UNUSED(event);
//     // Custom event 1 is used for send page scroll updates
//     // The view will automatically redraw when it receives input events
//     return true;
// }

App* app_alloc(void) {
    App* app = malloc(sizeof(App));
    memset(app, 0, sizeof(App));

    app->storage = furi_record_open(RECORD_STORAGE);
    app->view_dispatcher = view_dispatcher_alloc();

    app->home_menu = submenu_alloc();
    app->pcap_menu = submenu_alloc();
    app->config_menu = submenu_alloc();

    app->info_dialog = dialog_ex_alloc();
    app->text_input = text_input_alloc();

    send_page_create(&app->send_page);

    config_store_defaults(&app->config);
    config_store_load(app->storage, &app->config);

    // old default cursor
    app->pcap_cursor_item_id = 10;

    // Setup GUI/dispatcher
    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    // view_dispatcher_set_custom_event_callback(app->view_dispatcher, app_custom_event_callback);

    // Register views
    home_page_init(app->home_menu, app);
    view_dispatcher_add_view(app->view_dispatcher, ViewHome, submenu_get_view(app->home_menu));

    pcaps_page_init(app->pcap_menu, app);
    view_dispatcher_add_view(app->view_dispatcher, ViewPcapList, submenu_get_view(app->pcap_menu));

    config_page_init(app->config_menu, app);
    view_dispatcher_add_view(app->view_dispatcher, ViewConfig, submenu_get_view(app->config_menu));

    // Send: custom view
    send_page_bind(&app->send_page, app);
    view_dispatcher_add_view(app->view_dispatcher, ViewSend, send_page_get_view(&app->send_page));

    // Info dialog
    info_page_init(app->info_dialog, app);
    view_dispatcher_add_view(app->view_dispatcher, ViewInfo, dialog_ex_get_view(app->info_dialog));

    // TextInput (for config editing)
    config_page_setup_text_input(app->text_input, app);
    view_dispatcher_add_view(app->view_dispatcher, ViewTextInput, text_input_get_view(app->text_input));

    // default view
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewHome);
    return app;
}

void app_run(App* app) {
    // view_dispatcher_switch_to_view(app->view_dispatcher, ViewHome);
    view_dispatcher_run(app->view_dispatcher);
}

void app_free(App* app) {
    if(!app) return;

    view_dispatcher_remove_view(app->view_dispatcher, ViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, ViewInfo);
    view_dispatcher_remove_view(app->view_dispatcher, ViewSend);
    view_dispatcher_remove_view(app->view_dispatcher, ViewConfig);
    view_dispatcher_remove_view(app->view_dispatcher, ViewPcapList);
    view_dispatcher_remove_view(app->view_dispatcher, ViewHome);

    send_page_destroy(&app->send_page);

    text_input_free(app->text_input);
    dialog_ex_free(app->info_dialog);

    submenu_free(app->config_menu);
    submenu_free(app->pcap_menu);
    submenu_free(app->home_menu);

    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_STORAGE);

    free(app);
}
