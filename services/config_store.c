#include "config_store.h"
#include "../ui/ui_strings.h"
#include "../ui/ui_helpers.h"

static void ensure_app_data_dir(Storage* storage) {
    if(!ui_dir_exists(storage, APP_DATA_DIR)) {
        storage_common_mkdir(storage, APP_DATA_DIR);
    }
}

void config_store_defaults(ConfigStore* cfg) {
    if(!cfg) return;
    cfg->wifi_ssid[0] = '\0';
    cfg->wifi_pass[0] = '\0';
    cfg->server[0] = '\0';
}

static void parse_kv_line(ConfigStore* cfg, char* line) {
    ui_trim_newline(line);
    char* eq = strchr(line, '=');
    if(!eq) return;

    *eq = '\0';
    const char* key = line;
    const char* val = eq + 1;

    if(strcmp(key, "ssid") == 0) {
        strncpy(cfg->wifi_ssid, val, CONFIG_SSID_MAX);
        cfg->wifi_ssid[CONFIG_SSID_MAX] = '\0';
    } else if(strcmp(key, "pass") == 0) {
        strncpy(cfg->wifi_pass, val, CONFIG_PASS_MAX);
        cfg->wifi_pass[CONFIG_PASS_MAX] = '\0';
    } else if(strcmp(key, "server") == 0) {
        strncpy(cfg->server, val, CONFIG_SERVER_MAX);
        cfg->server[CONFIG_SERVER_MAX] = '\0';
    }
}

void config_store_load(Storage* storage, ConfigStore* cfg) {
    if(!storage || !cfg) return;
    config_store_defaults(cfg);

    File* f = storage_file_alloc(storage);
    if(!storage_file_open(f, CONFIG_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(f);
        return;
    }

    char buf[256];
    size_t pos = 0;

    while(true) {
        uint16_t r = storage_file_read(f, (uint8_t*)&buf[pos], 1);
        if(r == 0) break;

        if(buf[pos] == '\n' || pos >= (sizeof(buf) - 2)) {
            buf[pos + 1] = '\0';
            parse_kv_line(cfg, buf);
            pos = 0;
        } else {
            pos++;
        }
    }

    if(pos > 0) {
        buf[pos] = '\0';
        parse_kv_line(cfg, buf);
    }

    storage_file_close(f);
    storage_file_free(f);
}

void config_store_save(Storage* storage, const ConfigStore* cfg) {
    if(!storage || !cfg) return;
    ensure_app_data_dir(storage);

    File* f = storage_file_alloc(storage);
    if(!storage_file_open(f, CONFIG_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_free(f);
        return;
    }

    char line[256];

    snprintf(line, sizeof(line), "ssid=%s\n", cfg->wifi_ssid);
    storage_file_write(f, line, strlen(line));

    snprintf(line, sizeof(line), "pass=%s\n", cfg->wifi_pass);
    storage_file_write(f, line, strlen(line));

    snprintf(line, sizeof(line), "server=%s\n", cfg->server);
    storage_file_write(f, line, strlen(line));

    storage_file_close(f);
    storage_file_free(f);
}

void config_store_build_ready_check(
    const ConfigStore* cfg,
    uint32_t selected_count,
    char* out,
    size_t out_len) {

    if(!cfg || !out || out_len == 0) return;

    bool has_ssid = strlen(cfg->wifi_ssid) > 0;
    bool has_pass = strlen(cfg->wifi_pass) > 0;
    bool has_server = strlen(cfg->server) > 0;

    snprintf(
        out,
        out_len,
        "Local checks:\n"
        "SSID: %s\n"
        "PASS: %s\n"
        "Server: %s\n"
        "Selected: %lu\n\n"
        "ESP32 ping: later via UART\n\n"
        "Ready: %s",
        has_ssid ? "OK" : "MISSING",
        has_pass ? "OK" : "MISSING",
        has_server ? "OK" : "MISSING",
        (unsigned long)selected_count,
        (has_ssid && has_pass && has_server && selected_count > 0) ? "YES (local)" : "NO");
}
