#include "config_page.h"
#include "../app/app_state.h"
#include "../app/app_view_ids.h"
#include "../ui/ui_helpers.h"
#include "../services/config_store.h"
#include "../services/pcap_store.h"

typedef enum {
    EditNone = 0,
    EditSsid,
    EditPass,
    EditServer,
} EditField;

static EditField g_edit_field = EditNone;

static uint32_t text_input_prev_callback(void* context) {
    App* app = context;
    if(!app || !app->view_dispatcher) return ViewConfig;

    g_edit_field = EditNone;
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewConfig);
    return ViewConfig;
}

static void text_input_done_callback(void* context) {
    App* app = context;
    if(!app || !app->view_dispatcher || !app->storage) return;

    // app->input_buf is always NUL-terminated by TextInput as long as buffer size is correct
    if(g_edit_field == EditSsid) {
        strncpy(app->config.wifi_ssid, app->input_buf, CONFIG_SSID_MAX);
        app->config.wifi_ssid[CONFIG_SSID_MAX] = '\0';
    } else if(g_edit_field == EditPass) {
        strncpy(app->config.wifi_pass, app->input_buf, CONFIG_PASS_MAX);
        app->config.wifi_pass[CONFIG_PASS_MAX] = '\0';
    } else if(g_edit_field == EditServer) {
        strncpy(app->config.server, app->input_buf, CONFIG_SERVER_MAX);
        app->config.server[CONFIG_SERVER_MAX] = '\0';
    }

    g_edit_field = EditNone;
    config_store_save(app->storage, &app->config);

    config_page_build_menu(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewConfig);
}

static void open_text_input(App* app, EditField field) {
    if(!app || !app->text_input || !app->view_dispatcher) return;

    g_edit_field = field;

    const char* header = "Edit";
    const char* initial = "";

    memset(app->input_buf, 0, sizeof(app->input_buf));

    if(field == EditSsid) {
        header = "Wi-Fi SSID";
        initial = app->config.wifi_ssid;
    } else if(field == EditPass) {
        header = "Wi-Fi Password";
        initial = app->config.wifi_pass;
    } else if(field == EditServer) {
        header = "Server (IP/URL)";
        initial = app->config.server;
    } else {
        return;
    }

    // Copy initial value into the shared persistent buffer
    if(initial && initial[0]) {
        strlcpy(app->input_buf, initial, sizeof(app->input_buf));
    }

    text_input_reset(app->text_input);
    text_input_set_header_text(app->text_input, header);

    // IMPORTANT:
    // buffer size must be sizeof(app->input_buf), not CONFIG_*_MAX
    text_input_set_result_callback(
        app->text_input,
        text_input_done_callback,
        app,
        app->input_buf,
        sizeof(app->input_buf),
        false /* clear_default_text */
    );

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewTextInput);
}

static void run_ready_check(App* app) {
    if(!app) return;

    char msg[512];
    config_store_build_ready_check(
        &app->config,
        pcap_store_count_selected(&app->pcaps),
        msg,
        sizeof(msg)
    );

    ui_show_info(app->info_dialog, app->view_dispatcher, ViewInfo, "Ready check", msg);
}

static void config_menu_callback(void* context, uint32_t index) {
    App* app = context;
    if(!app || !app->view_dispatcher) return;

    if(index == 0) {
        view_dispatcher_switch_to_view(app->view_dispatcher, ViewHome);
        return;
    }

    if(index == 1) {
        open_text_input(app, EditSsid);
    } else if(index == 2) {
        open_text_input(app, EditPass);
    } else if(index == 3) {
        open_text_input(app, EditServer);
    } else if(index == 4) {
        run_ready_check(app);
    }
}

void config_page_build_menu(App* app) {
    if(!app || !app->config_menu) return;

    submenu_reset(app->config_menu);

    static char cfg_labels[5][128];

    snprintf(cfg_labels[1], sizeof(cfg_labels[1]), "Wi-Fi SSID: %s",
        strlen(app->config.wifi_ssid) ? app->config.wifi_ssid : "(not set)");
    snprintf(cfg_labels[2], sizeof(cfg_labels[2]), "Wi-Fi PASS: %s",
        strlen(app->config.wifi_pass) ? "********" : "(not set)");
    snprintf(cfg_labels[3], sizeof(cfg_labels[3]), "Server: %s",
        strlen(app->config.server) ? app->config.server : "(not set)");
    snprintf(cfg_labels[4], sizeof(cfg_labels[4]), "Ping / Ready check");

    submenu_add_item(app->config_menu, "< Back", 0, config_menu_callback, app);
    submenu_add_item(app->config_menu, cfg_labels[1], 1, config_menu_callback, app);
    submenu_add_item(app->config_menu, cfg_labels[2], 2, config_menu_callback, app);
    submenu_add_item(app->config_menu, cfg_labels[3], 3, config_menu_callback, app);
    submenu_add_item(app->config_menu, cfg_labels[4], 4, config_menu_callback, app);
}

void config_page_init(Submenu* menu, App* app) {
    (void)menu;
    (void)app;
}

void config_page_setup_text_input(TextInput* text_input, App* app) {
    if(!text_input || !app) return;

    View* ti_view = text_input_get_view(text_input);
    if(!ti_view) return;

    view_set_context(ti_view, app);

    // Do NOT set input callback here; TextInput manages its own input
    view_set_previous_callback(ti_view, text_input_prev_callback);

    text_input_reset(text_input);
}
