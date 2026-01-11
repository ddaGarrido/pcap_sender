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
    g_edit_field = EditNone; // cancel
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewConfig);
    return ViewConfig;
}

static void text_input_done_callback(void* context) {
    App* app = context;
    if(!app || !app->view_dispatcher || !app->storage) return;

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
    
    // Rebuild menu to show updated values
    config_page_build_menu(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewConfig);
}

static void open_text_input(App* app, EditField field) {
    if(!app || !app->text_input || !app->view_dispatcher) return;
    
    g_edit_field = field;

    const char* header = "Edit";
    size_t max_len = 0;

    // Clear and initialize buffer
    memset(app->input_buf, 0, sizeof(app->input_buf));
    app->input_buf[0] = '\0';

    if(field == EditSsid) {
        header = "Wi-Fi SSID";
        max_len = CONFIG_SSID_MAX;
        if(app->config.wifi_ssid[0] != '\0') {
            strncpy(app->input_buf, app->config.wifi_ssid, CONFIG_SSID_MAX);
            app->input_buf[CONFIG_SSID_MAX] = '\0';
        }
    } else if(field == EditPass) {
        header = "Wi-Fi Password";
        max_len = CONFIG_PASS_MAX;
        if(app->config.wifi_pass[0] != '\0') {
            strncpy(app->input_buf, app->config.wifi_pass, CONFIG_PASS_MAX);
            app->input_buf[CONFIG_PASS_MAX] = '\0';
        }
    } else if(field == EditServer) {
        header = "Server (IP/URL)";
        max_len = CONFIG_SERVER_MAX;
        if(app->config.server[0] != '\0') {
            strncpy(app->input_buf, app->config.server, CONFIG_SERVER_MAX);
            app->input_buf[CONFIG_SERVER_MAX] = '\0';
        }
    } else {
        return;
    }

    // Reset text input first
    text_input_reset(app->text_input);
    
    // Set header
    text_input_set_header_text(app->text_input, header);

    // Configure result callback - this tells TextInput what to do when user presses OK
    text_input_set_result_callback(
        app->text_input,
        text_input_done_callback,
        app,
        app->input_buf,
        max_len,
        true);  // clear_default_text = true

    // Switch to text input view
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewTextInput);
}

static void run_ready_check(App* app) {
    char msg[512];
    config_store_build_ready_check(&app->config, pcap_store_count_selected(&app->pcaps), msg, sizeof(msg));
    ui_show_info(app->info_dialog, app->view_dispatcher, ViewInfo, "Ready check", msg);
}

static void config_menu_callback(void* context, uint32_t index) {
    App* app = context;

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
    submenu_reset(app->config_menu);

    char label1[128];
    char label2[128];
    char label3[128];
    char label4[128];

    snprintf(label1, sizeof(label1), "Wi-Fi SSID: %s", strlen(app->config.wifi_ssid) ? app->config.wifi_ssid : "(not set)");
    snprintf(label2, sizeof(label2), "Wi-Fi PASS: %s", strlen(app->config.wifi_pass) ? "********" : "(not set)");
    snprintf(label3, sizeof(label3), "Server: %s", strlen(app->config.server) ? app->config.server : "(not set)");
    snprintf(label4, sizeof(label4), "Ping / Ready check");

    // Keep strings stable: store them inside submenu by adding immediately using static buffers is unsafe
    // So: reuse submenu_add_item with immediate strings? Submenu stores pointer; we must keep stable memory.
    // We'll reuse app->pcaps.labels storage? No. Best: use local static? Not safe per-instance.
    // Solution: Use dialog labels in App? We will keep small stable copies in app->input_buf? No.
    // So: simplest stable approach is to keep these labels inside App by recreating within a fixed array.
    // We'll use the TextInput buffer? no.
    // => We'll store into app->send_page.send_text segments? also no.
    // Instead: store into app->config.wifi_*? no.
    // We'll add 4 fixed buffers in App? easiest, but we promised minimal change.
    // We'll do minimal change: add 5 labels array in App? you had in monolith; we keep it by adding in AppState later if you want.
    // For now: allocate static file-scope buffers (single instance app) - ok in Flipper since app is single instance.
    static char cfg_labels[5][128];

    strncpy(cfg_labels[1], label1, sizeof(cfg_labels[1]) - 1);
    cfg_labels[1][sizeof(cfg_labels[1]) - 1] = '\0';
    strncpy(cfg_labels[2], label2, sizeof(cfg_labels[2]) - 1);
    cfg_labels[2][sizeof(cfg_labels[2]) - 1] = '\0';
    strncpy(cfg_labels[3], label3, sizeof(cfg_labels[3]) - 1);
    cfg_labels[3][sizeof(cfg_labels[3]) - 1] = '\0';
    strncpy(cfg_labels[4], label4, sizeof(cfg_labels[4]) - 1);
    cfg_labels[4][sizeof(cfg_labels[4]) - 1] = '\0';

    submenu_add_item(app->config_menu, "< Back", 0, config_menu_callback, app);
    submenu_add_item(app->config_menu, cfg_labels[1], 1, config_menu_callback, app);
    submenu_add_item(app->config_menu, cfg_labels[2], 2, config_menu_callback, app);
    submenu_add_item(app->config_menu, cfg_labels[3], 3, config_menu_callback, app);
    submenu_add_item(app->config_menu, cfg_labels[4], 4, config_menu_callback, app);
}

void config_page_init(Submenu* menu, App* app) {
    (void)menu;
    (void)app;
    // built on demand (same behavior)
}

void config_page_setup_text_input(TextInput* text_input, App* app) {
    if(!text_input || !app) return;
    
    View* ti_view = text_input_get_view(text_input);
    if(!ti_view) return;
    
    // Set context for callbacks
    view_set_context(ti_view, app);
    
    // CRITICAL: do NOT set input callback on the view - TextInput handles its own input
    // Setting an input callback would intercept events and break keyboard navigation
    view_set_previous_callback(ti_view, text_input_prev_callback);
    
    // Ensure TextInput is properly initialized
    text_input_reset(text_input);
}
