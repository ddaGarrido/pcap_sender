#pragma once
#include <storage/storage.h>

#define CONFIG_SSID_MAX 32
#define CONFIG_PASS_MAX 64
#define CONFIG_SERVER_MAX 96

typedef struct {
    char wifi_ssid[CONFIG_SSID_MAX + 1];
    char wifi_pass[CONFIG_PASS_MAX + 1];
    char server[CONFIG_SERVER_MAX + 1];
} ConfigStore;

void config_store_defaults(ConfigStore* cfg);
void config_store_load(Storage* storage, ConfigStore* cfg);
void config_store_save(Storage* storage, const ConfigStore* cfg);

void config_store_build_ready_check(
    const ConfigStore* cfg,
    uint32_t selected_count,
    char* out,
    size_t out_len);
